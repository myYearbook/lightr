<?php

/**
 * XMPPClient Class
 *
 * Makes use of the XMPP protocol with XEP-0124 (BOSH) integration
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-07-25
 * @uses BOSHClient
 */

final class XMPPClient extends BOSHClient
{

  const NS_SASL = 'urn:ietf:params:xml:ns:xmpp-sasl';
  const NS_ROSTER = 'jabber:iq:roster';

  /**
   * XMPPClient handler objects
   *
   * @var array
   */
  private $handlers = array();
   
  /**
   * The number of times this object has been "built" either by construct or wakeup (unserialize)
   *
   * @var int
   */
  private $buildCount = 0;
  
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
   * XMPPJID representation of the current connection's jid (Jabber ID)
   *
   * @var XMPPJID
   */
  private $jid;
  
  /**
   * Storage Handler for this instance
   *
   * @var StorageHandler
   */
  private $storageHandler;
  
  /**
   * Debug Handler for this instance
   *
   * @var Debugger
   */
  private $debugHandler;
  
  /**
   * Whether or not the current connection has authenticated
   *
   * @var bool
   */
  private $authenticated = false;
  
  /**
   * Get the BODY element for outgoing nodes
   *
   * @return DomElement
   */
  public function getBodyElement()
  {
    return $this->bodyElement;
  }
  
  /**
   * Return the current DOMDocument
   *
   * @return DOMDocument
   */
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
  function __construct( StorageHandler $storageHandler, $boshServer, $boshPort = 8080, $boshURI = '/http-bind/' )
  {
    $this->storageHandler = $storageHandler;
    
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
   * @param string $password
   * @return XMPPClient|false
   */
  static function get( StorageHandler $storageHandler, XMPPJID $XMPPJID, $password )
  {
    $node = $XMPPJID->node;
    $domain = $XMPPJID->domain;
    $resource = $XMPPJID->resource;
    
    $XMPPClient = $storageHandler->get( 'XMPPClient:' . $XMPPJID->getJID() );
    if ( false === ( $XMPPClient instanceof XMPPClient ) )
    {
      return false;
    }

    $XMPPClient->buildCount += 1;
    return $XMPPClient;
  }
  
  /**
   * XMPP Client Polling function (send a blank body to the BOSH server, wait for data)
   *
   * @return BOSHResult|BOSHError
   */
  final function poll()
  {
    // Make sure the dom document is fresh
    $this->resetDom();
    
    // Send the request
    $this->execute( );
    
    // Set the lastPoll member variable to now (when they "last polled)
    $this->lastPoll = microtime( true );
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

    foreach ( $rosterObj->groups as $groupName )
    {
      $groupMD5 = md5( $groupName );
      $this->roster[$groupMD5][] = $rosterObj;
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
    $currentBodyElement = simplexml_import_dom( $this->getBodyElement() );
    
    $return = parent::execute();
    
    // Call the custom handlers on the current body element
    $this->callCustomHandlers( $currentBodyElement, 0 );
    
    return $return;
  }


  /**
   * Connect the client to the server specified in the constructor, with given parameters
   *
   * @param XMPPJID $jid
   * @param string $password
   * @param string $domain
   * @param string $resource
   */
  function connect( XMPPJID $jid, $password )
  {
    // Set up that we're connecting
    $this->connecting = true;

    // If they supplied a domain: use that; else: use the server specified in the constructor
    $jid->domain = ($jid->domain !== null) ? ($jid->domain) : ($this->server);
    
    // Set default resource if needed
    $jid->resource = ( $jid->resource !== null ) ? ( $jid->resource ) : ( __CLASS__ . ':' . time() );
    
    // Set our instance variables for this username/password
    $this->jid = $jid;
    $this->password = $password;

    // The server doesn't really care what we set these to, which is a bad thing
    $this->create( $jid->domain, 1, 1.6, 'en', 10 );
  }

  /**
   * This function resets all of the member variables to prep for an initial connect
   */
  public function reset()
  {
    $this->debug( 'Reset Function Called' );
    parent::reset();
    $this->roster = array();
    $this->connected = false;
    $this->connecting = false;
    $this->authenticated = false;
    $this->storageHandler->delete( $this->getKeyName() );
  }
  
  /**
   * Gets the key name to use for storage/retrieval
   *
   * @return string
   */
  function getKeyName()
  {
    return 'XMPPClient:' . $this->jid->getJID();
  }

  /**
   * Gets the JID of the user
   *
   * @return XMPPJID
   */
  function getJID()
  {
    return $this->jid;
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
    // Create presence object
    $this->presence = new XMPPPresence( null, $to, $show, $status, $type, $priority );

    // Return presence xml and append to body
    $this->bodyElement->appendChild( $this->presence->buildDOMElement( $this->domDocument ) );

    // Execute presence
    $result = $this->execute();

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
   * Routes an XMPPStanzaObject
   *
   * @param XMPPStanzaObject $object
   */
  function route( XMPPStanzaObject $object )
  {
    $result = $object->buildDOMElement( $this->getDomDocument() );
    
    if ( false === ( $result instanceof DOMElement ) )
    {
      throw new Exception( 'Invalid return type from XMPPStanzaObject::buildDOMElement()' ); 
    }
    $this->bodyElement->appendChild( $result );
    $this->execute();
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
    $this->SASLMechanism = new SASLMechanism( $this->jid->node, $this->password, $streamFeaturesNode->mechanisms );
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
    $resource = $this->domDocument->createElement( 'resource', $this->jid->resource );
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
      $this->storageHandler->delete( $this->getKeyName() );
      throw new NotAuthorizedException( 'Failed to authenticate' );
    }
  }
  
  /**
   * This function is called whenever an empty body response comes in
   *
   * @param SimpleXMLElement $bodyElement
   */
  function handleEmptyBody( $bodyElement )
  {
    if ( $this->authenticated )
    {
      if ( $bodyElement['sid'] )
      {
        $streamFeaturesNode = $bodyElement->xpath( '//*[namespace-uri(.) = \'http://etherx.jabber.org/streams\']/features' );
        // Try connecting here...
        $this->SASLMechanism = new SASLMechanism( $this->jid->node, $this->password, $streamFeaturesNode->mechanisms );
        // Create our auth node
        $auth = $this->domDocument->createElement( 'auth' );
        $auth->setAttribute( 'xmlns', self::NS_SASL );
        $auth->setAttribute( 'mechanism', $this->SASLMechanism->getMechanism() );
        $this->bodyElement->appendChild( $auth );
    
        // Execute
        $result = $this->execute();
      }
    } else {
      return false;
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
    $iq->setAttribute( 'to', $this->jid->domain );
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
      $this->presence = new XMPPPresence( $this->jid, null, 'available' );
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
   * @param string $handlerName The name under which to store this handler
   * @param XMPPHandlerObject $object
   */
  public function registerHandler( $handlerName, &$object )
  {
    if ( false === ( $object instanceof XMPPHandlerObject ) )
    {
      throw new Exception( 'Invalid object passed to ' . __CLASS__ . '::' . __FUNCTION__ . ' method. Object must be of type XMPPHandlerObject.' );
    }
    $object->setOwner( $this );
    $this->handlers[$handlerName] = &$object;
  }
  
  /**
   * Get a handler object by name
   *
   * @param string $handlerName The name under which the handler was stored
   */
  public function getHandler( $handlerName )
  {
    return $this->handlers[$handlerName];
  }
  
  /**
   * Unregister a handler
   *
   * @param string $handlerName
   * @return bool Returns true if the handler was found and removed
   */
  public function unregisterHandler( $handlerName )
  {
    if ( array_key_exists( $handlerName, $this->handlers ) )
    {
      unset( $this->handlers[$handlerName] );
      return true;
    }
    return false;
  }
  
  /**
   * Handle the result of a BOSH execution
   *
   * @param BOSHResult $result
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

      $this->callCustomHandlers( $body, 1 );
      
    } else {
      throw new Exception( 'Invalid type passed to handleResult method; expecting BOSHResult or BOSHError' );
    }
    return $result;
  }

  /**
   * Call all custom handlers for a given body element
   *
   * @param SimpleXMLElement $body
   * @param int $direction 0 for out, 1 for in
   */
  private function callCustomHandlers( $body, $direction )
  {
    if ( count( $this->handlers ) > 0 )
    {
      $body->registerXPathNamespace( 'bosh', 'http://jabber.org/protocol/httpbind' );

      // Process handlers last
      foreach ( $this->handlers as $handler )
      {
        if ( is_array( $handler->namespace ) )
        {
          foreach( $handler->namespace as $namespace )
          {
            $body->registerXPathNamespace( $namespace['alias'], $namespace['namespace'] );
          }
        }

        foreach ( $handler->xpath as $index => $xpath )
        {
          if ( $direction == $xpath['direction'] || $xpath['direction'] == 2 )
          {
            $nodes = $body->xpath( $xpath['xpath'] );
            if ( $nodes !== false )
            {
              foreach ( $nodes as $node )
              {
                $handler->process( $index, $node, $direction );
              }
            }
          }
        }
      }
    }
  }
  
  /**
   * Debug conduit
   *
   * @param string $title
   * @param string $data
   */
  function debug( $title, $data = '' )
  {
    if ( $this->debugHandler !== null )
    {
      $this->debugHandler->debug( $title, $data );
    } 
  }
  
  /**
   * Register the debugger for this instance
   *
   * @param Debugger $debugger
   */
  function registerDebugger( Debugger $debugger )
  {
    if ( false !== ( $debugger instanceof Debugger ) )
    {
       $this->debugHandler = $debugger;
    }
  }
  
  /**
   * Class destructor
   */
  function __destruct()
  {
    $result = $this->storageHandler->set( $this->getKeyName(), $this, 200 );
    $this->debug( 'Storage save client', var_export( $result, true ) );
  }
  
}

?>
