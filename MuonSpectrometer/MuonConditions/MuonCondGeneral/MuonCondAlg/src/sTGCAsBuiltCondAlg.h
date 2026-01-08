/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_sTGCAsBuiltCondAlg_H
#define MUONCONDALG_sTGCAsBuiltCondAlg_H

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonAlignmentData/sTGCAsBuiltData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include <nlohmann/json.hpp>

/**
 * Conditions algorithm to load the alternativ sTGC as built constants.
*/
namespace Muon{
class sTGCAsBuiltCondAlg : public AthCondAlgorithm {
public:
    using AthCondAlgorithm::AthCondAlgorithm;
    virtual ~sTGCAsBuiltCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    /// Parse efficiency data from COOL
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 sTGCAsBuiltData& effiData) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

     /// Load the gasGap efficiencies from a JSON file
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };

    SG::WriteCondHandleKey<sTGCAsBuiltData> m_writeKey{this, "WriteKey", "sTGCAsBuilt", "Key of the efficiency data in the CondStore"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "",
                                                              "Folder of the STGC efficiencies as they're stored in COOL"};
};
}
#endif
