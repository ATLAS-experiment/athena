/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "StoreGate/ReadHandle.h"


#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthEventAuxContainer.h"

#include "xAODTruth/TruthPileupEvent.h"
#include "xAODTruth/TruthPileupEventContainer.h"
#include "xAODTruth/TruthPileupEventAuxContainer.h"
#include "TruthUtils/MagicNumbers.h"

#include "xAODTruthReader.h"

using namespace std;

namespace xAODReader {


  xAODTruthReader::xAODTruthReader(const string& name, ISvcLocator* svcLoc)
    : AthReentrantAlgorithm(name, svcLoc)
  {}


  StatusCode xAODTruthReader::initialize() {

    // initialize handles
    ATH_MSG_INFO("TruthContainerKey = " << m_xaodTruthEventContainerKey.key() );
    ATH_CHECK(m_xaodTruthEventContainerKey.initialize());
    // ATH_MSG_INFO("TruthPileupContainerKey = " << m_xaodTruthPUEventContainerKey.key() );
    // ATH_CHECK(m_xaodTruthPUEventContainerKey.initialize());
    return StatusCode::SUCCESS;

  }

  StatusCode xAODTruthReader::execute(const EventContext& ctx) const {
    // Retrieve the xAOD truth:
    /// @todo Can the main truth event be a singleton, not a container?

    SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer(m_xaodTruthEventContainerKey, ctx);
    if (xTruthEventContainer.isValid()) {
      ATH_MSG_INFO("Found " << m_xaodTruthEventContainerKey.key());
    }
    else {
      ATH_MSG_ERROR("Could NOT find " << m_xaodTruthEventContainerKey.key());
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Number of signal events in this Athena event: " << xTruthEventContainer->size());

    // Signal process loop
    ATH_MSG_INFO("Printing signal event...");
    for (const xAOD::TruthEvent* evt : *xTruthEventContainer) {
      cout << endl << endl;

      // Print hard-scattering info
      const xAOD::TruthVertex* vtx = evt->signalProcessVertex();
      ATH_MSG_INFO("Signal process vertex: " << vtx);
      if (vtx) ATH_MSG_INFO("Poistion = (" << vtx->x() << ", " << vtx->y() << ", " << vtx->z() << ")");
      else     ATH_MSG_INFO("Position n.a.");
      // Print the event particle/vtx contents
      printEvent(evt, m_do4momPtEtaPhi);

    }

    // if (m_doPUEventPrintout) {

    //   SG::ReadHandle<xAOD::TruthPileupEventContainer> xTruthPUEventContainer(m_xaodTruthPUEventContainerKey, ctx);
    //   if (!xTruthPUEventContainer.isValid()) {
    //     ATH_MSG_INFO("Found " << m_xaodTruthPUEventContainerKey.key());
    //   }
    //   else {
    //     ATH_MSG_ERROR("Could NOT find " << m_xaodTruthPUEventContainerKey.key());
    //     return StatusCode::FAILURE;
    //   }

    //   ATH_MSG_INFO("Number of pile-up events in this Athena event: " << xTruthPUEventContainer->size());

    //   // Pile-up loop
    //   ATH_MSG_INFO("Printing pileup events...");
    //   for (const xAOD::TruthPileupEvent* evt : *xTruthPUEventContainer) {
    //     cout << endl << endl;
    //     printEvent(evt, m_do4momPtEtaPhi);
    //   }
    // }

    return StatusCode::SUCCESS;
  }


  // Print method for event - mimics the HepMC dump.
  // Vertex print method called within here
  void xAODTruthReader::printEvent(const xAOD::TruthEventBase* event, bool do4momPtEtaPhi) {
    cout << "--------------------------------------------------------------------------------\n";
    cout << "GenEvent: #" << "NNN" << "\n";
    cout << " Entries this event: " << event->nTruthVertices() << " vertices, " << event->nTruthParticles() << " particles.\n";
    cout << "                                    GenParticle Legend\n";
    if (do4momPtEtaPhi) cout << "        UniqueID   PDG ID      ( pt,      eta,      phi,     E ) Stat  DecayVtx\n";
    else                cout << "        UniqueID   PDG ID      ( Px,       Py,       Pz,     E ) Stat  DecayVtx\n";
    cout << "--------------------------------------------------------------------------------\n";
    for (unsigned int iv = 0; iv < event->nTruthVertices(); ++iv) {
      printVertex(event->truthVertex(iv), do4momPtEtaPhi);
    }
    cout << "--------------------------------------------------------------------------------\n";
  }

  // Print method for vertex - mimics the HepMC dump.
  // Particle print method called within here
  void xAODTruthReader::printVertex(const xAOD::TruthVertex* vertex, bool do4momPtEtaPhi) {
    std::ios::fmtflags f( cout.flags() );
    if (vertex) {
      cout << "TruthVertex:";
      if (HepMC::uniqueID(vertex) != HepMC::UNDEFINED_ID) {
        if (vertex->x() != 0.0 && vertex->y() != 0.0 && vertex->z() != 0.0) {
          cout.width(9);
          cout << HepMC::uniqueID(vertex);
          cout << " Status:";
          cout.width(5);
          cout << HepMC::status(vertex);
          cout << " (X,cT)=";
          cout.width(9);
          cout.precision(2);
          cout.setf(ios::scientific, ios::floatfield);
          cout.setf(ios_base::showpos);
          cout << vertex->x() << ",";
          cout.width(9);
          cout.precision(2);
          cout << vertex->y() << ",";
          cout.width(9);
          cout.precision(2);
          cout << vertex->z() << ",";
          cout.width(9);
          cout.precision(2);
          cout << vertex->t();
          cout.setf(ios::fmtflags(0), ios::floatfield);
          cout.unsetf(ios_base::showpos);
          cout << endl;
        } else {
          cout.width(9);
          cout << HepMC::uniqueID(vertex);
          cout << " Status:";
          cout.width(5);
          cout << HepMC::status(vertex);
          cout << " (X,cT): 0";
          cout << endl;
        }
      } else {
        // If the vertex doesn't have a unique ID assigned, then
        //  we print its memory address instead... so that the
        //  print out gives us a unique tag for the particle.
        if (vertex->x() != 0.0 && vertex->y() != 0.0 && vertex->z() != 0.0) {
          cout.width(9);
          cout << (void*)vertex;
          cout << " Status:";
          cout.width(5);
          cout << HepMC::status(vertex);
          cout << " (X,cT)=";
          cout.width(9);
          cout.precision(2);
          cout.setf(ios::scientific, ios::floatfield);
          cout.setf(ios_base::showpos);
          cout << vertex->x();
          cout.width(9);
          cout.precision(2);
          cout << vertex->y();
          cout.width(9);
          cout.precision(2);
          cout << vertex->z();
          cout.width(9);
          cout.precision(2);
          cout << vertex->t();
          cout.setf(ios::fmtflags(0), ios::floatfield);
          cout.unsetf(ios_base::showpos);
          cout << endl;
        } else {
          cout.width(9);
          cout << (void*)vertex;
          cout << " Status:";
          cout.width(5);
          cout << HepMC::status(vertex);
          cout << " (X,cT):0";
          cout << endl;
        }
      }
      // Print out all the incoming, then outgoing particles
      for (unsigned int iPIn = 0; iPIn<vertex->nIncomingParticles(); ++iPIn) {
        if ( iPIn == 0 ) {
          cout << " I: ";
          cout.width(2);
          cout << vertex->nIncomingParticles();
        } else cout << "      ";
        printParticle(vertex->incomingParticle(iPIn), do4momPtEtaPhi);
      }
      for (unsigned int iPOut = 0; iPOut<vertex->nOutgoingParticles(); ++iPOut) {
        if ( iPOut == 0 ) {
          cout << " O: ";
          cout.width(2);
          cout << vertex->nOutgoingParticles();
        } else cout << "      ";
        printParticle(vertex->outgoingParticle(iPOut), do4momPtEtaPhi);
      }
    }
    cout.flags(f);
  }


  // Print method for particle - mimics the HepMC dump.
  void xAODTruthReader::printParticle(const xAOD::TruthParticle* particle, bool do4momPtEtaPhi) {
    std::ios::fmtflags f( cout.flags() );
    if (particle) {
      cout << " ";
      cout.width(9);
      cout << HepMC::uniqueID(particle);
      cout.width(9);
      cout << particle->pdgId() << " ";
      cout.width(9);
      cout.precision(2);
      cout.setf(ios::scientific, ios::floatfield);
      cout.setf(ios_base::showpos);
      if (do4momPtEtaPhi) cout << particle->pt() << ",";
      else                cout << particle->px() << ",";
      cout.width(9);
      cout.precision(2);
      if (do4momPtEtaPhi) cout << particle->eta() << ",";
      else                cout << particle->py() << ",";
      cout.width(9);
      cout.precision(2);
      if (do4momPtEtaPhi) cout << particle->phi() << ",";
      else                cout << particle->pz() << ",";
      cout.width(9);
      cout.precision(2);
      cout << particle->e() << " ";
      cout.setf(ios::fmtflags(0), ios::floatfield);
      cout.unsetf(ios_base::showpos);
      if ( particle->hasDecayVtx() ) {
        if ( HepMC::uniqueID(particle->decayVtx()) != HepMC::UNDEFINED_ID ) {
          cout.width(3);
          cout << particle->status() << " ";
          cout.width(9);
          cout << HepMC::uniqueID(particle->decayVtx());
        }
      } else {
        cout.width(3);
        cout << particle->status();
      }
    }
    cout << endl;
    cout.flags(f);
  }


} // namespace xAODReader
