<?php

class XMPPMUCHandler extends XMPPHandlerObject
{
  const NS_USER = 'http://jabber.org/protocol/muc#user';

  public $xpath = array();

  /**
   * Constructor
   *
   * @param XMPPJID $jid
   * @param StorageHandler $storageHandler
   */
  function __construct( $storageHandler )
  {
    $this->storageHandler = $storageHandler;
  }

  function process( $index, SimpleXMLElement $node, $direction ) { }

  function join( $room, $password = false )
  {
    // Get our dom object
    $dom = $this->owner->getDomDocument();
    $body = $this->owner->getBodyElement();
    
    $presenceNode = $dom->createElement('presence');
    $presenceNode->setAttribute( 'to', $room );
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
    
    //$this->presenceHandler = new MUCPresenceHandler( $this );
    //$this->messageHandler = new MUCMessageHandler( $this );
    
    // Set up the MUC handler since we're joining the room now
    //$this->owner->registerHandler( $this->presenceHandler );
    //$this->owner->registerHandler( '/message/x[]', $this->messageHandler );
    
    // And... execute!
    $this->owner->execute();
  }
  
}

?> 
