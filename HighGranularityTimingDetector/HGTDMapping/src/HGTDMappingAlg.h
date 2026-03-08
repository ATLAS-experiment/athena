/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTDMappingAlg_H
#define HGTDMappingAlg_H
/**   
 *   @file HGTDMappingAlg.h
 *   @date  23 January 2026
 *   @brief Online Identifier for HGTD
**/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "HGTDMapping/HGTDMappingData.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"

#include "HGTD_Identifier/HGTD_ID.h"

class HGTDMappingAlg: public AthReentrantAlgorithm  {
    public:
    HGTDMappingAlg(const std::string& name, ISvcLocator* svc);
    virtual ~HGTDMappingAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  
    private:
    StringProperty m_source{this, "DataSource", "HGTDData.dat", "A data file for the HGTDMaping"};
    //std::string m_configFilePath={};
    SG::WriteCondHandleKey<HGTDMappingData> m_writeKey{this, "WriteKey", "HGTDMappingData", "Key of output (derived) conditions data"};
    const HGTD_ID* m_idHelper{nullptr};
};

#endif 
