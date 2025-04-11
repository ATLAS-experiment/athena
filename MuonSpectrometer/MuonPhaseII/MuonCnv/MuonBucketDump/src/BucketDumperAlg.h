/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCSVDUMP_BucketDumperAlg_H
#define MUONCSVDUMP_BucketDumperAlg_H


#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include "StoreGate/ReadHandleKeyArray.h"

#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonSpacePoint/SpacePointContainer.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "CLHEP/Random/RandomEngine.h"


namespace MuonR4{
class BucketDumperAlg: public AthHistogramAlgorithm {

   public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    ~BucketDumperAlg() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode execute() override final;

   private:
      /** @brief Dumps the space point container with the associated muon segment container
       *  @param spacePointKey: Key to the space point key (Legacy /Nsw)
       *  @param segmentKey: Key to the fitted segments (Legacy / Nsw) */
      StatusCode dumpContainer(const EventContext& ctx,
                               const SG::ReadHandleKey<SpacePointContainer>& spacePointKey,
                               const SG::ReadHandleKey<SegmentContainer>& segmentKey);

    SG::ReadHandleKeyArray<SpacePointContainer> m_spacePointKeys{this, "SpacePointKeys", {"MuonSpacePoints"}, 
                                                     "Key to the space point container"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadHandleKeyArray<MuonR4::SegmentContainer> m_inSegmentKeys{this, "SegmentKey", {"R4MuonSegments"}};

    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    
    Gaudi::Property<bool> m_isMC{this, "isMC", true};
    Gaudi::Property<double> m_fracToKeep{this,"dataFracToKeep", 1.}; // 0.055 to balanced dataset without MC
    Gaudi::Property<std::string> m_streamName{this, "StreamName", ""};
    ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};

    /// Pattern visualization tool
    ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};
    CLHEP::HepRandomEngine* getRandomEngine(const EventContext&ctx) const;

    MuonVal::MuonTesterTree m_tree{"MuonBucketDump","MuonBucketDump"};

    MuonVal::ScalarBranch<float>&           m_bucket_min{m_tree.newScalar<float>("bucket_min", -1)};
    MuonVal::ScalarBranch<float>&           m_bucket_max{m_tree.newScalar<float>("bucket_max", -1)};
    MuonVal::ScalarBranch<uint16_t>&        m_bucket_spacePoints{m_tree.newScalar<uint16_t>("bucket_spacePoints", 0)};
    MuonVal::ScalarBranch<uint16_t>&        m_bucket_segments{m_tree.newScalar<uint16_t>("bucket_segments", 0)};
    MuonVal::ScalarBranch<uint16_t>&        m_bucket_layers{m_tree.newScalar<uint16_t>("bucket_layers", 0)}; 

    MuonVal::ThreeVectorBranch              m_spoint_localPosition{m_tree, "localPosition"}; 
    MuonVal::ThreeVectorBranch              m_spoint_globalPosition{m_tree, "globalPosition"}; 

    MuonVal::MuonIdentifierBranch           m_spoint_id{m_tree, "id"};
    MuonVal::VectorBranch<uint16_t>&        m_spoint_layer{m_tree.newVector<uint16_t>("Layer")};
    MuonVal::VectorBranch<unsigned short>&            m_spoint_isStrip{m_tree.newVector<unsigned short>("isStrip", false)};
    MuonVal::VectorBranch<unsigned short>&            m_spoint_isMdt{m_tree.newVector<unsigned short>("isMdt", false)};

    MuonVal::VectorBranch<uint16_t>&        m_spoint_adc{m_tree.newVector<uint16_t>("adc")};
    MuonVal::VectorBranch<uint16_t>&        m_spoint_tdc{m_tree.newVector<uint16_t>("tdc")};

    MuonVal::VectorBranch<float>&           m_spoint_covX{m_tree.newVector<float>("covX")};
    MuonVal::VectorBranch<float>&           m_spoint_covXY{m_tree.newVector<float>("covXY")};
    MuonVal::VectorBranch<float>&           m_spoint_covYX{m_tree.newVector<float>("covYX")};
    MuonVal::VectorBranch<float>&           m_spoint_covY{m_tree.newVector<float>("covY")};
    MuonVal::VectorBranch<float>&           m_spoint_driftR{m_tree.newVector<float>("driftR")};

    MuonVal::VectorBranch<unsigned short>&  m_spoint_measuresEta{m_tree.newVector<unsigned short>("measuresEta")};
    MuonVal::VectorBranch<unsigned short>&  m_spoint_measuresPhi{m_tree.newVector<unsigned short>("measuresPhi")};
    MuonVal::VectorBranch<unsigned short>&  m_spoint_trueLabel{m_tree.newVector<unsigned short>("trueLabel")};
    MuonVal::VectorBranch<unsigned int>&    m_spoint_nEtaInstances{m_tree.newVector<unsigned int>("nEtaInUse")};
    MuonVal::VectorBranch<unsigned int>&    m_spoint_nPhiInstances{m_tree.newVector<unsigned int>("nPhiInUse")};
    MuonVal::VectorBranch<unsigned int>&    m_spoint_dimension{m_tree.newVector<unsigned int>("dimension")};

    MuonVal::VectorBranch<uint16_t>&        m_spoint_nSegments{m_tree.newVector<uint16_t>("nSegments")};
    MuonVal::MatrixBranch<int16_t>&         m_spoint_mat{m_tree.newMatrix<int16_t>("sp_seg_matching",-1)};
    MuonVal::ThreeVectorBranch              m_segmentPos{m_tree, "segmentPosition"}; 
    MuonVal::ThreeVectorBranch              m_segmentDir{m_tree, "segmentDirection"};
    MuonVal::VectorBranch<float>&           m_segment_chiSquared{m_tree.newVector<float>("segment_chiSquared")};
    MuonVal::VectorBranch<float>&           m_segment_numberDoF{m_tree.newVector<float>("segment_numberDoF")};

};
}
#endif
