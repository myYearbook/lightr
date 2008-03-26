<?php

/**
 * XMPPMUC Class
 * 
 * For handling and messaging in MUCs 
 * 
 * @XEP 0045: Multi-User Chat
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-16 11:45:00 EST
 */

class XMPPMUC
{
  
  const NS_USER = 'http://jabber.org/protocol/muc#user';
  
  /**
   * XMPPClient through which this instance should execute it's commands
   *
   * @var XMPPClient
   */
  private $XMPPClient;

  /**
   * PresenceHandler for this object (created on join)
   *
   * @var MUCPresenceHandler
   */
  private $presenceHandler;
  
  /**
   * MessageHandler for this object (created on join)
   *
   * @var MUCMessageHandler
   */
  private $messageHandler;
  
  /**
   * StorageHandler for this class
   *
   * @var StorageHandler
   */
  static private $storageHandler;
  
  /**
   * Class constructor
   * 
   * Doesn't join/create on construct, must call join function
   *
   * @param XMPPClient $XMPPClient The client through which we should join/create this room
   * @param XMPPJID $room The JID for the room we'll be joining; use the resource field to specify your handle for this room.
   */
  function __construct( XMPPClient $XMPPClient, XMPPJID $room )
  {
    $this->XMPPClient = $XMPPClient;
    $this->room = $room;
  }
  
  /**
   * Get the XMPPMUC
   *
   * @static 
   * @param XMPPClient $client
   * @param XMPPJID $XMPPJID
   */
  static function get( $XMPPClient, $XMPPJID )
  {
    if ( self::$storageHandler instanceof StorageHandler )
    {
      if ( false === ( $muc = self::$storageHandler->get( $XMPPJID ) ) )
      {
        $muc = new XMPPMUC( $XMPPClient, $XMPPJID ); 
      }
    } else {
      $muc = new XMPPMUC( $XMPPClient, $XMPPJID ); 
    }
    return $muc;
  }
  
  /**
   * Static function for setting the storage handler for this class
   *
   * @param XMPPStorageHandler $XMPPStorageHandler
   */
  static function setStorageHandler( XMPPStorageHandler $XMPPStorageHandler )
  {
    $this->storageHandler = $XMPPStorageHandler;
  }
  
  /**
   * Join this room
   *
   * @param string $password
   */
  function join( $password = false )
  {
    // Get our dom object
    $dom = $this->XMPPClient->getDomDocument();
    $body = $this->XMPPClient->getBodyElement();
    
    $presenceNode = $dom->createElement('presence');
    $presenceNode->setAttribute( 'to', $this->room );
    $xNode = $dom->createElement('x');
    $xNode->setAttribute( 'xmlns', self::NS_USER );
    if ( $password && is_string( $password ) && $password != '' )
    {
      $passwordNode = $dom->createElement('password');
      $passwordNode->appendChild( $dom->createTextNode( $password ) );
      $xNode->appendChild( $passwordNode );
    }
    $presenceNode->appendChild( $xNode );
    $body->appendChild( $presenceNode );
    
    $this->presenceHandler = new MUCPresenceHandler( $this );
    $this->messageHandler = new MUCMessageHandler( $this );
    
    // Set up the MUC handler since we're joining the room now
    $this->XMPPClient->registerHandler( $this->presenceHandler );
    $this->XMPPClient->registerHandler( '/message/x[]', $this->messageHandler );
    
    // And... execute!
    $this->XMPPClient->execute();
  }

  /**
   * Part this room
   */
  function part( )
  {
    // Get our dom object
    $dom = $this->XMPPClient->getDomDocument();
    $body = $this->XMPPClient->getBodyElement();
    
    $presenceNode = $dom->createElement('presence');
    $presenceNode->setAttribute( 'to', $this->room );
    $presenceNode->setAttribute( 'type', 'unavailable' );
    
    $body->appendChild( $presenceNode );
    $dom->appendChild( $body );
    
    $this->XMPPClient->unregisterHandler( $this->presenceHandler );
    $this->XMPPClient->unregisterHandler( $this->messageHandler );
    
    $this->XMPPClient->execute();
  }
  
  /**
   * Static functions - don't require instantiation
   */
  
  /**
   * Static function for asking for room information
   * 
   * @param XMPPClient $XMPPClient The client through which we should make this query
   * @param XMPPJID $XMPPJID The JID for the MUC one wants to query
   */
  static function queryRoomInformation( $XMPPClient, $XMPPJID )
  {
    // Get our dom object
    $dom = $XMPPClient->getDomDocument();
    $body = $XMPPClient->getBodyElement();
    
    $iqNode = $dom->createElement( 'iq' );
    $iqNode->setAttribute( 'to', $XMPPJID->getJID( ) );
    $iqNode->setAttribute( 'type', 'get' );
    
    $queryNode = $dom->createElement( 'query' );
    $queryNode->setAttribute( 'xmlns', 'http://jabber.org/protocol/disco#info' );
    
    $iqNode->appendChild( $queryNode );
    
    $body->appendChild( $iqNode );
    $dom->appendChild( $body );
    
    $XMPPClient->execute();
  }
}

?>