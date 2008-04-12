<?php

class NotAuthorizedException extends Exception 
{
  function __construct( $message, $code = 0 )
  {
    if ( ! $message )
    {
      $message = 'Authentication failure: not authorized';
    }
    parent::__construct( $message, $code );
  }
}

?>