/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCDIGITJITTERCONDALG_H
#define MUONCONDALG_TGCDIGITJITTERCONDALG_H

// Gaudi includes
#include <nlohmann/json.hpp>

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitJitterData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"


namespace Muon{
class TgcDigitJitterCondAlg : public AthCondAlgorithm {
public:
    using AthCondAlgorithm::AthCondAlgorithm;
    virtual ~TgcDigitJitterCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    /// Load the Jitter constants from the JSON format
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 TgcDigitJitterData& jitterChannels) const;
   
    /// Use an external JSON file to load the Jitter constants from
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };

    SG::WriteCondHandleKey<TgcDigitJitterData> m_writeKey{this, "WriteKey", "TgcJitterData", "Key of output TGC condition data"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "", "Key of input TGC condition data"};
};
}

#endif
