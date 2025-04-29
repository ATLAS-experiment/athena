/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELTESTR4_GEOMODELMDTTEST_H
#define MUONGEOMODELTESTR4_GEOMODELMDTTEST_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <StoreGate/ReadHandleKey.h>

#include <set>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/IdentifierBranch.h>
#include <MuonTesterTree/ThreeVectorBranch.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonTesterTree/CoordTransformBranch.h>
namespace MuonGMR4{

class GeoModelMdtTest : public AthHistogramAlgorithm{
    public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;
       
        ~GeoModelMdtTest() = default;

        StatusCode execute() override;
        StatusCode initialize() override;
        StatusCode finalize() override;

        unsigned int cardinality() const override final { return 1; }

    private:
      void dumpReadoutSideXML() const;
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", 
                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
      /// Set of stations to be tested
      std::set<Identifier> m_testStations{};
  
      /// String should be formated like <stationName><stationEta><A/C><stationPhi>
      Gaudi::Property<std::vector<std::string>> m_selectStat{this, "TestStations", {}};
      Gaudi::Property<std::vector<std::string>> m_excludeStat{this, "ExcludeStations", {}};
      
      const MuonDetectorManager* m_detMgr{nullptr};

      Gaudi::Property<std::string> m_swapRead{this, "ReadoutSideXML", ""};

      StatusCode dumpToTree(const EventContext& ctx,
                            const ActsGeometryContext& gctx, const MdtReadoutElement* readoutEle);
     
      MuonVal::MuonTesterTree m_tree{"MdtGeoModelTree", "GEOMODELTESTER"};

      /// Identifier of the readout element
      MuonVal::ScalarBranch<unsigned short>& m_stIndex{m_tree.newScalar<unsigned short>("stationIndex")};
      MuonVal::ScalarBranch<short>& m_stEta{m_tree.newScalar<short>("stationEta")};
      MuonVal::ScalarBranch<short>& m_stPhi{m_tree.newScalar<short>("stationPhi")};
      MuonVal::ScalarBranch<short>& m_stML{m_tree.newScalar<short>("stationMultiLayer")};
      MuonVal::ScalarBranch<std::string>& m_chamberDesign{m_tree.newScalar<std::string>("chamberDesign")};

      MuonVal::ScalarBranch<double>& m_tubeRad{m_tree.newScalar<double>("tubeRadius")};
      MuonVal::ScalarBranch<double>& m_tubePitch{m_tree.newScalar<double>("tubePitch")};


      /// Transformation of the readout element (Translation, ColX, ColY, ColZ)
      MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};  
      MuonVal::CoordTransformBranch m_alignableNode {m_tree, "AlignableNode"};
      /// Number of tubes per layer
      MuonVal::ScalarBranch<unsigned short>& m_numTubes{m_tree.newScalar<unsigned short>("numTubes")};
      /// Number of tubes per layer
      MuonVal::ScalarBranch<unsigned short>& m_numLayers{m_tree.newScalar<unsigned short>("numLayers")};

      /// Readout each tube specifically
      MuonVal::VectorBranch<unsigned short>& m_tubeLay{m_tree.newVector<unsigned short>("tubeLayer")};
      MuonVal::VectorBranch<unsigned short>& m_tubeNum{m_tree.newVector<unsigned short>("tubeNumber")};

      /// Transformation to each tube
      MuonVal::CoordSystemsBranch m_tubeTransform{m_tree, "tubeTransform"};     

      MuonVal::VectorBranch<double>& m_tubeLength{m_tree.newVector<double>("tubeLength")};
      MuonVal::VectorBranch<double>& m_activeTubeLength{m_tree.newVector<double>("activeTubeLength")};
      MuonVal::VectorBranch<double>& m_wireLength{m_tree.newVector<double>("wireLength")};

      /// Position of the readout
      MuonVal::ThreeVectorBranch m_roPos{m_tree, "readOutPos"};
      /// Position of the tube in the sector frame
      MuonVal::ThreeVectorBranch m_tubePosInCh{m_tree, "chamberTubePos"};
};

}
#endif
