/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARPARAMSFROMSTDNTUPLE_H
#define LARPARAMSFROMSTDNTUPLE_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include <vector>
#include <string>

/** @class LArParamsFromStdNtuple

This algorithm allows to read wave forms from ntuples and builds a 
LArCaliPulseParamsComplete and/or LArDetCellParamsComplete . 
Version for standard Ntuple, produced by LArCalibTools algos....
 */


class LArParamsFromStdNtuple : public AthReentrantAlgorithm
{
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual ~LArParamsFromStdNtuple();

  //standard algorithm methods

  /// implements IAlgorithm::execute()  : Does nothing
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}

  /// IAlgorithm::stop() : Where the action takes place...
  virtual StatusCode stop() override;
 
 private:
  /// list of input ntuple file names 
  StringArrayProperty m_root_file_names { this, "FileNames", {} };
  /// ntuple name
  StringProperty m_ntuple_name { this, "NtupleName", "PARAMS" };
  /// key of the CaliPulseParams collection in Storegate
  StringProperty m_store_key_cali { this, "StoreKey_Cali", "FromStdNtuple" };
  /// key of the DetCellParams collection in Storegate
  StringProperty m_store_key_det { this, "StoreKey_Det", "FromStdNtuple" };
  /// Grouping type.  Default is Feedthrough.
  StringProperty m_groupingType { this, "GroupingType", "FeedThrough" };
};

#endif
