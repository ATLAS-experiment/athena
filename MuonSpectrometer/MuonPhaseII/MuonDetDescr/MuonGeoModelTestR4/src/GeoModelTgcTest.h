/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELTgcTEST_H
#define MUONGEOMODELTESTR4_GEOMODELTgcTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <set>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/IdentifierBranch.h>
#include <MuonTesterTree/ThreeVectorBranch.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonTesterTree/CoordTransformBranch.h>
#include <MuonTesterTree/TwoVectorBranch.h>
namespace MuonGMR4{

class GeoModelTgcTest : public AthHistogramAlgorithm{
    public:
        GeoModelTgcTest(const std::string& name, ISvcLocator* pSvcLocator);

        ~GeoModelTgcTest() = default;

        StatusCode execute() override; 
        StatusCode initialize() override;        
        StatusCode finalize() override;

        unsigned int cardinality() const override final {return 1;}

    private:
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      SG::ReadCondHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

      /// Set of stations to be tested
      std::set<Identifier> m_testStations{};
  
      /// String should be formated like <stationName><stationEta><A/C><stationPhi>
      Gaudi::Property<std::vector<std::string>> m_selectStat{this, "TestStations", {}};
      
      const MuonDetectorManager* m_detMgr{nullptr};
     
      StatusCode dumpToTree(const EventContext& ctx,
                            const ActsGeometryContext& gctx, 
                            const TgcReadoutElement* readoutEle);
     
      MuonVal::MuonTesterTree m_tree{"TgcGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
    MuonVal::ScalarBranch<std::string>& m_stLayout{m_tree.newScalar<std::string>("stationDesign")};
    MuonVal::ScalarBranch<uint8_t>& m_nGasGaps{m_tree.newScalar<uint8_t>("nGasGaps")};
  
    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};
    MuonVal::ScalarBranch<float>& m_shortWidth{m_tree.newScalar<float>("ChamberWidthS")};
    MuonVal::ScalarBranch<float>& m_longWidth{m_tree.newScalar<float>("ChamberWidthL")};
    MuonVal::ScalarBranch<float>& m_height{m_tree.newScalar<float>("ChamberHeight")};
    MuonVal::ScalarBranch<float>& m_thickness{m_tree.newScalar<float>("ChamberThickness")};

    /// Alignment parameters
    MuonVal::ScalarBranch<float>& m_ALineTransS{m_tree.newScalar<float>("ALineTransS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransT{m_tree.newScalar<float>("ALineTransT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransZ{m_tree.newScalar<float>("ALineTransZ", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotS{m_tree.newScalar<float>("ALineRotS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotT{m_tree.newScalar<float>("ALineRotT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotZ{m_tree.newScalar<float>("ALineRotZ", 0.)};

    
    MuonVal::ThreeVectorBranch m_stripCenter{m_tree,"stripCenter"};
    MuonVal::ThreeVectorBranch m_stripBottom{m_tree,"stripBottom"};
    MuonVal::ThreeVectorBranch m_stripTop{m_tree,"stripTop"};

    MuonVal::TwoVectorBranch m_locStripCenter{m_tree, "stripLocalCenter"};
    MuonVal::TwoVectorBranch m_locStripBottom{m_tree, "stripLocalBottom"};
    MuonVal::TwoVectorBranch m_locStripTop{m_tree, "stripLocalTop"};
    
    
    MuonVal::VectorBranch<uint8_t>& m_stripGasGap{m_tree.newVector<uint8_t>("stripGasGap")};
    MuonVal::VectorBranch<unsigned int>& m_stripNum{m_tree.newVector<unsigned int>("stripNumber")};
    MuonVal::VectorBranch<float>& m_stripShortWidth{m_tree.newVector<float>("stripShortWidth")};
    MuonVal::VectorBranch<float>& m_stripLongWidth{m_tree.newVector<float>("stripLongWidth")};
    MuonVal::VectorBranch<float>& m_stripPitch{m_tree.newVector<float>("stripPitch")};
    MuonVal::VectorBranch<float>& m_stripLength{m_tree.newVector<float>("stripLength")};


    /// Wire gangs
    MuonVal::ThreeVectorBranch m_gangCenter{m_tree, "gangCenter"};
    MuonVal::TwoVectorBranch m_locGangPos{m_tree, "gangLocalPos"};
    MuonVal::VectorBranch<uint8_t>& m_gangGasGap{m_tree.newVector<uint8_t>("gangGasGap")};
    MuonVal::VectorBranch<unsigned int>& m_gangNum{m_tree.newVector<unsigned int>("gangNumber")};
    MuonVal::VectorBranch<uint8_t>& m_gangNumWires{m_tree.newVector<uint8_t>("gangNumWires")};
    MuonVal::VectorBranch<float>& m_gangLength{m_tree.newVector<float>("gangLength")};
    
    /// Layer dimensions
    MuonVal::CoordSystemsBranch m_layTans{m_tree, "layer"};   
    MuonVal::VectorBranch<bool>& m_layMeasPhi{m_tree.newVector<bool>("layerMeasPhi")};
    MuonVal::VectorBranch<uint8_t>& m_layNumber{m_tree.newVector<uint8_t>("layerNumber")};
    MuonVal::VectorBranch<float>& m_layShortWidth{m_tree.newVector<float>("layerWidthS")};
    MuonVal::VectorBranch<float>& m_layLongWidth{m_tree.newVector<float>("layerWidthL")};
    MuonVal::VectorBranch<float>& m_layHeight{m_tree.newVector<float>("layerHeight")};
    MuonVal::VectorBranch<uint16_t>& m_layNumWires{m_tree.newVector<uint16_t>("layerNumWires")};
};
}
#endif
