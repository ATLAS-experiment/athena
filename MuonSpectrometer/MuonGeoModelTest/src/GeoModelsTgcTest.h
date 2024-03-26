/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELSTGCTEST_H
#define MUONGEOMODELTESTR4_GEOMODELSTGCTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/TwoVectorBranch.h"
#include "MuonTesterTree/CoordTransformBranch.h"

namespace MuonGM {

class GeoModelsTgcTest : public AthHistogramAlgorithm {
   public:
    GeoModelsTgcTest(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    unsigned int cardinality() const override final { return 1; }

   private:
     
     StatusCode dumpToTree(const EventContext& ctx, const sTgcReadoutElement* readoutEle);

    /// MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{
        this, "DetectorManagerKey", "MuonDetectorManager",
        "Key of input MuonDetectorManager condition data"};

     ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    /// Set of stations to be tested
    std::set<Identifier> m_testStations{};

    /// String should be formated like
    /// <stationName><stationEta><A/C><stationPhi>
    Gaudi::Property<std::vector<std::string>> m_selectStat{
        this, "TestStations", {}, "Constrain the stations to be tested"};

    MuonVal::MuonTesterTree m_tree{"sTgcGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<short>& m_stIndex{m_tree.newScalar<short>("stationIndex")}; // 57(S) or 58(L)
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")}; // [-3, 3]
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")}; // [1, 8]
    MuonVal::ScalarBranch<short>& m_stML{m_tree.newScalar<short>("stationMultilayer")}; // {1, 2}
    MuonVal::ScalarBranch<std::string>& m_chamberDesign{m_tree.newScalar<std::string>("chamberDesign")};
   
    //// Chamber Details

    MuonVal::ScalarBranch<short>& m_numLayers{m_tree.newScalar<short>("numLayers")}; // 4
    MuonVal::ScalarBranch<float>& m_yCutout{m_tree.newScalar<float>("yCutout")}; // yCutoutCathode
    MuonVal::ScalarBranch<float>& m_gasTck{m_tree.newScalar<float>("gasTck")}; // gasTck 2.85mm
    /// Chamber Length for debug
    MuonVal::ScalarBranch<float>& m_sChamberLength{m_tree.newScalar<float>("sChamberLength")}; 
    MuonVal::ScalarBranch<float>& m_lChamberLength{m_tree.newScalar<float>("lChamberLength")}; 
    MuonVal::ScalarBranch<float>& m_chamberHeight{m_tree.newScalar<float>("chamberHeight")}; 
    /// GasGap Lengths for debug
    MuonVal::ScalarBranch<float>& m_sGapLength{m_tree.newScalar<float>("sGapLength")}; 
    MuonVal::ScalarBranch<float>& m_lGapLength{m_tree.newScalar<float>("lGapLength")}; 
    MuonVal::ScalarBranch<float>& m_gapHeight{m_tree.newScalar<float>("gapHeight")}; 

    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};

    /// Rotation matrix of the respective strip layers
    MuonVal::CoordSystemsBranch m_stripRot{m_tree, "stripRot"};    
    MuonVal::VectorBranch<uint8_t>& m_stripRotGasGap{m_tree.newVector<uint8_t>("stripRotGasGap")};

    /// Rotation matrix of the respective wireGroup layers
    MuonVal::CoordSystemsBranch m_wireGroupRot{m_tree, "wireGroupRot"};    
    MuonVal::VectorBranch<uint8_t>& m_wireGroupRotGasGap{m_tree.newVector<uint8_t>("wireGroupRotGasGap")};

    /// Rotation matrix of the respective pad layers
    MuonVal::CoordSystemsBranch m_padRot{m_tree, "padRot"};    
    MuonVal::VectorBranch<uint8_t>& m_padRotGasGap{m_tree.newVector<uint8_t>("padRotGasGap")};

    //// Wire Dimensions
    MuonVal::VectorBranch<uint>& m_numWires{m_tree.newVector<uint>("numWires")}; // nWires 
    MuonVal::VectorBranch<uint>& m_firstWireGroupWidth{m_tree.newVector<uint>("firstWireGroupWidth")}; // firstWireGroup <= 20
    MuonVal::VectorBranch<uint>& m_numWireGroups{m_tree.newVector<uint>("numWireGroups")}; // nWireGroups >19
    MuonVal::VectorBranch<float>& m_wireCutout{m_tree.newVector<float>("wireCutout")}; // wireCutout ~ 800mm
    MuonVal::ScalarBranch<float>& m_wirePitch{m_tree.newScalar<float>("wirePitch")}; // wirePitch 1.8mm
    MuonVal::ScalarBranch<float>& m_wireWidth{m_tree.newScalar<float>("wireWidth")}; // wireWidth 0.015mm
    MuonVal::ScalarBranch<uint>& m_wireGroupWidth{m_tree.newScalar<uint>("wireGroupWidth")}; // wireGroupWidth 20
    MuonVal::TwoVectorBranch m_localWireGroupPos{m_tree, "localWireGroupPos"};
    MuonVal::ThreeVectorBranch m_globalWireGroupPos{m_tree, "globalWireGroupPos"};
    MuonVal::VectorBranch<uint8_t>& m_wireGroupNum{m_tree.newVector<uint8_t>("wireGroupNum")}; // wire Group number
    MuonVal::VectorBranch<uint8_t>& m_wireGroupGasGap{m_tree.newVector<uint8_t>("wireGroupGasGap")}; // gas gap number

    /// Strip dimensions 
    MuonVal::ScalarBranch<uint>& m_numStrips{m_tree.newScalar<uint>("numStrips")}; // nStrips
    MuonVal::ScalarBranch<float>& m_stripPitch{m_tree.newScalar<float>("stripPitch")}; // stripPitch 3.2mm
    MuonVal::ScalarBranch<float>& m_stripWidth{m_tree.newScalar<float>("stripWidth")}; // stripWidth 2.7mm
    MuonVal::VectorBranch<float>& m_firstStripPitch{m_tree.newVector<float>("firstStripPitch")}; // firstStripWidth 1.6/3.2mm
    MuonVal::TwoVectorBranch m_localStripPos{m_tree, "localStripPos"};
    MuonVal::ThreeVectorBranch m_globalStripPos{m_tree, "globalStripPos"};
    MuonVal::VectorBranch<uint>& m_stripNum{m_tree.newVector<uint>("stripNumber")}; // strip number
    MuonVal::VectorBranch<uint8_t>& m_stripGasGap{m_tree.newVector<uint8_t>("stripGasGap")}; // gas gap number
    MuonVal::VectorBranch<float>& m_stripLengths{m_tree.newVector<float>("stripLengths")}; // Length of each strip

   /// Pad dimensions 
    MuonVal::VectorBranch<uint>& m_numPads{m_tree.newVector<uint>("numPads")}; //total number of pads in a layer
    MuonVal::ScalarBranch<float>& m_sPadLength{m_tree.newScalar<float>("sPadLength")}; // sPadWidth 
    MuonVal::ScalarBranch<float>& m_lPadLength{m_tree.newScalar<float>("lPadLength")}; // lPadWidth 
    MuonVal::VectorBranch<uint>& m_numPadEta{m_tree.newVector<uint>("numPadEta")}; //nPadH
    MuonVal::VectorBranch<uint>& m_numPadPhi{m_tree.newVector<uint>("numPadPhi")}; //nPadPhi
    MuonVal::VectorBranch<float>& m_firstPadHeight{m_tree.newVector<float>("firstPadHeight")}; //firstPadH
    MuonVal::VectorBranch<float>& m_padHeight{m_tree.newVector<float>("padHeight")}; //PadH
    MuonVal::VectorBranch<float>& m_padPhiShift{m_tree.newVector<float>("padPhiShift")}; //PadPhiShift_A
    MuonVal::VectorBranch<float>& m_firstPadPhiDiv{m_tree.newVector<float>("firstPadPhiDiv")}; //firstPadPhiDivision_A
    MuonVal::ScalarBranch<float>& m_anglePadPhi{m_tree.newScalar<float>("anglePadPhi")}; // anglePadPhi
    MuonVal::ScalarBranch<float>& m_beamlineRadius{m_tree.newScalar<float>("beamlineRadius")}; 

    MuonVal::TwoVectorBranch m_localPadPos{m_tree, "localPadPos"};
    MuonVal::TwoVectorBranch m_localPadCornerBR{m_tree, "localPadCornerBR"};
    MuonVal::TwoVectorBranch m_localPadCornerBL{m_tree, "localPadCornerBL"};
    MuonVal::TwoVectorBranch m_localPadCornerTR{m_tree, "localPadCornerTR"};
    MuonVal::TwoVectorBranch m_localPadCornerTL{m_tree, "localPadCornerTL"};

    MuonVal::ThreeVectorBranch m_globalPadPos{m_tree, "globalPadPos"};
    MuonVal::ThreeVectorBranch m_globalPadCornerBR{m_tree, "globalPadCornerBR"};
    MuonVal::ThreeVectorBranch m_globalPadCornerBL{m_tree, "globalPadCornerBL"};
    MuonVal::ThreeVectorBranch m_globalPadCornerTR{m_tree, "globalPadCornerTR"};
    MuonVal::ThreeVectorBranch m_globalPadCornerTL{m_tree, "globalPadCornerTL"};
    MuonVal::VectorBranch<uint8_t>& m_padGasGap{m_tree.newVector<uint8_t>("padGasGap")}; // gas gap number
    MuonVal::VectorBranch<uint>& m_padEta{m_tree.newVector<uint>("padEtaNumber")}; // pad number in eta direction
    MuonVal::VectorBranch<uint>& m_padPhi{m_tree.newVector<uint>("padPhiNumber")}; // pad number in phi direction
    
};

}
#endif