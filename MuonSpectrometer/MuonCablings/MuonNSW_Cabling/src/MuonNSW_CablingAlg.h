/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonNSW_CablingAlg reads raw condition data and writes derived condition data
   to the condition store
*/

#ifndef MuonNSW_CABLING_MuonNSW_CABLINGALG_H
#define MuonNSW_CABLING_MuonNSW_CABLINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCablingData/Nsw_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteCondHandleKey.h"


class MuonNSW_CablingAlg : public AthReentrantAlgorithm {
   public:
    MuonNSW_CablingAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonNSW_CablingAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

    virtual bool isReEntrant() const override final { return false; }

   private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKeyArray<CondAttrListCollection> m_readCablingKeys{this, "CablingFolder", { "/MDT/MM/CABLING"}, 
                                                                        "Keys of the input conditions folder for MM & sTGC cabling map"};
    SG::WriteCondHandleKey<Nsw_CablingMap> m_writeKey{this, "WriteKey", "NswCabling", "Key of output NSW cabling map"};

    Gaudi::Property<std::string> m_JSONFile{this, "JSONFile", "", "External path to read the cabling from"};

    StatusCode loadCablingSchema(const std::string& payload,
                                 Nsw_CablingMap& cabling_map) const;
};

#endif
