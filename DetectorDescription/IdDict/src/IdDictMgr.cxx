/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Header: /DetectorDescription/IdDict/src/IdDictMgr.cxx,v 1.43 2008-12-09 09:49:43 dquarrie Exp $  

#include "IdDict/IdDictDefs.h"  
#include "Identifier/RangeIterator.h"
#include "Identifier/MultiRange.h"
#include "src/Debugger.h"
#include <fstream>  
#include <iostream>  
 
#include <stdio.h>
#include <cstdlib>
#include <atomic>
#include <stdexcept>
#include <format>
  

class TypeIdBuilder  {  
public:  
  static int build_id ();  
};  
  
template <class T>  
class TypeId  {  
public:  
  operator int () {  
    static const int i = TypeIdBuilder::build_id ();  
    return (i);  
  }  
};  
  
int 
TypeIdBuilder::build_id ()  {  
  static std::atomic<int> i = 0;  
  i++;  
  return (i);  
}  

IdDictMgr::~IdDictMgr(){
  clear();
}

const std::string&   	
IdDictMgr::tag() const
{
    return m_tag;
}

const std::string&   	
IdDictMgr::DTD_version() const {
  return m_DTD_version;
}

bool 
IdDictMgr::do_checks() const{
    return m_do_checks;
}

void		
IdDictMgr::set_do_checks	(bool do_checks){
  m_do_checks = do_checks;
  for (const auto& p : m_dictionaries) {
    IdDictDictionary* d = p.second; 
    d->set_do_checks(do_checks);
  } 
}

bool 
IdDictMgr::do_neighbours () const{
    return m_do_neighbours;
}

void		
IdDictMgr::set_do_neighbours(bool do_neighbours){
  m_do_neighbours = do_neighbours;
  for (const auto& p : m_dictionaries) {
    IdDictDictionary* d = p.second; 
    d->set_do_neighbours(do_neighbours);
  } 
}

const std::string&
IdDictMgr::find_metadata (const std::string& name) const{
    metadata_map::const_iterator it = m_metadata.find(name);
    static const std::string empty;
    if (it != m_metadata.end()) return (it->second);
    else return empty;
}

void       
IdDictMgr::add_metadata(const std::string& name, const std::string& value){
  if(!m_metadata.insert(metadata_map::value_type(name, value)).second) {
      std::cout << "IdDictMgr::add_metadata> unable to add name/value " << name << "/" << value << std::endl; 
  }
}

void
IdDictMgr::set_DTD_version(const std::string& DTD_version){
  m_DTD_version = DTD_version;
}


const IdDictMgr::dictionary_map& 
IdDictMgr::get_dictionary_map () const { 
  return (m_dictionaries); 
} 
 
IdDictDictionary* 
IdDictMgr::find_dictionary (const std::string& name) const  {  
  dictionary_map::const_iterator it;  
  it = m_dictionaries.find (name);  
  if (it == m_dictionaries.end ()) return (0);  
  return ((*it).second);  
}  
  
void 
IdDictMgr::add_dictionary (IdDictDictionary* dictionary)  {  
  if (dictionary == 0) return; 
  std::string& name = dictionary->m_name;  
  // Delete entry if already there
  dictionary_map::iterator it = m_dictionaries.find(name);
  if (it != m_dictionaries.end()) delete (*it).second;
  m_dictionaries[name] = dictionary;  
  if (Debugger::debug ()) { 
    dictionary_map::iterator it; 
    for (it = m_dictionaries.begin (); it != m_dictionaries.end (); ++it) { 
      std::string s = (*it).first; 
      IdDictDictionary* d = (*it).second; 
      std::cout << "IdDictMgr::add_dictionary> d[" << s << "]=" << d << std::endl; 
    } 
  } 
}  

void 
IdDictMgr::add_subdictionary_name (const std::string& name){
  m_subdictionary_names.insert(name);
}

  
void 
IdDictMgr::resolve_references ()  {  
  dictionary_map::iterator it;  
  for (it = m_dictionaries.begin (); it != m_dictionaries.end (); ++it){  
    // From mgr, only resolve refs for top-level dictionaries
    IdDictDictionary* dictionary = (*it).second;  
    if (m_subdictionary_names.find(dictionary->m_name) != m_subdictionary_names.end()) continue;
    dictionary->resolve_references (*this);  
  }  
} 
 
void 
IdDictMgr::generate_implementation (const std::string& tag){  
  if (Debugger::debug ()){ 
      std::cout << "IdDictMgr::generate_implementation>" << std::endl; 
  } 
  // Must reset the implementation for multiple passes, this resets
  // the generated flags
  if (m_generated_implementation && tag != m_tag) reset_implementation();
  if (!m_generated_implementation) {
    m_tag = tag;
    dictionary_map::iterator it;  
    for (it = m_dictionaries.begin (); it != m_dictionaries.end (); ++it) {  
      // From mgr, only generate impl for top-level dictionaries
      IdDictDictionary* dictionary = (*it).second;  
      if (m_subdictionary_names.find(dictionary->m_name) != m_subdictionary_names.end()) continue;
      dictionary->generate_implementation (*this, tag);  
    } 
    m_generated_implementation = true;
  }  
} 
 
void 
IdDictMgr::reset_implementation (){
  std::cout << "IdDictMgr::reset_implementation" << std::endl;
  if (m_generated_implementation) {
    dictionary_map::iterator it;  
    for (it = m_dictionaries.begin (); it != m_dictionaries.end (); ++it) {  
      // From mgr, only generate impl for top-level dictionaries
      IdDictDictionary* dictionary = (*it).second;  
      if (m_subdictionary_names.find(dictionary->m_name) != m_subdictionary_names.end()) continue;
      dictionary->reset_implementation ();  
    } 
    m_generated_implementation = false;
  }  
}


bool 
IdDictMgr::verify () const { 
  dictionary_map::const_iterator it;  
  for (it = m_dictionaries.begin (); it != m_dictionaries.end (); ++it){  
    const IdDictDictionary* dictionary = (*it).second;  
    if (!dictionary->verify ()) return (false);  
  } 
  return (true); 
} 
 
void 
IdDictMgr::clear () { 
  dictionary_map::iterator it;  
  
  for (it = m_dictionaries.begin (); it != m_dictionaries.end (); ++it) {  
    IdDictDictionary* dictionary = (*it).second;  
    dictionary->clear (); 
    delete dictionary; 
  } 
  m_dictionaries.clear (); 
  m_resolved_references      = false;
  m_generated_implementation = false;

} 

 