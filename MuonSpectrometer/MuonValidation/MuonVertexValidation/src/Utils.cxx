/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Utils.h"


namespace MSVtxValidationAlgUtils {

double getCTau(const xAOD::TruthVertex *decVtx){
    // compute the ctau for a given decay vertex via: ctau = |r|/gamma
    return decVtx->v4().Vect().Mag() / decVtx->genvecV4().Gamma();
}


double getCalEnergyLogRatio(double EMF){
    double logRatio{-99.};
    double zero{0.};
    double one{1.};

    if (CxxUtils::fpcompare::greater(EMF,zero)){
        if(CxxUtils::fpcompare::greater_equal(EMF,one)) logRatio = -999.;
        else logRatio = std::log10(double(1./EMF - 1.));
    } 
    else {
        logRatio = 999;
    }

    return logRatio;
}


bool comparePt(const xAOD::TruthParticle* part1, const xAOD::TruthParticle* part2){
	return (part1->pt()>part2->pt());
}


std::vector<const xAOD::TruthParticle*> getChildren(const xAOD::TruthParticle* mother){
    // returns pointers to children of a given particle
    if (!mother) return {};

    std::vector<const xAOD::TruthParticle*> children{};
    for (size_t i=0; i<mother->nChildren(); ++i) {
        const xAOD::TruthParticle* child = mother->child(i);
        // avoid infinite loop
        if (!child || child == mother) continue;
        children.push_back(child);
    }

    return children;
}


std::vector<const xAOD::TruthParticle*> getStableChildrenRecursive(const xAOD::TruthParticle* particle, bool findOnlyGenStable, std::unordered_set<const xAOD::TruthParticle*>& visited){
    // Can either find all stable particles or only those that are generator stable only.
    
    std::vector<const xAOD::TruthParticle*> stableChildren;
    // Base case: if the particle is null or has already been visited, return empty vector
    if (!particle || visited.count(particle)) return stableChildren;
    visited.insert(particle);

    // Return the particle if it is stable
    bool particleIsStable = findOnlyGenStable ? particle->isGenStable() : particle->isStable();
    if (particleIsStable) {
        stableChildren.push_back(particle);
        return stableChildren;
    }

    // Recursive case: get the children of the particle and traverse their decay chains
    std::vector<const xAOD::TruthParticle*> children = getChildren(particle);
    for (const xAOD::TruthParticle* child : children) {
        if (!child) continue;
        std::vector<const xAOD::TruthParticle*> grandChildren = getStableChildrenRecursive(child, findOnlyGenStable, visited);
        for (const xAOD::TruthParticle* c : grandChildren) {
            if (std::none_of(stableChildren.begin(), stableChildren.end(), [&](const auto& x) {return x==c;})) stableChildren.push_back(c);
        }
    }

    return stableChildren;
}


std::vector<const xAOD::TruthParticle*> getStableChildren( const xAOD::TruthParticle* particle, bool findOnlyGenStable){
    // Finds the stable decay products of a given particle. Can either find all stable particles or only those that are generator stable only.
    // Interface to the recursive function that traverses the decay chain of the particle.
    std::unordered_set<const xAOD::TruthParticle*> visited; // keeps track of visited particles to avoid infinite loops in decay chains
    return getStableChildrenRecursive(particle, findOnlyGenStable, visited);
}


JetVtxApprox getJetVtxApprox(const xAOD::Jet* jet, const xAOD::TruthParticleContainer& truthParticles){
    // Finds the vertex in the ancestry tree Truth particles close to the jet 
    // The vertex is selected to have to most secondary particles associated to it (and the most displaced from the beam line in case of tie)
    
    std::set<const xAOD::TruthVertex*> seenVertices; // vertices already seen in the ancestry tree
    const xAOD::TruthVertex* mostActiveVertex = nullptr;
    size_t nChildren{0};        
    size_t maxChildren{0};
    size_t maxDecayDepth{0};

    for (const xAOD::TruthParticle* tp : truthParticles){
        if (!tp || !tp->isStable() || jet->p4().DeltaR(tp->p4()) > 0.4) continue; // only final state particles close to the jet axis will pass
        if (tp->barcode() < HepMC::SIM_BARCODE_THRESHOLD ) continue; // only simulated particles will pass. For samples made with athena 24.0 onwards, use tp->status() < HepMC::SIM_STATUS_THRESHOLD instead

        int decayDepth{0};
        const xAOD::TruthParticle* current = tp;
        while (current) {
            if (decayDepth > 200) break; // safety break 
            // prevent loop from going too deep where the truth record contains information used for generator internal book keeping 
            // add current->status()<HepMC::SIM_STATUS_THRESHOLD to limit to GEANT4 layer of ancestry
            if (!MC::isPhysical(current)) break;
            const xAOD::TruthVertex* prodVtx = current->prodVtx();
            if (!prodVtx || seenVertices.count(prodVtx)) break;  // No more ancestry or already visited. 
            if (prodVtx->v4().Mag2() < 0) break; // minimal requirements on the vertex: physical spacetime interval
            seenVertices.insert(prodVtx);
            decayDepth++;
            // update the mostActiveVertex is one with more children is found. Need at least two children. 
            // if there is a tie in the number of children, precedence is given to the more displaced vertex 
            nChildren = prodVtx->nOutgoingParticles();                
            if ((nChildren >= 2) && ((nChildren > maxChildren) || (mostActiveVertex && nChildren == maxChildren && prodVtx->v4().Vect().Mag2() > mostActiveVertex->v4().Vect().Mag2()))) { 
                mostActiveVertex = prodVtx;
                maxChildren = nChildren;
                maxDecayDepth = decayDepth;
            }
            current = prodVtx->incomingParticle(0); // move up one level in the ancestor tree
        }
    }
    JetVtxApprox jetVtx{mostActiveVertex, maxChildren, maxDecayDepth};

    return jetVtx;
}


VtxIso getIso(const xAOD::Vertex *vtx, const xAOD::TrackParticleContainer& Tracks, const xAOD::JetContainer& Jets, 
              double trackIso_pT, double softTrackIso_R, double jetIso_pT, double jetIso_LogRatio){
    // compute the isolation metrics of the MS vertex: 
    // - delta R to closest hard track
    // - delta R to closest punch-through candidate jet
    // - sum of soft track pT in a cone around the vertex 

    VtxIso iso{};
    const Amg::Vector3D vtx_pos = vtx->position();

    // isolation towards tracks 
    Amg::Vector3D softTrack_pTsum{Amg::Vector3D::Zero()};
    double hardTrack_mindR{99.}; 
    for(const xAOD::TrackParticle* Track : Tracks){
        double dR = xAOD::P4Helpers::deltaR(vtx_pos.eta(), vtx_pos.phi(), Track->eta(), Track->phi());
        // hard tracks
        if(Track->pt() >= trackIso_pT && dR < hardTrack_mindR) hardTrack_mindR = dR; 
        // soft tracks
        if(Track->pt() < trackIso_pT && dR < softTrackIso_R) softTrack_pTsum += Amg::Vector3D(Track->p4()[0], Track->p4()[1], Track->p4()[2]);
    }

    iso.track_mindR = hardTrack_mindR != 99. ? hardTrack_mindR : -1.;
    iso.track_pTsum = softTrack_pTsum.mag() != 0. ? softTrack_pTsum.perp()/Gaudi::Units::GeV : -1.;

    // isolation towards jets
    double jet_mindR{99.}; 
    for(const xAOD::Jet* Jet : Jets){
        if(Jet->pt() < jetIso_pT) continue;
        // if(!Jet->getAttribute<char>("passJVT")) continue; 
        double logratio = getCalEnergyLogRatio(Jet->getAttribute<float>("EMFrac"));
        if(logratio >= jetIso_LogRatio) continue;

        double dR = xAOD::P4Helpers::deltaR(vtx_pos.eta(), vtx_pos.phi(), Jet->eta(), Jet->phi());
        if(dR < jet_mindR){
            jet_mindR = dR;
        } 
    }
    iso.jet_mindR = jet_mindR != 99. ? jet_mindR : -1.;

    return iso;
}

} // namespace MSVtxValidationAlgUtils
