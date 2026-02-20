/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonMDT_CablingAlg reads raw condition data and writes derived condition data to the condition store
*/

#ifndef MUONMDT_CABLING_MUONMDT_CABLINGALG_H
#define MUONMDT_CABLING_MUONMDT_CABLINGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "nlohmann/json.hpp"


class MuonMDT_CablingAlg : public AthCondAlgorithm {
public:
    MuonMDT_CablingAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonMDT_CablingAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;


    using CablingData = MuonMDT_CablingMap::CablingData;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyMez{this, "MezzanineFolders", "/MDT/CABLING/MEZZANINE_SCHEMA"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyMap{this, "MapFolders", "/MDT/CABLING/MAP_SCHEMA"};
    SG::WriteCondHandleKey<MuonMDT_CablingMap> m_writeKey{this, "WriteKey", "MuonMDT_CablingMap", "Key of output MDT cabling map"};

    Gaudi::Property<bool> m_isRun3{this, "isRun3", false, "Auxillary property to load the BIS78 cabling by hand"};
    StatusCode dbPayloadToJson(SG::ReadCondHandle<CondAttrListCollection>& readHandle, nlohmann::json& json) const;
    /// Retrieves the general MDT station info from the coral attribute
    bool extractStationInfo(const coral::AttributeList& atr, CablingData& map_data) const;
    /// Retrieves the channel info from the coral attribute
    bool extractLayerInfo(std::vector<std::string>&, CablingData& map_data) const;

    /// Load the mezzanine schema into the cabling
    StatusCode loadMezzanineSchema(const EventContext& ctx,SG::WriteCondHandle<MuonMDT_CablingMap>& writeHandle,
                                   MuonMDT_CablingMap& cabling_map) const;
    
    StatusCode loadMezzanineFromJSON(nlohmann::json&& payload, MuonMDT_CablingMap& cabling_map) const;
    
    /// Load the cabling schema of the tubes

    StatusCode loadCablingSchema(const EventContext& ctx,SG::WriteCondHandle<MuonMDT_CablingMap>& writeHandle,
                                 MuonMDT_CablingMap& cabling_map) const;
    
    StatusCode loadCablingSchemaFromJSON(nlohmann::json&& payload, MuonMDT_CablingMap& cabling_map) const;
    
    Gaudi::Property<std::string> m_mezzJSON{this, "MezzanineJSON", "" , 
                                            "External JSON file to read the mezzanine mapping from"};

    Gaudi::Property<std::string> m_chambJSON{this, "CablingJSON", "", 
                                             "External JSON file to read the MDT cabling from"};

    Gaudi::Property<bool> m_useJSONFormat{this, "UseJSONFormat", false, 
                                          "Read out the cabling database JSON based"};

};

#endif
