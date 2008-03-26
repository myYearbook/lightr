<?php

/**
 * MUC Message Handler Object
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-16 14:12:00 EST
 */

class MUCMessageHandler extends XMPPHandlerObject 
{
  /**
   * The xpath string for which this handler should be called
   *
   * @var string
   */
  public $xpath = '/message[type="groupchat"]';
  
  /**
   * Process a node
   *
   * @param SimpleXMLElement $node
   */
  public function process( $node, $owner )
  {
    $this->owner->debug( 'MUC Message', print_r( $node, true ) );
  }
}

?>