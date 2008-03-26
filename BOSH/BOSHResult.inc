<?php

/**
 * BOSHResult Class
 *
 * Contains data from a BOSH result
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-07-27
 */

class BOSHResult
{
  private $data;
  public $bodySXML;

  /**
   * Constructor
   *
   * @param string $data
   * @param int $httpCode
   * @param int $resultCode
   */
  function __construct( $data )
  {
    $this->data = $data;
  }

  /**
   * Get the data for this result
   *
   * @return string Result data
   */
  function getData()
  {
    return $this->data;
  }
}

?>