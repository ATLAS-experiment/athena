/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTEST_GEOMODELMMTEST_H
#define MUONGEOMODELTEST_GEOMODELMMTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/TwoVectorBranch.h"
#include "MuonTesterTree/CoordTransformBranch.h"

namespace MuonGM {

class GeoModelMmTest : public AthHistogramAlgorithm {
   public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    
    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

   private:
     
    /// MuonDetectorManager from the conditions store
    // ReadCondHandleKey is a class template is used for handling and managing conditions.
    //this declaration is creating an instance of ReadCondHandleKey specialized for MuonGM::MuonDetectorManager 
    //and associating it with "DetectorManagerKey" in the current class. 
    //The property "DetectorManagerKey"  will be used to access condition data related to the MuonDetectorManager.
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};

    // handling (like data access) for the service of related to handling muon identifiers (like MicroMegas)
     ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    /// Set of stations to be tested
    std::set<Identifier> m_testStations{};

    /// String should be formated like
    /// MM<L or S><1 or 2><A/C><layer>
    /// Example string MML1A6 , MMS2C5
    Gaudi::Property<std::vector<std::string>> m_selectStat{
        this, "TestStations", {}, "Constrain the stations to be tested"};
    
    Gaudi::Property<std::vector<std::string>> m_excludeStat{this, "ExcludeStations", {}};
    StatusCode dumpToTree(const EventContext& ctx, const MuonGM::MMReadoutElement* detEl);

    MuonVal::MuonTesterTree m_tree{"MmGeoModelTree", "GEOMODELTESTER"};

    MuonVal::ScalarBranch<unsigned short>& m_stationIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stationEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stationPhi{m_tree.newScalar<short>("stationPhi")};
    MuonVal::ScalarBranch<int>& m_stationName{m_tree.newScalar<int>("stationName")};
    MuonVal::ScalarBranch<short>& m_multilayer{m_tree.newScalar<short>("multilayer")};

    MuonVal::ScalarBranch<float>& m_stStripPitch{m_tree.newScalar<float>("stripPitch")};

    
    MuonVal::VectorBranch<bool>& m_isStereo{m_tree.newVector<bool>("isStereo")};
    MuonVal::VectorBranch<short>& m_gasGap{m_tree.newVector<short>("gasGap")};
    MuonVal::VectorBranch<uint>& m_channel{m_tree.newVector<uint>("channel")};
    
    MuonVal::VectorBranch<float>& m_stripLength{m_tree.newVector<float>("stripLength")};
    MuonVal::VectorBranch<float>& m_stripActiveLength{m_tree.newVector<float>("stripActiveLength")};
    MuonVal::VectorBranch<float>& m_stripActiveLengthLeft{m_tree.newVector<float>("stripActiveLengthLeft")};
    MuonVal::VectorBranch<float>& m_stripActiveLengthRight{m_tree.newVector<float>("stripActiveLengthRight")};
    
    MuonVal::ThreeVectorBranch m_stripCenter{m_tree, "stripCenter"};
    MuonVal::ThreeVectorBranch m_stripLeftEdge{m_tree, "stripLeftEdge"};
    MuonVal::ThreeVectorBranch m_stripRightEdge{m_tree, "stripRightEdge"};
    MuonVal::TwoVectorBranch m_locStripCenter{m_tree, "locStripCenter"};

    MuonVal::ScalarBranch<float>& m_ActiveHeightR{m_tree.newScalar<float>("ActiveHeightR")}; //active area's Height
    MuonVal::ScalarBranch<float>& m_ActiveWidthS{m_tree.newScalar<float>("ActiveWidthS")}; //active area's small width
    MuonVal::ScalarBranch<float>& m_ActiveWidthL{m_tree.newScalar<float>("ActiveWidthL")};   //active area's large width 


    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};
    MuonVal::CoordTransformBranch m_alignableNode {m_tree, "AlignableNode"};


    /// Rotation matrix of the respective strip layers
    MuonVal::CoordSystemsBranch m_stripRot{m_tree, "stripRot"};    
    MuonVal::VectorBranch<uint8_t>& m_stripRotGasGap{m_tree.newVector<uint8_t>("stripRotGasGap")};
    MuonVal::TwoVectorBranch m_firstStripPos{m_tree, "firstStripPos"};
    MuonVal::VectorBranch<int>& m_readoutSide{m_tree.newVector<int>("stripReadoutSide")};
    MuonVal::VectorBranch<unsigned>& m_readoutFirstStrip{m_tree.newVector<unsigned int>("stripFirstStrip")};
    
};

}
#endif