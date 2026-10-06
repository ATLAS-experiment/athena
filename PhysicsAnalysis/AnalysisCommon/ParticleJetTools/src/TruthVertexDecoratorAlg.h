/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PARTICLEJETTOOLS_TRUTHVERTEXDECORATORALG_H
#define PARTICLEJETTOOLS_TRUTHVERTEXDECORATORALG_H

#include "AthContainers/AuxElement.h"
#include "xAODTruth/TruthParticleContainerFwd.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"
#include "TruthClassification/TruthClassificationTool.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "ParticleJetTools/FatVertex.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace ParticleJetTools{

class TruthVertexDecoratorAlg: public AthReentrantAlgorithm {
    public:
        TruthVertexDecoratorAlg(const std::string& name,
                            ISvcLocator* pSvcLocator );

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ) const override;

    private:
        Gaudi::Property<bool> m_use_barcode {
            this, "useBarcode", false, "use barcode rather than UID"
        };
        SG::ConstAccessor<int> m_acc_uid{"uid"};
        // Input for truth particles and truth vertices
        SG::ReadHandleKey< xAOD::TruthParticleContainer > m_TruthContainerKey {
        this, "truthContainer", "TruthParticles",
        "Key for the input truth particle collection"};

        SG::ReadHandleKey< xAOD::TruthVertexContainer > m_TruthPVsKey {
        this, "TruthPrimaryVertices", "TruthPrimaryVertices",
        "Key for the input truth PV collection"};
        SG::ReadHandleKey<xAOD::TruthVertexContainer> m_TruthVertexContainerKey {
        this, "TruthVertices", "TruthVertices",
        "Key for the input truth vertex collection"};

        // Input for tracks and constituents
        SG::ReadHandleKey< xAOD::TrackParticleContainer > m_TrackContainerKey {
        this, "trackContainer", "InDetTrackParticles",
            "Key for the input track collection"};

        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trackTPDecayVertexID {
        this, "ftagTrackDecayVertexID", "ftagTrackDecayVertexID",
            "Vertex ID that this track is the decay product of"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trackTPDecayVertexType  {
        this, "ftagTrackDecayVertexType", "ftagTrackDecayVertexType",
            "Detailed vertex type that this track is the decay product of"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trackTPDecaySimpleVertexType  {
            this, "ftagTrackDecaySimpleVertexType", "ftagTrackDecaySimpleVertexType",
                "Simple Vertex type that this track is the decay product of"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trackPDGID  {
        this, "trackPDGID", "trackPDGID",
            "PDGID of truth-matched particle"};
        SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trackParentPDGID  {
        this, "trackParentPDGID", "trackParentPDGID",
            "PDGID of truth-matched particle's parent"};

        // Accessors for writing vertex info to truth particles
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexValid{
        this, "ftagTPValid", "ftagTPValid",
        "Is this truth particle physical - excludes gluons, free quarks, (bosons?)"};

        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayType {
        this, "ftagTPDecayVertexType", "ftagTPDecayVertexType",
        "Exclusive label for this truth particles decay type"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexSimpleDecayType {
            this, "ftagTPDecaySimpleVertexType", "ftagTPDecaySimpleVertexType",
            "Exclusive label for this truth particles decay type"};

        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayID {
        this, "ftagTPDecayVertexID", "ftagTPDecayVertexID",
        "ID for this truth particle vertex ID"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayParentID {
        this, "ftagTPDecayVertexParentID", "ftagTPDecayVertexParentID",
        "ID for this truth particles parent ID"};
        // We write charged and neutral summary at truth level - in case some decays are too
        // low pT to be kept
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayNCharged {
        this, "ftagTPDecayVertexNCharged", "ftagTPDecayVertexNCharged",
        "Number of charged particles in the decay vertex"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayNNeutral {
        this, "ftagTPDecayVertexNNeutral", "ftagTPDecayVertexNNeutral",
        "Number of neutral particles in the decay vertex (with pT threshold)"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayAllNCharged {
        this, "ftagTPDecayVertexAllNCharged", "ftagTPDecayVertexAllNCharged",
        "Number of charged particles in the decay vertex"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayAllNNeutral {
        this, "ftagTPDecayVertexAllNNeutral", "ftagTPDecayVertexAllNNeutral",
        "Number of neutral particles in the decay vertex (no pT threshold)"};

        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayPVDistance {
        this, "ftagTPDecayPVDistance", "ftagTPDecayPVDistance",
        "Three distance from PV->this particle decay"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexDecayDistance {
        this, "ftagTPDecayDistance", "ftagTPDecayDistance",
        "Three distance from this particle creation->this particle decay"};
        SG::WriteDecorHandleKey< xAOD::TruthParticleContainer > m_vertexIsVertex {
        this, "ftagTPIsVertex", "ftagTPIsVertex",
        "Is this particle a vertex (decays with >= 2 children)"};

        SG::ReadDecorHandleKey< xAOD::TruthParticleContainer > m_acc_truth_particle_vertex_id {
        this, "ftagTPDecayVertexIDReader", "ftagTPDecayVertexID",
            "Accessor for the truth type label of the truth particle"};

        ToolHandle<InDet::InDetTrackTruthOriginTool> m_trackTruthOriginTool {
        this, "trackTruthOriginTool", "InDet::InDetTrackTruthOriginTool",
            "track truth origin tool"};

        Gaudi::Property<float> m_truthVertexMergeDistance {
        this, "truthVertexMergeDistance", 1.0,
            "Merge any truth vertices within this distance [mm]. Default 1.0 mm "
            "folds pi0 decays (ctau ~25 nm) into their parent vertex."};
        Gaudi::Property<float> m_truthParticleMinimumPt {
        this, "truthParticleMinimumPt", 0,
            "Minimum pT [MeV] for counting charged/neutral multiplicities in NCharged/NNeutral (AllNCharged/AllNNeutral have no cut)"};
        Gaudi::Property<bool> m_alwaysKeepStableNeutrals {
        this, "alwaysKeepStableNeutrals", true,
            "Always keep final state neutrals"};
        Gaudi::Property<bool> m_alwaysKeepTracklessStableCharged {
        this, "alwaysKeepTracklessStableCharged", true,
            "Always keep final state charged particles that don't have an associated track"};
        Gaudi::Property<bool> m_alwaysKeepStableCharged {
        this, "alwaysKeepStableCharged", true,
            "Always keep final state charged particles, even the particle has an associated track"};

        Gaudi::Property<std::string> m_truthMatchProbabilityAuxName {
            this, "truthMatchProbabilityAuxName", "truthMatchProbability",
            "Name of the aux variable on track which contains its TMP score"};
        Gaudi::Property<float> m_truthMatchProbabilityCut {
            this, "truthMatchProbabilityCut", 0.5,
            "Tracks below this TMP value are assigned as Fakes"};

        SG::ConstAccessor<float> m_truthMatchProbabilityAcc{"truthMatchProbability"};


    }; // End of TruthVertexDecoratorAlg
} // End of ParticleJetTools namespace
#endif // PARTICLEJETTOOLS_TRUTHVERTEXDECORATORALG_H
