/** emacs: this is -*- c++ -*- **/
/**
 **   @file    MuonRegSelCondAlg.h        
 **                   
 **   @author  sutt
 **   @date    Tue  4 Feb 2020 15:25:00 CET
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/
 
#ifndef MuonRegSelCondAlg_h
#define MuonRegSelCondAlg_h

#include "GaudiKernel/EventIDRange.h"
#include "GaudiKernel/ISvcLocator.h"

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "IRegionSelector/IRegSelLUTCondData.h"
#include "RegSelLUT/RegSelSiLUT.h"

#include <string>


/////////////////////////////////////////////////////////////////////////////

class MuonRegSelCondAlg : public AthCondAlgorithm {

public:

  MuonRegSelCondAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

  virtual std::unique_ptr<RegSelSiLUT> createTable( const EventContext& ctx, EventIDRange& id_range ) const = 0;   

 protected:
    /// MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};
 private:

  Gaudi::Property<bool>        m_printTable{this, "PrintTable", false};
  Gaudi::Property<std::string> m_mangerName{this, "ManagerName", "", "Property no where used"};
 
  /// Output conditions object
  SG::WriteCondHandleKey<IRegSelLUTCondData> m_tableKey  
    { this, "RegSelLUT", "RegSelLUTCondData", "Region Selector lookup table" };

};

#endif // MuonRegSelCondAlg_h
