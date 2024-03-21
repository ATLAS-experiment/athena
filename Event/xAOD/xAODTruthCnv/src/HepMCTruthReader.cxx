/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "StoreGate/ReadHandle.h"

#include "GeneratorObjects/McEventCollection.h"
#include "HepMCTruthReader.h"
#include "TruthUtils/HepMCHelpers.h"

using std::cout;
using std::endl;

HepMCTruthReader::HepMCTruthReader(const std::string& name, ISvcLocator* svcLoc)
  : AthReentrantAlgorithm(name, svcLoc)
{}


StatusCode HepMCTruthReader::initialize() {
  ATH_MSG_INFO("HepMC container name = " << m_hepMCContainerKey );

  // initialize handles
  ATH_CHECK(m_hepMCContainerKey.initialize());
  ATH_MSG_DEBUG("HepMCContainerKey = " << m_hepMCContainerKey.key() );
  return StatusCode::SUCCESS;
}


StatusCode HepMCTruthReader::execute(const EventContext& ctx) const {

  // Retrieve the HepMC truth:
  SG::ReadHandle<McEventCollection> mcColl(m_hepMCContainerKey, ctx);
  // validity check is only really needed for serial running. Remove when MT is only way.
  if (!mcColl.isValid()) {
    ATH_MSG_ERROR("Could not retrieve HepMC with key:" << m_hepMCContainerKey.key());
    if (m_hepMCContainerKey.key() == "GEN_EVENT") ATH_MSG_ERROR("Try to set 'HepMCContainerKey' to 'TruthEvent'");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG( "Retrieved HepMC with key: " << m_hepMCContainerKey.key() );
  }

  ATH_MSG_INFO("Number of pile-up events in this Athena event: " << mcColl->size()-1);

  // Loop over events
  for (unsigned int cntr = 0; cntr < mcColl->size(); ++cntr) {
    const HepMC::GenEvent* genEvt = (*mcColl)[cntr]; 

    // Print the event particle/vtx contents
    if (cntr==0) ATH_MSG_INFO("Printing signal event...");
    if (cntr>0) ATH_MSG_INFO("Printing pileup events...");  
    printEvent(genEvt, m_do4momPtEtaPhi);

  }

  return StatusCode::SUCCESS;
}


// Print method for event - mimics the HepMC dump.
// Vertex print method called within here
void HepMCTruthReader::printEvent(const HepMC::GenEvent* event, bool do4momPtEtaPhi) {
  cout << "--------------------------------------------------------------------------------\n";
  cout << "GenEvent: #" << "NNN" << "\n";
  cout << " Entries this event: " << event->vertices_size() << " vertices, " << event->particles_size() << " particles.\n";
  cout << "                                    GenParticle Legend\n";
  if (do4momPtEtaPhi) cout << "        Barcode   PDG ID      ( pt,      eta,      phi,     E ) Stat  DecayVtx\n";
  else                cout << "        Barcode   PDG ID      ( Px,       Py,       Pz,     E ) Stat  DecayVtx\n";    
  cout << "--------------------------------------------------------------------------------\n";
#ifdef HEPMC3
  for (const auto& iv: event->vertices()) {  printVertex(iv, do4momPtEtaPhi);  } 
#else
  for (HepMC::GenEvent::vertex_const_iterator iv = event->vertices_begin(); iv != event->vertices_end(); ++iv) {
    printVertex(*iv, do4momPtEtaPhi);
  }
#endif
  cout << "--------------------------------------------------------------------------------\n";
}

// Print method for vertex - mimics the HepMC dump.
// Particle print method called within here
void HepMCTruthReader::printVertex(const HepMC::ConstGenVertexPtr& vertex, bool do4momPtEtaPhi) {
  if (!vertex) return;
  std::ios::fmtflags f( cout.flags() ); 
  cout << "GenVertex (" << vertex << "):";
  if (HepMC::barcode(vertex) != 0) {
    if (vertex->position().x() != 0.0 && vertex->position().y() != 0.0 && vertex->position().z() != 0.0) {
      cout.width(9);
      cout <<HepMC::barcode(vertex);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT)=";
      cout.width(9);
      cout.precision(2);
      cout.setf(std::ios::scientific, std::ios::floatfield);
      cout.setf(std::ios_base::showpos);
      cout << vertex->position().x() << ",";
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().y() << ",";
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().z() << ",";
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().t();
      cout.setf(std::ios::fmtflags(0), std::ios::floatfield);
      cout.unsetf(std::ios_base::showpos);
      cout << endl;
    } else {
      cout.width(9);
      cout << HepMC::barcode(vertex);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT): 0";
      cout << endl;
    }
  } else {
    // If the vertex doesn't have a unique barcode assigned, then
    //  we print its memory address instead... so that the
    //  print out gives us a unique tag for the particle.
    if (vertex->position().x() != 0.0 && vertex->position().y() != 0.0 && vertex->position().z() != 0.0) {
      cout.width(9);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT)=";
      cout.width(9);
      cout.precision(2);
      cout.setf(std::ios::scientific, std::ios::floatfield);
      cout.setf(std::ios_base::showpos);
      cout << vertex->position().x();
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().y();
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().z();
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().t();
      cout.setf(std::ios::fmtflags(0), std::ios::floatfield);
      cout.unsetf(std::ios_base::showpos);
      cout << endl;
    } else {
      cout.width(9);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT):0";
      cout << endl;
    }
  }

  // Print out all the incoming, then outgoing particles
#ifdef HEPMC3
  for (const auto&  iPIn: vertex->particles_in()) {       
    if ( iPIn == vertex->particles_in().front() ) {
      cout << " I: ";
      cout.width(2);
      cout << vertex->particles_in().size();
    } else cout << "      ";
    printParticle(iPIn, do4momPtEtaPhi);
  }
  for (const auto& iPOut: vertex->particles_out()) {
    if ( iPOut == vertex->particles_out().front()) {
      cout << " O: ";
      cout.width(2);
      cout << vertex->particles_out().size();
    } else cout << "      ";
    printParticle(iPOut, do4momPtEtaPhi);
  }  
#else  
  for (HepMC::GenVertex::particles_in_const_iterator iPIn = vertex->particles_in_const_begin();
       iPIn != vertex->particles_in_const_end(); ++iPIn) {       
    if ( iPIn == vertex->particles_in_const_begin() ) {
      cout << " I: ";
      cout.width(2);
      cout << vertex->particles_in_size();
    } else cout << "      ";
    printParticle(*iPIn, do4momPtEtaPhi);
  }
  for (HepMC::GenVertex::particles_out_const_iterator iPOut = vertex->particles_out_const_begin();
       iPOut != vertex->particles_out_const_end(); ++iPOut) {
    if ( iPOut == vertex->particles_out_const_begin() ) {
      cout << " O: ";
      cout.width(2);
      cout << vertex->particles_out_size();
    } else cout << "      ";
    printParticle(*iPOut, do4momPtEtaPhi);
  }

#endif
  cout.flags(f); 
}


// Print method for particle - mimics the HepMC dump.
void HepMCTruthReader::printParticle(const HepMC::ConstGenParticlePtr& particle, bool do4momPtEtaPhi) {
  if (!particle) return;
  std::ios::fmtflags f( cout.flags() ); 
  cout << " ";
  cout.width(9);
  cout << HepMC::barcode(particle);
  cout.width(9);
  cout << particle->pdg_id() << " ";
  cout.width(9);
  cout.precision(2);
  cout.setf(std::ios::scientific, std::ios::floatfield);
  cout.setf(std::ios_base::showpos);
  if (do4momPtEtaPhi) cout << particle->momentum().perp() << ",";
  else                  cout << particle->momentum().px() << ",";
  cout.width(9);
  cout.precision(2);
  if (do4momPtEtaPhi) cout << particle->momentum().pseudoRapidity() << ",";
  else                  cout << particle->momentum().py() << ",";
  cout.width(9);
  cout.precision(2);
  if (do4momPtEtaPhi) cout << particle->momentum().phi() << ",";
  else                  cout << particle->momentum().pz() << ",";
  cout.width(9);
  cout.precision(2);
  cout << particle->momentum().e() << " ";
  cout.setf(std::ios::fmtflags(0), std::ios::floatfield);
  cout.unsetf(std::ios_base::showpos);
  if ( MC::isDecayed(particle) ) {
    if ( HepMC::barcode(particle->end_vertex())!=0 ) {
      cout.width(3);
      cout << particle->status() << " ";
      cout.width(9);
      cout << HepMC::barcode(particle->end_vertex());
    }
  } else {
    cout.width(3);
    cout << particle->status();
  }
  cout << endl;
  cout.flags(f); 
}


