/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file XMLCoreParser/src/XMLParserUtilities.h
 * @author Shaun Roe
 * @date April, 2026
 * @brief Various text/ptr utilities to use in the ExpatCoreParser.
 */
 
#ifndef XMLCoreParser_XMLParserUtilities_H
#define XMLCoreParser_XMLParserUtilities_H

#include <expat.h>
#include <cstdlib> //getenv
#include <string>
#include <source_location>
#include <memory>
#include <iostream>

 
namespace XmlParser{
   ///find the xml file locally or on the datapath, return the full filename
   std::string
   xmlFileName(const std::string & fname, const std::string & prefix="");
   
  ///true if XML debug mode is enabled in the environment
  inline bool 
  debug_enabled() {
    static const bool enabled = (std::getenv("XMLDEBUG") != nullptr);
    return enabled;
  }
  
  ///report function name in a string, for debugging
  inline std::string 
  label(const std::source_location loc = std::source_location::current()){
    return loc.function_name();
  }
  
  ///deleter for a unique_ptr<XML_Parser>
  struct XMLParserDeleter {
    void operator()(XML_Parser p) const noexcept {
      if (p) {
        XML_ParserFree(p);
      }
    }
  };
  ///RAII XMLParser pointer
  using XMLParserPtr = std::unique_ptr<std::remove_pointer_t<XML_Parser>, XMLParserDeleter>;
  
  ///Create an RAII XMLParser pointer
  inline XMLParserPtr make_parser(){
    XML_Parser p = XML_ParserCreate(nullptr);
    if (!p) {
      std::cout << "ExpatCoreParser::Couldn't allocate memory for parser" << std::endl;
      std::abort();
    }
    return XMLParserPtr{p};
  }
  ///Trim newline from end of a XML_Char * , return the trimmed string as std::string
  inline std::string 
  rtrim(const XML_Char* s, int len){
    while (len > 0 && s[len - 1] == '\n') {
      --len;
    }
    return (len > 0) ? std::string{s, static_cast<std::size_t>(len)} : std::string{};
  }
}


#endif