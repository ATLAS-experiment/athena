/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELRCSCTEST_H
#define MUONGEOMODELTESTR4_GEOMODELRCSCTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <set>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/CoordTransformBranch.h"
namespace MuonGM {

class GeoModelCscTest : public AthHistogramAlgorithm {
   public:
    GeoModelCscTest(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;
    unsigned int cardinality() const override final { return 1; }

   private:
     
     StatusCode dumpToTree(const EventContext& ctx, const CscReadoutElement* readoutEle);

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

    MuonVal::MuonTesterTree m_tree{"CscGeoModelTree", "GEOMODELTESTER"};

    /// Identifier of the readout element
    MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
    MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
    MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
    MuonVal::ScalarBranch<short>& m_stMultiLayer{m_tree.newScalar<short>("stationMultiLayer")};
  
    /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
    MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "ElementTransform"};   
    
    /// Alignment parameters
    MuonVal::ScalarBranch<float>& m_ALineTransS{m_tree.newScalar<float>("ALineTransS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransT{m_tree.newScalar<float>("ALineTransT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineTransZ{m_tree.newScalar<float>("ALineTransZ", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotS{m_tree.newScalar<float>("ALineRotS", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotT{m_tree.newScalar<float>("ALineRotT", 0.)};
    MuonVal::ScalarBranch<float>& m_ALineRotZ{m_tree.newScalar<float>("ALineRotZ", 0.)};

    
    MuonVal::CoordSystemsBranch m_layerTrans{m_tree,"Layer"};
    MuonVal::VectorBranch<bool>& m_layMeasPhi{m_tree.newVector<bool>("LayerMeasPhi")};
    MuonVal::VectorBranch<uint8_t>& m_layNumber{m_tree.newVector<uint8_t>("LayerNumber")};

};

}
#endif