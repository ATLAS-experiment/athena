/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCCONDDBALG_H
#define MUONCONDALG_TGCCONDDBALG_H

// STL includes
#include <zlib.h>


// Gaudi includes
#include <nlohmann/json.hpp>

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcCondDbData.h"
#include "CxxUtils/StringUtils.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"



class TgcCondDbAlg : public AthCondAlgorithm {
public:
    TgcCondDbAlg(const std::string& name, ISvcLocator* svc);
    virtual ~TgcCondDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    /// Load the detector status from cool
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 TgcCondDbData& deadChannels) const;
   
    /// Load the detector status from a JSON file
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::WriteCondHandleKey<TgcCondDbData> m_writeKey{this, "WriteKey", "TgcCondDbData", "Key of output TGC condition data"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "", "Key of input TGC condition data"};
};

#endif
