/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "XMLCoreParser/XMLCoreParser.h" 

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <format>
 
namespace{ 
  class XMLCoreParserDebugger{ 
  public:
    static bool get_debug_state(){
      return ::getenv ("XMLDEBUG") != 0;
    }
    static bool debug (){
      static const bool debug_state = get_debug_state();
      return debug_state;
    }
  };
}

#include "ExpatCoreParser.h"


/* 
 *
 *   XMLCoreFactory implementation 
 *
 */ 
 
XMLCoreFactory::~XMLCoreFactory() { 
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreFactory::~XMLCoreFactory> factory=" << this << std::endl; 
  }
} 
 
void 
XMLCoreFactory::start (XMLCoreParser& parser, const XMLCoreNode& node) { 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreFactory::start> factory=" << this << std::endl; 
    }
  do_start (parser, node); 
} 
 
void 
XMLCoreFactory::end (XMLCoreParser& parser, const XMLCoreNode& node) { 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreFactory::end>" << std::endl; 
  }
  do_end (parser, node); 
} 
 
void 
XMLCoreFactory::comment (XMLCoreParser& parser, const std::string& comment) { 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreFactory::comment>" << std::endl; 
  }
  do_comment (parser,comment); 
} 
 
void 
XMLCoreFactory::do_start (XMLCoreParser& /*parser*/, const XMLCoreNode& /*node*/) { 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreFactory::do_start>" << std::endl; 
  }
} 
 
void 
XMLCoreFactory::do_end (XMLCoreParser& /*parser*/, const XMLCoreNode& /*node*/) { 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreFactory::do_end>" << std::endl; 
    }
} 
 
void 
XMLCoreFactory::do_comment (XMLCoreParser& /*parser*/, const std::string& /*comment*/) { 
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreFactory::do_comment>" << std::endl; 
  }
} 
 
bool 
XMLCoreFactory::has_attribute (const XMLCoreNode& node, const std::string& name) {
  return node.has_attrib (name);
} 
 
int 
XMLCoreFactory::get_int (const XMLCoreNode& node, const std::string& name) { 
  int result = 0; 
  std::string s = get_value (node, name); 
  sscanf (s.c_str (), "%80d", &result); 
  return (result); 
} 
 
double 
XMLCoreFactory::get_double (const XMLCoreNode& node, const std::string& name) { 
  double result = 0; 
  std::string s = get_value (node, name); 
  sscanf (s.c_str (), "%80lg", &result); 
  return (result); 
} 
 
bool 
XMLCoreFactory::get_boolean (const XMLCoreNode& node, const std::string& name) { 
  bool result = false; 
  std::string s = get_token (node, name); 
  if (s == "TRUE") result = true; 
  return (result); 
} 
 
std::string XMLCoreFactory::get_ID (const XMLCoreNode& node, const std::string& name) { 
  std::string result = get_value (node, name); 
  return (result); 
} 
 
std::string 
XMLCoreFactory::get_value (const XMLCoreNode& node, const std::string& name) {
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreFactory::get_value> name=" << name << std::endl; 
  }
  std::string result;
  if (node.has_attrib (name))
    result = node.get_attrib (name);
  if (XMLCoreParserDebugger::debug ()) {
    std::cout << "XMLCoreFactory::get_value>2 value=" << result << std::endl; 
  }
  return (result); 
} 
 
std::string 
XMLCoreFactory::get_token (const XMLCoreNode& node, const std::string& name) { 
  std::string result = get_value (node, name); 
    // trim the value 
  while ((result.length () > 0) &&  
         (result.at(0) == ' ')) result.erase (0, 1); 
 
  while ((result.length () > 0) &&  
         (result.at(result.length () - 1) == ' ')) result.erase (result.length () - 1, 1); 
  // Convert to upper case 
  for (std::string::size_type i = 0; i < result.length (); ++i){
    result[i] = std::toupper (result[i]);
  } 
  return (result); 
} 
 
 
bool 
XMLCoreFactory::check_int (const int n, const XMLCoreNode& node, const std::string& name) { 
  std::string checkstring = get_value (node, name); 
  int counter = 0; 
    // 
    // concatenate two times same string to  
    // check the last number of the string 
    // explicitly! 
    // 
  std::string t = checkstring + " " + checkstring; 
  std::istringstream tmpstr (t.c_str()); 
  while (tmpstr.good ()) { 
      int ii; 
      counter++; 
      tmpstr >> ii; 
  } 
  if (counter/2 != n) { 
    std::cerr << "XMLCoreFactory::check_int error: no " << n  
              << " ints in \"" << checkstring << "\" for attribute " <<  
        name << ". exit." << std::endl; 

    std::string nodename   = get_value (node, "name"); 
    std::string volume = get_value (node, "volume"); 

    if (nodename != "" ) std::cerr << "for name=" << nodename << std::endl; 
    if (volume != "" ) std::cerr << "for volume=" << volume << std::endl; 

    std::abort();
  } 
  return true; 
} 
 
bool 
XMLCoreFactory::check_double (const int n, const XMLCoreNode& node, const std::string& name) { 
  std::string checkstring = get_value (node, name); 
  int counter = 0; 
    // 
    // concatenate two times same string to  
    // check the last number of the string 
    // explicitly! 
    // 
  std::string t = checkstring + " " + checkstring; 
  std::istringstream tmpstr (t.c_str()); 
  while (tmpstr.good ()) { 
    double ii{}; 
    counter++; 
    tmpstr >> ii; 
  } 
  if (counter/2 != n) { 
      std::cerr << "XMLCoreFactory::check_double error: (" << counter << ") no " << n  
                << " doubles in \"" << checkstring << "\" for attribute " <<  
          name << ". exit." << std::endl; 
      std::string name1  = get_value (node, "name"); 
      std::string volume = get_value (node, "volume"); 
      if (name1 != "" ) std::cerr << "for name=" << name << std::endl; 
      if (volume != "" ) std::cerr << "for volume=" << volume << std::endl; 
      std::abort();
    } 
  return true; 
} 
 
class DummyFactory : public XMLCoreFactory{
}; 

 
std::unique_ptr<XMLCoreNode>
XMLCoreParser::parse (std::string_view file_name) {
  m_level = 0;
  std::unique_ptr<XMLCoreNode> doc = ExpatCoreParser::parse (file_name);
  if (XMLCoreParserDebugger::debug ()){
    if (doc != nullptr) doc->print ("============ ALL =============");
  }
  if (not doc){
    throw std::runtime_error(std::format("XMLCoreParser: no such file [{}]", file_name));
  }
  return doc;
}

std::unique_ptr<XMLCoreNode>
XMLCoreParser::parse_string (const std::string& text) {
  m_level = 0;
  std::unique_ptr<XMLCoreNode> doc = ExpatCoreParser::parse_string (text);
  if (XMLCoreParserDebugger::debug ()){
    if (doc != nullptr) doc->print ("============ ALL =============");
  }
  if (not doc){
    throw std::runtime_error("XMLCoreParser: cannot parse string");
  }
  return doc;
}

void 
XMLCoreParser::visit (std::string_view file_name) { 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << std::format("XMLCoreParser::visit file_name {}\n", file_name); 
  }
  std::unique_ptr<XMLCoreNode> n = parse (file_name);
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreParser::visit node=" << n.get() << std::endl;
  }
  visit (*n);
} 
 
void 
XMLCoreParser::visit (const XMLCoreNode& node){
  // Get the name and value out for convenience 
  const std::string& nodeName = node.get_name(); 
  const std::string& nodeValue = node.get_value(); 
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreParser::visit node(" << &node << ") " << nodeName << std::endl;
  }
  XMLCoreFactory* factory = find_factory (nodeName); 
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreParser::visit factory " << factory << std::endl; 
  }

  switch (node.get_type()) { 
  case XMLCoreNode::DOCUMENT_NODE : {
    std::vector<const XMLCoreNode*> children = node.get_children();
    for (const XMLCoreNode* child : children) {
      visit (*child);
    } 
    break;
  }
  case XMLCoreNode::ELEMENT_NODE : {
    if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreParser::visit ELEMENT_NODE "
                << " factory=" << factory
                << std::endl;
    }
    if (factory != 0){
      factory->start (*this, node);
    } else {
      std::cerr << "XMLCoreParser> Cannot find factory for element "
                << nodeName << std::endl;
      register_factory (nodeName, std::make_unique<DummyFactory>());
    }
    std::vector<const XMLCoreNode*> children = node.get_children();
    for (const XMLCoreNode* child : children) {
      visit (*child);
    }
    if (factory != 0) factory->end (*this, node);
    break;
  }
  case XMLCoreNode::COMMENT_NODE : {
    if (factory != 0) factory->comment (*this, nodeValue);
    break;
  }
  case XMLCoreNode::ENTITY_NODE:{
    std::cout << "ENTITY_NODE " << nodeValue << std::endl;
    break;
  }
  case XMLCoreNode::ENTITY_REFERENCE_NODE:{
    std::cout << "ENTITY_REFERENCE_NODE " << nodeValue << std::endl;
    break;
  }
  case XMLCoreNode::TEXT_NODE:
    break;
  default:
    std::cerr << "Unrecognized node type = " << (long) node.get_type() << std::endl;
    break;
  }
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreParser::visit-2" << std::endl; 
  }
} 
 
void 
XMLCoreParser::register_default_factory (std::unique_ptr<XMLCoreFactory> factory) {
  m_default_factory = std::move (factory);
}

void 
XMLCoreParser::register_factory (const std::string& name,
                                      std::unique_ptr<XMLCoreFactory> factory) { 
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreFactory::register_factory> name=" << name
              << " factory=" << factory.get() << std::endl; 
  }
  m_factories[name] = std::move (factory);
}

void 
XMLCoreParser::register_external_entity (const std::string& name, const std::string& file_name){
  if (XMLCoreParserDebugger::debug ()){
    std::cout << "XMLCoreParser::register_external_entity> name=" << name
    << " file_name=" << file_name << std::endl; 
  }
  ExpatCoreParser::register_external_entity (name, file_name); 
}
 
void 
XMLCoreParser::register_text_entity (const std::string& name, const std::string& text){
  if (XMLCoreParserDebugger::debug ()){
      std::cout << "XMLCoreParser::register_text_entity> name=" << name
		<< std::endl; 
    }
  ExpatCoreParser::register_text_entity (name, text); 
}
 
 
XMLCoreFactory* 
XMLCoreParser::find_factory (const std::string& name) { 
  FactoryMap::iterator it = m_factories.find (name);
  if (it != m_factories.end ()) { 
    return (*it).second.get();
  }
  return m_default_factory.get();
} 


void 
XMLCoreParser::up(){
  m_level += 1;
}


void 
XMLCoreParser::down(){
  m_level -= 1;
}


int 
XMLCoreParser::level() const{
  return m_level;
}
