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
   * @var XMPPJID
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
   * @var string
   */
  public $body;
  /**
   * The time this message was built
   * 
   * @var int
   */
  public $buildTime;
  /**
   * String for this message type
   *
   * @var string
   */
  public $type;
  
  
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
    $this->buildTime = time();
  }
  
  /**
   * A function to return the built DOMElement to represent this object
   *
   * @param DOMDocument $domObject
   * @return DOMElement
   */
  function buildDOMElement( $domDocument )
  {
    $message = $domDocument->createElement('message');
    $message->setAttribute( 'to', $this->to->getJID() );
    $message->setAttribute( 'from', $this->from->getJID() );
    $message->setAttribute( 'type', $this->type );
    
    $body = $domDocument->createElement( 'body', $this->body );
    $message->appendChild( $body );

    return $message;
  }
}

?>