<?php

/**
 * HTMLDebugger class for debugging straight to output
 *
 * @author Dallas Gutauckis <dallas@gutauckis.com>
 * @since 2008-02-10 05:51:00 EST
 */

class HTMLDebugger extends Debugger 
{
  /**
   * Mandatory debug function
   *
   * @param string $title
   * @param string $data
   */
  function debug( $title, $data )
  {
    echo "<hr/>" . '[' . microtime( true ) . '] ' . $title . "<hr/>\n";
    echo "<pre>" . htmlentities( $data ). "</pre>\n"; 
  } 
}

?>