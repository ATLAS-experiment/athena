/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IDDICTMGR_H
#define IDDICT_IDDICTMGR_H

#include <map>
#include <set>
#include <string>
#include <vector>

class IdDictDictionary;

class IdDictMgr  {  
public:  
    using dictionary_vec = std::vector<const IdDictDictionary*>;
    typedef std::map<std::string, IdDictDictionary*> dictionary_map; 
    typedef std::map<std::string, std::string>       metadata_map; 

    IdDictMgr();
    ~IdDictMgr();
 
    /// Version tag
    const std::string&    tag                     () const;

    /// Access dictionary by name
    IdDictDictionary*     find_dictionary         (const std::string& name) const;  

    /// Access to all dictionaries
    const dictionary_map& get_dictionary_map      () const;  
    dictionary_vec        get_dictionaries        () const;

    /// DTD version
    const std::string&    DTD_version             () const;

    /// Check whether or not to do checks for ids
    bool                  do_checks               () const;

    /// Check whether or not to init neighbours
    bool                  do_neighbours           () const;

    /// Access to meta data, name/value pairs
    const std::string&    find_metadata           (const std::string& name) const;

    ///  Fillers:
    void                  add_dictionary          (IdDictDictionary* dictionary);  
    void                  add_subdictionary_name  (const std::string& name);  
    void                  add_metadata            (const std::string& name, const std::string& value);
    void                  set_DTD_version         (const std::string& DTD_version);
    void                  set_do_checks           (bool do_checks);
    void                  set_do_neighbours       (bool do_neighbours);
    
    
    ///  Construct dictionary after parsing
    void                  resolve_references      ();  
    void                  generate_implementation (const std::string& tag = "");  

    ///  Reset of implementation
    void                  reset_implementation    ();  
    void                  clear                   (); 
    
    bool                  verify                  () const;  
    
 
private:

    void                  find_subdicts(IdDictDictionary* dict);

    std::string           m_DTD_version;  
    std::string           m_tag; 
    dictionary_map        m_dictionaries; 
    metadata_map          m_metadata;
    std::set<std::string> m_subdictionary_names; 
    bool                  m_resolved_references;
    bool                  m_generated_implementation;
    bool                  m_do_checks;
    bool                  m_do_neighbours;
}; 

#endif
