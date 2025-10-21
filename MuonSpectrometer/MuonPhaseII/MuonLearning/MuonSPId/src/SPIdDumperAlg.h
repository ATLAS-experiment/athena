#ifndef MUON_SP_ID_DUMPER_H
#define MUON_SP_ID_DUMPER_H
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AthenaBaseComps/AthAlgorithm.h>
#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonSpacePoint/SpacePointContainer.h>
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include "MuonInferenceInterfaces/IGraphInferenceTool.h"

namespace MuonR4 {
    
   class SPIdDumperAlg : public AthHistogramAlgorithm {
   public:
       using AthHistogramAlgorithm::AthHistogramAlgorithm;

       virtual StatusCode initialize() override;
       virtual StatusCode execute() override;
       virtual StatusCode finalize() override;

   private:
      SG::ReadHandleKey<MuonR4::SpacePointContainer>  m_readKey{this, "ReadSpacePoints", "MuonSpacePoints", "Input SpacePoints"};
      ServiceHandle<Muon::IMuonIdHelperSvc>           m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      SG::ReadHandleKey<MuonR4::SegmentContainer>     m_inSegmentKey{this, "SegmentKey", "R4MuonSegments"};

      ToolHandle<MuonML::IGraphInferenceTool>         m_graphFilterTool{this, "GraphFilterTool", "", "Graph inference tool"};

      Gaudi::Property<bool>                           m_isMC{this, "isMC", true};
      Gaudi::Property<double>                         m_SPId_cut{this,"spIdValue", -7};

      MuonVal::MuonTesterTree m_tree{"MuonSPId","MuonSPId"};
      MuonVal::VectorBranch<float>&           m_spoint_x{m_tree.newVector<float>("x")};
      MuonVal::VectorBranch<float>&           m_spoint_y{m_tree.newVector<float>("y")};
      MuonVal::VectorBranch<float>&           m_spoint_z{m_tree.newVector<float>("z")};

      MuonVal::VectorBranch<uint8_t>&         m_spoint_station{m_tree.newVector<uint8_t>("stationIndex")};
      MuonVal::VectorBranch<float>&           m_spoint_driftR{m_tree.newVector<float>("driftR")};

      MuonVal::VectorBranch<uint16_t>&        m_spoint_layer{m_tree.newVector<uint16_t>("Layer")};

      MuonVal::VectorBranch<uint8_t>&         m_spoint_label{m_tree.newVector<uint8_t>("label")};
      MuonVal::VectorBranch<float>&           m_spoint_predictions{m_tree.newVector<float>("predictions")};

   };

}  // namespace MuonR4

#endif
