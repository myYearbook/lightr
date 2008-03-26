<?php

/**
 * BOSHError Object
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-11-08 10:20:30 EST
 */

class BOSHError
{
  const TYPE_BOSH = 'BOSH';
  const TYPE_APP = 'APP';
  const TYPE_HTTP = 'HTTP';
  const TYPE_XMPP = 'XMPP';

  /**
   * These error type constants, prefixed with 'XMPP', are errors thrown by the
   * XMPP server and are contained within various XMPP stanzas
   */
  const XMPP_NOT_AUTHORIZED = 'not-authorized';
  
  /**
   * These error type constants, prefixed with 'HTTP', are errors thrown by the
   * connection manager, which (for some reason) are not in a standard XML stanza
   */
  const HTTP_INVALID_SID = 'Invalid SID.';
  const HTTP_UNLOCATABLE_CONNECTION  = 'Could not locate connection: %s';

  /**
   * These error type constants, prefixed with 'APP', are application-detected
   * errors. That is, either the XMPPClient or BOSHClient detected an error with 
   * what was happening
   */
  const APP_NOT_CONNECTED = 'not-connected';
  const APP_LOST_CONNECTION = 'lost-connection';
  const APP_AUTHENTICATION_FAILURE = 'authentication-failure';
  const APP_POLLING_TOO_FAST = 'polling-too-fast';

  /**
   * These error type constants, prefixed with 'BOSH', are standard XMPP errors
   * in stanza (as defined by BOSH
   */
  const BOSH_BAD_REQUEST = 'bad-request';
  const BOSH_HOST_GONE = 'host-gone';
  const BOSH_HOST_UNKNOWN = 'host-unknown';
  const BOSH_IMPROPER_ADDRESSING = 'improper-addressing';
  const BOSH_INTERNAL_SERVER_ERROR = 'internal-server-error';
  const BOSH_ITEM_NOT_FOUND = 'item-not-found';
  const BOSH_OTHER_REQUEST = 'other-request';
  const BOSH_POLICY_VIOLATION = 'policy-violation';
  const BOSH_REMOTE_CONNECTION_FAILED = 'remote-connection-failed';
  const BOSH_REMOTE_STREAM_ERROR = 'remote-stream-error';
  const BOSH_SEE_OTHER_URI = 'see-other-uri';
  const BOSH_SYSTEM_SHUTDOWN = 'system-shutdown';
  const BOSH_UNDEFINED_CONDITION = 'undefined-condition';

  /**
   * Error Type
   *
   * @var string
   */
  private $type;
  private $code;
  
  /**
   * Is this error terminal?
   *
   * @var bool
   */
  private $terminal = false;
  
  private $terminalCodes = array(
    BOSHError::BOSH_BAD_REQUEST,
    BOSHError::BOSH_POLICY_VIOLATION,
    BOSHError::APP_NOT_CONNECTED,
    BOSHError::APP_LOST_CONNECTION,
    BOSHError::APP_AUTHENTICATION_FAILURE,
    BOSHError::APP_POLLING_TOO_FAST,
    BOSHError::HTTP_INVALID_SID,
    BOSHError::XMPP_NOT_AUTHORIZED,
  );

  /**
   * Class Constructor
   *
   * @param string $type
   * @param string|array $context
   */
  function __construct( $type, $context )
  {

    $this->type = $type;

    switch ( $type )
    {
      case self::TYPE_XMPP:
        if ( is_string( $context ) )
        {
          $this->code = $context;
        } else {
          throw new Exception( 'Invalid context type passed to BOSHError $context for XMPP error type; expecting string' );
        }
      break;
      case self::TYPE_BOSH:
        if ( $context instanceof SimpleXMLElement )
        {
          $this->code = (string) $context['condition'];
        } else {
          throw new Exception( 'Invalid context type passed to BOSHError $context for BOSH error type; expecting SimpleXMLElement' );
        }
      break;
      case self::TYPE_APP:
        if ( is_string( $context ) )
        {
          $this->code = $context;
        } else {
          throw new Exception( 'Invalid context type passed to BOSHError $context for APP error type; expecting string' );
        }
      break;

      case self::TYPE_HTTP:
        if ( $context instanceof SimpleXMLElement )
        {
          $this->code = (string) $context->body->pre;
          $this->terminal = true;
        } else {
          throw new Exception( 'Invalid context type passed to BOSHError $context for HTTP error type; expecting SimpleXMLElement' );
        }
      break;
      default:
        $this->type = false;
        throw new Exception( 'Invalid typed passed to BOSHError constructor' );
      break;
    }

    // If we haven't already set it to terminal (which we may have)
    if ( false === $this->terminal )
    {
      // If our code is in the terminalCodes array, then it is terminal
      $this->terminal = in_array( $this->code, $this->terminalCodes );
    }
  }

  /**
   * Get the type of error this is
   *
   * @return string
   */
  function getType()
  {
    return $this->type;
  }

  /**
   * Get the code that this error is represented by
   *
   * @return string
   */
  function getCode()
  {
    return $this->code;
  }

  /**
   * Tells us whether this should cause the client to switch to a disconnected state
   */
  function isTerminal()
  {
    return $this->terminal;
  }

}

?>