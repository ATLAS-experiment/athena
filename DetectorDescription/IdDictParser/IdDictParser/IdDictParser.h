/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICTPARSER_IDDICTPARSER_H
#define IDDICTPARSER_IDDICTPARSER_H
  
#include "XMLCoreParser/XMLCoreParser.h"  
#include "IdDict/IdDictMgr.h"
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
  IdDictMgr& parse (const std::string& file_name, const std::string& tag = ""); 
 
  IdDictMgr             m_idd; 
  IdDictDictionary*     m_dictionary; 
  IdDictField*          m_field; 
  IdDictRegion*         m_region; 
  IdDictAltRegions*	m_altregions;
  IdDictSubRegion*      m_subregion; 
  IdDictRegionEntry*    m_regionentry;
}; 
  
#endif  
