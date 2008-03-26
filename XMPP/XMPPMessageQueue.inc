<?php

/**
 * XMPP LIFO Message Queue
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-03-05 18:04:00 EST
 */
class XMPPMessageQueue implements IStorable
{

  private $queue;
  
  /**
   * Enqueue an XMPPMessage in the message queue
   *
   * @param unknown_type $XMPPMessage
   * @return unknown
   */
  function enqueue( $XMPPMessage )
  {
    $this->queue[] = $XMPPMessage;
    return true;
  }
  
  /**
   * Returns the last item out of the queue, LIFO style
   * 
   * @return XMPPMessage
   */
  function dequeue()
  {
    return array_pop( $this->queue );
  }
  
  function __construct( $node )
  {
    $this->node = $node;
  }
  
  /**
   * Storage Handler
   *
   * @var StorageHandler
   */
  static $storageHandler = false;
  
  /**
   * Set the sotrage handling object for this class
   *
   * @param SessionHandler $object
   */
  static public function setStorageHandler( $object )
  {
    if ( false === ( $object instanceof StorageHandler ) && $object !== false )
    {
      throw new Exception( 'Given invalid StorageHandler parameter' );
    }
    self::$storageHandler = & $object;
    return true;
  }
  
  /**
   * Get storage handler for this object
   *
   * @return StorageHandler
   */
  public static function getStorageHandler()
  {
    return self::$storageHandler;
  }
  
  function __destruct()
  {
    if ( false !== ( $storageHandler = XMPPMessage::getStorageHandler() ) )
    {
      $storageHandler->set( $this->node, $this, 120 );
    }
  }
  
}

?>