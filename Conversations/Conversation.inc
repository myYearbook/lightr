<?php

class Conversation
{
  
  /**
   * XMPPJID 'to' of this conversation
   *
   * @var XMPPJID
   */
  public $toJID;

  /**
   * XMPPJID 'from' of this conversation
   *
   * @var XMPPJID
   */
  public $fromJID;
  
  /**
   * Array of messages sent/received (in order of add)
   *
   * @var array
   */
  private $messages;
  
  /**
   * Storage handler for saving conversation
   *
   * @var StorageHandler
   */
  private $storageHandler;
  
  /**
   * Count of new incoming messages
   *
   * @var int
   */
  public $newCount = 0;
  
  /**
   * Class constructor
   *
   * @param XMPPJID $jid
   */
  function __construct( XMPPJID $fromJID, XMPPJID $toJID, StorageHandler $storageHandler )
  {
    $this->fromJID = $fromJID;
    $this->toJID = $toJID;
    $this->storageHandler = $storageHandler;
  }
  
  /**
   * Add a message to the conversation list
   *
   * @param XMPPMessage $XMPPMessage
   */
  function addMessage( XMPPMessage $XMPPMessage )
  {
    $message = new stdClass();
    $message->direction = ( $XMPPMessage->to->getJID() == $this->toJID ) ? ( 0 ) : ( 1 );
    $message->body = $XMPPMessage->body;
    $message->time = $XMPPMessage->buildTime;
    
    $this->messages[] = $message;
    if ( $message->direction == 1 )
    {
      $this->newCount += 1;
    }
  }
  
  /**
   * Get messages in this conversation
   *
   * @param int $since Since what time would you like new messages from (Unix Timestamp)
   * @param boolean $incoming Only return incoming messages
   * @example getMessages( 1207245347 )
   */
  function getMessages( $since = false, $incoming = false )
  {
    $toReturn = $this->messages;
    
    if ( $since !== false )
    {
      foreach ( $toReturn as $index => $message )
      {
        if ( $message->buildTime > $since )
        {
          $toReturn = array_slice( $toReturn, $index );
          break;
        }
      }
    }
    
    $messages = $toReturn;
    $toReturn = array();
    if ( $incoming === true )
    {
      foreach ( $messages as $index => $message )
      {
        if ( $message->direction == 1 )
        {
          $toReturn[] = $message;
        }
      }
    }
    
    $this->newCount = 0;
    return $toReturn;
  }
  
  /**
   * Get instance of Conversation for a given JID
   *
   * @param XMPPJID $fromJID
   * @param XMPPJID $toJID
   * @param StorageHandler $storageHandler
   * @return Conversation
   */
  static function get( XMPPJID $fromJID, XMPPJID $toJID, StorageHandler $storageHandler )
  {
    $instance = $storageHandler->get( Conversation::getKeyName( $fromJID, $toJID ) );
    if ( false === ( $instance instanceof Conversation ) )
    {
      $instance = new Conversation( $fromJID, $toJID, $storageHandler );
    }
    return $instance;
  }
  
  /**
   * Destroys this conversation, specifically removing it from cache
   */
  function destroy()
  {
    $this->storageHandler->delete( Conversation::getKeyName( $this->fromJID, $this->toJID ) );
  }
  
  /**
   * Enter description here...
   *
   */
  function save()
  {
    $this->storageHandler->set( Conversation::getKeyName( $this->fromJID, $this->toJID ), $this );
  }
  
  /**
   * Get the key name to use for storing/getting for a given conversation
   *
   * @param XMPPJID $fromJID
   * @param XMPPJID $toJID
   * @return string Key name
   */
  static function getKeyName( XMPPJID $fromJID, XMPPJID $toJID )
  {
    $keyName = 'Conversation:' . md5( $fromJID->getJID() . '->' . $toJID->getJID() );
    return $keyName;
  }
  
  /**
   * Class destructor
   */
  function __destruct()
  {
    $this->save();
  }
}

?>