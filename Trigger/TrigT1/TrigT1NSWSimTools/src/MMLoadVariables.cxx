/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMLoadVariables.h"
#include "TruthUtils/MagicNumbers.h"

MMLoadVariables::MMLoadVariables() : AthMessaging(Athena::getMessageSvc(), "MMLoadVariables") {}

StatusCode MMLoadVariables::getTruthInfo(const EventContext& ctx,
                                         const McEventCollection *truthContainer,
                                         const TrackRecordCollection* trackRecordCollection,
                                         std::map<std::pair<uint64_t,unsigned int>,evInf_entry>& Event_Info) const {
      //*******Following MuonPRD code to access all the variables**********
      std::vector<ROOT::Math::PtEtaPhiEVector> truthParticles, truthParticles_ent, truthParticles_pos;
      std::vector<int> pdg;
      std::vector<ROOT::Math::XYZVector> vertex;
      float phiEntry_tmp    = 0;
      float phiPosition_tmp = 0;
      float etaEntry_tmp    = 0;
      float etaPosition_tmp = 0;
      int pdg_tmp           = 0;
      ROOT::Math::XYZVector vertex_tmp(0.,0.,0.);
 
      ROOT::Math::PtEtaPhiEVector thePart, theInfo;
      auto MuEntry_Particle_n = (trackRecordCollection!=nullptr)?trackRecordCollection->size():0;
      int j=0; // iteration of particle entries
      if( truthContainer != nullptr ){
      for(const auto subEvent : *truthContainer) {
        for(const auto& particle : *subEvent){
          const HepMC::FourVector momentum = particle->momentum();
          if( HepMC::generations(particle) < 1 && std::abs(particle->pdg_id())==13){
            thePart.SetCoordinates(momentum.perp(),momentum.eta(),momentum.phi(),momentum.e());
            if(trackRecordCollection!=nullptr){
            for(const auto & mit : *trackRecordCollection ) {
              const CLHEP::Hep3Vector mumomentum = mit.GetMomentum();
              const CLHEP::Hep3Vector muposition = mit.GetPosition();
              if(!trackRecordCollection->empty() && HepMC::barcode(particle) == mit.barcode()) { // FIXME barcode-based
                pdg_tmp         = particle->pdg_id();
                phiEntry_tmp    = mumomentum.getPhi();
                etaEntry_tmp    = mumomentum.getEta();
                phiPosition_tmp = muposition.getPhi();
                etaPosition_tmp = muposition.getEta();
              }
            }//muentry loop
            } // trackRecordCollection is not null
#ifdef HEPMC3
            vertex_tmp = subEvent->vertices().front()->position();
#else
            int l=0;
            for(const auto vit : subEvent->vertex_range())
            {
              if(l!=0){break;}//get first vertex of iteration, may want to change this
              l++;
              const HepMC::GenVertex *vertex1 = vit;
              const HepMC::FourVector& position = vertex1->position();
              vertex_tmp.SetXYZ(position.x(),position.y(),position.z());
            }//end vertex loop
#endif
          }
          j++;

            if(thePart.Pt() > 0. && HepMC::generations(particle) < 1){
              bool addIt = true;
              for(unsigned int ipart=0; ipart < truthParticles.size(); ipart++){
                if( std::abs(thePart.Pt()-truthParticles[ipart].Pt()) < 0.001 ||
                    std::abs(thePart.Eta()-truthParticles[ipart].Eta()) < 0.001 ||
                    std::abs(xAOD::P4Helpers::deltaPhi(thePart.Phi(), truthParticles[ipart].Phi())) < 0.001 ||
                    std::abs(thePart.E()-truthParticles[ipart].E()) < 0.001 ) addIt = false;
              }
              if(addIt){
                truthParticles.push_back(thePart);
                //new stuff
                vertex.push_back(vertex_tmp);
                pdg.push_back(pdg_tmp);
                truthParticles_ent.push_back(ROOT::Math::PtEtaPhiEVector(momentum.perp(),etaEntry_tmp   ,phiEntry_tmp   ,momentum.e()));
                truthParticles_pos.push_back(ROOT::Math::PtEtaPhiEVector(momentum.perp(),etaPosition_tmp,phiPosition_tmp,momentum.e()));
              }
            }

        } //end particle loop
      } //end truth container loop (should be only 1 container per event)
      } // if truth container is not null

      uint64_t event = ctx.eventID().event_number();
      for(unsigned int i=0; i<truthParticles.size(); i++) {
        evInf_entry particle_info(event, pdg[i],
                    truthParticles[i].E(), truthParticles[i].Pt(),
                    truthParticles[i].Eta(), truthParticles_pos[i].Eta(), truthParticles_ent[i].Eta(),
                    truthParticles[i].Phi(), truthParticles_pos[i].Phi(), truthParticles_ent[i].Phi(),
                    truthParticles[i].Theta(), truthParticles_pos[i].Theta(), truthParticles_ent[i].Theta(), truthParticles_ent[i].Theta()-truthParticles_pos[i].Theta(),
                    j,MuEntry_Particle_n,vertex[i]);
        Event_Info[std::make_pair(event,i)] = particle_info;
      }

    return StatusCode::SUCCESS;
}

evInf_entry::evInf_entry(uint64_t event,int pdg,double e,double p,double ieta,double peta,double eeta,double iphi,double pphi,double ephi,double ithe,double pthe,double ethe,double dth,
                         int trn,int mun,const ROOT::Math::XYZVector& tex):
  athena_event(event),pdg_id(pdg),E(e),pt(p),eta_ip(ieta),eta_pos(peta),eta_ent(eeta),phi_ip(iphi),phi_pos(pphi),phi_ent(ephi),theta_ip(ithe),theta_pos(pthe),theta_ent(ethe),
  dtheta(dth),truth_n(trn),mu_n(mun),vertex(tex) {}
