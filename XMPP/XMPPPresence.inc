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
   * XMPPJID for this presence object
   *
   * @var XMPPJID
   */
  public $jid;
  
  /**
   * XMPPJID to whom this was sent
   *
   * @var XMPPJID
   */
  public $to;

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
      $jid = XMPPJID::getByString( $sxmlPresence['from'] );
      $newPresence = new XMPPPresence( $jid, $show, $status ); 
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
  function __construct( $jid, $show = null, $status = null )
  {
    if ( $show === null )
    {
      $show = self::PRESENCE_AVAILABLE; 
    }
    $this->jid = $jid;
    $this->show = $show;
    $this->status = $status;
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
    $presence->setAttribute( 'to', $this->to->getJID() );
    $presence->setAttribute( 'from', $this->from->getJID() );
    
    if ( $this->status )
    {
      $status = $domDocument->createElement('status');
      $status->appendChild( $domDocument->createTextNode( $this->status );
      $presence->appendChild( $status ); 
    }
    
    if ( $this->show && $this->show !== XMPPPresence::PRESENCE_AVAILABLE )
    {
      $show = $domDocument->createElement('show');
      $show->appendChild( $domDocument->createTextNode( $this->show );
      $presence->appendChild( $show ); 
    }
    
    return $presence;
  }
  
}

?>
