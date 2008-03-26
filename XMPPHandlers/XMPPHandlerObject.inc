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
  private $owner;
  
  /**
   * The xpath string for which this handler should be called
   *
   * @var string
   */
  public $xpath;
  
  /**
   * Process a node
   *
   * @param SimpleXMLElement $node
   */
  abstract public function process( $node, $owner );
}

?>