/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARAUTOCORRFROMSTDNTUPLE_H
#define LARAUTOCORRFROMSTDNTUPLE_H

#include "LArRawConditions/LArMCSym.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <vector>
#include <string>

/** @class LArAutoCorrFromStdNtuple

This algorithm allows to read autocorrs from ntuples and builds a 
Version for standard Ntuple, produced by LArCalibTools algos....
With hardcoded numbers for sFcal
 */


class LArAutoCorrFromStdNtuple : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual ~LArAutoCorrFromStdNtuple();

  //standard algorithm methods
  /// implements IAlgorithm::initialize() 
  virtual StatusCode initialize() override;

  /// implements IAlgorithm::execute()  : Does nothing
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}

  virtual StatusCode stop() override;
 
 private:
  IntegerProperty m_nsamples { this, "Nsamples", 7 };
  /// list of input ntuple file names 
  StringArrayProperty m_root_file_names { this, "FileNames", {} };
  /// ntuple name
  StringProperty m_ntuple_name { this, "NtupleName", "AUTOCORR" };
  /// key of the LArAutoCorr collection in Storegate
  StringProperty m_store_key { this, "StoreKey", "FromStdNtuple" };
  /// Grouping type.  
  StringProperty m_groupingType { this, "GroupingType", "ExtendedSubDetector" };
  ///  type
  BooleanProperty m_isComplete { this, "isComplete", false };
  /// drop FCAL and change to sFCal
  BooleanProperty m_sFcal { this, "doSFcal", false };

   SG::ReadCondHandleKey<LArMCSym> m_mcSymKey
   {this, "MCSymKey", "LArMCSym", "SG Key of LArMCSym object"};
   SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
};

#endif
