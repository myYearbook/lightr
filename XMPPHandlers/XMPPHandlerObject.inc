<?php

/**
 * XMPPHandlerObject Abstract Class
 * 
 * @author Dallas Gutauckis <dallas@gutauckis.com>
 * @since 2008-02-10 3:19:00 EST
 * @abstract
 */
abstract class XMPPHandlerObject
{
  
  /**
   * The owner that will be sending us nodes to process
   *
   * @var XMPPClient
   */
  public $owner;
  
  /**
   * The xpath strings for which this handler should be called
   *
   * @var array
   * @example array( array( 'xpath' => '//bosh:message', direction => 2 ) )
   */
  public $xpath;

  /**
   * XML name spaces to register
   *
   * @var array
   */
  public $namespace;

  /**
   * Process a node
   *
   * @param int $index
   * @param SimpleXMLElement $node
   * @param int $direction
   */
  abstract public function process( $index, SimpleXMLElement $node, $direction );
  
  /**
   * Set the owner of this object
   *
   * @param XMPPClient $object
   */
  public function setOwner( XMPPClient &$object )
  {
    $this->owner = &$object;
  }
}

?>