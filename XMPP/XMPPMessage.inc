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
   * @var XMPPJID
   */
  public $to;
  /**
   * The body of the message
   *
   * @var XMPPJID
   */
  public $body;
  /**
   * The past tense of read, read. Has this message been read?
   *
   * @var boolean
   */
  private $read = false;

  /**
   * Build an XMPPMessage by SimpleXMLElement node and return it
   *
   * @param SimpleXMLElement $sxmlMessage
   * @return XMPPMessage
   */
  static function buildBySimpleXML ( $sxmlMessage )
  {
    $return = new XMPPMessage( XMPPJID::getByString( $sxmlMessage['from'] ), XMPPJID::getByString( $sxmlMessage['to'] ), (string) $sxmlMessage['type'], (string) $sxmlMessage->body );
    return $return;
  }  
  
  /**
   * Set whether or not this message has been read. Defaults to false
   *
   * @param boolean $read
   */
  function setIsRead( $read = true )
  {
    $this->read = $read;
  }
  
  /**
   * Class constructor
   *
   * @param XMPPJID $from
   * @param XMPPJID $to
   * @param string $type Default is chat
   * @param string $body
   */
  function __construct( XMPPJID $from, XMPPJID $to, $type = 'chat', $body )
  {
    $this->from = $from;
    $this->to = $to;
    $this->type = $type;
    $this->body = $body;
  }
}

?>