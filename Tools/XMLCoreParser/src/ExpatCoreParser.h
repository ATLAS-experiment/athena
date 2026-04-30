/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XMLCOREPARSER_EXPATCOREPARSER_H
#define XMLCOREPARSER_EXPATCOREPARSER_H

#include "XMLCoreParser/XMLCoreNode.h"
#include "CxxUtils/checker_macros.h"

#include <expat.h>
#include <iosfwd>
#include <map>
#include <string>
#include <mutex>
#include <memory>


class ExpatCoreParser
{
public:

  typedef std::map <std::string, std::string> ExternalEntityMap; 

  static std::unique_ptr<XMLCoreNode> parse (const std::string& file_name);
  static std::unique_ptr<XMLCoreNode> parse_string (const std::string& text);

  static void register_external_entity (const std::string& name, const std::string& file_name);  
  static void register_text_entity (const std::string& name, const std::string& text);  
 
private:

  ExpatCoreParser (const std::string& prefix);
  void configure_parser(XML_Parser p);
  XMLCoreNode* add_node (std::unique_ptr<XMLCoreNode> node);
  void do_start (const char* el, const char** attr);
  void do_end (const char* el);
  void do_char_data (const XML_Char* s, int len);
  void do_default_handler (const XML_Char* s, int len);
  void do_comment (const XML_Char* s);
  int generic_parse (XML_Parser p, const std::string& file_name);
  int generic_text_parse (XML_Parser p, const std::string& text);
  int stream_parse(XML_Parser p, std::istream& is, const std::string & source_name = {});
  int do_external_entity (XML_Parser parser, const XML_Char* context, const XML_Char* systemId);
  static void start (void* /*user_data*/, const char* el, const char** attr);
  static void end (void* /*user_data*/, const char* el);
  static void char_data (void* /*user_data*/, const XML_Char* s, int len);
  static void default_handler (void* /*user_data*/, const XML_Char* s, int len);
  static void comment (void* /*userData*/, const XML_Char* s);
  static int external_entity (XML_Parser parser, const XML_Char* context,
			      const XML_Char* /*base*/, const XML_Char* systemId, 
			      const XML_Char* /*publicId*/);
  std::unique_ptr<XMLCoreNode> get_document ();
  std::string find_external_entity (const std::string& name);
  std::string find_text_entity (const std::string& name);
  std::string find_entity (const std::string& name, const ExternalEntityMap & mapChoice);
  void clean ();
  std::unique_ptr<XMLCoreNode> m_top;
  XMLCoreNode* m_last{};
  std::string m_prefix;
  static std::mutex s_mutex;
  typedef std::lock_guard<std::mutex> lock_t;
  static ExternalEntityMap s_entities ATLAS_THREAD_SAFE;
  static ExternalEntityMap s_text_entities ATLAS_THREAD_SAFE;
};

#endif

