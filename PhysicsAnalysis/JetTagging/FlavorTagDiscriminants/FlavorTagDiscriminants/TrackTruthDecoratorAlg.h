/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACK_TRUTH_DECORATOR_ALG_HH
#define TRACK_TRUTH_DECORATOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainerFwd.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "TruthClassification/TruthClassificationTool.h"


namespace FlavorTagDiscriminants {

  class TrackTruthDecoratorAlg: public AthReentrantAlgorithm {
  public:
    TrackTruthDecoratorAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ) const override;

  private:
    // Input Containers
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackContainerKey {
      this, "trackContainer", "InDetTrackParticles",
        "Key for the input track collection"};
    SG::ReadHandleKey< xAOD::MuonContainer > m_MuonContainerKey {
      this, "muonContainer", "Muons",
        "Key for the input muon collection"};

    // Accessors for truth particles
    using RDHK = SG::ReadDecorHandleKey< xAOD::TruthParticleContainer >;
    RDHK m_acc_type_label {
      this, "acc_ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Accessor for the truth type label of the truth particle"};
    RDHK m_acc_source_label {
      this, "acc_ftagTruthSourceLabel", "ftagTruthSourceLabel", 
        "Accessor for the truth label for the source of secondary particles"};
    RDHK m_acc_vertex_index {
      this, "acc_ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "Accessor for the vertex index of the truth particle"};
    RDHK m_acc_parent_barcode {
      this, "acc_ftagTruthParentBarcode", "ftagTruthParentBarcode", 
        "Accessor for the barcode of the parent of linked truth particle"};

    // Decorators for tracks
    using WDHK = SG::WriteDecorHandleKey< xAOD::TrackParticleContainer >;
    WDHK m_dec_origin_label {
      this, "dec_ftagTruthOriginLabel", "ftagTruthOriginLabel", 
        "Exclusive origin label of the track"};
    WDHK m_dec_type_label {
      this, "dec_ftagTruthTypeLabel", "ftagTruthTypeLabel", 
        "Exclusive truth type label of the track"};
    WDHK m_dec_source_label {
      this, "dec_ftagTruthSourceLabel", "ftagTruthSourceLabel", 
        "Exclusive truth label for the source of secondary tracks"};
    WDHK m_dec_vertex_index {
      this, "dec_ftagTruthVertexIndex", "ftagTruthVertexIndex", 
        "ftagTruth vertex index of the track"};
    WDHK m_dec_barcode {
      this, "dec_ftagTruthBarcode", "ftagTruthBarcode", 
        "Barcode of linked truth particle"};
    WDHK m_dec_parent_barcode {
      this, "dec_ftagTruthParentBarcode", "ftagTruthParentBarcode", 
        "Barcode of parent of linked truth particle"};
    WDHK m_dec_muon_origin_label {
      this, "dec_ftagTruthMuonOriginLabel", "ftagTruthMuonOriginLabel", 
        "Exclusive origin label of the muon"};

    // Truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};
    // Truth Lepton Origin tool (https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/AnalysisCommon/TruthClassification)
    ToolHandle<CP::IClassificationTool> m_truthLeptonTool{"TruthClassificationTool/truthLeptonTool"};

    // Defining a map for proper muon origin labels 
    std::map<unsigned int,unsigned int> m_muTruthMap = {
      {0, 0}, //Fake(Uknown) muon
      {1, 1}, //KnownUknown muon
      {4, 2}, //Prompt muon
      {8, 3}, //From B muon
      {13, 4}, //From B/C (decay chain) muon (not included in the muon TruthClassificationTool)
      {9, 5}, //From C muon 
      {10, 6}, //From light muon
    };
  };
}

#endif
