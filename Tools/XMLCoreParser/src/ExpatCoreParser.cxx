/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "XMLCoreParser/XMLCoreParser.h" 

#include "ExpatCoreParser.h"
#include "XMLParserUtilities.h"
#include <expat.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <array>
#include <iostream>
#include <filesystem>

namespace {
  constexpr std::size_t BUFFSIZE = 1000;
  //empty string to use in returns
  const std::string emptyString;
  constexpr int errorValue{0};
  constexpr int ok{1};
}

using namespace XmlParser;

std::mutex ExpatCoreParser::s_mutex;
ExpatCoreParser::ExternalEntityMap ExpatCoreParser::s_entities;
ExpatCoreParser::ExternalEntityMap ExpatCoreParser::s_text_entities;


void 
ExpatCoreParser::start (void* user_data, const char* el, const char** attr){
  auto& me = *static_cast<ExpatCoreParser*> (user_data);
  me.do_start (el, attr);
}

void 
ExpatCoreParser::end (void* user_data, const char* el){
  auto& me = *static_cast<ExpatCoreParser*> (user_data);
  me.do_end (el);
}

void 
ExpatCoreParser::char_data (void* user_data, const XML_Char* s, int len){
  auto& me = *static_cast<ExpatCoreParser*> (user_data);
  me.do_char_data (s, len);
}

void 
ExpatCoreParser::default_handler (void* user_data, const XML_Char* s, int len){
  auto& me = *static_cast<ExpatCoreParser*> (user_data);
  me.do_default_handler (s, len);
}

void 
ExpatCoreParser::comment (void* user_data, const XML_Char* s){
  auto& me = *static_cast<ExpatCoreParser*> (user_data);
  me.do_comment (s);
}

int 
ExpatCoreParser::external_entity (XML_Parser parser, const XML_Char* context,
    const XML_Char* /*base*/,const XML_Char* systemId, const XML_Char* /*publicId*/){
  void* user_data = XML_GetUserData (parser);
  auto& me = *static_cast<ExpatCoreParser*> (user_data);
  return (me.do_external_entity (parser, context, systemId));
}
  
void 
ExpatCoreParser::register_external_entity (const std::string& name, const std::string& file_name) {
  if (debug_enabled()){
    std::cout << label()<<"> name=" << name << " file_name=" << file_name << "\n"; 
  }
  lock_t lock (s_mutex);
  s_entities[name] = file_name; 
} 
  
void 
ExpatCoreParser::register_text_entity (const std::string& name, const std::string& text) {
  if (debug_enabled()) {
    std::cout << label()<<"> name=" << name<< "\n"; 
  }
  lock_t lock (s_mutex);
  s_text_entities[name] = text; 
} 


std::unique_ptr<XMLCoreNode>
ExpatCoreParser::get_document (){
  return std::move(m_top);
}

ExpatCoreParser::ExpatCoreParser (const std::string& prefix)
  : m_top (nullptr),
    m_last (nullptr),
    m_prefix (prefix){
}

void 
ExpatCoreParser::configure_parser(XML_Parser p) {
  XML_SetParamEntityParsing(p, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetElementHandler(p, start, end);
  XML_SetCharacterDataHandler(p, char_data);
  XML_SetExternalEntityRefHandler(p, external_entity);
  XML_SetCommentHandler(p, comment);
  XML_SetUserData(p, this);
}

XMLCoreNode*
ExpatCoreParser::add_node (std::unique_ptr<XMLCoreNode> node){
  if (!m_top){
    m_top = std::make_unique<XMLCoreNode> (XMLCoreNode::DOCUMENT_NODE);
    m_last = m_top.get();
  }

  return m_last->add_child (std::move(node));
}

void
ExpatCoreParser::do_start (const char* el, const char** attr){
  auto node = std::make_unique<XMLCoreNode> (XMLCoreNode::ELEMENT_NODE, el);
  if (debug_enabled()) {
    std::cout << label()<< "> el=" << el << " top=" << m_top.get() << " last=" << m_last << " node=" << node << "\n"; 
  }
  for (int i = 0; attr[i]; i += 2) {
    const char* name = attr[i];
    const char* value = attr[i+1];
    node->set_attrib (name, value);
  }
  m_last = add_node (std::move(node));
}

void 
ExpatCoreParser::do_end (const char* el){
  if (debug_enabled()){
    std::cout <<label()<<"> el=" << el << "\n"; 
  }
  m_last = m_last->get_parent();
}

void 
ExpatCoreParser::do_char_data (const XML_Char* s, int len){
  std::string text = rtrim(s, len);
  if (text.empty()) return;
  if (debug_enabled()) {
    std::cout << label()<<"> [" << text << "]\n";
  }
  add_node (std::make_unique<XMLCoreNode>(XMLCoreNode::TEXT_NODE, "", text));
}

void 
ExpatCoreParser::do_default_handler (const XML_Char* s, int len){
  std::string text = rtrim(s, len);
  if (text.empty()) return;
  if (debug_enabled()) {
      std::cout << label()<<"> [" << text << "]\n";
  }
}
  
void 
ExpatCoreParser::do_comment (const XML_Char* s){
  auto node = std::make_unique<XMLCoreNode> (XMLCoreNode::COMMENT_NODE, "", s);
  if (debug_enabled()) {
    std::cout << label()<<"> s=" << s << " top=" << m_top.get() << " last=" << m_last << " node=" << node << "\n"; 
  }
  add_node (std::move(node));
}

int 
ExpatCoreParser::stream_parse(XML_Parser p, std::istream& is, const std::string & source_name){
  int result = ok;
  configure_parser(p);
  if (debug_enabled()) {
    std::cout << label() << "> starting\n";
  }
  std::array<char, BUFFSIZE> buff{};
  for (;;) {
    is.read(buff.data(), buff.size());
    const auto nbytes = is.gcount();
    const bool done = is.eof();
    if (is.bad()) {
      std::cout << "Read error";
      if (!source_name.empty()) {
        std::cout << " in " << source_name;
      }
      std::cout << "\n";
      result = errorValue;
      break;
    }
    if (XML_Parse(p, buff.data(), static_cast<int>(nbytes), done) == XML_STATUS_ERROR) {
      std::cout << "ExpatCoreParser::Parse error at line "
                << XML_GetCurrentLineNumber(p);
      if (!source_name.empty()) {
        std::cout << " of " << source_name;
      }
      std::cout << ":\n" << XML_ErrorString(XML_GetErrorCode(p)) << "\n";
      result = errorValue;
      break;
    }
    if (done) {
      break;
    }
  }
  return result;
}


int 
ExpatCoreParser::generic_parse(XML_Parser p, const std::string& file_name){
  const std::string& path = xmlFileName(file_name, m_prefix);
  if (path.empty()) return errorValue;
  if (debug_enabled()) {
    std::cout << label() << "> file_name=" << file_name
              << " prefix=" << m_prefix << "\n";
  }
  std::ifstream fs{path, std::ios::binary};
  if (!fs) {
    std::cout << "Could not open file " << path << "\n";
    return errorValue;
  }
  return stream_parse(p, fs, file_name);
}

int 
ExpatCoreParser::generic_text_parse(XML_Parser p, const std::string& text){
  if (debug_enabled()) {
    std::cout << label() << ">\n";
  }
  std::istringstream is(text);
  return stream_parse(p, is, {});
}

int
ExpatCoreParser::do_external_entity(XML_Parser parser, const XML_Char* context, const XML_Char* systemId){
  const std::string context_str = context ? context : "none";
  if (context != nullptr) {
    const std::string replacement = find_text_entity(context_str);
    if (!replacement.empty()) {
      if (debug_enabled()) {
        std::cout << label() << ">  context=[" << context_str
                  << "] replacement=[" << replacement << "]\n";
      }
      XMLParserPtr p{XML_ExternalEntityParserCreate(parser, context, nullptr)};
      if (!p)  return errorValue;
      return generic_text_parse(p.get(), replacement);
    }
  }
  std::string replacement = find_external_entity(context_str);
  if (replacement == "NULL") {
    return ok;
  }
  std::string originalSystemId = systemId ? systemId : "";
  std::string effectiveSystemId = originalSystemId;
  if (!replacement.empty()) {
    effectiveSystemId = replacement;
  }
  if (debug_enabled()) {
    std::cout << label() << ">  context=[" << context_str
              << "]  systemId=[" << originalSystemId
              << "] replacement=[" << replacement << "]\n";
  }
  XMLParserPtr p{XML_ExternalEntityParserCreate(parser, context, nullptr)};
  if (!p) return errorValue;
  return generic_parse(p.get(), effectiveSystemId);
}

const std::string& 
ExpatCoreParser::find_entity (const std::string& name, const ExternalEntityMap& mapChoice){ 
  lock_t lock (s_mutex);
  ExternalEntityMap::const_iterator it = mapChoice.find(name); 
  return (it == mapChoice.end()) ? emptyString : it->second;
}
const std::string& 
ExpatCoreParser::find_external_entity (const std::string& name){ 
  return find_entity(name, s_entities);
} 

const std::string& 
ExpatCoreParser::find_text_entity (const std::string& name){ 
  return find_entity(name, s_text_entities);
} 

std::unique_ptr<XMLCoreNode>
ExpatCoreParser::parse (const std::string& file_name){
  const std::filesystem::path path{file_name};
  ExpatCoreParser me{path.parent_path().string()};
  auto p = make_parser();
  int result = me.generic_parse (p.get(), path.filename().string());
  if (result == errorValue) return nullptr;
  return me.get_document ();
}


std::unique_ptr<XMLCoreNode>
ExpatCoreParser::parse_string (const std::string& text){
  ExpatCoreParser me ("");
  auto p = make_parser();
  int result = me.generic_text_parse (p.get(), text);
  if (result == errorValue) return nullptr;
  return me.get_document ();
}

