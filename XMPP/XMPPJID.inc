<?php

/**
 * Class for representing a JID and manipulating said JID
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-01 12:39:00 EST
 * @final
 */

final class XMPPJID 
{
  /**
   * The node for this JID representation
   * 
   * The node precedes the domain, seperated by an '@'
   *
   * @var string
   */
  public $node;
  
  /**
   * The domain portion of the JID (required)
   *
   * @var string
   */
  public $domain;
  
  /**
   * The resource portion of the JID
   *
   * @var string
   */
  public $resource;
  
  /**
   * The characters, key to escaped value, that should be replaced in a JID node
   *
   * @var array
   */
  static $replacements = array( 
    '\\' => '\5c',
    ' ' => '\20',
    '"' => '\22',
    '&' => '\26',
    '\'' => '\27',
    '/' => '\2f',
    ':' => '\3a',
    '<' => '\3c',
    '>' => '\3e',
    '@' => '\40',
  );
  
  /**
   * Class constructor
   *
   * @param string $node
   * @param string $domain
   * @param string $resource
   */
  function __construct( $node = null, $domain = null, $resource = null )
  {
    // Is this node valid according to spec?
    if ( XMPPJID::isValidNode( $node ) )
    {
      $this->node = $node;
    } else {
      throw new Exception( 'Invalid node passed to XMPPJID constructor' );
    }

    // Is this domain valid according to spec?
    if ( XMPPJID::isValidDomain( $domain ) )
    {
      $this->domain = $domain;
    } else {
      throw new Exception( 'Invalid domain passed to XMPPJID constructor' );
    }
    
    // Is this resource valid according to spec?
    if ( XMPPJID::isValidResource( $resource ) )
    {
      $this->resource = $resource;
    } else {
      throw new Exception( 'Invalid resource passed to XMPPJID constructor' );
    }
  }
  
  /**
   * Get the JID for this instance
   *
   * @param bool $withResource
   * @return string
   */
  function getJID( $withResource = false )
  {
    // Create our base variable for the jid
    $jid = '';
    
    // If the node isn't empty
    if ( $this->node !== null )
    {
      $jid = $this->node . '@';
    }
    
    // We should always have a domain
    $jid .= $this->domain;
    
    // If the resource for this instance has been set, and a resource should be returned
    if ( $this->resource !== null && $withResource === true )
    {
      $jid .= '/' . $this->resource;
    }
    
    // Return the jid
    return $jid;
  }

  /**
   * Function for getting the string-value for this object; conduit for getJID( true )
   *
   * @magic
   * @return string
   */
  function __toString()
  {
    return $this->getJID( true );
  }
  
  /**
   * Static function for building an instance based on the JID passed in
   *
   * @param string $jid
   */
  static function getByString( $jid )
  {
    if ( false !== strpos( $jid, '@' ) )
    {
      // Separate the node from the rest of the jid
      list( $node, $after ) = explode( '@', $jid, 2 );

      // Okay, they have a node, so they're also allowed to have a resource. Check for that.
      if ( false !== strpos( $after, '/' ) )
      {
        // Split the remaining parts into the appropriate aspects, domain and resource
        list( $domain, $resource ) = explode( '/', $after, 2 );
      } else {
        $domain = $after;
      }
    } else {
      $domain = $jid;
    }
    
    // Construct the object to return
    $XMPPJID = new XMPPJID( $node, $domain, $resource );
    return $XMPPJID;
  }
  
  /**
   * Returns true if the passed node is valid according to protocol
   *
   * @param string $node
   */
  static function isValidNode( $node )
  {
    return true;
  }

  /**
   * Returns true if the passed domain is valid according to protocol
   *
   * @param string $domain
   */
  static function isValidDomain( $domain )
  {
    return true;
  }
  
  /**
   * Returns true if the passed resource is valid according to protocol
   *
   * @param string $resource
   */
  static function isValidResource( $resource )
  {
    return true;
  }
  
  /**
   * Unencode a JID node based on XEP-0106
   *
   * @param string $node
   */
  static function unescapeNode( $node )
  {
    // We have to flip this replacement array to get the opposite of escaping.
    $replacements = array_flip( XMPPJID::$replacements );
    // Go through each and replace
    foreach ( $replacements as $escaped => $unescaped )
    {
      $node = str_replace( $escaped, $unescaped, $node );
    }
    return $node;
  }
  
  /**
   * Encode node based on XEP-0106
   * 
   * @param string $node
   */
  static function escapeNode( $node )
  {
    // Go through each replacement element and replace it in the node
    foreach( XMPPJID::$replacements as $unescaped => $escaped )
    {
      $node = str_replace( $unescaped, $escaped, $node );
    }
    
    return $node;
  }
}

?>