/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCSVDUMP_MuonStripCsvDumperAlg_H
#define MUONCSVDUMP_MuonStripCsvDumperAlg_H

#include <AthenaBaseComps/AthAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonSpacePoint/SpacePointContainer.h>


/** The MuonStripCsvDumperAlg reads the RpcStripContainer and dumps information to csv files
 *  The files are used for the algorithm development in acts **/

namespace MuonR4{
class SpacePointCsvDumperAlg: public AthAlgorithm {

   public:

     using AthAlgorithm::AthAlgorithm;
    ~SpacePointCsvDumperAlg() = default;

     StatusCode initialize() override;
     StatusCode execute() override;

   private:

    
    SG::ReadHandleKeyArray<SpacePointContainer> m_readKeys{this, "SpacePointKeys", {"MuonSpacePoints"}, "Key to the space point container"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    size_t m_event{0};

};
}
#endif