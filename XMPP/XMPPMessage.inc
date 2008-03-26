<?php

/**
 * XMPPMessage object, contains info about a message to or from a user
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-10-13 13:37:42 EST
 *
 */

class XMPPMessage extends XMPPStanzaObject
{
  /**
   * Who the message is said to be from
   *
   * @var string
   */
  public $from;
  /**
   * Who the target of the message is
   *
   * @var string
   */
  public $to;
  /**
   * The body of the message
   *
   * @var string
   */
  public $body;

  static function buildBySimpleXML ( $sxmlMessage )
  {
    $return = new XMPPMessage( (string) $sxmlMessage['from'], (string) $sxmlMessage['to'], (string) $sxmlMessage['type'], (string) $sxmlMessage->body );
    return $return;
  }  
  
  function __construct( $from = null, $to = null, $type = null, $body = null )
  {
    $this->from = $from;
    $this->to = $to;
    $this->type = $type;
    $this->body = $body;
  }
}

?>