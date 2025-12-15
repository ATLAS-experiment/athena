/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_MmDigitEffiCondAlg_H
#define MUONCONDALG_MmDigitEffiCondAlg_H

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/DigitEffiData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include <nlohmann/json.hpp>

/**
 * Conditions algorithm to load the sTGC efficiency constants that are used in digitization.
*/
class MmDigitEffiCondAlg : public AthCondAlgorithm {
public:
    MmDigitEffiCondAlg(const std::string& name, ISvcLocator* svc);
    virtual ~MmDigitEffiCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    /// Parse efficiency data from COOL
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 Muon::DigitEffiData& effiData) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

     /// Load the gasGap efficiencies from a JSON file
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };

    SG::WriteCondHandleKey<Muon::DigitEffiData> m_writeKey{this, "WriteKey", "MmDigitEff", "Key of the efficiency data in the CondStore"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "",
                                                              "Folder of the MM efficiencies as they're stored in COOL"};
    Gaudi::Property<double> m_defaultEffi{this, "defaultEffi", 1.};
};

#endif
