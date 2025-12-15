/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_MDTASBUILTCONDALG_H
#define MUONCONDALG_MDTASBUILTCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CoralUtilities/blobaccess.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "nlohmann/json.hpp"

class MdtAsBuiltCondAlg: public AthCondAlgorithm {
    public:
        MdtAsBuiltCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~MdtAsBuiltCondAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;
    
    private:
        StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                MdtAsBuiltContainer& asBuilt) const;
        
        StatusCode legacyFormatToJSON(const std::string& bloblines, 
                                        nlohmann::json& lines) const;
        
        StatusCode setFromAscii(const std::string& asciiData,
                                   nlohmann::json& newChannel) const;

        SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/MUONALIGN/MDT/ASBUILTPARAMS",
                                                                      "Key of MDT/ASBUILTPARAMS input condition data"};
                    
        SG::WriteCondHandleKey<MdtAsBuiltContainer> m_writeKey{this, "WriteKey", "MdtAsBuiltContainer",
                                                                         "Key of output muon alignment MDT/AsBuilt condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        // new folder format 2020
        Gaudi::Property<bool> m_newFormat2020{this, "NewFormat2020", false, 
                            "The database folders are given in the new JSON format"};
        /// @brief Load the alignment parameters from a JSON file
        Gaudi::Property<std::string> m_readFromJSON{this,"readFromJSON", "",
                                    "Reads the A & B lines parameters from a JSON file instead from COOL"};



};

#endif