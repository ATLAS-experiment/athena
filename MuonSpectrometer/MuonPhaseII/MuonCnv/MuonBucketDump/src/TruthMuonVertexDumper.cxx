/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthMuonVertexDumper.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"


namespace MuonR4{
    StatusCode TruthMuonVertexDumperAlg::initialize() {
        m_tree.addBranch(std::make_shared<MuonVal::EventHashBranch>(m_tree.tree()));
        m_truthMuonP4 = std::make_unique<MuonVal::IParticleFourMomBranch>(m_tree, "truthMuon");
        m_tree.addBranch(m_truthMuonP4);
        ATH_CHECK(m_tree.init(this));

        ATH_CHECK(m_truthMuonsKey.initialize(!m_truthMuonsKey.empty()));
        ATH_CHECK(m_truthVertexKey.initialize(!m_truthVertexKey.empty()));

        ATH_MSG_ALWAYS("Successfully initialized");

        return StatusCode::SUCCESS;
    }

    StatusCode TruthMuonVertexDumperAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    
    StatusCode TruthMuonVertexDumperAlg::execute(const EventContext& ctx) {
                
        const xAOD::TruthParticleContainer* truthMuons{nullptr};
        ATH_CHECK(SG::get(truthMuons, m_truthMuonsKey, ctx));
        const xAOD::TruthVertexContainer* truthVertices{nullptr};
        ATH_CHECK(SG::get(truthVertices, m_truthVertexKey, ctx));


        std::vector<const xAOD::TruthVertex*> bsmVertices{};
        for (const auto* vertex : *truthVertices) {
            if(!selectDecayVertex(vertex)) continue;
            bsmVertices.push_back(vertex); 
            m_truthMuonVertexPosition.push_back(Amg::Vector3D(vertex->x(), vertex->y(), vertex->z()));
        }
        
        // Filling the information about all final state muons
        for (const auto* particle : *truthMuons) {
            m_truthMuonP4->push_back(particle);
            printParents(particle, 20);
            for(uint i_vertex=0; i_vertex<bsmVertices.size(); ++i_vertex) {
                const auto* bsmVertex = bsmVertices[i_vertex];
                if(isFromVertexOfInterest(particle, bsmVertex)) {
                    m_truthMuonVertexMuonLinks[i_vertex].push_back((particle->index()));
                }
            }
        }



        if(!m_tree.fill(ctx)) {
            return StatusCode::FAILURE; 
        }
        return StatusCode::SUCCESS;
    }

/*
@brief Selects vertices of BSM particles decaying to muons
*/

bool TruthMuonVertexDumperAlg::selectDecayVertex(const xAOD::TruthVertex* vertex) const {
    
    if (vertex->nIncomingParticles() != 1)
      return false;
  
    if (vertex->nOutgoingParticles() < 2)
      return false;
  
    const xAOD::TruthParticle *truthPart = vertex->incomingParticle(0);
    if (not truthPart)
      return false;
  
    //-- Keep particles of the pdgid requested (if any set requested, else
    // everything is kept)
    if (m_pdgIdsToKeepVertex.size() > 0 && 
          std::find(m_pdgIdsToKeepVertex.begin(), m_pdgIdsToKeepVertex.end(),
                  std::abs(truthPart->pdgId())) == m_pdgIdsToKeepVertex.end()){
                      return false;
          }
  
    return true;
}

void TruthMuonVertexDumperAlg::printChildren(const xAOD::TruthParticle* particle, int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    ATH_MSG_VERBOSE(indent << "Particle: PDG ID = " << particle->pdgId() << ", pT = " << particle->pt() 
                    << ", eta = " << particle->eta() << ", phi = " << particle->phi() << ", charge = " << particle->charge());
    for (size_t i=0; i<particle->nChildren(); ++i) {
        const auto* child = particle->child(i);
        printChildren(child, indentLevel + 1);
    }
}

void TruthMuonVertexDumperAlg::printParents(const xAOD::TruthParticle* particle, int indentLevel) const {
    std::string indent(indentLevel * 2, ' ');
    ATH_MSG_VERBOSE(indent << "Particle: PDG ID = " << particle->pdgId() << ", pT = " << particle->pt() 
                    << ", eta = " << particle->eta() << ", phi = " << particle->phi() << ", charge = " << particle->charge());
    if (particle->prodVtx()) {
        for (size_t i=0; i<particle->prodVtx()->nIncomingParticles(); ++i) {
            const auto* parent = particle->prodVtx()->incomingParticle(i);
            printParents(parent, indentLevel + 1);
        }
    }
}

bool TruthMuonVertexDumperAlg::isFromVertexOfInterest(const xAOD::TruthParticle* particle, const xAOD::TruthVertex* vertex) const {
        if (!particle->prodVtx()) {
            return false;
        }
        if (particle->prodVtx() == vertex) return true; 

        for (size_t i=0; i<particle->prodVtx()->nIncomingParticles(); ++i) {
            const auto* parent = particle->prodVtx()->incomingParticle(i);
            if (isFromVertexOfInterest(parent, vertex)) {
                return true;
            }
        }
        return false;
}

} // namespace MuonR4