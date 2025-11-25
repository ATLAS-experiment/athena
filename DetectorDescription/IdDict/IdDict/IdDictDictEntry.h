/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictDictEntry_H
#define IDDICT_IdDictDictEntry_H

#include <string>
class Range;
class IdDictMgr;
class IdDictDictionary;

class IdDictDictEntry { 
public: 
    IdDictDictEntry (); 
    virtual ~IdDictDictEntry (); 
    virtual Range build_range () const = 0; 
    virtual std::string group_name () const = 0; 
    virtual void set_index (size_t index) = 0;
    virtual void resolve_references (IdDictMgr& idd,
                                     IdDictDictionary& dictionary) = 0;  
    virtual void generate_implementation (const IdDictMgr& idd,  
                                          IdDictDictionary& dictionary, 
                                          const std::string& tag = "") = 0;
    virtual void reset_implementation () = 0;  
    virtual bool verify () const = 0;
    virtual void clear () = 0; 
 
}; 

#endif

