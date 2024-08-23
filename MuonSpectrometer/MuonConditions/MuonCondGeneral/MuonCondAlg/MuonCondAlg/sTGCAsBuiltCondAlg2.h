/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_sTGCAsBuiltCondAlg2_H
#define MUONCONDALG_sTGCAsBuiltCondAlg2_H

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonAlignmentData/sTGCAsBuiltData2.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include <nlohmann/json.hpp>

/**
 * Conditions algorithm to load the alternativ sTGC as built constants.
*/
class sTGCAsBuiltCondAlg2 : public AthReentrantAlgorithm {
public:
    sTGCAsBuiltCondAlg2(const std::string& name, ISvcLocator* svc);
    virtual ~sTGCAsBuiltCondAlg2() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual bool isReEntrant() const override { return false; }

private:
    /// Parse efficiency data from COOL
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 sTGCAsBuiltData2& effiData) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

     /// Load the gasGap efficiencies from a JSON file
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };

    SG::WriteCondHandleKey<sTGCAsBuiltData2> m_writeKey{this, "WriteKey", "sTGCAsBuilt2", "Key of the efficiency data in the CondStore"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "",
                                                              "Folder of the STGC efficiencies as they're stored in COOL"};
};

#endif
