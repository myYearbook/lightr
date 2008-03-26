<?php

/**
 * Abstract class for Debugging
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2008-02-08 09:20:00 EST
 * @abstract 
 */
abstract class Debugger
{
  /**
   * This function is called to debug data
   *
   * @param string $title
   * @param string $data
   */
  abstract function debug( $title, $data );
}

?>