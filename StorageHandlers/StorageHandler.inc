<?php

/**
 * StorageHandler Object
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-13 18:45:00 EST
 * @abstract
 */

abstract class StorageHandler
{
  /**
   * Save an entity
   *
   * @param string $key
   * @param mixed $entitiy
   */
  abstract function set( $key, $entity, $expire = 3600 );
  /**
   * Load an entity
   *
   * @param string $key
   */
  abstract function get( $key );
  /**
   * Remove an entity
   *
   * @param string $key
   */
  abstract function delete( $key );
}

?>