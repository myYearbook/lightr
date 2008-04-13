<?php

/**
 * XEP-0030 Handler (Discovery)
 * 
 * @author Dallas Gutauckis <dallas@gutauckis.com>
 * @since 2008-04-13 10:44:00 EST
 */

class XEP0030Handler extends XMPPHandlerObject
{
  /**
   * Storage Handler for this object
   *
   * @var StorageHandler
   */
  private $storageHandler;
   
  /**
   * The xpath string for which this handler should be called
   *
   * @var array
   */
  public $xpath = array( '' );
  
  /**
   * Class Constructor
   *
   * @param StorageHandler $storageHandler
   */
  function __construct( StorageHandler $storageHandler )
  {
    $this->storageHandler = $storageHandler;
  }
   
  /**
   * Process a node
   *
   * @param int $index
   * @param SimpleXMLElement $node
   */
  public function process( $index, SimpleXMLElement $node )
  {

  }
}

?>