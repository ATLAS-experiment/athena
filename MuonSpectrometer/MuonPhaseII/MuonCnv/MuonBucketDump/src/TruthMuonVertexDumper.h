/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCSVDUMP_TruthMuonVertexDumperAlg_H
#define MUONCSVDUMP_TruthMuonVertexDumperAlg_H


#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include <MuonTesterTree/IParticleFourMomBranch.h>


namespace MuonR4{
class TruthMuonVertexDumperAlg: public AthHistogramAlgorithm {

   public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    ~TruthMuonVertexDumperAlg() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode execute() override final;

   private:
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonsKey{this, "TruthMuonsKey", "MuonTruthParticles", "Key to the truth particle container"};
    SG::ReadHandleKey<xAOD::TruthVertexContainer> m_truthVertexKey{this, "TruthVerticesKey", "TruthVertices", "Key to the truth vertex container"};

    Gaudi::Property<std::vector<int>> m_pdgIdsToKeepVertex{this, "PdgIdsToKeepVertex", {25, 36, 50, 72, 31, 32, 3000001}, "List of PDG IDs to keep vertices for"};

    MuonVal::MuonTesterTree m_tree{"MuonVertexDump","MuonBucketDump"};

    bool selectDecayVertex(const xAOD::TruthVertex* vertex) const;
    void printChildren(const xAOD::TruthParticle* particle, int indentLevel) const;
    void printParents(const xAOD::TruthParticle* particle, int indentLevel) const;

    bool isFromVertexOfInterest(const xAOD::TruthParticle* particle, const xAOD::TruthVertex* vertex) const;

    //Truth muon information  
    MuonVal::ThreeVectorBranch              m_truthMuonVertexPosition{m_tree, "truthMuonVertexPosition"};
    MuonVal::MatrixBranch<uint16_t>&        m_truthMuonVertexMuonLinks{m_tree.newMatrix<uint16_t>("truthMuonVertexMuonLinks")};
    std::shared_ptr<MuonVal::IParticleFourMomBranch> m_truthMuonP4{};
};
}
#endif
