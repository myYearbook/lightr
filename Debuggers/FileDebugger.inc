<?php

/**
 * FileDebugger class for debugging to a file
 *
 * @author Dallas Gutauckis <dallas@gutauckis.com>
 * @since 2008-02-21 14:43:00 EST
 */

class FileDebugger extends Debugger 
{
  
  private $filePath;
  
  /**
   * FileDebugger Constructor
   *
   * @param string $filePath The full path of the file which we should write to
   * @throws Exception
   */
  function __construct( $filePath )
  {
    if ( false === is_writeable( $filePath ) )
    {
      throw new Exception( 'Can\'t write to required file, "' . $filePath . '"' );
    }
    $this->filePath = $filePath;
  }
  
  /**
   * Outputs $output to file
   *
   * @param string $output
   */
  private function output( $output )
  {
    file_put_contents( $this->filePath, $output, FILE_APPEND );
  }
  
  /**
   * Mandatory debug function
   *
   * @param string $title
   * @param string $data
   */
  function debug( $title, $data )
  {
    $this->output(  "--- " . '[' . date('F d Y h:i:s A') . '] ' . $title . "---" );
    $this->output(  "\n" . $data . "\n\n" );
  }
}

?>
