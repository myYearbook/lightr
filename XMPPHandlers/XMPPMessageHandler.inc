<?php

/**
 * Message Handler Object
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-16 14:12:00 EST
 */

class XMPPMessageHandler extends XMPPHandlerObject 
{ 
  /**
   * Storage Handler for this object
   *
   * @var StorageHandler
   */
  private $storageHandler;
  
  /**
   * The JID that represents the current user
   *
   * @var XMPPJID
   */
  private $jid;
  
  /**
   * Conversations instance
   *
   * @var Conversations
   */
  private $conversations;
  
  /**
   * The xpath string for which this handler should be called
   *
   * @var string
   */
  public $xpath = array( '/message[type="chat"]' );
  
  public $buildCount = 0;
  
  /**
   * Constructor
   *
   * @param XMPPJID $jid
   * @param StorageHandler $storageHandler
   */
  function __construct( XMPPJID $jid, Conversations $conversations, StorageHandler $storageHandler )
  {
    $this->jid = $jid;
    $this->conversations = $conversations;
    $this->storageHandler = $storageHandler;
  }
   
  /**
   * Process a node
   *
   * @param SimpleXMLElement $node
   */
  public function process( $index, SimpleXMLElement $node )
  {
    $message = XMPPMessage::buildBySimpleXML( $node );
    $conversation = $this->conversations->getConversation( $message->from );
    $conversation->addMessage( $message );
  }
  
  /**
   * Process an XMPPMessage and send it via the XMPPClient owner object
   *
   * @param XMPPMessage $message
   * @not-implemented
   */
  public function sendMessage( XMPPMessage $message )
  {
    $dom = $this->owner->getDomDocument();
  }
  
  /**
   * Get the messages received
   *
   * @param boolean $new Only return new messages?
   * @return array Array of messages
   */
  public function getMessages( $new = false )
  {
    return $this->messages;
  }
  
  /**
   * Class destructor
   */
  function __destruct()
  {
    $this->save();
  }
  
  private function retrieve()
  {
    $this->messages = $this->storageHandler->get( 'XMPPMessages:' . $this->jid->getJID() );
  }
  
  private function save()
  {
    $this->storageHandler->set( 'XMPPMessages:' . $this->jid->getJID(), $this->messages );
  }
}

?>