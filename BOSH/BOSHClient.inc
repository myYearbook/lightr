<?php

/**
 * BOSHClient Class
 *
 * BOSH Protocol Integration
 *  - XEP-0124: Bidirectional-streams Over Synchronous HTTP
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-07-25
 * @abstract
 */

abstract class BOSHClient
{
  /**
   * The DOMDocument which contains our document to be sent
   *
   * @var DOMDocument
   */
  protected $domDocument;

  /**
   * The body node of our DOMDocument
   *
   * @var DOMNode
   */
  protected $bodyElement;

  /**
   * The BOSH URL to connect to for HTTP requests
   *
   * @var string
   */
  protected $boshURL = '';
  
  /**
   * Session id for this connection
   *
   * @var string
   */
  public $sid;
  
  /**
   * The longest time (in seconds) that it will wait before responding to any request during the session
   *
   * @var int
   */
  protected $wait;
  
  /**
   * This attribute specifies the shortest allowable polling interval (in seconds). This enables the client to not send empty request elements more often than desired
   * 
   * @var int
   */
  protected $polling;
  
  /**
   * The maximum amount of time a client is allowed to go without activity
   *
   * @var int
   */
  protected $inactivity;
  
  /**
   * The number of simultaneous requests the client is allowed to make
   *
   * @var int
   */
  protected $requests;
  
  /**
   * The maximum number of requests the connection manager may keep waiting at any one time during the session
   *
   * @var int
   */
  protected $hold;
  
  /**
   * The maximum pause for this client
   *
   * @var int
   */
  protected $maxpause;
  
  /**
   * The current rid for the current client
   *
   * @var int
   */
  private $rid;

  /**
   * The last time the client polled (empty body)
   *
   * @var int
   */
  protected $lastPoll = false;

  /**
   * Is the client connecting?
   *
   * @var bool
   */
  public $connecting = false;
  
  /**
   * Is the client connected?
   *
   * @var bool
   */
  protected $connected = false;
  
  /**
   * List of debugger objects to debug with
   *
   * @var array
   */
  static protected $debuggers = array();
   
  /**
   * This should be extended to handle all incoming data instead of doing it through the execute function
   *
   * @param BOSHResult $BOSHResult
   */
  protected function handleResult( $BOSHResult )
  {
    if ( $BOSHResult instanceof BOSHResult )
    {
      $this->debug( 'HandleResult Called', false );
      $returnSXML = $BOSHResult->bodySXML;
      
      if ( $returnSXML instanceof SimpleXMLElement )
      {
        if ( @$returnSXML->xpath('//stream:features') )
        {
          $this->sid = (string)$returnSXML['sid'];
          $this->wait = (int)$returnSXML['wait'];
          $this->debug( 'Wait', $this->wait );
          $this->polling = (int)$returnSXML['polling'];
          $this->inactivity = (int)$returnSXML['inactivity'];
          $this->requests = (int)$returnSXML['requests'];
          $this->hold = (int)$returnSXML['hold'];
          $this->maxpause = (int)$returnSXML['maxpause'];
        }
      } else {
        $this->debug( 'Expecting SimpleXML in ' . __FILE__ . ':' . __LINE__ );
      }
    } else {
      $this->debug( 'Handleresult given improper type, expecting BOSHResult' );
    }
  }
  
  /**
   * Constructor
   *
   * @param string|bool $sid Supplied int is the SID, else false for creating a new client
   */
  function __construct( $boshURL, $sid = null )
  {
    $this->boshURL = $boshURL;
    $this->sid = $sid;
    // Reset (build) our DOMDocument object and bodyNode
    $this->resetDom();
  }

  /**
   * This function is a magix function that resets the dom when this object is unserialized.
   * 
   * @magic
   */
  function __wakeup()
  {
    $this->resetDom();
  }
  
  /**
   * Reset the DOMDocument and bodyElement
   *
   * @return bool
   *
   */
  protected function resetDom()
  {
    // Create the new DOMDocument on our domDocument member variable
    $this->domDocument = new DOMDocument();
    // Create our body DOMElement
    $this->bodyElement = $this->domDocument->createElement( 'body' );
    // Set our rid
    $this->setBodyAttribute( 'xmlns', 'http://jabber.org/protocol/httpbind' );

    // This always works =)
    return true;
  }

  /**
   * Set a given attribute of the DOMDocument body element
   *
   * @param string $attribute
   * @param string $value
   */
  protected function setBodyAttribute($attribute, $value)
  {
    // Set the attribute
    $this->bodyElement->setAttribute( $attribute, $value );
  }

  /**
   * Create a new BOSH session
   *
   * @param string $to Target domain of the first stream
   * @param int $hold The maximum number of requests the connection manager is allowed to keep waiting at any one time during athe session
   * @param int $ver The highest version of the BOSH protocol that the client supports.
   * @param string $xmlLang Specifies the default language of any human-readable XML character data sent or received during the session (Section 2.12 of XML 1.0)
   * @param int $wait
   * @return string Data (excluding <body/>)
   */
  protected function create( $to, $hold = 10, $ver = 1.6, $xmlLang = 'en', $wait = 10 )
  {
    // We should be considered connecting at this point
    $this->connecting = true;
    // Set the target domain for this connection
    $this->to = $to;
    // Reset our sid
    $this->sid = null;

    $this->setBodyAttribute( 'hold', $hold );
    $this->setBodyAttribute( 'to', $this->to );
    $this->setBodyAttribute( 'ver', $ver );
    $this->setBodyAttribute( 'xml:lang', $xmlLang );
    $this->setBodyAttribute( 'wait', $wait );
    $this->setBodyAttribute( 'secure', 'true' );

    if ( ( $BOSHResult = $this->execute() ) instanceof BOSHError )
    {
      // An error occurred..
      return $BOSHResult;
    }
  }

  /**
   * Returns the last used RID... meaning you need to increment this number before using it on the same session...
   *
   * @return int rid
   */
  public function getLastRid() {
     return $this->rid;
  }

  /**
   * Execute a BOSH request
   * 
   * This function will execute the request with the current member variable DOMDocument
   *
   * @return BOSHResult|BOSHError Request result, or error if one occurred
   */
  protected function execute( $appendBody = true )
  {
    // If the sid is null, we need to make sure the rid is also null (just in case rid didn't get reset)
    if ( $this->sid === null )
    {
      $this->rid = null;
    }

    // Set our RID
    if ( $this->rid !== null )
    {
      $this->rid += 1;
    } else {
      $this->rid = str_replace( '.', '', substr( microtime(true), 4 ) );
    }

    $this->setBodyAttribute( 'rid', $this->rid );

    // This won't usually eval to true because we have our SID after the very first execute()
    if ( $this->sid !== null )
    {
      $this->setBodyAttribute( 'sid', $this->sid );
    }

    // If the sid is null, and we're not connecting, then this shouldn't be happening.
    if ( $this->sid === null && $this->connecting == false )
    {
      $result = new BOSHError( BOSHError::TYPE_APP, BOSHError::APP_NOT_CONNECTED );
    }

    if ( $appendBody === true )
    {
      $this->domDocument->appendChild( $this->bodyElement );
    }

    // Create our CURL resource
    $curlResource = curl_init( $this->boshURL );
    
    $this->debug( 'HTTP Address', $this->boshURL );
    
    // Set option to return transfer instead of returning true on success
    curl_setopt( $curlResource, CURLOPT_RETURNTRANSFER, 1);
    // Tell CURL we want to POST
    curl_setopt( $curlResource, CURLOPT_POST, 1 );
    // Set our POST information
    $toSend = $this->domDocument->saveXML();
       
    curl_setopt( $curlResource, CURLOPT_POSTFIELDS, $toSend );

    curl_setopt( $curlResource, CURLOPT_FOLLOWLOCATION, true);

    // Set our header info
    $header = array(
      'Accept-Encoding: gzip, deflate',
      'Content-Type: text/xml; charset=utf-8'
    );
    curl_setopt( $curlResource, CURLOPT_HTTPHEADER, $header );

    $this->debug( 'CURL timeout', $this->wait + 2 );
    
    // Maximum connection time; I set this to wait + 2 to be safe. Better safe than sorry!
    curl_setopt( $curlResource, CURLOPT_CONNECTTIMEOUT, $this->wait + 2 );
    curl_setopt( $curlResource, CURLOPT_TIMEOUT, $this->wait + 2 );

    $this->debug( 'Outgoing', $toSend );
    
    // Execute our CURL session
    $data = curl_exec( $curlResource );

    if ( $data == '' )
    {
      $this->debug( 'Curl Error', curl_error( $curlResource ) );
      throw new BOSHFailureException( 'Curl Error: ' . curl_error( $curlResource ), BOSHFailureException::CODE_NO_DATA );
    }
    
    // Our HTTP result code (200, 400, 403, or 404)
    $httpCode = curl_getinfo( $curlResource, CURLINFO_HTTP_CODE );

    if ( in_array( $httpCode, array( '404', '403' )  ) )
    {
      $this->debug( 'Error Code ' . $httpCode . ' received. Resetting', $data );
      $this->reset();
      return false;
    }
   
    $body = simplexml_load_string( $data );
      
    $this->debug( 'Incoming', var_export( $data, true ) );
    
    if ( $body->body->pre )
    {
       $this->sid = null;
       $result = new BOSHError( BOSHError::TYPE_HTTP, $body );
       return $result;
    }

    // There was an error... we're SOL
    if ( $body['type'] == 'terminal' )
    {
      $this->reset();
      // Set up our BOSHError object because this connection is terminal!
      // @todo This could be taken care of a bit better, but would probably require the restructuring of objects (and that, I don't have time for)
      $BOSHError = new BOSHError( BOSHError::TYPE_BOSH, $body );
      return $BOSHError;
    }

    if ( $body['wait'] != null && $this->connecting === false )
    {
      // We got an initial session creation packet, and we (according to code) weren't expecting it
      $this->resetDom();
      $return = new BOSHError( BOSHError::TYPE_APP, BOSHError::APP_LOST_CONNECTION  );
      return $return;
    }

    // Build our BOSHResult object to return
    $result = new BOSHResult( $data, $httpCode );
    $result->bodySXML = $body;
    
    if ( $result->bodySXML['sid'] )
    {
      $this->sid = (string)$result->bodySXML['sid'];
    }

    // Reset the dom structure
    $this->resetDom();
    
    // Call our extending object's result handler
    $this->handleResult( $result );
    
    return $result;
  }
  
  /**
   * This function resets all of the member variables to prep for an initial connect
   */
  protected function reset()
  {  
    $this->sid = null;
    $this->resetDom();
  }
}

?>
