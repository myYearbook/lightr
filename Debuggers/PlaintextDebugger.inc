<?php

/**
 * PlaintextDebugger class for debugging in plaintext (good for something like viewing in firebug)
 *
 * @author Dallas Gutauckis <dallas@gutauckis.com>
 * @since 2008-02-10 05:51:00 EST
 */

class PlaintextDebugger extends Debugger 
{
  /**
   * Mandatory debug function
   *
   * @param string $title
   * @param string $data
   */
  function debug( $title, $data )
  {
    echo "--- " . '[' . microtime( true ) . '] ' . $title . "---";
    echo "\n" . $data . "\n\n"; 
  } 
}

?>