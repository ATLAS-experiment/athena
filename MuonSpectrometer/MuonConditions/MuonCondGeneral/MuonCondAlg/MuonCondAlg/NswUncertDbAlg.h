/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_NSWUNCERTDBALG_H
#define MUONCONDALG_NSWUNCERTDBALG_H

// Gaudi includes
#include <nlohmann/json.hpp>

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
// Muon includes
#include "MuonCondData/NswErrorCalibData.h"




class NswUncertDbAlg: public AthCondAlgorithm {

public:
    NswUncertDbAlg(const std::string& name, ISvcLocator* svc);
    virtual ~NswUncertDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext&) const override;

 
private:

    /// Load the Jitter constants from the JSON format
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 NswErrorCalibData& errorCalibData) const;
   
    /// Use an external JSON file to load the Jitter constants from
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                        "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    SG::WriteCondHandleKey<NswErrorCalibData> m_writeKey{this, "WriteKey", "NswUncertData",
                                                          "Key of the parametrized NSW uncertainties"};
    
    SG::ReadCondHandleKeyArray<CondAttrListCollection> m_readKeysDb{this, "ReadKeys", {}, 
                                                              "Key to the parametrized NSW uncertainty COOL folder"};
};


#endif
