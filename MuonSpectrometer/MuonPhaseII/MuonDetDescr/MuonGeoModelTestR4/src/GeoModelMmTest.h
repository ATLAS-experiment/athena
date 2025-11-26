/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELMmTEST_H
#define MUONGEOMODELTESTR4_GEOMODELMmTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <set>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/IdentifierBranch.h>
#include <MuonTesterTree/ThreeVectorBranch.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonTesterTree/CoordTransformBranch.h>
#include <MuonTesterTree/TwoVectorBranch.h>
namespace MuonGMR4{

class GeoModelMmTest : public AthHistogramAlgorithm{
    public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;

        ~GeoModelMmTest() = default;

        StatusCode execute() override; 
        StatusCode initialize() override;        
        StatusCode finalize() override;

        unsigned int cardinality() const override final {return 1;}

    private:
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

      /// Set of stations to be tested
      std::set<Identifier> m_testStations{};
  
      /// String should be formated like MM_<Large/Small Sector + Module type><Quadruplet number>
      //e.g. MM_SM2Q1
      Gaudi::Property<std::vector<std::string>> m_selectStat{this, "TestStations", {}};
      Gaudi::Property<std::vector<std::string>> m_excludeStat{this, "ExcludeStations", {}};
      
      const MuonDetectorManager* m_detMgr{nullptr};
     
      StatusCode dumpToTree(const EventContext& ctx,
                            const ActsTrk::GeometryContext& gctx, 
                            const MmReadoutElement* readoutEle);
     
      MuonVal::MuonTesterTree m_tree{"MmGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")}; // 55(S) or 56(L)
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")}; // [-2, 2]
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")}; // [1, 8]
    MuonVal::ScalarBranch<short>& m_stML{m_tree.newScalar<short>("multilayer")}; // {1, 2}
    MuonVal::ScalarBranch<std::string>& m_chamberDesign{m_tree.newScalar<std::string>("chamberDesign")};
    MuonVal::ScalarBranch<float>& m_stStripPitch{m_tree.newScalar<float>("stripPitch")};


    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};
    MuonVal::CoordTransformBranch m_alignableNode {m_tree, "AlignableNode"};

    /// Rotation matrix of the respective strip layers
    MuonVal::CoordSystemsBranch m_stripRot{m_tree, "stripRot"};    
    MuonVal::VectorBranch<uint8_t>& m_stripRotGasGap{m_tree.newVector<uint8_t>("stripRotGasGap")};
    MuonVal::TwoVectorBranch m_firstStripPos{m_tree, "firstStripPos"};
    MuonVal::VectorBranch<int>& m_readoutSide{m_tree.newVector<int>("stripReadoutSide")};

    MuonVal::VectorBranch<unsigned>& m_firstStrip{m_tree.newVector<unsigned>("firstStrip")};
    MuonVal::VectorBranch<unsigned>& m_nStrips{m_tree.newVector<unsigned>("nStrips")};


    //// Chamber Details
    MuonVal::VectorBranch<short>& m_gasGap{m_tree.newVector<short>("gasGap")}; // gas gap number
    MuonVal::VectorBranch<bool>& m_isStereo{m_tree.newVector<bool>("isStereo")};
    MuonVal::ThreeVectorBranch m_stripCenter{m_tree, "stripCenter"};
    MuonVal::ThreeVectorBranch m_stripLeftEdge{m_tree,"stripLeftEdge"};
    MuonVal::ThreeVectorBranch m_stripRightEdge{m_tree,"stripRightEdge"};
    MuonVal::TwoVectorBranch m_locStripCenter{m_tree, "locStripCenter"};

    MuonVal::VectorBranch<float>& m_stripLength{m_tree.newVector<float>("stripLength")};
    MuonVal::VectorBranch<uint>& m_channel{m_tree.newVector<uint>("channel")}; // strip number  

    /// Chamber Length for debug
    MuonVal::ScalarBranch<float>& m_moduleHeight{m_tree.newScalar<float>("moduleHeight")}; //active area's Height
    MuonVal::ScalarBranch<float>& m_moduleWidthS{m_tree.newScalar<float>("moduleWidthS")}; //active area's small width
    MuonVal::ScalarBranch<float>& m_moduleWidthL{m_tree.newScalar<float>("moduleWidthL")};   //active area's large width 
    /// GasGap Lengths for debug
    MuonVal::ScalarBranch<float>& m_ActiveHeightR{m_tree.newScalar<float>("ActiveHeightR")}; //active area's Height
    MuonVal::ScalarBranch<float>& m_ActiveWidthS{m_tree.newScalar<float>("ActiveWidthS")}; //active area's small width
    MuonVal::ScalarBranch<float>& m_ActiveWidthL{m_tree.newScalar<float>("ActiveWidthL")};   //active area's large width 

    
  };
}
#endif
