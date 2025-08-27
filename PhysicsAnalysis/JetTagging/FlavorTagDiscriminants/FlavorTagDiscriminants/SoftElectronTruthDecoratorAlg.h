/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELECTRON_TRUTH_DECORATOR_ALG_HH
#define ELECTRON_TRUTH_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "AthLinks/ElementLink.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "xAODTruth/TruthEventContainer.h"

#include <set>

namespace FlavorTagDiscriminants {

  class SoftElectronTruthDecoratorAlg: public AthReentrantAlgorithm {
  public:
    SoftElectronTruthDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:
    // Input Containers
    SG::ReadHandleKey< xAOD::ElectronContainer > m_ElectronContainerKey {
      this, "electronContainer", "Electrons",
        "Key for the input electron collection"};

    // Accessors for truth particles
    using RDHK = SG::ReadDecorHandleKey< xAOD::TruthParticleContainer >;
    RDHK m_acc_origin_label {
      this, "acc_ftagTruthOriginLabel", "ftagTruthOriginLabel", 
        "Accessor for the truth origin label of the truth particle"};
    RDHK m_acc_type_label {
      this, "acc_ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Accessor for the truth type label of the truth particle"};
    RDHK m_acc_source_label {
      this, "acc_ftagTruthSourceLabel", "ftagTruthSourceLabel", 
        "Accessor for the truth source label of the truth particle"};
    RDHK m_acc_vertex_index {
      this, "acc_ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "Accessor for the truth vertex index of the truth particle"};
    RDHK m_acc_parent_uniqueID {
      this, "acc_ftagTruthParentBarcode", "ftagTruthParentBarcode",
        "Accessor for the truth parent uniqueID of the truth particle"};

    // Decorators for electrons
    using WDHK = SG::WriteDecorHandleKey< xAOD::ElectronContainer >;
    WDHK m_dec_origin_label {
      this, "dec_ftagTruthOriginLabel", "ftagTruthOriginLabel", 
        "Exclusive origin label of the electron"};
    WDHK m_dec_type_label {
      this, "dec_ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Exclusive truth type label of the electron"};
    WDHK m_dec_source_label {
      this, "dec_ftagTruthSourceLabel", "ftagTruthSourceLabel", 
        "Exclusive truth label for the immedate parent of the truth particle"};
    WDHK m_dec_vertex_index {
      this, "dec_ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "Truth vertex index of the electron"};
    WDHK m_dec_uniqueID {
      this, "dec_ftagTruthBarcode", "ftagTruthBarcode",
        "UniqueID of linked truth particle"};
    WDHK m_dec_parent_uniqueID {
      this, "dec_ftagTruthParentBarcode", "ftagTruthParentBarcode",
        "UniqueID of parent of linked truth particle"};

    // truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};

    // Electron types are defined in
    // PhysicsAnalysis/MCTruthClassifier/MCTruthClassifier/MCTruthClassifierDefs.h#L28
    const std::set<int> m_valid_types{1, 2, 3, 4};

    // Accessors
    template <typename T> using Acc = SG::AuxElement::ConstAccessor<T>;
    Acc<unsigned int> m_classifierParticleType{"classifierParticleType"};
    Acc<ElementLink<xAOD::TruthParticleContainer>> m_truthParticleLink{"truthParticleLink"};
  };
}

#endif
