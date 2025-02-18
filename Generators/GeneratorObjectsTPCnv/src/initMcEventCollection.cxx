/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeneratorObjectsTPCnv/initMcEventCollection.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

// CLHEP includes
#include "CLHEP/Units/SystemOfUnits.h"

#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p1.h"
#include "StoreGate/WriteHandle.h"
#include "GeneratorObjects/McEventCollection.h"
#include "TruthUtils/MagicNumbers.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "TestTools/initGaudi.h"

namespace Athena_test {
  bool initMcEventCollection(ISvcLocator*& pSvcLoc, std::vector<HepMC::GenParticlePtr>& genPartList, const bool initGaudi)
  {
    if (initGaudi && !Athena_test::initGaudi(pSvcLoc)) {
      std::cerr << "This test can not be run" << std::endl;
      return false;
    }
    // create dummy input McEventCollection with a name that
    // HepMcParticleLink knows about
    SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
    inputTestDataHandle = std::make_unique<McEventCollection>();

    // create a dummy EventContext
    EventContext ctx;
    ctx.setExtension( Atlas::ExtendedEventContext( SG::CurrentEventStore::store() ) );
    Gaudi::Hive::setCurrentContext( ctx );

    // Add a dummy GenEvent
    const int process_id1(20);
    const int event_number1(17);
    inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
    HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
    populateGenEvent(ge1,-11,11,genPartList);
    populateGenEvent(ge1,-13,13,genPartList);
    populateGenEvent(ge1,-11,11,genPartList);
    populateGenEvent(ge1,-13,13,genPartList);
    populateGenEvent(ge1,-11,11,genPartList);
    populateGenEvent(ge1,22,22,genPartList);
    inputTestDataHandle->push_back(new HepMC::GenEvent(ge1));
    HepMC::GenEvent& ge2 = *(inputTestDataHandle->at(1));
    const int event_number2(89);
    ge2.set_event_number(event_number2);
    populateFilteredGenEvent(ge2,genPartList);
    return true;
  }

  int maximumBarcode(std::vector<HepMC::GenParticlePtr>& genPartList)
  {
    int maxBarcode{0};
    if (genPartList.empty()) { return maxBarcode; }
    for (HepMC::GenParticlePtr genPart : genPartList)
      {
        maxBarcode = std::max(maxBarcode, HepMC::barcode(genPart));
      }
    return maxBarcode;
  }

  void populateGenEvent(HepMC::GenEvent & ge, int pdgid1, int pdgid2, std::vector<HepMC::GenParticlePtr>& genPartList)
  {
    int maxBarcode = maximumBarcode(genPartList);
    HepMC::FourVector myPos( 0.0, 0.0, 0.0, 0.0);
    HepMC::GenVertexPtr myVertex = HepMC::newGenVertexPtr( myPos, -1 );
    HepMC::FourVector fourMomentum1( 0.0, 0.0, 1.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle1 = HepMC::newGenParticlePtr(fourMomentum1, pdgid1, 2);
    myVertex->add_particle_in(inParticle1);
    HepMC::FourVector fourMomentum2( 0.0, 0.0, -1.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle2 = HepMC::newGenParticlePtr(fourMomentum2, pdgid2, 2);
    myVertex->add_particle_in(inParticle2);
    HepMC::FourVector fourMomentum3( 0.0, 1.0, 0.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle3 = HepMC::newGenParticlePtr(fourMomentum3, pdgid1, 1);
    myVertex->add_particle_out(inParticle3);
    genPartList.push_back(inParticle3);
    HepMC::FourVector fourMomentum4( 0.0, -1.0, 0.0, 1.0*CLHEP::TeV);
    HepMC::GenParticlePtr inParticle4 = HepMC::newGenParticlePtr(fourMomentum4, pdgid2, 1);
    myVertex->add_particle_out(inParticle4);
    genPartList.push_back(inParticle4);
    ge.add_vertex( myVertex );
    HepMC::suggest_barcode(inParticle1,maxBarcode+1);
    HepMC::suggest_barcode(inParticle2,maxBarcode+2);
    HepMC::suggest_barcode(inParticle3,maxBarcode+3);
    HepMC::suggest_barcode(inParticle4,maxBarcode+4);
    HepMC::set_signal_process_vertex(&ge, myVertex );
    ge.set_beam_particles(inParticle1,inParticle2);
  }

  void populateFilteredGenEvent(HepMC::GenEvent & ge, std::vector<HepMC::GenParticlePtr>& genPartVector)
  {
    //.......Create new particle (geantino) to link  hits from pileup
    HepMC::GenParticlePtr genPart=HepMC::newGenParticlePtr();
    genPart->set_pdg_id(999); //Geantino
    genPart->set_status(1); //!< set decay status
    HepMC::suggest_barcode(genPart, HepMC::SUPPRESSED_PILEUP_BARCODE );

    HepMC::GenVertexPtr genVertex=HepMC::newGenVertexPtr();
    genVertex->add_particle_out(genPart);
    genPartVector.push_back(genPart);

    //to set geantino vertex as a truth primary vertex
    HepMC::GenVertexPtr  hScatVx = HepMC::barcode_to_vertex(&ge,-3);
    if (hScatVx!=nullptr) {
      HepMC::FourVector pmvxpos=hScatVx->position();
      genVertex->set_position(pmvxpos);
      //to set geantino kinematic phi=eta=0, E=p=E_hard_scat
#ifdef HEPMC3
      auto itrp =hScatVx->particles_in().cbegin();
      if (hScatVx->particles_in().size()==2) {
        HepMC::FourVector mom1=(*itrp)->momentum();
        HepMC::FourVector mom2=(*(++itrp))->momentum();
        HepMC::FourVector vxmom;
        vxmom.setPx(mom1.e()+mom2.e());
        vxmom.setPy(0.);
        vxmom.setPz(0.);
        vxmom.setE(mom1.e()+mom2.e());
        genPart->set_momentum(vxmom);
      }
#else
      HepMC::GenVertex::particles_in_const_iterator itrp =hScatVx->particles_in_const_begin();
      if (hScatVx->particles_in_size()==2) {
        HepMC::FourVector mom1=(*itrp)->momentum();
        HepMC::FourVector mom2=(*(++itrp))->momentum();
        HepMC::FourVector vxmom;
        vxmom.setPx(mom1.e()+mom2.e());
        vxmom.setPy(0.);
        vxmom.setPz(0.);
        vxmom.setE(mom1.e()+mom2.e());
        genPart->set_momentum(vxmom);
      }
#endif
    }

#ifdef HEPMC3
    if (!ge.vertices().empty()) {
      std::vector<HepMC::GenVertexPtr> vtxvec;
      for (const auto& vtx: ge.vertices()) {
        vtxvec.push_back(vtx);
        ge.remove_vertex(vtx);
      }
      vtxvec.clear();
    }
#else
    if (!ge.vertices_empty()) {
      HepMC::GenEvent::vertex_iterator itvtx = ge.vertices_begin();
      while (itvtx != ge.vertices_end()) {
        HepMC::GenVertexPtr pvtx = *itvtx;
        ++itvtx;
        ge.remove_vertex(pvtx);
        delete pvtx;
      }
    }
#endif

    //.....add new vertex with geantino
    ge.add_vertex(std::move(genVertex));
    HepMC::suggest_barcode(genPart, HepMC::SUPPRESSED_PILEUP_BARCODE );
  }
}
