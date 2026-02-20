/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_NSWPASSIVATIONDBALG_H
#define MUONCONDALG_NSWPASSIVATIONDBALG_H


// Athena includes
#include <AthenaBaseComps/AthCondAlgorithm.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/WriteCondHandleKey.h>

// Muon includes
#include <AthenaPoolUtilities/CondAttrListCollection.h>
#include <MuonAlignmentData/NswPassivationDbData.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <nlohmann/json.hpp>

// Forward declarations
class CondAttrListCollection;

namespace Muon{
class NswPassivationDbAlg: public AthCondAlgorithm{

public:

    using AthCondAlgorithm::AthCondAlgorithm;
    virtual ~NswPassivationDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext&) const override;

 
private:
    StatusCode parseData(const nlohmann::json & json,
                         NswPassivationDbData& writeCdo) const;

    using writeKey_t = SG::WriteCondHandleKey<NswPassivationDbData>;
    using readKey_t = SG::ReadCondHandleKey<CondAttrListCollection>;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
 
    writeKey_t m_writeKey{this, "WriteKey", "NswPassivationDbData", "Key of output passivation data" };

    readKey_t m_readKey_data_mm{this, "ReadKey_MM", "/MDT/MM/PASSIVATION", "Key of input MM condition data"};

    Gaudi::Property<std::string> m_readFromJSON{this,"readFromJSON", "",
                                 "Reads the passivation parameters from a JSON file instead of cool"};
 
};
}

#endif
