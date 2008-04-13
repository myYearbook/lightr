<?php

/**
 * StorageHandler for storing to memcached
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-13 19:02:00 EST
 */

class MemcachedStorageHandler extends StorageHandler
{

  /**
   * Memcached instance for storing the client
   *
   * @var Memcache
   */
  private $memcachedInstance;
  
  /**
   * Debugger object
   *
   * @var Debugger
   */
  private $debugger;
  
  public function __construct( $debugger )
  {
    $this->debugger = $debugger;
  }
  
  /**
   * Get the memcached object for this instance
   *
   * @return Memcache
   */
  protected function getMemcachedObject()
  {
    if ( false === ( $this->memcachedInstance instanceof Memcache ) )
    {
      $instance = $this->memcachedInstance = new Memcache();
      $this->addConfiguredServers( );
    }

    return $this->memcachedInstance;
  }
  
  /**
   * Add the memcache servers listed in config to this memcached instance
   *
   * @param Memcache $instance
   */
  private function addConfiguredServers( )
  {
    $xml = simplexml_load_file( __LIGHTR_PATH . 'config.xml' );
    $servers = $xml->storage->memcached->server;
    
    $val = array();
    foreach ( $servers as $server )
    {
      list( $host, $port, $persistent, $weight ) = explode( ':', $server );
      $val[] = $this->memcachedInstance->addServer( $host, $port, $persistent, $weight );
    }
    
    if ( ! in_array( true, $val ) )
    {
      throw new Exception( 'Failed to connect to a memcached server' ); 
    }
  }
  
  /**
   * Load a client
   *
   * @param string $key
   * @return XMPPClient
   */
  function get( $key )
  {
    $this->debugger->
    $key = $this->prepareKey( $key );
    // Get our memcache object
    $memcache = $this->getMemcachedObject();
    if ( false !== ( $XMPPClient = $memcache->get( $key ) ) )
    {
      $XMPPClient = unserialize( $XMPPClient );
      if ( $XMPPClient instanceof XMPPClient )
      {
        return $XMPPClient;
      }
    }
    return false;
  }
  
  /**
   * Save the client
   *
   * @param string $key
   * @param mixed $entity
   * @return bool Whether we saved or not
   */
  function set( $key, $entity, $expire = 60 )
  {
    $key = $this->prepareKey( $key );
    $entity = serialize( $entity );
    // Get our memcache object
    $memcache = $this->getMemcachedObject();
    $returnValue = $memcache->set( $key, $entity, 0, $expire );
    return $returnValue;
  }
  
  /**
   * Remove a client
   *
   * @param key $key
   * @return bool Whether or not the item existed before trying to remove.
   */
  function delete( $key )
  {
    $key = $this->prepareKey( $key );
    // Get our memcache object
    $memcache = $this->getMemcachedObject();
    $returnValue = $memcache->delete( $key );
    return $returnValue;
  }
  
  /**
   * A function for preparing the key before insertion
   * 
   * @param string $key
   * @return string
   */
  function prepareKey( $key )
  {
    return $key; //md5( $key );
  }
}

?>
