/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDALG_ILINESCONDALG_H
#define MUONCONDALG_ILINESCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CoralUtilities/blobaccess.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "nlohmann/json.hpp"

namespace Muon{
class CscILinesCondAlg : public AthCondAlgorithm {

    public:
        using AthCondAlgorithm::AthCondAlgorithm;
        virtual ~CscILinesCondAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;
   
    private:
        SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/MUONALIGN/CSC/ILINES",
                                                                      "Key of input CSC/ILINES condition data"};
                                                                
        SG::WriteCondHandleKey<ALineContainer> m_writeKey{this, "WriteKey", "CscInternalAlignmentContainer",
                                                                             "Key of output muon alignment CSC/ILine condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        // new folder format 2020
        Gaudi::Property<bool> m_newFormat2020 {this, "NewFormat2020", false, 
                          "The database folders are given in the new JSON format"};
        /// @brief Load the alignment parameters from a JSON file
        Gaudi::Property<std::string> m_readFromJSON{this,"readFromJSON", "",
                                    "Reads the A & B lines parameters from a JSON file instead from COOL"};

    
        /**
         *  Load the Alignment data from the legacy format where the channels are parsed line wise 
         *  The data is then transferred into a modern JSON blob
         **/
        StatusCode loadDataFromLegacy(const std::string& data, nlohmann::json& json) const;
        /**
         *  Parse the JSON blob to fill the I Line container
         * */
        StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                    ALineContainer& writeCdo) const;
};
}
#endif