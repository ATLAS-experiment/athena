/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICTPARSER_IDDICTPARSER_H
#define IDDICTPARSER_IDDICTPARSER_H
  
#include "XMLCoreParser/XMLCoreParser.h"  
#include "IdDict/IdDictMgr.h"
#include <memory>
class IdDictDictionary;
class IdDictField;
class IdDictRegion;
class IdDictAltRegions;
class IdDictSubRegion;
class IdDictRegionEntry;
  
class IdDictParser : public XMLCoreParser  
{ 
public:  
  IdDictParser ();  
  ~IdDictParser ();
  IdDictMgr& parse (const std::string& file_name, const std::string& tag = ""); 
 
  IdDictMgr             m_idd; 
  std::unique_ptr<IdDictDictionary>  m_dictionary;
  std::unique_ptr<IdDictField>       m_field;
  std::unique_ptr<IdDictRegion>      m_region;
  std::unique_ptr<IdDictAltRegions>  m_altregions;
  std::unique_ptr<IdDictSubRegion>   m_subregion;
  std::unique_ptr<IdDictRegionEntry> m_regionentry;
}; 
  
#endif  
