<?php

/**
 * XMPPRosterItem Object
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-11-08 13:38:02 EST
 *
 */

class XMPPRosterItem extends XMPPStanzaObject
{

  /**
   * Group(s) the user is stored under
   *
   * @var unknown_type
   */
  public $groups = array();

  /**
   * The name to show for this roster item
   *
   * @var string
   */
  public $name;

  /**
   * The JID of this roster item
   *
   * @var XMPPJID
   */
  public $jid;

  /**
   * Subscription type for the given roster item
   *
   * @var string
   */
  public $subscription;

  /**
   * Protocol of this user
   *
   * @var string
   */
  public $protocol;
 
  /**
   * The presence information for this user
   *
   * @var XMPPPresence
   */
  public $presence;
  
  const PROTOCOL_AIM = 'aim';
  const PROTOCOL_ICQ = 'icq';
  const PROTOCOL_GTALK = 'gtalk';
  const PROTOCOL_YAHOO = 'yahoo';
  const PROTOCOL_MSN = 'msn';
  
  private $protocols = array( self::PROTOCOL_AIM , self::PROTOCOL_GTALK , self::PROTOCOL_ICQ , self::PROTOCOL_MSN , self::PROTOCOL_YAHOO );

  /**
   * Static function to build an XMPPRosterItem based on a simplexml item element
   *
   * @param SimpleXMLElement $sxmlItem
   * @return XMPPRosterItem
   */
  static function buildBySimpleXML( $sxmlItem )
  {
    // Get their JID
    $jid = XMPPJID::getByString( (string)$sxmlItem['jid'] );
    // Set their name
    $name = (string) ( ($sxmlItem['name'] ) ? ( $sxmlItem['name'] ) : ( $jid->node ) );
    // Set their subscription type
    $subscription = (string) $sxmlItem['subscription'];

    $sxmlGroups = $sxmlItem->group;

    foreach ( $sxmlGroups as $group )
    {
      $groups[] = (string) $group;
    }
       
    $XMPPRosterItem = new XMPPRosterItem( $jid, $name, $subscription, $groups );
    
    return $XMPPRosterItem;
  }

  /**
   * Class Constructor
   *
   * @param SimpleXMLElement $sxmlItem
   */
  function __construct( $jid, $name = null, $subscription = null, $groups = null, $XMPPPresence = null )
  {
    // Force the JID to be an XMPPJID object instead of a string if it isn't already
    if ( false === ( $jid instanceof XMPPJID ) )
    {
      $jid = XMPPJID::getByString( $jid );
    }
    
    $jid = new XMPPJID();
    
    $this->jid = $jid;
    $this->name = ( $name ) ? ( $name ) : ( $jid->node );
    $this->subscription = $subscription;
    $this->groups = (array)$groups;

    // Set their protocol
    list( $this->protocol, $nothing ) = explode( '.', $this->jid->domain, 2 );
    if ( false === in_array( $this->protocol, $this->protocols ) )
    {
      $this->protocol = 'myb'; 
    }
    unset( $nothing );
    
    // Set our presence if they have it
    if ( $XMPPPresence instanceof XMPPPresence )
    {
      $this->presence = $XMPPPresence;
    } else {
      if ( $XMPPPresence instanceof SimpleXMLElement )
      {
        $this->presence = XMPPPresence::buildBySimpleXML( $XMPPPresence ); 
      } else {
        $this->presence = new XMPPPresence( $jid, null, XMPPPresence::PRESENCE_UNAVAILABLE );
      }
    }
  }
  
  /**
   * Function for returning the string-value of this XMPPRosterItem object; conduit for $this->name
   *
   * @return string
   */
  function __toString()
  {
    return $this->name;
  }
  
  /**
   * Don't use this
   *
   * @param DOMDocument $domDocument
   * @return false
   * @ignore 
   */
  function buildDOMElement( $domDocument )
  {
    return false; 
  }
}

?>
