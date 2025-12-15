/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_MmCTPCondDbAlg_H
#define MUONCONDALG_MmCTPCondDbAlg_H

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteCondHandleKey.h"
#include <nlohmann/json.hpp>
#include "MuonCondData/mmCTPClusterCalibData.h"

class MmCTPCondDbAlg : public AthCondAlgorithm {
public:
    //No need for individual constructor
    using AthCondAlgorithm::AthCondAlgorithm ;
    virtual ~MmCTPCondDbAlg() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    /// Parse data from COOL
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 Muon::mmCTPClusterCalibData& ctpClusterCondData) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};



    SG::WriteCondHandleKey<Muon::mmCTPClusterCalibData> m_writeKey{this, "WriteKey", "mmCTPClusterCalibData", "Key of the CTP slope data in the CondStore"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "", "Folder of the MM CTP corrections as they are stored in COOL"};


};

#endif

