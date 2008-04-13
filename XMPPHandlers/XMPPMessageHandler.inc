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
  public $xpath = array( 
    array( 
      'xpath' => '//bosh:message[@type="chat"]', 
      'direction' => 2
    )
  );

  /**
   * Not documented?
   *
   * @var int
   */
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
  public function process( $index, SimpleXMLElement $node, $direction )
  {
    $message = XMPPMessage::buildBySimpleXML( $node );
    $conversation = $this->conversations->getConversation( $message->from );

    if ( $direction == 1 )
    {
      if ( isset($node->composing) )
      {
        $conversation->to->state = Conversation::STATE_COMPOSING;
      } else if ( isset($node->paused) ) {
        $conversation->to->state = Conversation::STATE_PAUSED;
      } else if ( isset($node->inactive) ) {
        $conversation->to->state = Conversation::STATE_INACTIVE;
      } else if ( isset($node->active) ) {
        $conversation->to->state = Conversation::STATE_ACTIVE;
      } else if ( isset($node->gone) ) {
        $conversation->to->state = Conversation::STATE_GONE;
      }
    }

    if ( isset($node->body) )
    {
      $conversation->addMessage( $message );
    }
  }
  
  /**
   * Process an XMPPMessage and send it via the XMPPClient owner object
   *
   * @param XMPPMessage $message
   */
  public function sendMessage( XMPPMessage $message )
  {
    $dom = $this->owner->getDomDocument();
    $body = $this->owner->getBodyElement();
    
    $domMessage = $dom->createElement( 'message' );
    $domMessage->setAttribute( 'type', $message->type );
    $domMessage->setAttribute( 'to', $message->to->getJID() );
    $domMessage->setAttribute( 'from', $message->from->getJID() );
    
    $domBody = $dom->createElement( 'body', $message->body );
    $domMessage->appendChild( $domBody );
    
    $body->appendChild( $domMessage );
    $this->owner->execute();
  }
}

?>