<?php

/**
 * XMPPClient Class
 *
 * @desc Makes use of the XMPP protocol with XEP-0124 (BOSH) integration
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-07-25
 * @uses BOSHClient
 */

class XMPPClient extends BOSHClient
{

  const NS_SASL = 'urn:ietf:params:xml:ns:xmpp-sasl';
  const NS_ROSTER = 'jabber:iq:roster';

  /**
   * XMPPClient handler objects
   *
   * @var array
   */
  static private $handlers = array();
  
  /**
   * Public member variable to tell this class whether or not it should save messages sent to it.
   *
   * @var bool
   */
  public $saveMessages = true;
  
  /**
   * SASLMechanism
   *
   * @var SASLMechanism
   */
  private $SASLMechanism;
  
  /**
   * The server (resolvable) that we're connecting to. This differs from the domain in that this is the hostname/ip we need to connect to.
   *
   * @var string
   */
  public $server;
  
  /**
   * The domain for our connection (and Jabber ID)
   *
   * @var string
   */
  public $domain;
  
  /**
   * The username portion of the jid ("node") we'll be authenticating with
   *
   * @var string
   */
  public $username;
  
  /**
   * The password we'll be authenticating with
   *
   * @var string
   */
  public $password;
  
  /**
   * The stored roster information
   *
   * @var array
   */
  protected $roster = array();

  /**
   * This client's current presence information
   *
   * @var XMPPPresence
   */
  private $presence;

  /**
   * An array containing all of the uncleared messages not retrieved by the client
   *
   * @var array
   */
  private $messages;
  
  /**
   * XMPPJID representation of the current connection's jid (Jabber ID)
   *
   * @var XMPPJID
   */
  private $jid;
  
  /**
   * Whether or not the current connection has authenticated
   *
   * @var bool
   */
  private $authenticated = false;
  
  /**
   * Gets the username for the current XMPPClient session
   *
   * @return string Username
   */
  public function getUsername()
  {
    return $this->username;
  }

  public function getBodyElement()
  {
    return $this->bodyElement;
  }
  
  public function getDomDocument()
  {
    return $this->domDocument;
  }
  
  /**
   * XMPPClient Constructor
   *
   * Calls the parent(BOSHClient) constructor by building the connection string based on the input given to this constructor
   *
   * @param string $boshServer The hostname of the BOSH server to connect to
   * @param int $boshPort The port of the BOSH server to connect to (default: 8080)
   * @param string $boshUri The URI the BOSH server should query (default: '/http-bind/')
   */
  function __construct( $boshServer, $boshPort = 8080, $boshURI = '/http-bind/' )
  {
    // Set up our boshServer address (and default XMPP server)
    $this->server = $boshServer;

    $connectString = 'http://' . $boshServer;
    if ($boshPort !== null)
    {
      $connectString .= ':' . $boshPort;
    }

    if ($boshURI !== null)
    {
      $connectString .= $boshURI;
    }

    // Create our BOSHClient
    parent::__construct( $connectString );
  }

  /**
   * XMPPClient get function to retrieve an XMPPClient
   *
   * @param XMPPJID $XMPPJID
   * @return XMPPClient
   */
  static function get( $XMPPJID, $password )
  {
    $node = $XMPPJID->node;
    $domain = $XMPPJID->domain;
    $resource = $XMPPJID->resource;
    
    // If our storage handler variable is a StorageHandler object
    if ( BOSHClient::$storageHandler instanceof StorageHandler )
    {
      // If we can't get the XMPPClient from cache, otherwise, set $XMPPClient to it's instance
      if ( false === ( $XMPPClient = BOSHClient::$storageHandler->get( 'XMPPClient:' . $XMPPJID->node ) ) )
      {
        // Create a new XMPPClient
        $XMPPClient = new XMPPClient( BOSHClient::$boshHost, BOSHClient::$boshPort, BOSHClient::$boshURI );
        /// And... connect.
        $XMPPClient->connect( $node, $password, $domain, $resource );
      }
    } else {
      // If our storage handler variable isn't a StorageHandler object, create a new one
      $XMPPClient = new XMPPClient( BOSHClient::$boshHost, BOSHClient::$boshPort, BOSHClient::$boshURI );
      // And... connect.
      $XMPPClient->connect( $node, $password, $domain, $resource );
    }

    return $XMPPClient;
  }
  
  /**
   * XMPP Client Polling function (send a blank body to the BOSH server, wait for data)
   *
   * @return BOSHResult|BOSHError
   */
  final function poll()
  {
    if ( ( microtime( true ) - $this->lastPoll + 2 ) > $this->polling )
    {
      // Make sure the dom document is fresh
      $this->resetDom();
      
      // Send the request
      $this->execute( );
      
      // Set the lastPoll member variable to now (when they "last polled)
      $this->lastPoll = microtime( true );
    } else {
      throw new PollingFrequencyException( 'Polling too fast' );
    }
    return $result;
  }

  /**
   * Remove the resource portion (all following the first /) from the jid
   *
   * @param string $jid
   */
  private function normalizeJID( $jid )
  {
    // If the jid still contains the resource still --- remove it.
    if ( false !== ( $slashPos = strpos( $jid, '/' ) ) )
    {
      $jid = substr( $jid, 0, $slashPos );
    } else {
      $jid = (string) $jid; 
    }
    return $jid;
  }
  
  /**
   * Set presence information
   *
   * @param XMPPPresence $presenceObject
   */
  private function setPresenceInfo( $presenceObject )
  {
    // Can we find the roster item?
    foreach ( $this->roster as &$rosterObj )
    {
      if ( $rosterObj->jid->getJID() == $presenceObject->jid->getJID() )
      {
        $rosterObj->presence = $presenceObject;
        return true;
      }
    }
    
    $newRosterItem = new XMPPRosterItem( $jid, null, null, null, $presenceObject );
    return $this->setRosterInfo( $newRosterItem );
  }
  
  /**
   * Set roster data for a given jid
   *
   * @param XMPPJID $jid
   * @param XMPPRosterItem $XMPPRosterItem
   * @return boolean true if the object was found and set, false on no roster item yet
   */
  private function setRosterInfo( $XMPPRosterItem )
  {
    // The jid for this roster item object
    $jid = $XMPPRosterItem->jid;
    
    // If this jid doesn't contain a node, then we don't care about their presence (in this version of XMPPClient =P)
    if ( $jid->node === null )
    {
      return false;
    }

    // Try to find an existing roster object in the roster class variable
    foreach ( $this->roster as &$rosterObj )
    {
      if ( $rosterObj->jid == $jid )
      {
        // We found them... now set the information to what we've been given in the method call
        if ( $rosterObj->presence instanceof XMPPPresence )
        {
          $oldPresence = $rosterObj->presence;
          $rosterObj = $XMPPRosterItem;
          $rosterObj->presence = $oldPresence;
        } else {
          $rosterObj = $XMPPRosterItem;   
        }
        return true;
      }
    }

    // Roster item didn't exist.... so we should create it
    $this->roster[] = $XMPPRosterItem;
    return true;
  }

  /**
   * Function to execute the current BOSH command with the current bodyElement setup
   *
   * @return BOSHResult
   */
  function execute()
  {
    $return = parent::execute();
  
    if ( $return instanceof BOSHError )
    {
      if ( $return->isTerminal() == true )
      {
        $this->authenticated = false;
        $this->connected = false; 
      }
    }
    
    return $return;
  }


  /**
   * Connect the client to the server specified in the constructor, with given parameters
   *
   * @param string $username
   * @param string $password
   * @param string $domain
   * @param string $resource
   */
  function connect( $username, $password, $domain = null, $resource = null )
  {
    // Set up that we're connecting
    $this->connecting = true;
    
    // Set our instance variables for this username/password
    $this->username = $username;
    $this->password = $password;

    // If they supplied a domain: use that; else: use the server specified in the constructor
    $domain = ($domain !== null) ? ($domain) : ($this->server);
    $this->domain = $domain;
    
    // Set default resource if needed
    $this->resource = $resource = ($resource !== null) ? ($resource) : (__CLASS__);
    $this->create( $domain, 2, 1.6, 'en', 10 );
  }

  /**
   * This function resets all of the member variables to prep for an initial connect
   */
  protected function reset()
  {
    $this->debug( 'Reset Function Called' );
    parent::reset();
    $this->roster = array();
    $this->connected = false;
    $this->connecting = false;
    $this->authenticated = false;
    if ( self::$storageHandler instanceof StorageHandler )
    {
      self::$storageHandler->delete( 'XMPPClient:' . $this->username );
    }
  }
  
  /**
   * Disconnect from XMPP Server
   *
   * @return BOSHResult
   */
  function disconnect()
  {
    $this->bodyElement->setAttribute( 'type', 'terminate' );

    $presence = $this->domDocument->createElement( 'presence' );
    $presence->setAttribute( 'type', 'unavailable' );
    $presence->setAttribute( 'xmlns', 'jabber:client' );

    $this->bodyElement->appendChild( $presence );

    $this->execute();
    $this->reset();
  }

  /**
   * Send presence to the server with specified information
   *
   * @param string $to
   * @param string $type
   * @param string $show
   * @param string $status
   * @param int $priority
   * @return BOSHResult
   */
  function presenceSend( $to = null, $type = null, $show = null, $status = null, $priority = null )
  {

    // Create presence element
    $presence = $this->domDocument->createElement( 'presence' );

    // Set to attribute
    if ( $to !== null )
    {
      $presence->setAttribute( 'to', $to );
    }

    // Set type attribute
    if ( $type !== null )
    {
      $presence->setAttribute( 'type', $type );
    }

    // Create status element and give it the $status string
    if ( $status !== null )
    {
      $statusDom = $this->domDocument->createElement( 'status', $status );
      $presence->appendChild( $statusDom );
    }

    // Create show element and give it the $show string
    if ( $show !== null )
    {
      $showDom = $this->domDocument->createElement( 'show', $show );
      $presence->appendChild( $showDom );
    }

    // Create priority element and give it the $priority integer
    if ( $priority !== null )
    {
      $priority = intval( $priority );
      $priorityDom = $this->domDocument->createElement( 'priority', $priority );
      $presence->appendChild( $priorityDom );
    }

    $this->bodyElement->appendChild( $presence );

    $result = $this->execute();

    if ( $to == null )
    {
      $this->presence = new XMPPPresence( $this->jid, $show, $status );
    }
    
    return $result;
  }

  /**
   * Get the presence information for this client
   *
   *  @return XMPPPresence
   */
  function getPresence( )
  {
    // Return our current presence information, unaltered.
    return $this->presence;
  }
  
  /**
   * Function for sending messages
   *
   * @param string $to JID of the user to send the message to
   * @param string $text Message body
   * @return BOSHResult or BOSHError
   */
  function messageSend( $to, $text )
  {
    $message = $this->domDocument->createElement('message');
    $message->setAttribute( 'to', $to );

    $body = $this->domDocument->createElement( 'body', $text );
    $message->appendChild( $body );

    $this->bodyElement->appendChild( $message );
    $result = $this->execute();

    return $result;
  }

  /**
   * Function for handling errors
   *
   * @param BOSHError $BOSHError
   */
  protected function handleError( BOSHError $BOSHError ) { }

  /**
   * This function is called any time a stream:features node appears
   *
   * @param SimpleXMLElement $streamFeaturesNode
   */
  protected function handleStreamFeatures( $streamFeaturesNode )
  {
    $this->SASLMechanism = new SASLMechanism( $this->username, $this->password, $streamFeaturesNode->mechanisms );
    // Create our auth node
    $auth = $this->domDocument->createElement( 'auth' );
    $auth->setAttribute( 'xmlns', self::NS_SASL );
    $auth->setAttribute( 'mechanism', $this->SASLMechanism->getMechanism() );
    $this->bodyElement->appendChild( $auth );
   
    // Execute
    $this->execute();
  }
  
  /**
   * This function is called any time a challenge node appears
   *
   * @param SimpleXMLElement $challengeNode
   */
  protected function handleChallenge( $challengeNode )
  {
    $strResponse = $this->SASLMechanism->getChallengeResponse( (string)$challengeNode );
    
    $response = $this->domDocument->createElement( 'response', $strResponse );
    $response->setAttribute( 'xmlns', self::NS_SASL );
    $this->bodyElement->appendChild( $response );
    
    $this->execute();
  }
  
  /**
   * This function is called any time a success node appears
   *
   * @param SimpleXMLElement $successNode
   */
  protected function handleSuccess( $successNode )
  {
    $this->authenticated = true;
    
    // Not used (rspauth?) 
    $value = (string)$successNode;
    
    // Resource binding request
    $iq = $this->domDocument->createElement( 'iq' );
    $iq->setAttribute( 'id', 'bind_1');
    $iq->setAttribute( 'type', 'set');
    $iq->setAttribute( 'xmlns', 'jabber:client' );

    $bind = $this->domDocument->createElement( 'bind' );
    $bind->setAttribute( 'xmlns', 'urn:ietf:params:xml:ns:xmpp-bind' );
    $resource = $this->domDocument->createElement( 'resource', $this->resource );
    $bind->appendChild( $resource );
    $iq->appendChild( $bind );
    $this->bodyElement->appendChild( $iq );
    $result = $this->execute();
    // End Resource binding request
  }
  
  protected function handleFailure( $failureNode )
  {
    if ( $failureNode->{'not-authorized'} )
    {
      $jidToRemove = new XMPPJID( $this->username, $this->domain );
      BOSHClient::$storageHandler->delete( $jidToRemove->getJID() );
      throw new NotAuthorizedException( 'Failed to authenticate' );
    }
  }
  
  /**
   * This function is called any time a message node appears
   *
   * @param SimpleXMLElement $messageNode
   */
  protected function handleMessage( $messageNode )
  {
    if ( $this->saveMessages === true )
    {
      $this->messages[] = XMPPMessage::buildBySimpleXML( $message );
    }
  }
  
  /**
   * This function is called whenever an empty body response comes in
   *
   * @param SimpleXMLElement $bodyElement
   */
  function handleEmptyBody( $bodyElement )
  {
    if ( $bodyElement['sid'] )
    {
      $streamFeaturesNode = $bodyElement->{'stream:features'};
      // Try connecting here...
      $this->SASLMechanism = new SASLMechanism( $this->username, $this->password, $streamFeaturesNode->mechanisms );
      // Create our auth node
      $auth = $this->domDocument->createElement( 'auth' );
      $auth->setAttribute( 'xmlns', self::NS_SASL );
      $auth->setAttribute( 'mechanism', $this->SASLMechanism->getMechanism() );
      $this->bodyElement->appendChild( $auth );
  
      // Execute
      $result = $this->execute();
    }
  }
  
  /**
   * A function for handling iq nodes
   *
   * @param SimpleXMLElement $iqNode
   */
  protected function handleIQ( $iqNode )
  {
    foreach ( $iqNode->bind as $bindNode )
    {
      $this->handleIQBind( $bindNode );
    }
    
    foreach ( $iqNode->query as $queryNode )
    {
      $this->handleIQQuery( $queryNode );
    }
      
  }
  
  /**
   * This function is called by the handleIQ function any time a query node appears in an iq node
   *
   * @param SimpleXMLElement $queryNode
   */
  protected function handleIQQuery( $queryNode )
  {
    foreach ( $queryNode->item as $itemNode )
    {
      $this->handleIQQueryRosterItem( $itemNode );
    }
  }
  
  /**
   * This function is called by the handleIQ function any time a bind node appears in an iq node
   *
   * @param SimpleXMLElement $bindNode
   */
  protected function handleIQBind( $bindNode )
  {
    // Set our personal JID
    $this->jid = XMPPJID::getByString( (string)$bindNode->jid );
    
    $this->connecting = false;
    $this->connected = true;
    
    // Session request
    $iq = $this->domDocument->createElement( 'iq' );
    $iq->setAttribute( 'id', 'sess_1' );
    $iq->setAttribute( 'from', $this->jid->getJID() );
    $iq->setAttribute( 'to', $this->domain );
    $iq->setAttribute( 'type', 'set' );
    $iq->setAttribute( 'xmlns', 'jabber:client' );

    $session = $this->domDocument->createElement('session');
    $session->setAttribute('xmlns','urn:ietf:params:xml:ns:xmpp-session');
    $iq->appendChild($session);
    $this->bodyElement->appendChild($iq);
    $this->execute();
    // End session request

    // Send the roster request
    // Send out our presence notification
    $this->getRosterWithPresence();
  }
  
  /**
   * This function is called by the handleIQQuery function any time an item node appears in the child of a query node with xmlns = 'jabber:iq:roster' by the handleIQ function
   *
   * @param SimpleXMLElement $iqItemNode
   */
  protected function handleIQQueryRosterItem( $iqItemNode )
  {
    $XMPPRosterItem = XMPPRosterItem::buildBySimpleXML( $iqItemNode );
    $this->setRosterInfo( $XMPPRosterItem );
  }
  
  /**
   * A function for handling a presence node
   *
   * @param SimpleXMLElement $presenceNode
   */
  protected function handlePresence( $presenceNode )
  {
    $XMPPPresence = XMPPPresence::buildBySimpleXML( $presenceNode );
    $this->setPresenceInfo( $XMPPPresence );
    //$this->debug( 'Presence Info', print_r( array( 'show' => $presenceNode->show, 'jid' => $presenceNode['from'], 'object' => $XMPPPresence ) , true ) );
  }
  
  /**
   * Get all messages [from a given user]
   *
   * @param string $fromJID
   */
  public function getMessages( $fromJID = null )
  {
    if ( $fromJID === null )
    {
      // If the JID is null, they want any queued messages
      $messages = $this->messages;
      return $messages;
    } else {
      $toSendMessages = array();
      foreach ( $this->messages as &$message )
      {
        if ( $message->from == $fromJID )
        {
          $toSendMessages[] = $message;
        }
      }
      return $toSendMessages;
    }
  }
  
  /**
   * Clear a message from the message queue
   *
   * @param XMPPMessage $XMPPMessage
   * @return integer How many messages that were found and removed
   */
  public function clearMessage( $XMPPMessage )
  {
    $findCount = 0;
    foreach( $this->messages as $message )
    {
      if ( $XMPPMessage == $message )
      {
        $findCount += 1;
        unset( $message );
      }
    }
    return $findCount;
  }

  /**
   * Send a roster request to the server
   *
   * @return BOSHResult
   */
  function getRoster()
  {
    // Root (iq) element
    $iq = $this->domDocument->createElement( 'iq' );
    $iq->setAttribute( 'type', 'get' );

    // Query element
    $query = $this->domDocument->createElement( 'query' );
    $query->setAttribute( 'xmlns', 'jabber:iq:roster' );

    // Add query to iq
    $iq->appendChild( $query );

    // Add iq to body
    $this->bodyElement->appendChild( $iq );

    $result = $this->execute();

    return $result;
  }

  function getRosterWithPresence( $show = null, $status = null)
  {
    // Root (iq) element
    $iq = $this->domDocument->createElement( 'iq' );
    $iq->setAttribute( 'type', 'get' );

    // Query element
    $query = $this->domDocument->createElement( 'query' );
    $query->setAttribute( 'xmlns', 'jabber:iq:roster' );

    // Add query to iq
    $iq->appendChild( $query );

    // Add iq to body
    $this->bodyElement->appendChild( $iq );

    // Create presence element
    $presence = $this->domDocument->createElement( 'presence' );
    
    if ( $this->presence->show != 'available' && isset( $this->presence->show ) )
    {
      $showNode = $this->domDocument->createElement('show');
      $showNode->appendChild( $this->domDocument->createTextNode( $this->presence->show ) );
      $presence->appendChild( $showNode );
    } else {
      $this->presence = new XMPPPresence( $this->jid, 'available' );
    }
    
    $this->bodyElement->appendChild( $presence );
    $this->execute();
  }
  
  /**
   * Get the roster for the current user (does not include subscribing for status)
   *
   * @return Array XMPPRosterItem objects
   */
  function rosterGetArray()
  {
    // Make sure this is sorted
    natcasesort( $this->roster );
    return $this->roster;
  }

  /**
   * Add a roster item
   *
   * @param string $jid
   * @param string $group
   * @param string $name
   * @return BOSHResult
   */
  function rosterAdd( $jid, $group = 'Buddies', $name = null )
  {
    // Set our default "name" (nickname) to their jid
    $name = ($name === null) ? ($jid) : ($name);

    $iq = $this->domDocument->createElement( 'iq' );
    $query = $this->domDocument->createElement( 'query' );
    $item = $this->domDocument->createElement( 'item' );
    $group = $this->domDocument->createElement( 'group', $group );

    $iq->setAttribute( 'type', 'set' );

    $query->setAttribute( 'xmlns', 'jabber:iq:roster' );

    $item->setAttribute( 'jid', $jid );
    $item->setAttribute( 'name', $name );

    $query->appendChild( $item );
    $item->appendChild( $group );
    $iq->appendChild( $query );

    $this->bodyElement->appendChild( $iq );

    // Create our presence request
    $presence = $this->domDocument->createElement( 'presence' );
    $presence->setAttribute( 'to', $jid );
    $presence->setAttribute( 'type', 'subscribe' );

    $this->bodyElement->appendChild( $presence );

    // Send our roster add
    $result = $this->execute();

    return $result;
  }

  /**
   * Remove a roster item with given jid
   *
   * @param string $jid The jid to remove from the current user's roster
   * @return BOSHResult
   */
  function rosterRemove( $jid )
  {
    $iq = $this->domDocument->createElement( 'iq' );
    $iq->setAttribute( 'from', $this->jid->getJID() );
    $iq->setAttribute( 'type', 'set' );

    $query = $this->domDocument->createElement( 'query' );
    $query->setAttribute( 'xmlns', 'jabber:iq:roster' );

    $item = $this->domDocument->createElement( 'item' );
    $item->setAttribute( 'jid', $jid );
    $item->setAttribute( 'subscription', 'remove' );

    $query->appendChild( $item );
    $iq->appendChild( $query );

    $this->bodyElement->appendChild($iq);
    $result = $this->execute( );
    return $result;

  }

  /**
   * Respond to a subscription request
   *
   * @param string $jid
   * @param bool $accept
   * @param string $group
   * @param string $name
   * @return BOSHResult
   */
  function subcribeReply ( $jid, $accept = true, $group = 'Buddies', $name = null )
  {
    $acceptType = 'subscribed';
    $denyType = 'unsubscribed';

    if ($accept === true)
    {
      $this->rosterAdd( $jid, $group, $name );
      $response = $acceptType;
    } else {
      $response = $denyType;
    }

    $this->presenceSend( $jid, $response );

    $result = $this->execute();

    return $result;
  }

  /**
   * Register a handler for the given node based on passed in xpath to the passed object instance.
   *
   * @param string $xpathSelector
   * @param XMPPHandlerObject $object
   */
  static public function registerHandler( &$object )
  {
    if ( false === ( $object instanceof XMPPHandlerObject ) )
    {
      throw new Exception( 'Invalid object passed to ' . __CLASS__ . '::' . __FUNCTION__ . ' method. Object must be of type XMPPHandlerObject.' );
    }
    self::$handlers[] = & $object;
  }
  
  /**
   * Unregister a handler
   *
   * @param XMPPHandlerObject $object
   * @return bool Returns true if the handler was found and removed
   */
  static public function unregisterHandler( &$object )
  {
    foreach ( self::$handlers as &$handler )
    {
      if ( $object === $handler )
      {
        unset( $handler );
      }
      return true;
    }
    return false;
  }
  
  /**
   * Handle the result of a BOSH execution
   *
   * @param BOSHResult|BOSHError $result
   * @return true
   */
  protected function handleResult( $result )
  {
    parent::handleResult( $result );
    if ( $result instanceof BOSHResult ) {
          
      // This is good
      $body = $result->bodySXML;

      if ( false === ( $body instanceof SimpleXMLElement ) )
      {
        return false; 
      }
      
      if ( count( $body->children() ) == 0 && count( @$body->xpath('//stream:features') ) == 0 )
      {
        $this->handleEmptyBody( $body ); 
        return;
      }
      
      // Parse out stream features
      $streamFeaturesNodes = @$body->xpath('//stream:features');
      if ( $streamFeaturesNodes )
      {
        foreach ( $streamFeaturesNodes as $streamFeatures )
        {
          $this->handleStreamFeatures( $streamFeatures );
        }
      }
      
      foreach ( $body->failure as $failureNode )
      {
        $this->handleFailure( $failureNode );
      }
      
      // Success response
      foreach ( $body->success as $successNode )
      {
        $this->handleSuccess( $successNode );
      }
      
      // Parse out challenge response
      foreach ( $body->challenge as $challengeNode )
      {
        $this->handleChallenge( $challengeNode );
      }

      // Parse out iq nodes
      foreach ( $body->iq as $iqNode )
      {
        $this->handleIQ( $iqNode );
      }
     
      // Find any presence notifications
      foreach ( $body->presence as $presence )
      {
        $this->handlePresence( $presence );
      }

      // Find any messages
      foreach ( $body->message as $message )
      {
        $this->handleMessage( $message );
      }

      if ( count( $this->handlers ) > 0 )
      { 
        // Process handlers last
        foreach ( $this->handlers as $handler )
        {
          if ( false !== ( $nodes = $body->xpath( $handler->xpath ) ) )
          {
            foreach ( $nodes as $node )
            {
              $handler->process( $node );
            }
          }
        }
      }
      
    } else {
      throw new Exception( 'Invalid type passed to handleResult method; expecting BOSHResult or BOSHError' );
    }
    return $result;
  }

  /**
   * Class destructor
   */
  function __destruct()
  {
    if ( BOSHClient::$storageHandler instanceof StorageHandler )
    {
      if ( $this->connected )
      {
        BOSHClient::$storageHandler->set( 'XMPPClient:' . $this->username, $this, 60 );
      } else {
        BOSHClient::$storageHandler->delete( 'XMPPClient:' . $this->username );
      }
    }
  }
  
}

?>
