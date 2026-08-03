/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARSHAPEFROMSTDNTUPLE_H
#define LARSHAPEFROMSTDNTUPLE_H

#include "LArRawConditions/LArMCSym.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <vector>
#include <string>

/** @class LArShapeFromStdNtuple

This algorithm allows to read wave forms from ntuples and builds a 
LArPhysWaveContainer containing the corresponding PhysWave. 
Version for standard Ntuple, produced by LArCalibTools algos....
 */

class LArShapeFromStdNtuple : public AthReentrantAlgorithm
{
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual ~LArShapeFromStdNtuple();

  //standard algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}
  virtual StatusCode stop() override;
 
 private:
  /// the first  m_skipPoints points of the waveform in the ntuple are skipped
  UnsignedIntegerProperty m_skipPoints { this, "SkipPoints", 0 };
  /// make a Shape with the first m_prefixPoints as zeros
  UnsignedIntegerProperty m_prefixPoints { this, "PrefixPoints", 0 };
  /// list of input ntuple file names 
  StringArrayProperty m_root_file_names { this, "FileNames", {} };
  /// ntuple name
  StringProperty m_ntuple_name { this, "NtupleName", "SHAPE" };
  /// key of the LArShape collection in Storegate
  StringProperty m_store_key { this, "StoreKey", "FromStdNtuple" };
  /// Grouping type.  
  StringProperty m_groupingType { this, "GroupingType", "ExtendedSubDetector" };
  /// Shape type
  BooleanProperty m_isComplete { this, "isComplete", false };

  bool m_done = false;

  SG::ReadCondHandleKey<LArMCSym> m_mcSymKey
  {this, "MCSymKey", "LArMCSym", "SG Key of LArMCSym object"};
};

#endif
