/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    RDHK m_acc_parent_uniqueID {
      this, "acc_ftagTruthParentBarcode", "ftagTruthParentBarcode",
        "Accessor for the uniqueID of the parent of linked truth particle"};

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
    WDHK m_dec_uniqueID {
      this, "dec_ftagTruthBarcode", "ftagTruthBarcode",
        "UniqueID of linked truth particle"};
    WDHK m_dec_parent_uniqueID {
      this, "dec_ftagTruthParentBarcode", "ftagTruthParentBarcode",
        "UniqueID of parent of linked truth particle"};
    WDHK m_dec_muon_origin_label {
      this, "dec_ftagTruthMuonOriginLabel", "ftagTruthMuonOriginLabel", 
        "Exclusive origin label of the muon"};


    // Truth origin tool
    ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
      this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool", 
        "track truth origin tool"};
    // Truth Lepton Origin tool (https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/AnalysisCommon/TruthClassification)
    ToolHandle<CP::IClassificationTool> m_truthLeptonTool{
      this, "truthLeptonTool", "",
        "Lepton truth classification tool"};

    // Defining a map for proper muon origin labels 
    std::unordered_map<Truth::Type,unsigned int> m_muTruthMap = {
      {Truth::Type::Unknown, 0}, //Fake(Uknown) muon
      {Truth::Type::KnownUnknown, 1}, //KnownUknown muon
      {Truth::Type::PromptMuon, 2}, //Prompt muon
      {Truth::Type::BHadronDecay, 3}, //From B muon
      // FromBC and FromC decay label are handled in the implementation since FromBC is not directly available for TruthClassificationTool
      // FromBC and FromC decay label are handled in the implementation since FromBC is not directly available for TruthClassificationTool 
      {Truth::Type::LightFlavorDecay, 6}, //From light muon
      {Truth::Type::TauDecay, 7}, //From tau muon
      {Truth::Type::TauDecayLike, 8}, //4-like muons
      {Truth::Type::BHadronDecayLike, 8}, //4-like muons
      {Truth::Type::CHadronDecayLike, 8}, //4-like muons
      {Truth::Type::NonMuonlike, 8}, //Non-muons like muons
      // There are also cases where the muon has the truth particle associated to the ID track that is not a muon. Those muons are labelled as 9 (NotInnerDetector muons)
    };

    // ATLASRECTS-8290: this is for backward compatability, remove eventually
    Gaudi::Property<bool> m_use_barcode {
      this, "useBarcode", false, "use barcode rather than UID"
    };
    SG::ConstAccessor<int> m_uid{"uid"};

  };
}

#endif
