/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Algorithm to dump the R4-style Rpc cabling maps into a JSON file
*/

#ifndef MUONCONDDUMP_RPCCABLINGJSONDUMPALG_H
#define MUONCONDDUMP_RPCCABLINGJSONDUMPALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

/** @brief: Simple algorithm to generate toy cabling maps for the RPC detector exploiting the 
 *          NRpcCabling maps.
 * 
 */


namespace Muon {
    class RpcToyCablingJsonDumpAlg : public AthAlgorithm {
        public:
            using AthAlgorithm::AthAlgorithm;
            virtual ~RpcToyCablingJsonDumpAlg() = default;
            virtual StatusCode initialize() override;
            virtual StatusCode execute() override;
            virtual unsigned int cardinality() const override final{return 1;}

        private:
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            Gaudi::Property<std::string> m_cablingJSON{this, "OutCablingJSON", "RpcCabling.json", "Cabling JSON"};
    };
}
#endif
