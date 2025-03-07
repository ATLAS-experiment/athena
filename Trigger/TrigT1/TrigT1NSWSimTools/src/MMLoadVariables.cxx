/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMLoadVariables.h"
#include "TruthUtils/MagicNumbers.h"

MMLoadVariables::MMLoadVariables(const MuonGM::MuonDetectorManager* detManager, const MmIdHelper* idhelper):
   AthMessaging(Athena::getMessageSvc(), "MMLoadVariables") {
      m_detManager = detManager;
      m_MmIdHelper = idhelper;
}

StatusCode MMLoadVariables::getMMDigitsInfo(const EventContext& ctx,
                                            const McEventCollection *truthContainer,
                                            const TrackRecordCollection* trackRecordCollection,
                                            const MmDigitContainer *nsw_MmDigitContainer,
                                            std::map<std::pair<int,unsigned int>,std::vector<digitWrapper> >& entries,
                                            std::map<std::pair<int,unsigned int>,std::vector<hitData_entry> >& Hits_Data_Set_Time,
                                            std::map<std::pair<int,unsigned int>,evInf_entry>& Event_Info) const {
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
      for(const auto it : *truthContainer) {
        const HepMC::GenEvent *subEvent = it;
#ifdef HEPMC3
        for(const auto& particle : subEvent->particles()){
#else
        for(const auto pit : subEvent->particle_range()){
          const HepMC::GenParticle *particle = pit;
#endif
          const HepMC::FourVector momentum = particle->momentum();
          if( HepMC::generations(particle) < 1 && std::abs(particle->pdg_id())==13){
            thePart.SetCoordinates(momentum.perp(),momentum.eta(),momentum.phi(),momentum.e());
            if(trackRecordCollection!=nullptr){
            for(const auto & mit : *trackRecordCollection ) {
              const CLHEP::Hep3Vector mumomentum = mit.GetMomentum();
              const CLHEP::Hep3Vector muposition = mit.GetPosition();
              if(!trackRecordCollection->empty() && HepMC::barcode(particle) ==mit.GetBarCode()) {
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

      int event = ctx.eventID().event_number();
      int TruthParticle_n = j;
      unsigned int digit_particles = 0;
      for(auto digitCollectionIter : *nsw_MmDigitContainer) {
        // a digit collection is instanciated for each container, i.e. holds all digits of a multilayer
        const MmDigitCollection* digitCollection = digitCollectionIter;
        // loop on all digits inside a collection, i.e. multilayer
        std::vector<digitWrapper> entries_tmp;

        for (const auto item:*digitCollection) {
            // get specific digit and identify it
            const MmDigit* digit = item;
            Identifier id = digit->identify();
            if (!m_MmIdHelper->is_mm(id)) continue;

            std::string stName   = m_MmIdHelper->stationNameString(m_MmIdHelper->stationName(id));
            int stationName      = m_MmIdHelper->stationName(id);
            int stationEta       = m_MmIdHelper->stationEta(id);
            int stationPhi       = m_MmIdHelper->stationPhi(id);
            int multiplet        = m_MmIdHelper->multilayer(id);
            int gas_gap          = m_MmIdHelper->gasGap(id);
            int channel          = m_MmIdHelper->channel(id);

            const MuonGM::MMReadoutElement* rdoEl = m_detManager->getMMReadoutElement(id);

            std::vector<float>  time{digit->stripResponseTime()};
            std::vector<float>  charge{digit->stripResponseCharge()};
            std::vector<int>    stripPosition{channel};
            std::vector<int>    MMFE_VMM{channel};
            std::vector<int>    VMM{channel};

            bool isValid = false;
            std::vector<double> localPosX;
            std::vector<double> localPosY;
            std::vector<double> globalPosX;
            std::vector<double> globalPosY;
            std::vector<double> globalPosZ;

            int nstrip = 0; //counter of the number of firing strips
            for (const auto &i: stripPosition) {

	      isValid = false; //reset
              // take strip index form chip information
              int cr_strip = i;
              localPosX.push_back (0.);
              localPosY.push_back (0.);
              globalPosX.push_back(0.);
              globalPosY.push_back(0.);
              globalPosZ.push_back(0.);
              ++nstrip;

              Identifier cr_id = m_MmIdHelper->channelID(stationName, stationEta, stationPhi, multiplet, gas_gap, cr_strip, isValid);
              if (!isValid) {
                ATH_MSG_WARNING("MicroMegas digitization: failed to create a valid ID for (chip response) strip n. " << cr_strip
                               << "; associated positions will be set to 0.0.");
              } else {
                  // asking the detector element to get local position of strip
                  Amg::Vector2D cr_strip_pos(0., 0.);
                  if ( !rdoEl->stripPosition(cr_id,cr_strip_pos) ) {
                    ATH_MSG_WARNING("MicroMegas digitization: failed to associate a valid local position for (chip response) strip n. " << cr_strip
                                   << "; associated positions will be set to 0.0.");
                  } else {
                    localPosX[nstrip-1] = cr_strip_pos.x();
                    localPosY[nstrip-1] = cr_strip_pos.y();
                  }

                  // asking the detector element to transform this local to the global position
                  Amg::Vector3D cr_strip_gpos(0., 0., 0.);
                  rdoEl->surface(cr_id).localToGlobal(cr_strip_pos, Amg::Vector3D(0., 0., 0.), cr_strip_gpos);
                  globalPosX[nstrip-1] = cr_strip_gpos[0];
                  globalPosY[nstrip-1] = cr_strip_gpos[1];
                  globalPosZ[nstrip-1] = cr_strip_gpos[2];
              }
            }//end of strip position loop
            if(globalPosY.empty()) continue;

            if (!time.empty()) entries_tmp.push_back(
              digitWrapper(digit, stName, -1.,
                           ROOT::Math::XYZVector(-999, -999, -999),
                           ROOT::Math::XYZVector(localPosX[0], localPosY[0], -999),
                           ROOT::Math::XYZVector(globalPosX[0], globalPosY[0], globalPosZ[0] )
                          ) );
        } //end iterator digit loop

        if (!entries_tmp.empty()) {
          std::vector<std::string> stNames;
          for(const auto &dW : entries_tmp) stNames.push_back(dW.stName);
          if(std::all_of(stNames.begin(), stNames.end(), [&] (const std::string & name) { return name == stNames[0]; })) {
            entries[std::make_pair(event,digit_particles)]=entries_tmp;
            digit_particles++;
          } else {
            ATH_MSG_WARNING("Digits belonging to different stations, skipping items");
          }
          stNames.clear();
        }
      } // end digit container loop

      for(unsigned int i=0; i<truthParticles.size(); i++) {
        evInf_entry particle_info(event, pdg[i],
                    truthParticles[i].E(), truthParticles[i].Pt(),
                    truthParticles[i].Eta(), truthParticles_pos[i].Eta(), truthParticles_ent[i].Eta(),
                    truthParticles[i].Phi(), truthParticles_pos[i].Phi(), truthParticles_ent[i].Phi(),
                    truthParticles[i].Theta(), truthParticles_pos[i].Theta(), truthParticles_ent[i].Theta(), truthParticles_ent[i].Theta()-truthParticles_pos[i].Theta(),
                    TruthParticle_n,MuEntry_Particle_n,vertex[i]);
        Event_Info[std::make_pair(event,i)] = particle_info;
      }

      //Loop over entries, which has digitization info for each event
      unsigned int ient=0;
      for (auto it=entries.begin(); it!=entries.end(); it++){

        /* Identifying the wedge from digits:
         * now the digit is associated with the corresponding station, so lambda function can be exploited to check if they are all the same
         */
        double tru_phi = -999, tru_theta = -999;
        std::pair<int, unsigned int> pair (event,ient);
        auto tru_it = Event_Info.find(pair);
        if (tru_it != Event_Info.end()) {
          tru_phi = tru_it->second.phi_pos;
          tru_theta = tru_it->second.theta_pos;
        }

        std::string station = it->second[0].stName;
        std::vector<hitData_entry> hit_info;
        hit_info.reserve(it->second.size());

        //Now we need to loop on digits
        for (const auto &dW : it->second) {
          Identifier tmpID      = dW.id();
          int thisMultiplet     = m_MmIdHelper->multilayer( tmpID );
          int thisGasGap        = m_MmIdHelper->gasGap( tmpID );
          int thisTime          = dW.digit->stripResponseTime();
          int thisCharge        = 2; //dW.digit->stripChargeForTrigger().at(0);
          int thisStripPosition = m_MmIdHelper->channel(tmpID);
          double thisLocalPosX  = dW.strip_lpos.X();
          int thisVMM           = m_MmIdHelper->channel( tmpID);
          int thisMMFE_VMM      = m_MmIdHelper->channel( tmpID);
          int thisStationEta    = m_MmIdHelper->stationEta( tmpID );
          int thisStationPhi    = m_MmIdHelper->stationPhi( tmpID );
          int thisPlane = (thisMultiplet-1)*4+thisGasGap-1;
          int BC_id = std::ceil( thisTime / 25. );
          ROOT::Math::XYZVector mazin_check(
            dW.strip_gpos.X(),
            dW.strip_gpos.Y(),
            dW.strip_gpos.Z()
            );

          hitData_entry hit_entry(event,
                               thisTime,
                               thisCharge,
                               thisVMM,
                               thisMMFE_VMM,
                               thisPlane,
                               thisStripPosition,
                               thisStationEta,
                               thisStationPhi,
                               thisMultiplet,
                               thisGasGap,
                               thisLocalPosX,
                               tru_theta,
                               tru_phi,
                               true,
                               BC_id,
                               mazin_check,
                               mazin_check);

          hit_info.push_back(hit_entry);
          ATH_MSG_DEBUG("Done filling hit_info structure");
        }//end digit wrapper loop

        Hits_Data_Set_Time[std::make_pair(event,ient)] = hit_info;
        ient++;
      }
    return StatusCode::SUCCESS;
}
