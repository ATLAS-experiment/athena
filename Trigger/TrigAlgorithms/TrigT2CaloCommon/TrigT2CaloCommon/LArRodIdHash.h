/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 
 NAME:     LArRodIdHash.h
 PACKAGE:  Trigger/TrigAlgorithms/TrigT2CaloCommon
 
 AUTHOR:   Denis Oliveira Damazio

 PURPOSE:  LArReadoutModuleID to integer (hash ID) mapping.
 *******************************************************************/

#ifndef TRIGT2CALOCOMMON_LARRODIDHASH_H
#define TRIGT2CALOCOMMON_LARRODIDHASH_H

#include <vector> 
#include <unordered_map>

class HWIdentifier;

/** class that provides LArReadoutModuleID to integer
    hash ID mapping.  */
class LArRodIdHash  {

 public:

  /** definition of ID type */
  using ID = unsigned int;

  /** Initialize. Here real map is built */
  void initialize(int offset, const std::vector<HWIdentifier>& roms ); 

  /** convert ID to index
   *  throws std::out_of_range if not found */
  size_t operator() (ID id) const ;

  /** return maximum number of IDs */
  size_t max() const { return m_int2id.size(); }

  /** reverse conversion */
  ID identifier(size_t i) const { return m_int2id[i]; }

  /** return  offset */
  int offset() const { return m_offset; }

 private:

  /** lookup map */
  std::unordered_map<ID, size_t> m_lookup ;

  /** reverse lookup */
  std::vector<ID> m_int2id;

  int m_offset{0};

};

#endif

