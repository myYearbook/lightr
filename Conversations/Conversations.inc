<?php

/**
 * Conversations class
 * 
 * Stores and retrieves conversations currently active for a given user
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-04-03 14:15:00 EST
 */

class Conversations
{
  /**
   * XMPPJID this instance is for
   *
   * @var XMPPJID
   */
  private $jid;
  
  /**
   * Storage Handler for getting/saving
   *
   * @var StorageHandler
   */
  private $storageHandler;
  
  /**
   * Array of conversations for this user
   *
   * @var array
   */
  private $conversations = array();
  
  /**
   * Class Constructor
   *
   * @param XMPPJID $jid
   * @param StorageHandler $storageHandler
   */
  private function __construct( XMPPJID $jid, StorageHandler $storageHandler )
  {
    $this->jid = $jid;
    $this->storageHandler = $storageHandler;
  }
  
  /**
   * Get the Conversations instance for a given user
   *
   * @param XMPPJID $jid
   * @param StorageHandler $storageHandler
   * @return Conversations
   */
  static function get( XMPPJID $jid, StorageHandler $storageHandler )
  {
    $instance = $storageHandler->get( 'Conversations:' . $jid->getJID() );
    if ( false === ( $instance instanceof Conversations ) )
    {
      $instance = new Conversations( $jid, $storageHandler );
    }
    return $instance;
  }
  
  /**
   * Add a conversation to this conversations object, stores JID for referential purposes...
   *
   * @param XMPPJID $XMPPJID
   */
  function add( XMPPJID $XMPPJID )
  {
    if ( false === array_key_exists( md5($XMPPJID->__toString()), $this->conversations ) )
    {
      $this->conversations[md5($XMPPJID->__toString())] = $XMPPJID;
    }
  }

  /**
   * Remove a conversation
   *
   * @param XMPPJID $XMPPJID
   */
  function remove( XMPPJID $XMPPJID )
  {
    foreach ( $this->conversations as $index => $jid )
    {
      if ( $jid->getJID() == $XMPPJID->getJID() )
      {
        Conversation::get( $this->jid, $this->conversations[$index], $this->storageHandler )->destroy();
        unset( $this->conversations[$index] );
      }
    }
  }
  
  /**
   * Get the built array of conversations
   */
  function getConversations( )
  {
    $toReturn = array();
    foreach ( $this->conversations as $XMPPJID )
    {
      $toReturn[] = Conversation::get( $this->jid, $XMPPJID, $this->storageHandler );
    }
    return $toReturn;
  }
  
  /**
   * Get the conversation with a given person
   *
   * @param XMPPJID $to
   * @return Conversation
   */
  function getConversation( XMPPJID $to )
  {
    $conversation = Conversation::get( $this->jid, $to, $this->storageHandler );
    $this->add($to);
    return $conversation;
  }
  
  /**
   * Save the current conversations state
   */
  function save()
  {
    $this->storageHandler->set( 'Conversations:' . $this->jid->getJID(), $this );
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