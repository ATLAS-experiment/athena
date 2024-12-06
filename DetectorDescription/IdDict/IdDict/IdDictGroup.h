/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictGroup_H
#define IDDICT_IdDictGroup_H

#include <string>
#include <vector>

class IdDictDictEntry;
class IdDictRegion;
class IdDictMgr;
class IdDictDictionary;
class MultiRange;


 
class IdDictGroup{ 
public: 
    IdDictGroup () = default; 
    ~IdDictGroup () = default; 
    IdDictGroup (const std::string& name); 

    const std::string&  name();
    const std::vector<IdDictDictEntry*>& entries();
    const std::vector<IdDictRegion*>&    regions();

    ///  Get MultiRange for this group
    MultiRange build_multirange () const; 
    void add_dictentry (IdDictDictEntry* entry);

    void resolve_references (const IdDictMgr& idd,  
                             IdDictDictionary& dictionary,
                             size_t& index);  
    void generate_implementation (const IdDictMgr& idd,  
                                  IdDictDictionary& dictionary, 
                                  const std::string& tag = "");
    void reset_implementation ();  
    bool verify () const;
    void sort   ();  
    void clear  (); 


private:

    std::string                   m_name;  
    std::vector<IdDictDictEntry*> m_entries;  // just the RegionEntries
    std::vector<IdDictRegion*>    m_regions;  // regions derived from entries
    bool m_generated_implementation{};
}; 

#endif