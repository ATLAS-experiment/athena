/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "JpsiUpsilonTools/JpsiUpsilonCommon.h"
#include "TLorentzVector.h"
#include "xAODBPhys/BPhysHelper.h"
#include "xAODTracking/VertexContainer.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"

namespace Analysis {   
    // *********************************************************************************
    
    // ---------------------------------------------------------------------------------
    // getPt: returns the pT of a track pair
    // ---------------------------------------------------------------------------------
    double JpsiUpsilonCommon::getPt(std::span<const xAOD::TrackParticle* const> tracks)
    {
        if (tracks.empty()) {
            return 0.0;
        }
    
        // Start with the four-momentum of the first track
        auto momentum = tracks[0]->genvecP4();
    
        // Add the rest
        for (size_t i = 1; i < tracks.size(); ++i) {
            momentum += tracks[i]->genvecP4();
        }
    
        return std::sqrt(momentum.Perp2());
    }

    double JpsiUpsilonCommon::getInvariantMass(const xAOD::TrackParticle* trk1, double mass1, const xAOD::TrackParticle* trk2, double mass2)
    {
        auto mom1 = trk1->genvecP4();
        mom1.SetM(mass1);
        auto mom2 = trk2->genvecP4();
        mom2.SetM(mass2);
        return (mom1 + mom2).M();
    }
    
    double JpsiUpsilonCommon::getInvariantMass(std::span<const xAOD::TrackParticle*> tracks,
                                             std::span<const double> masses)
    {
        assert(tracks.size() == masses.size());
        // Start with the four-momentum of the first track
        auto Totalmomentum = tracks[0]->genvecP4();
        Totalmomentum.SetM(masses[0]);

        for (size_t i = 1; i < tracks.size(); ++i) {
           auto momentum = tracks[i]->genvecP4();
           momentum.SetM(masses[i]);
           Totalmomentum += momentum;
        }
        return Totalmomentum.M();
    }


    // -------------------------------------------------------------------------------------------------
    // isContainedIn: boolean function which checks if a track (1st argument) is also contained in a
    // vector (second argument)
    // -------------------------------------------------------------------------------------------------
    
    bool JpsiUpsilonCommon::isContainedIn(const xAOD::TrackParticle* theTrack, std::span<const xAOD::TrackParticle* const> theColl) noexcept {
        return std::find(theColl.begin(), theColl.end(), theTrack) != theColl.end();
    }
    
    bool JpsiUpsilonCommon::isContainedIn(const xAOD::TrackParticle* theTrack, const xAOD::MuonContainer* theColl) {
        bool isContained(false);
        xAOD::MuonContainer::const_iterator muItr;
        for (muItr=theColl->begin(); muItr!=theColl->end(); ++muItr) {
            auto& link = ( *muItr )->inDetTrackParticleLink();
            if ( link.isValid() && ( *link == theTrack ) ) {isContained=true; break;}
        }
        return isContained;
    }

    bool JpsiUpsilonCommon::cutRange(double value, double min, double max) noexcept {
        return (min<=0.0 || value >= min) && (max <= 0.0 || value <= max);
    }

    bool JpsiUpsilonCommon::cutRangeOR(std::span<double const> values, double min, double max) noexcept {
        for(double m : values) {
           if( (min<=0.0 || m >= min) && (max <= 0.0 || m <= max)) return true;
        }
        return false;
    }

    bool JpsiUpsilonCommon::cutAcceptGreater(double value, double min ) noexcept {
        return (min <=0.0 || value >= min);
    }

    bool JpsiUpsilonCommon::cutAcceptGreaterOR(std::span<double const> values, double min) noexcept {
        for(double m : values) {
           if(min <=0.0 || m >= min) return true;
        }
        return false;
    }

    Analysis::CleanUpVertex JpsiUpsilonCommon::ClosestRefPV(xAOD::BPhysHelper& bHelper,
							const xAOD::VertexContainer* importedPVerticesCollection,
							const Analysis::PrimaryVertexRefitter *pvRefitter){
       const xAOD::Vertex* vtx_closest = nullptr; // vertex closest to bVertex track
       if(importedPVerticesCollection->empty()) return Analysis::CleanUpVertex(nullptr, false);
       double dc = 1e10;
       std::vector<const xAOD::Vertex*> tocleanup;
       if(pvRefitter) tocleanup.reserve(importedPVerticesCollection->size());
       bool vertexrefitted = false;
       for (const xAOD::Vertex* PV : *importedPVerticesCollection) {
          const xAOD::Vertex* refPV = pvRefitter ? pvRefitter->refitVertex(PV, bHelper.vtx(), false) : nullptr;
          const xAOD::Vertex* vtx = refPV ? refPV : PV;
          if(refPV) tocleanup.push_back(refPV);
          TVector3 posPV(vtx->position().x(),vtx->position().y(),vtx->position().z());
          auto &helperpos = bHelper.vtx()->position();
          TVector3 posV(helperpos.x(), helperpos.y(), helperpos.z());
          TVector3 nV = bHelper.totalP().Unit();
          TVector3 dposV = posPV-posV;
          double dposVnV = dposV*nV;
          double d = std::sqrt(std::abs(dposV.Mag2()-dposVnV*dposVnV));
          if (d<dc) {
             dc = d;
             vtx_closest = vtx;
             vertexrefitted = (vtx_closest == refPV);
          }
       }
       for(auto ptr : tocleanup){
           if(ptr != vtx_closest) delete ptr;
       }
       return Analysis::CleanUpVertex(vtx_closest, vertexrefitted);
    }

    void JpsiUpsilonCommon::RelinkVertexTracks(std::span<const xAOD::TrackParticleContainer* const> trkcols, xAOD::Vertex* vtx) {
      std::vector<ElementLink<DataVector<xAOD::TrackParticle> > > newLinkVector;
      auto size = vtx->trackParticleLinks().size();
      for(size_t i = 0; i<size; i++){
          const xAOD::TrackParticle* mylink= *(vtx->trackParticleLinks()[i]);
          for(const xAOD::TrackParticleContainer* trkcol : trkcols){
              auto itr = std::find(trkcol->begin(), trkcol->end(), mylink);
              if(itr != trkcol->end()){
                auto mylink=vtx->trackParticleLinks()[i];
                mylink.setStorableObject(*trkcol, true);
                newLinkVector.push_back( mylink );
                break;
              }
          }
      }
      if(size != newLinkVector.size()){
        throw std::runtime_error("JpsiUpsilonCommon::RelinkVertexTracks: Could not relink all tracks");
      }
      vtx->clearTracks();
      vtx->setTrackParticleLinks( newLinkVector );
    }
    
    void JpsiUpsilonCommon::RelinkVertexMuons(std::span<const xAOD::MuonContainer* const> muoncols, xAOD::Vertex* vtx){
       using MuonLink = ElementLink<xAOD::MuonContainer>;
       using MuonLinkVector = std::vector<MuonLink>;
       static const SG::AuxElement::Decorator<MuonLinkVector> muonLinksDecor("MuonLinks");
       const MuonLinkVector &mlinksold = muonLinksDecor(*vtx);
       auto size = mlinksold.size();
       MuonLinkVector newmulinks;
       for(size_t i = 0; i<size; i++){
          const xAOD::Muon* mylink= *(mlinksold[i]);
          for(const xAOD::MuonContainer* mucol : muoncols){
            auto itr = std::find(mucol->begin(), mucol->end(), mylink);
            if(itr != mucol->end()){
                auto mylink=mlinksold[i];
                mylink.setStorableObject(*mucol, true);
                newmulinks.push_back( mylink );
                break;
            }
          }
       }
       if(size != newmulinks.size()){
        throw std::runtime_error("JpsiUpsilonCommon::RelinkVertexMuons: Could not relink all tracks");
       }
       muonLinksDecor(*vtx) = std::move(newmulinks);
    }
}

