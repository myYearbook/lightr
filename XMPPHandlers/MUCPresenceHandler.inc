<?php

/**
 * MUC Presence Handler Object
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-16 14:07:00 EST
 */

class MUCPresenceHandler extends XMPPHandlerObject 
{
  /**
   * The xpath string for which this handler should be called
   *
   * @var string
   */
  public $xpath = array( '/presence/x[@xmlns="http://jabber.org/protocol/muc#user"]/..' );
  
  /**
   * Process a node
   *
   * @param SimpleXMLElement $node
   */
  public function process( SimpleXMLElement $node )
  {
    $this->owner->debug( 'MUC Presence', print_r( $node, true ) );
  }
}

?>