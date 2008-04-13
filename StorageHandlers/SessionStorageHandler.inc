<?php

/**
 * StorageHandler for storing to session
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-13 19:02:00 EST
 */

class SessionStorageHandler extends StorageHandler
{
  /**
   * Load an entity
   *
   * @param string $key
   * @return mixed
   */
  function get( $key )
  {
    $key = $this->prepareKey( $key );
    $entity = unserialize( $_SESSION[$key] );
    return $entity;
  }
  
  /**
   * Save the entity
   *
   * @param string $key
   * @param mixed $entity
   * @return bool Whether we saved or not
   */
  function set( $key, $entity, $expire = null )
  {
    $key = $this->prepareKey( $key );
    if ( is_array( $_SESSION ) )
    {
      $entity = serialize( $entity );
      $_SESSION[$key] = $entity;
      return true;
    }
    return false;
  }
  
  /**
   * Remove an entity
   *
   * @param string $key
   * @return bool Whether or not the item existed before trying to remove.
   */
  function delete( $key )
  {
    $key = $this->prepareKey( $key );
    if ( is_array( $_SESSION ) && array_key_exists( $key, $_SESSION ) )
    {
      unset( $_SESSION[$key] );
      return true;
    }
    return false;
  }
  
  /**
   * Sanitize the key for Session storage
   *
   * @param string $key
   * @return string
   */
  function prepareKey( $key )
  {
    return md5( $key );
  }
}

?>