<?php 

/**
 * Interface to enforce the methods for a Storable object
 */
interface IStorable 
{
  static function setStorageHandler( $object );
  static function getStorageHandler();
}

?>