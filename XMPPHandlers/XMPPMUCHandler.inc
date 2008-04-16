<?php

/**
 * Multi-User Chat (MUC) Handler
 * http://www.xmpp.org/extensions/xep-0045.html
 *
 * @author Fred Ghosn <fredghosn@gmail.com>
 * @since 2008-04-15
 */

class XMPPMUCHandler extends XMPPHandlerObject
{
  const NS_MUC      = 'http://jabber.org/protocol/muc';
  const NS_MUC_USER = 'http://jabber.org/protocol/muc#user';

  /**
   * Error codes
   *
   * @const int error code
   */
  const ERROR_PASSWORD_REQUIRED = 401;
  const ERROR_BANNED            = 403;
  const ERROR_NOSUCH_ROOM       = 404;
  const ERROR_NEED_ROOMNICK     = 406;
  const ERROR_NOT_ON_MEMBERLIST = 407;
  const ERROR_NICKNAME_INUSE    = 409;
  const ERROR_ROOM_FULL         = 503;

  /**
   * List of namespaces to register before trying to process this handlers xpaths
   *
   * @var array
   */
  public $namespace = array(
    array( 'alias' => 'muc',     'namespace' => self::NS_MUC ),
    array( 'alias' => 'mucuser', 'namespace' => self::NS_MUC_USER )
  );

  /**
   * List of handlers containg xpaths this handler should process
   *
   * @var array
   */
  public $xpath = array( 
    'PRESENCE_HANDLER' => array( 'xpath' => '//bosh:presence/mucuser:x/..',        'direction' => 2 ),
    'MESSAGE_HANDLER'  => array( 'xpath' => '//bosh:message[@type="groupchat"]',   'direction' => 2 ),
    'ERROR_HANDLER'    => array( 'xpath' => '//bosh:error/../muc:x/../bosh:error', 'direction' => 2 )
  );

  /**
   * XMPPJID for room
   *
   * @var XMPPJID
   */
  private $room = null;

  /**
   * Optional password for room
   *
   * @var string
   */
  private $password = '';

  /**
   * Connection state to room, if joined or not
   *
   * @var bool
   */
  private $connected = false;

  /**
   * List of occupants in the room
   *
   * @var array
   */
  private $occupants = array();

  /**
   * Constructor
   *
   * @param XMPPJID $jid
   * @param StorageHandler $storageHandler
   */
  public function __construct( XMPPJID $room, $password = '', $storageHandler )
  {
    $this->room           = $room;
    $this->password       = $password;
    $this->storageHandler = $storageHandler;
    $this->connected      = true;
  }

  /**
   * Catch when this handler is registered and automatically attempt to join the channel
   *
   * @param object $owner
   */
  public function setOwner( $owner )
  {
    parent::setOwner( $owner );
    $this->join();
  }

  /**
   * Return an array of occupants currently participating in the room
   */
  public function getOccupants()
  {
    return $this->occupants;
  }

  /**
   * Send a message to the current room
   *
   * @param String $nick
   */
  public function sendMessage( $body = '', $nick = null )
  {
    if ( $this->connected == false )
    {
      return false;
    }

    $to = &$this->room;
    $to->resource = $nick;

    $this->owner->route( new XMPPMessage( $this->owner->getJID(), $to, 'groupchat', $body ) );
  }

  /**
   * Invite a user to the room
   *
   * @param XMPPJID $user
   * @param String $reason
   */
  public function inviteUser( XMPPJID $user, $reason = '' )
  {
    $dom  = $this->owner->getDomDocument();
    $body = $this->owner->getBodyElement();

    $myJID = $this->owner->getJID();
    $messageNode = $dom->createElement('message');
    $messageNode->setAttribute('from', $myJID->__toString());
    $messageNode->setAttribute('to', $this->room->getJID(false));

    $xNode = $dom->createElement('x');
    $xNode->setAttribute('xmlns', self::NS_MUC_USER);

    $inviteNode = $dom->createElement('invite');
    $inviteNode->setAttribute('to', $user->getJID(false));

    if ( $reason != '' )
    {
      $reasonNode = $dom->createElement('reason');
      $reasonNode->appendChild($dom->createTextNode($reason));
      $inviteNode->appendChild($reasonNode);
    }

    $xNode->appendChild($inviteNode);
    $messageNode->appendChild($xNode);
    $body->appendChild($messageNode);

    $this->owner->execute();
  }

  /**
   * Change nickname
   *
   * @param String $nick
   */
  public function nick( $nick = '' )
  {
    if ( $nick == '' )
    {
      return false;
    }

    $this->room->resource = $nick;
    $this->join($this->room, $this->password);
  }

  /**
   * Handle incoming data that was proccessed and matched a defined xpath
   *
   * @param mixed $index Array key of the matching xpath
   * @param SimpleXMLElement $node Matching XML data
   * @param int $direction Direction of data (0 for out, 1 for in) 
   */
  public function process( $index, SimpleXMLElement $node, $direction )
  {
    switch( $index )
    {
      case 'PRESENCE_HANDLER':
        $this->handlePresence( $node );
        break;
      case 'MESSAGE_HANDLER':
        $this->handleMessage( $node );
        break;
      case 'ERROR_HANDLER':
        $this->handleError( $node );
        break;
    }
  }

  /**
   * Handle errors
   *
   * @param SimpleXMLElement $node
   */
  private function handleError( SimpleXMLElement $node )
  {
    switch( intval($node['code']) )
    {
      case ERROR_NOT_ON_MEMBERLIST:
      case ERROR_PASSWORD_REQUIRED:
      case ERROR_ROOM_FULL:
      case ERROR_BANNED:
        $this->connected = false;
        break;
    }
  }

  /**
   * Populate the internal occupants list based on incoming presence data
   *
   * @param SimpleXMLElement $node
   */
  private function handlePresence( SimpleXMLElement $node )
  {
    $jid = strval( $node->x->item[0]['jid'] );
    $key = md5( $jid );

    if ( !isset( $this->occupants[$key] ) )
    {
      $this->occupants[$key] = new stdClass();
      $this->occupants[$key]->jid = XMPPJID::getByString( $jid );
    }

    $this->occupants[$key]->priority    = strval( $node->priority );
    $this->occupants[$key]->nick        = array_pop( explode( '/', strval( $node['from'] ) ) );
    $this->occupants[$key]->affiliation = strval( $node->x->item[0]['affiliation'] );
    $this->occupants[$key]->role        = strval( $node->x->item[0]['participant'] );
  }

  /**
   * Store room conversations
   *
   * @param SimpleXMLElement $node
   */
  private function handleMessage( SimpleXMLElement $node )
  {
    // *** WARNING - FOR TESTING PURPOSES ONLY ***
    // The following is test code strictly for development purposes and should be removed for before any release of this object
    if ( isset($node->x['stamp']) ) return false;
    if ( strval($node->body) == '!occupants' )
    {
      $occupantList = $this->getOccupants();
      if ( sizeof($occupantList ) )
      {
        $string = 'Occupants: ';
        foreach( $occupantList as $item )
        {
          $string .= $item->nick . ' (' . $item->jid->__toString() . '), ';
        }
      }
      $this->sendMessage( rtrim( $string, ', ' ) );
    } else if ( substr( strval( $node->body ), 0, 5 ) == '!dump' ) {
      // *** WARNING - NOT FOR PUBLIC USE ***
      // Careful with this.  Can be used to be very harmful.
      $body = strval( $node->body );
      $var  = array_pop( explode( ' ', $body, 2 ) );
      $go   = true;
      $bad  = array( 'system', 'exec', 'passthru', '`', 'pcntl_exec', 'popen', 'escapeshellcmd' );
      foreach( $bad as $check )
      {
        if ( strpos($var, $check) !== false )
        {
          $go = false;
          break;
        }
      }
      if ( $go )
      { 
        $data = eval("return $var;");
        $this->sendMessage( var_export( $data, true ) );
      }
    } else if ( strval( $node->body ) == '!ping' ) {
      $this->sendMessage('Pong!');
    }
    // End test code
  }

  /**
   * Send initial presence to join the room
   */
  private function join()
  {
    $dom  = $this->owner->getDomDocument();
    $body = $this->owner->getBodyElement();

    $presenceNode = $dom->createElement('presence');
    $presenceNode->setAttribute( 'to', $this->room->__toString() );
    $xNode = $dom->createElement('x');
    $xNode->setAttribute( 'xmlns', self::NS_MUC );
    if ( $password !== '' )
    {
      $passwordNode = $dom->createElement('password');
      $passwordNode->appendChild( $dom->createTextNode( $this->password ) );
      $xNode->appendChild( $passwordNode );
    }
    $presenceNode->appendChild( $xNode );
    $body->appendChild( $presenceNode );

    $this->owner->execute();
  }

}

?>