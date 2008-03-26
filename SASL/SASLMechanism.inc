<?php

/**
 * SASL Mechanism Handling Class
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-11-15 11:47:02 EST
 */

class SASLMechanism
{

  /**
   * The username to authenticate with
   *
   * @var string
   */
  public $username;
  
  /**
   * The password to authenticate with
   *
   * @var string
   */
  public $password;
  
  /**
   * Which mechanism that should be used based on the server's support for mechanisms.
   *
   * @var unknown_type
   */
  public $mechanismToUse;
  
  /**
   * An array mapping each supported mechanism to which function to call for them
   *
   * @var array
   */
  private $mechanismsToFunctions = array(
    'DIGEST-MD5' => 'digestMD5',
    'PLAIN' => 'plain'
  );

  /**
   * An array of strings in the order in which they should be used if available by the server
   *
   * @var array
   */
  private $orderOfPreference = array( 'DIGEST-MD5', 'PLAIN' );
  
  /**
   * An array of the mechanisms supported by the associated connection
   *
   * @var array
   */
  private $serverMechanisms;
  
  /**
   * Constructor function
   *
   * @param string $username
   * @param string $password
   * @param SimpleXMLElement $mechanisms
   */
  public function __construct( $username, $password, $mechanisms )
  {
    
    $mechanisms = (array)$mechanisms;
    $mechanisms = $mechanisms['mechanism'];
    
    // Set the server mechanisms
    $this->setServerMechanisms( $mechanisms );
    $this->determineMechanismToUse();
    
    if ( count( $this->serverMechanisms ) == 0 )
    {
      throw new Exception( 'The server mechanisms list was invalid.' );
    }
    
    $supported = 0;
    foreach ( $this->orderOfPreference as $mechanism )
    {
      if ( in_array( $mechanism, $this->serverMechanisms ) )
      {
        $supported += 1;
      }
    }
    
    if ( $supported == 0 )
    {
      throw new Exception( __CLASS__ . ' does not support any of the mechanisms supported by the server.' );
    }
      
    // Set our username and password member variables
    $this->username = $username;
    $this->password = $password;
  }

  /**
   * Determine which mechanism this object will be using
   *
   * @return string
   */
  private function determineMechanismToUse()
  {
    if ( $this->mechanismToUse === null )
    {
      // This could easily be dynamic, the problem is that currently the DIGEST-MD5 implementation doesn't work with openfire
      foreach ( $this->orderOfPreference as $mechanism )
      {
        if ( in_array( $mechanism, $this->serverMechanisms ) )
        {
          $this->mechanismToUse = $mechanism;
          break;
        }
      }
    }
    
    return $this->mechanismToUse;
  }
  
  /**
   * Set the server's supported mechanisms
   *
   * @param array $mechanismNodes
   */
  private function setServerMechanisms( $mechanismNodes )
  {
    $this->serverMechanisms = (array) $mechanismNodes;
  }
  
  /**
   * Get which mechanism is going to be used
   *
   * @return string
   */
  function getMechanism()
  {
    return $this->mechanismToUse;
  }
  
  /**
   * Get the challenge response to the given base64-encoded challenge
   *
   * @param string $b64challenge
   * @param string $username
   * @param string $password
   * @return string
   */
  public function getChallengeResponse( $b64challenge )
  {
    // Set the function that needs to be called
    $functionToCall = $this->mechanismsToFunctions[$this->mechanismToUse];
    
    // Get the result of the called function
    $result = $this->{$functionToCall}( $b64challenge );

    if ( $result === false )
    {
      throw new Exception( 'Bad result in ' . __CLASS__ . ' response for SASLMechanism function.' );
    } else {
      return $result;
    }
  }

  /**
   * KVPrb64e (KEY->VALUE Pairs, return base64_encode())
   * 
   * @param array Key->Value Pairs
   * @return string base64_encoded string
   */
  private function KVPrb64e( $arrayVars )
  {
    $finalString = '';

    foreach ($arrayVars as $key => $value)
    {
      $finalString .= $key . '="' . $value . '",';
    }

    $finalString = substr( $finalString, 0, -1);
    $finalString = base64_encode($finalString);
    
    return $finalString;
  }

  /**
   * b64drKVP (base64_decode(), Return KEY->VALUE Pairs)
   * 
   * @param string $encodedString
   * @return array Key->Value Pairs
   */
  private function b64drKVP( $encodedString )
  {

    $decoded = base64_decode( $encodedString );
    $decodedSplit = explode( ',', $decoded );

    foreach ($decodedSplit as $item)
    {
      $exploded = explode( '=', $item, 2 );
      // The substr breaks the string out of quotes, if present
      if ($exploded[1][0] == '"')
        $final[$exploded[0]] = substr( $exploded[1], 1, -1 );
      else
        $final[$exploded[0]] = $exploded[1];
    }

    return $final;
  }

  /**
   * DIGEST-MD5 Mechanism Function
   *
   * @author Joe Hansche <jhansche@myyearbook.com>
   * 
   * @param string $challenge Base64-encoded challenge from the server
   * @param string $username
   * @param password $password
   * @return string
   */
  private function digestMD5( $b64challenge )
  {
    $challenge = $this->b64drKVP( $b64challenge );
    $nonce = $challenge['nonce'];
    $qop = $challenge['qop'];
    $charset = $challenge['charset'];
    $realm = $challenge['realm'];
    $domain = $realm;
    $digestUri = 'xmpp/' . $domain;   
    $nc = '00000001';
    
    $cnonce = str_replace( '=', '', md5( $challenge['nonce'] ) );
    
    $x = sprintf('%s:%s:%s', $this->username, $domain, $this->password);
    $a1 = sprintf('%s:%s:%s', md5($x, true), $nonce, $cnonce);
    
    $ha1 = md5($a1);
    
    $ha2 = md5('AUTHENTICATE:' . $digestUri );
    
    $response = sprintf('%s:%s:%s:%s:%s:%s',
      $ha1, $nonce, $nc, $cnonce, 'auth', $ha2);

    $response = md5($response);

    $arrVars = array(
      'username'    => $this->username,
      'realm'       => $realm,
      'nonce'       => $nonce,
      'cnonce'      => $cnonce,
      'nc'          => $nc,
      'qop'         => $qop,
      'digest-uri'  => $digestUri,
      'response'    => $response,
      'charset'     => $charset,
    );
    
    // Key-value pairs, returning base64-encoded
    $response = $this->KVPrb64e( $arrVars );
    
    return $response;
  }

  /**
   * This is a function for if this is ever created
   *
   * @param string $b64challenge
   * @return false
   */
  private function cramMD5( $b64challenge )
  {
    return false;
  }

  /**
   * This function preps a string to be sent as PLAIN according to protocol
   *
   * @param string $b64challenge
   * @return string
   */
  private function plain( $b64challenge )
  {
    return base64_encode( $this->username . chr(0) . $this->password );
  }
}

?>