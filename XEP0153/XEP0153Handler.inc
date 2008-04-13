<?php

/**
 * XEP-0153 Implementation
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-04-14 18:54:00 EST
 */

class XEP0153Handler extends XMPPHandlerObject 
{
  /**
   * Storage Handler for this object
   *
   * @var StorageHandler
   */
  private $storageHandler;
  
  /**
   * An array of the avatars sent to this client
   *
   * @var array
   */
  private $avatars = array();
  
  /**
   * The xpath string for which this handler should be called
   *
   * @var array
   */
  public $xpath = array( 
    array( 
      'xpath' => '/presence/x[xmlns="vcard-temp:x:update"]/photo/../../.', 
      'direction' => 1 
    ), 
    array( 
      '/iq/vCard[xmlns="vcard-temp"]/PHOTO/BINVAL/../../../.', 
      'direction' => 1 
    ) 
  );
  
  /**
   * Class Constructor
   *
   * @param StorageHandler $storageHandler
   */
  function __construct( StorageHandler $storageHandler )
  {
    $this->jid = $jid;
    $this->storageHandler = $storageHandler;
  }
   
  /**
   * Process a node
   *
   * @param int $index
   * @param SimpleXMLElement $node
   */
  public function process( $index, SimpleXMLElement $node )
  {
    $fromJID = XMPPJID::getByString( $node['from'] );
    if ( $index == 0 )
    {
      // Ask XMPP server for the data
      $this->getAvatarByJID( $fromJID );
    } elseif ( $index == 1 ) {
      // Store data
      $imageData = $node->vCard->PHOTO->BINVAL;
      $this->avatars[md5($fromJID->getJID())] = $node->vCard->PHOTO->BINVAL;
      // Set in cache
      $this->storageHandler->set( 'XEP0153Avatar:' . $fromJID->getJID(), $imageData );
    }
  }
  
  /**
   * Get an avatar by the user's JID
   *
   * @param XMPPJID $jid
   */
  public function getAvatarByJID( XMPPJID $jid )
  {
    $avatar = $this->storageHandler->get( 'XEP0153Avatar:' . $jid->getJID() );
    if ( $avatar === false )
    {
      // Get it from the XMPPClient
      
      // Get the DOMDocument object
      $dom = $this->owner->getDomDocument();
      
      // Build iq node
      $iq = $dom->createElement( 'iq' );
      $iq->setAttribute( 'to', $jid->getJID() );
      $iq->setAttribute( 'type', 'get' );
      
      // vCard node
      $vCard = $dom->createElement( 'vCard' );
      $vCard->setAttribute('xmlns', 'vcard-temp');
      $iq->appendChild( $iq );

      // Append iq node for processing
      $body = $this->owner->getBodyElement()->appendChild( $iq );
      
      $this->owner->execute();
    }
  }
}

?>