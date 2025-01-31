/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELRPCTEST_H
#define MUONGEOMODELTESTR4_GEOMODELRPCTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <set>
#include <StoreGate/ReadHandleKey.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/IdentifierBranch.h>
#include <MuonTesterTree/ThreeVectorBranch.h>
#include <MuonTesterTree/TwoVectorBranch.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonTesterTree/CoordTransformBranch.h>

namespace MuonGMR4{

class GeoModelRpcTest : public AthHistogramAlgorithm{
    public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;

        ~GeoModelRpcTest() = default;

        StatusCode execute() override;        
        StatusCode initialize() override;        
        StatusCode finalize() override;

        unsigned int cardinality() const override final { return 1; }

    private:
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

      /// Set of stations to be tested
      std::set<Identifier> m_testStations{};
  
      /// String should be formated like <stationName><stationEta><A/C><stationPhi>
      Gaudi::Property<std::vector<std::string>> m_selectStat{this, "TestStations", {}};
      Gaudi::Property<std::vector<std::string>> m_excludeStat{this, "ExcludeStations", {}};
      
      const MuonDetectorManager* m_detMgr{nullptr};
     
      StatusCode dumpToTree(const EventContext& ctx,
                            const ActsGeometryContext& gctx, const RpcReadoutElement* readoutEle);
     
      MuonVal::MuonTesterTree m_tree{"RpcGeoModelTree", "GEOMODELTESTER"};

      /// Identifier of the readout element
      MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
      MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
      MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
      MuonVal::ScalarBranch<uint8_t>& m_doubletR{m_tree.newScalar<uint8_t>("stationDoubletR")};
      MuonVal::ScalarBranch<uint8_t>& m_doubletZ{m_tree.newScalar<uint8_t>("stationDoubletZ")};
      MuonVal::ScalarBranch<uint8_t>& m_doubletPhi{m_tree.newScalar<uint8_t>("stationDoubletPhi")};
      MuonVal::ScalarBranch<std::string>& m_chamberDesign{m_tree.newScalar<std::string>("chamberDesign")};
      /// Number of strips, strip pitch in eta & phi direction
      MuonVal::ScalarBranch<uint8_t>& m_numStripsEta{m_tree.newScalar<uint8_t>("numEtaStrips")};
      MuonVal::ScalarBranch<uint8_t>& m_numStripsPhi{m_tree.newScalar<uint8_t>("numPhiStrips")};
      
      /// Strip dimensions 
      MuonVal::ScalarBranch<float>& m_stripEtaPitch{m_tree.newScalar<float>("stripEtaPitch")};
      MuonVal::ScalarBranch<float>& m_stripPhiPitch{m_tree.newScalar<float>("stripPhiPitch")};
      MuonVal::ScalarBranch<float>& m_stripEtaWidth{m_tree.newScalar<float>("stripEtaWidth")};
      MuonVal::ScalarBranch<float>& m_stripPhiWidth{m_tree.newScalar<float>("stripPhiWidth")};
      MuonVal::ScalarBranch<float>& m_stripEtaLength{m_tree.newScalar<float>("stripEtaLength")};
      MuonVal::ScalarBranch<float>& m_stripPhiLength{m_tree.newScalar<float>("stripPhiLength")};    
      /// Number of eta & phi gas gaps
      
      MuonVal::ScalarBranch<uint8_t>& m_numRpcLayers{m_tree.newScalar<uint8_t>("numRpcLayers")};      
      MuonVal::ScalarBranch<uint8_t>& m_numGasGapsPhi{m_tree.newScalar<uint8_t>("numPhiGasGaps")};
      MuonVal::ScalarBranch<uint8_t>& m_numPhiPanels{m_tree.newScalar<uint8_t>("numPhiPanels")};
      /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
      MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};
      MuonVal::CoordTransformBranch m_alignableNode {m_tree, "AlignableNode"};

      
      /// Alignment parameters
      MuonVal::ScalarBranch<float>& m_ALineTransS{m_tree.newScalar<float>("ALineTransS", 0.)};
      MuonVal::ScalarBranch<float>& m_ALineTransT{m_tree.newScalar<float>("ALineTransT", 0.)};
      MuonVal::ScalarBranch<float>& m_ALineTransZ{m_tree.newScalar<float>("ALineTransZ", 0.)};
      MuonVal::ScalarBranch<float>& m_ALineRotS{m_tree.newScalar<float>("ALineRotS", 0.)};
      MuonVal::ScalarBranch<float>& m_ALineRotT{m_tree.newScalar<float>("ALineRotT", 0.)};
      MuonVal::ScalarBranch<float>& m_ALineRotZ{m_tree.newScalar<float>("ALineRotZ", 0.)};
      
      /// Rotation matrix of the respective layers
      MuonVal::CoordSystemsBranch m_stripRot{m_tree, "stripRot"};      
      MuonVal::VectorBranch<uint8_t>& m_stripRotGasGap{m_tree.newVector<uint8_t>("stripRotGasGap")};
      MuonVal::VectorBranch<uint8_t>& m_stripRotDblPhi{m_tree.newVector<uint8_t>("stripRotDoubletPhi")};
      MuonVal::VectorBranch<bool>& m_stripRotMeasPhi{m_tree.newVector<bool>("stripRotMeasPhi")};
      
      /// Strip positions
      MuonVal::ThreeVectorBranch m_stripPos{m_tree, "stripPos"};
      MuonVal::TwoVectorBranch m_locStripPos{m_tree, "stripLocPos"};
      MuonVal::VectorBranch<bool>& m_stripPosMeasPhi{m_tree.newVector<bool>("stripPosMeasPhi")};
      MuonVal::VectorBranch<uint8_t>& m_stripPosGasGap{m_tree.newVector<uint8_t>("stripPosGasGap")};
      MuonVal::VectorBranch<uint8_t>& m_stripPosNum{m_tree.newVector<uint8_t>("stripPosNum")};
      MuonVal::VectorBranch<uint8_t>& m_stripDblPhi{m_tree.newVector<uint8_t>("stripPosDoubletPhi")};
  
};
}
#endif
