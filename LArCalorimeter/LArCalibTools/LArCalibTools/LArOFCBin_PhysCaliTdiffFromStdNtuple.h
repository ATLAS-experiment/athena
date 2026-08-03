/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LAROFCBIN_PHYSCALITDIFFFROMSTDNTUPLE_H
#define LAROFCBIN_PHYSCALITDIFFFROMSTDNTUPLE_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include <vector>
#include <string>


class LArOFCBin_PhysCaliTdiffFromStdNtuple : public AthReentrantAlgorithm
{
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual ~LArOFCBin_PhysCaliTdiffFromStdNtuple();

  //standard algorithm methods
  /// implements IAlgorithm::initialize() 
  virtual StatusCode initialize() override;

  /// implements IAlgorithm::execute()  : Does nothing
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}

  virtual StatusCode stop() override;
 
 private:
  /// list of input ntuple file names 
  StringArrayProperty m_root_file_names { this, "FileNames", {} };
  /// ntuple name
  StringProperty m_ntuple_name { this, "NtupleName", "PARAMS" };
  /// key of the OFCBin collection in Storegate
  StringProperty m_store_key_ofcbin { this, "StoreKey_OFC", "LArOFC" };
  BooleanProperty m_fillofc { this, "Store_OFC", false };
  /// key of the PhysCaliTdiff collection in Storegate
  StringProperty m_store_key_tdiff { this, "StoreKey_Tdiff", "LArPhysCaliTdiff" };
  BooleanProperty m_filltdiff { this, "Store_Tdiff", false };
  /// Grouping type.  Default is Feedthrough.
  StringProperty m_groupingType { this, "GroupingType", "FeedThrough" };
};

#endif
