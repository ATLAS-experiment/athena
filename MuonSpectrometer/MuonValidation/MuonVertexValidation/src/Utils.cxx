/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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


std::vector<const xAOD::TruthParticle*> getGenStableChildren(const xAOD::TruthParticle* particle){
    // returns pointers to the generator stable children of a given particle

    std::vector<const xAOD::TruthParticle*> genStableChildren{};
    if(!particle) return {};
    if (particle->isGenStable()){
        genStableChildren.push_back(particle);
        return genStableChildren;
    } 

    std::vector<const xAOD::TruthParticle*> children = getChildren(particle);
    for (const xAOD::TruthParticle* child : children){
        // avoid infinite loops featured in some generators
        if (!child || child == particle) continue;
        if (child->hasDecayVtx() && child->decayVtx()->v4() == particle->decayVtx()->v4()) continue;
        // go deeper in the decay chain
        std::vector<const xAOD::TruthParticle*> grandChildren = getGenStableChildren(child);
        genStableChildren.insert(genStableChildren.end(), grandChildren.begin(), grandChildren.end());
    }

    return genStableChildren;
}

VtxIso getIso(const xAOD::Vertex *MSVtx, const xAOD::TrackParticleContainer& Tracks, const xAOD::JetContainer& Jets, 
              double trackIso_pT, double softTrackIso_R, double jetIso_pT, double jetIso_LogRatio){
    // compute the isolation metrics of the MS vertex: 
    // - delta R to closest hard track
    // - delta R to closest punch-through candidate jet
    // - sum of soft track pT in a cone around the vertex 

    VtxIso iso{};
    const Amg::Vector3D vtx_pos = MSVtx->position();

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
