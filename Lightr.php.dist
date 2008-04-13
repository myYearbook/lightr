<?php

/**
 * Lightr.php
 * 
 * The main file for handling the Lightr client
 *
 * @author Dallas Gutauckis <dallas@gutauckis.com>
 * @since 2008-02-10 00:14:00 EST
 */

/**
 * This constant should be set to the folder (with trailing slash) which contains this file
 * 
 * @example /home/dallas/public_html/Lightr/
 * @var String
 */
define( '__LIGHTR_PATH', '/home/dallas/public_html/Lightr/' );

// Build map of folders
$files = scandir( __LIGHTR_PATH );
$folders = array();
foreach ( $files as $file )
{
  $fullPath = __LIGHTR_PATH . $file;
  if ( is_dir( $fullPath ) && $file[0] != '.' )
  {
    $folders[] = $fullPath;
  }
}

/**
 * This function is a php magic function called when PHP can't find a class. We include required files this way.
 *
 * @param string $className
 * @magic
 */
function __autoload( $className )
{
  global $folders;

  foreach ( $folders as $folder )
  {
    $file = $folder . '/' . $className . '.inc';
    
    if ( file_exists( $file ) )
    {
      require( $file );
      return;
    }
  }
}

?>
