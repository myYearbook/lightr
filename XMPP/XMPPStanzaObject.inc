<?php

/**
 * XMPPStanzaObject
 *
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since 2007-11-08 10:30:27 EST
 */

abstract class XMPPStanzaObject
{
  /**
   * A function to return an instance of the object which extends this one
   *
   * @param SimpleXMLElement $simpleXMLElement
   */
  abstract static function buildBySimpleXML( $simpleXMLElement );
  
  /**
   * A function to return the built DOMElement to represent this object
   *
   * @param DOMDocument $domDocument
   * @return DOMElement
   */
  abstract function buildDOMElement( $domDocument );
}

?>