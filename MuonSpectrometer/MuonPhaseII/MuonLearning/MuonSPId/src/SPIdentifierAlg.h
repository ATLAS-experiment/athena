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
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "CLHEP/Random/RandomEngine.h"

// onnx runtime
#include <onnxruntime_cxx_api.h>


namespace MuonR4{
class SPIdentifierAlg: public AthHistogramAlgorithm {

   public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    ~SPIdentifierAlg() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode execute() override final;

   private:

    void fillChamberInfo(const MuonGMR4::Chamber* chamber); 

    SG::ReadHandleKey<SpacePointContainer> m_readKey{this, "ReadKey", "MuonSpacePoints", "Key to the space point container"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadHandleKey<MuonR4::SegmentContainer> m_inSegmentKey{this, "SegmentKey", "R4MuonSegments"};

    //SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    
    Gaudi::Property<bool> m_isMC{this, "isMC", true};
    //Gaudi::Property<double> m_fracToKeep{this,"dataFracToKeep", 1}; // 0.055 to balanced dataset without MC
    Gaudi::Property<std::string> m_streamName{this, "StreamName", ""};
    ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};
    CLHEP::HepRandomEngine* getRandomEngine(const EventContext&ctx) const;

    MuonVal::MuonTesterTree m_tree{"MuonSPId","MuonSPId"};

    MuonVal::VectorBranch<float>&           m_bucket_density{m_tree.newVector<float>("bucket_density", 0)};
    MuonVal::VectorBranch<uint8_t>&         m_bucket_layers{m_tree.newVector<uint8_t>("bucket_layers", 0)}; 

    MuonVal::VectorBranch<uint8_t>&         m_spoint_bucket{m_tree.newVector<uint8_t>("bucket_index")};

    //MuonVal::ThreeVectorBranch              m_spoint_localPosition{m_tree, "localPosition"}; 
    MuonVal::VectorBranch<float>&           m_spoint_x{m_tree.newVector<float>("x")};
    MuonVal::VectorBranch<float>&           m_spoint_y{m_tree.newVector<float>("y")};
    MuonVal::VectorBranch<float>&           m_spoint_z{m_tree.newVector<float>("z")};

    //MuonVal::MuonIdentifierBranch           m_spoint_id{m_tree, "id"};
    MuonVal::VectorBranch<uint8_t>&         m_spoint_station{m_tree.newVector<uint8_t>("stationIndex")};
    MuonVal::VectorBranch<uint8_t>&         m_spoint_layer{m_tree.newVector<uint8_t>("layer")};

    MuonVal::VectorBranch<float>&           m_spoint_driftR{m_tree.newVector<float>("driftR")};
    MuonVal::VectorBranch<float>&           m_spoint_neighbors{m_tree.newVector<float>("neighbors")};

    MuonVal::VectorBranch<uint8_t>&         m_spoint_label{m_tree.newVector<uint8_t>("label")};
    MuonVal::VectorBranch<uint8_t>&         m_spoint_predictions{m_tree.newVector<uint8_t>("predictions")};
    MuonVal::VectorBranch<uint16_t>&        m_spoint_edges{m_tree.newVector<uint16_t>("edges")};

    size_t m_event{0};

};
}
#endif

