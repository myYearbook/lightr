<?php

/**
 * BOSHFailureException
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-13 15:32:00 EST
 */

class BOSHFailureException extends Exception
{
  const CODE_NO_DATA = 1;
  
  function __construct( $message, $code = 0 )
  {
    if ( ! $message ) { 
      switch ( $code )
      {
        case self::CODE_NO_DATA:
          $message = "No response from BOSH server";
        break;
        default:
          $message = "Unknown error.";
        break;
      } 
    }
    parent::__construct( $message, $code );
  }
}

?>