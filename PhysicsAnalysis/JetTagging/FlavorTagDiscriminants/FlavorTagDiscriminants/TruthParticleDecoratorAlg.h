/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_PARTICLE_DECORATOR_ALG_HH
#define TRUTH_PARTICLE_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainerFwd.h"
#include "xAODTruth/TruthVertexContainerFwd.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"


namespace FlavorTagDiscriminants {

  class TruthParticleDecoratorAlg: public AthReentrantAlgorithm {
  public:
    TruthParticleDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:

    // Input Containers
    SG::ReadHandleKey< xAOD::TruthParticleContainer > m_TruthContainerKey {
      this, "truthContainer", "TruthParticles",
        "Key for the input truth particle collection"};
    SG::ReadHandleKey< xAOD::TruthVertexContainer > m_TruthPVsKey {
      this, "TruthPrimaryVertices", "TruthPrimaryVertices",
        "Key for the input truth PV collection"};

    // Decorators for truth particles
    SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_dec_origin_label {
      this, "ftagTruthOriginLabel", "ftagTruthOriginLabel", 
        "Exclusive origin label of the truth particle"};
    SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_dec_type_label {
      this, "ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Exclusive truth type label of the truth particle"};
    SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_dec_source_label {
      this, "ftagTruthSourceLabel", "ftagTruthSourceLabel", 
        "Exclusive truth label for the source of secondary particles"};
    SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_dec_vertex_index {
      this, "ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "ftagTruth vertex index of the truth particle"};
    SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_dec_parent_barcode {
      this, "ftagTruthParentBarcode", "ftagTruthParentBarcode", 
        "Barcode of parent of linked truth particle"};

    // truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_truthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};

    Gaudi::Property<float> m_truthVertexMergeDistance {
      this, "truthVertexMergeDistance", 0.1, 
        "Merge any truth vertices within this distance [mm]"};
  };
}

#endif
