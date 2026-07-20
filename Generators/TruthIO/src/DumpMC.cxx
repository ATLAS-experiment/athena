/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthIO/DumpMC.h"
#include "GaudiKernel/PhysicalConstants.h"
#include <CLHEP/Vector/LorentzVector.h>
using namespace Gaudi::Units;


DumpMC::DumpMC(const std::string& name, ISvcLocator* pSvcLocator)
  : GenBase(name, pSvcLocator)
{
  declareProperty("McEventOutputKey", m_keyout="GEN_EVENT");
  declareProperty("VerboseOutput", m_VerboseOutput=true);
  declareProperty("DeepCopy", m_DeepCopy=false);
  declareProperty("EtaPhi", m_EtaPhi=false);
  declareProperty("PrintQuasiStableParticles", m_PrintQuasiStableParticles=false);
}


StatusCode DumpMC::initialize() {
  CHECK(GenBase::initialize());
  if (m_mcEventKey == m_keyout && m_DeepCopy) {
    ATH_MSG_FATAL("Input and output MC event keys cannot be the same (" << m_mcEventKey << ")");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}


StatusCode DumpMC::execute(const EventContext& ctx) {
  if (m_DeepCopy) {
    McEventCollection* mcCollptra = new McEventCollection();
    // Fill the new McEventCollection with a copy of the initial HepMC::GenEvent
    for(const HepMC::GenEvent* evt : *events_const(ctx)) {
      mcCollptra->push_back(new HepMC::GenEvent(*evt));
    }
    // Loop over all events in McEventCollection
    for (McEventCollection::iterator evt = mcCollptra->begin(); evt != mcCollptra->end(); ++evt) {
    std::vector<HepMC::GenVertexPtr> lambda_vertices_to_remove;
    for (auto part: **evt){
    const int abspid = std::abs(part->pdg_id());
     if (abspid != 310 && abspid != 3122 && abspid != 3222 && abspid != 3112 && abspid != 3322 && abspid != 3312 && abspid != 3334 ) continue;
     if (!part->end_vertex()) continue;
     lambda_vertices_to_remove.push_back(part->end_vertex());
     part->set_status(1);
    }
    for (auto v: lambda_vertices_to_remove)  (*evt)->remove_vertex(std::move(v));
    }

    if (evtStore()->record(mcCollptra, m_keyout).isFailure()){
      ATH_MSG_ERROR("Could not record McEventCollection");
      return StatusCode::FAILURE;
    }
  }

  // Loop over all events in McEventCollection
  for(const HepMC::GenEvent* evt : *events_const(ctx)) {
    auto pdfinfo = evt->pdf_info();
    auto ion = evt->heavy_ion();
    if (pdfinfo) {
      std::cout << "PdfInfo: "
                << pdfinfo->parton_id[0] << ", "
                << pdfinfo->parton_id[1] << ", "
                << pdfinfo->x[0] << ", "
                << pdfinfo->x[1] << ", "
                << pdfinfo->scale << ", "
                << pdfinfo->xf[0] << ", "
                << pdfinfo->xf[1] << ", "
                << pdfinfo->pdf_id[0] << ", "
                << pdfinfo->pdf_id[1]
                <<      std::endl;
    }

    if (ion) {
      std::cout << std::endl;
      std::cout << "Heavy Ion: "
                << ion->Ncoll_hard <<", "
                << ion->Npart_proj <<" , "
                << ion->Npart_targ<< ", "
                << ion->Ncoll<< ", "
                << ion->spectator_neutrons << ", "
                << ion->spectator_protons << ", " 
                << ion->N_Nwounded_collisions << ", "
                << ion->Nwounded_N_collisions << ", "
                << ion->Nwounded_Nwounded_collisions << ", "
                << ion->impact_parameter << ", "
                << ion->event_plane_angle << ", "
                << ion->eccentricity << ", "
                << ion->sigma_inel_NN 
                << std::endl;
                                               }
    if (m_VerboseOutput) {
      if (!m_EtaPhi) {
        HepMC::Print::line(std::cout,*evt); // standard HepMc dump
      } else { // sort particles by rapidity and then dump
        // Loop over all particles in the event and build up the grid
        for (auto part: *evt) {
          double rapid =part->momentum().pseudoRapidity();
          double phi =  part->momentum().phi(); //phi is in range -pi to pi
          double et=part->momentum().perp();
          int p_stat=part->status();
          int p_id = part->pdg_id();
          std::cout << " eta = " << rapid<< "  Phi = " << phi << "   Et = " <<et/GeV << "  Status= " << p_stat << " PDG ID= "<< p_id << std::endl;
        }
      }
      if(m_PrintQuasiStableParticles) {
        for (const auto& pitr: *evt) {
          int p_stat=pitr->status();
          if(p_stat==2 && pitr->production_vertex() && pitr->end_vertex()) {
            const auto& prodVtx = pitr->production_vertex()->position();
            const auto& endVtx = pitr->end_vertex()->position();
            const CLHEP::HepLorentzVector lv0( prodVtx.x(), prodVtx.y(), prodVtx.z(), prodVtx.t() );
            const CLHEP::HepLorentzVector lv1( endVtx.x(), endVtx.y(), endVtx.z(), endVtx.t() );
            
            CLHEP::HepLorentzVector dist4D(lv1);
            dist4D-=lv0;
            CLHEP::Hep3Vector dist3D=dist4D.vect();
            if(dist3D.mag()>1*Gaudi::Units::mm) { 
              const auto& GenMom = pitr->momentum();
              const CLHEP::HepLorentzVector mom( GenMom.x(), GenMom.y(), GenMom.z(), GenMom.t() );
              ATH_MSG_INFO("Quasi stable particle "<<pitr);
              ATH_MSG_INFO("  Prod  V:"<<pitr->production_vertex ());
              ATH_MSG_INFO("  Decay V:"<<pitr->end_vertex ());
              ATH_MSG_INFO("  gamma(Momentum)="<<mom.gamma()<<" gamma(Vertices)="<<dist4D.gamma());
            }  
          }
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}
