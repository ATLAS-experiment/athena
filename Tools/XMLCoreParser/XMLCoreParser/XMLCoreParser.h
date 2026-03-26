/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XMLCoreParser_hpp
#define XMLCoreParser_hpp

#include "XMLCoreParser/XMLCoreNode.h"
#include <string>
#include <map>
#include <memory>

class XMLCoreParser;
class XMLCoreParserImpl;

class XMLCoreFactory { 
public: 
   
  virtual ~XMLCoreFactory(); 

  void start (XMLCoreParser& parser, const XMLCoreNode& node); 
  void end (XMLCoreParser& parser, const XMLCoreNode& node); 
  void comment (XMLCoreParser& parser, const std::string& comment);
 
  virtual void do_start (XMLCoreParser& parser, const XMLCoreNode& node); 
  virtual void do_end (XMLCoreParser& parser, const XMLCoreNode& node); 
  virtual void do_comment (XMLCoreParser& parser, const std::string& comment);
  
  static bool has_attribute (const XMLCoreNode& node, const std::string& name); 
  static int get_int (const XMLCoreNode& node, const std::string& name); 
  static double get_double (const XMLCoreNode& node, const std::string& name); 
  static bool get_boolean (const XMLCoreNode& node, const std::string& name); 
  static std::string get_ID (const XMLCoreNode& node, const std::string& name); 
  static std::string get_value (const XMLCoreNode& node, const std::string& name); 
  static std::string get_token (const XMLCoreNode& node, const std::string& name); 
  
  static bool check_int  (const int n, const XMLCoreNode& node, const std::string& name); 
  static bool check_double (const int n, const XMLCoreNode& node, const std::string& name);
 
protected: 
 
  std::string m_xmlelementname; 
}; 

class XMLCoreParser { 
public: 

  std::unique_ptr<XMLCoreNode> parse (const std::string& file_name);
  std::unique_ptr<XMLCoreNode> parse_string (const std::string& text);
  void visit (const std::string& file_name); 
 
  void register_default_factory (std::unique_ptr<XMLCoreFactory> factory); 
  void register_factory (const std::string& name,
                         std::unique_ptr<XMLCoreFactory> factory); 
  void register_external_entity (const std::string& name, const std::string& file_name); 
  void register_text_entity (const std::string& name, const std::string& text);

  void up();
  void down();
  int level() const;
 
  
private: 
 
  void visit (const XMLCoreNode& node);
  void terminate ();
  XMLCoreFactory* find_factory (const std::string& name);

  typedef std::map <std::string, std::unique_ptr<XMLCoreFactory> > FactoryMap; 
  FactoryMap m_factories;
  std::unique_ptr<XMLCoreFactory> m_default_factory;
  int m_level = 0;
}; 
 
#endif 
