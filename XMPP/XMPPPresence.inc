<?php

/**
 * XMPPPresence Object -- parses a presence node from XMPPClient
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-11-05 15:08:05 EST
 */

class XMPPPresence extends XMPPStanzaObject
{
  const PRESENCE_AWAY = 'away';
  const PRESENCE_DO_NOT_DISTURB = 'dnd';
  const PRESENCE_AVAILABLE = 'available';
  const PRESENCE_EXTENDED_AWAY = 'xa';
  const PRESENCE_CHAT = 'chat';
  const PRESENCE_UNAVAILABLE = 'unavailable';
  
  /**
   * An array of acceptable presence values
   *
   * @var array
   */
  static $acceptablePresenceValues = array( 'away', 'dnd', 'available', 'xa', 'chat', 'unavailable' );
  
  /**
   * The status the user has set
   *
   * @var string
   */
  public $status;
  
  /**
   * @desc Show value for this user (PRESENCE_* constant equivalent)
   * @var string
   */
  public $show;

  /**
   * @var string
   */
  public $type;

  /**
   * XMPPJID to whom this is coming from
   *
   * @var XMPPJID
   */
  public $from;

  /**
   * XMPPJID to whom this was sent
   *
   * @var XMPPJID
   */
  public $to;

  /**
   * Priority
   *
   * @var int
   */
  public $priority;

  /**
   * Build an XMPPPresence object based on the passed in presence SimpleXMLElement 
   *
   * @param SimpleXMLElement $sxmlPresence
   * @return XMPPPresence Will return false if it couldn't be built.
   */
  static function buildBySimpleXML( $sxmlPresence )
  {
    if ( $sxmlPresence instanceof SimpleXMLElement )
    {
      if ( $sxmlPresence->getName() != 'presence' );
      $show = ( (string)$sxmlPresence->show == '' ) ? ( null ) : ( (string) $sxmlPresence->show );
      if ( in_array( $sxmlPresence['type'], array( 'error', 'unavailable' ) ) )
      {
        $show = XMPPPresence::PRESENCE_UNAVAILABLE;
      }
      $status = (string)$sxmlPresence->status;      
      $fromJID = XMPPJID::getByString( $sxmlPresence['from'] );
      $toJID = XMPPJID::getByString( $sxmlPresence['to'] );
      $newPresence = new XMPPPresence( $fromJID, $toJID, $show, $status ); 
      return $newPresence;
    } else {
      return false;
    }
  }

  /**
   * Class constructor
   *
   * @param XMPPJID $jid
   * @param string $show
   * @param string $status
   * @param SimpleXMLElement $sxmlPresence
   */
  function __construct( XMPPJID $from = null, XMPPJID $to = null, $show = null, $status = null, $type = null, $priority = null )
  {
    $this->jid      = $jid;
    $this->show     = $show;
    $this->status   = $status;
    $this->type     = $type;
    $this->priority = $priority;
  }

  /**
   * A function to return the built DOMElement to represent this object
   *
   * @param DOMDocument $domObject
   * @return DOMElement
   */
  function buildDOMElement( $domDocument )
  {
    $presence = $domDocument->createElement('presence');

    if ( $this->to !== null )
    {
      $presence->setAttribute( 'to', $this->to->getJID() );
    }

    if ( $this->from !== null )
    {
      $presence->setAttribute( 'from', $this->from->getJID() );
    }

    if ( $this->type !== null )
    {
      $presence->setAttribute( 'type', $this->type );
    }

    if ( $this->status )
    {
      $status = $domDocument->createElement('status');
      $status->appendChild( $domDocument->createTextNode( $this->status ) );
      $presence->appendChild( $status ); 
    }
    
    if ( $this->show && $this->show !== XMPPPresence::PRESENCE_AVAILABLE )
    {
      $show = $domDocument->createElement('show');
      $show->appendChild( $domDocument->createTextNode( $this->show ) );
      $presence->appendChild( $show ); 
    }

    if ( $this->priority !== null )
    {
      $priority = intval( $this->priority );
      $priorityDom = $domDocument->createElement( 'priority', $this->priority );
      $presence->appendChild( $priorityDom );
    }

    return $presence;
  }
  
}

?>
