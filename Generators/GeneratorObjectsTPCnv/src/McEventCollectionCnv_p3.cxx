///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv_p3.cxx
// Implementation file for class McEventCollectionCnv_p3
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////


// STL includes
#include <utility>
#include <cmath>

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p3.h"
#include "HepMcDataPool.h"

///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////


McEventCollectionCnv_p3::McEventCollectionCnv_p3() :
  Base_t( )
{}

McEventCollectionCnv_p3::McEventCollectionCnv_p3( const McEventCollectionCnv_p3& rhs )

= default;

McEventCollectionCnv_p3&
McEventCollectionCnv_p3::operator=( const McEventCollectionCnv_p3& rhs )
{
  if ( this != &rhs ) {
    Base_t::operator=( rhs );
  }
  return *this;
}

///////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////

McEventCollectionCnv_p3::~McEventCollectionCnv_p3()
= default;


void McEventCollectionCnv_p3::persToTrans( const McEventCollection_p3* persObj,
                                           McEventCollection* transObj,
                                           MsgStream& msg )
{
  msg << MSG::DEBUG << "Loading McEventCollection from persistent state..."
      << endmsg;

  // elements are managed by DataPool
  transObj->clear(SG::VIEW_ELEMENTS);
  HepMC::DataPool datapools;
  const unsigned int nVertices = persObj->m_genVertices.size();
  datapools.vtx.prepareToAdd(nVertices);
  const unsigned int nParts = persObj->m_genParticles.size();
  datapools.part.prepareToAdd(nParts);
  const unsigned int nEvts = persObj->m_genEvents.size();
  datapools.evt.prepareToAdd(nEvts);

  transObj->reserve( nEvts );
  for ( std::vector<GenEvent_p3>::const_iterator
          itr = persObj->m_genEvents.begin(),
          itrEnd = persObj->m_genEvents.end();
        itr != itrEnd;
        ++itr ) {
    const GenEvent_p3& persEvt = *itr;

    HepMC::GenEvent * genEvt        = datapools.getGenEvent();
    genEvt->add_attribute (HepMCStr::barcodes, std::make_shared<HepMC::GenEventBarcodes>());
    genEvt->add_attribute(HepMCStr::signal_process_id,std::make_shared<HepMC3::IntAttribute>(persEvt.m_signalProcessId));
    genEvt->set_event_number(persEvt.m_eventNbr);
    genEvt->add_attribute(HepMCStr::event_scale,std::make_shared<HepMC3::DoubleAttribute>(persEvt.m_eventScale));
    genEvt->add_attribute(HepMCStr::alphaQCD,std::make_shared<HepMC3::DoubleAttribute>(persEvt.m_alphaQCD));
    genEvt->add_attribute(HepMCStr::alphaQED,std::make_shared<HepMC3::DoubleAttribute>(persEvt.m_alphaQED));
    genEvt->weights()= persEvt.m_weights;
    genEvt->add_attribute(HepMCStr::random_states,std::make_shared<HepMC3::VectorLongIntAttribute>(persEvt.m_randomStates));
    transObj->push_back( genEvt );

    ParticlesMap_t partToEndVtx( (persEvt.m_particlesEnd- persEvt.m_particlesBegin)/2 );

    // create the vertices
    const unsigned int endVtx = persEvt.m_verticesEnd;
    for ( unsigned int iVtx= persEvt.m_verticesBegin; iVtx != endVtx; ++iVtx ) {
      createGenVertex( *persObj, persObj->m_genVertices[iVtx],partToEndVtx, datapools, genEvt );
    }

    // set the signal process vertex
    const int sigProcVtx = persEvt.m_signalProcessVtx;
    if ( sigProcVtx != 0 ) {
      auto Vtx=HepMC::barcode_to_vertex(genEvt, sigProcVtx );
      HepMC::set_signal_process_vertex(genEvt, Vtx );
    }

    // connect particles to their end vertices
    for (auto & p : partToEndVtx) {
      auto decayVtx=HepMC::barcode_to_vertex(genEvt, p.second );
      if ( decayVtx ) {
        decayVtx->add_particle_in( p.first );
      } else {
        msg << MSG::ERROR
            << "GenParticle points to null end vertex !!"
            << endmsg;
      }
    }
  } //> end loop over m_genEvents

  msg << MSG::DEBUG << "Loaded McEventCollection from persistent state [OK]"
      << endmsg;
}

void McEventCollectionCnv_p3::transToPers( const McEventCollection* /*transObj*/,
                                           McEventCollection_p3* /*persObj*/,
                                           MsgStream& msg )
{
  msg << MSG::ERROR
      << "This transient-to-persistent converter method has been RETIRED !!"
      << endmsg
      << "You are not supposed to end-up here ! Go away !"
      << endmsg;

  throw std::runtime_error( "Retired McEventCollectionCnv_p3::transToPers() !!" );
}


HepMC::GenVertexPtr
McEventCollectionCnv_p3::createGenVertex( const McEventCollection_p3& persEvt,
                                          const GenVertex_p3& persVtx,
                                          ParticlesMap_t& partToEndVtx,
                                          HepMC::DataPool& datapools, HepMC::GenEvent* parent )
{
  HepMC::GenVertexPtr vtx = datapools.getGenVertex();
  if (parent) parent->add_vertex(vtx);

  vtx->set_position( HepMC::FourVector(persVtx.m_x,persVtx.m_y,persVtx.m_z,persVtx.m_t) );
  vtx->set_status(persVtx.m_id);
  // cast from std::vector<float> to std::vector<double>
  std::vector<double> weights( persVtx.m_weights.begin(), persVtx.m_weights.end() );
  vtx->add_attribute(HepMCStr::weights,std::make_shared<HepMC3::VectorDoubleAttribute>(weights));
  HepMC::suggest_barcode(vtx,persVtx.m_barcode);
  // handle the in-going (orphans) particles
  //Is this needed for HEPMC3?
  const unsigned int nPartsIn = persVtx.m_particlesIn.size();
  for ( unsigned int i = 0; i != nPartsIn; ++i ) {
    createGenParticle( persEvt.m_genParticles[persVtx.m_particlesIn[i]], partToEndVtx, datapools, vtx, false );
  }
  // now handle the out-going particles
  const unsigned int nPartsOut = persVtx.m_particlesOut.size();
  for ( unsigned int i = 0; i != nPartsOut; ++i ) {
     createGenParticle( persEvt.m_genParticles[persVtx.m_particlesOut[i]], partToEndVtx, datapools, vtx );
  }

  return vtx;
}

HepMC::GenParticlePtr
McEventCollectionCnv_p3::createGenParticle( const GenParticle_p3& persPart,
                                            ParticlesMap_t& partToEndVtx,
                                            HepMC::DataPool& datapools, const HepMC::GenVertexPtr& parent, bool add_to_output )
{
  HepMC::GenParticlePtr p    = datapools.getGenParticle();
  if (parent) add_to_output?parent->add_particle_out(p):parent->add_particle_in(p);

  p->set_pdg_id(persPart.m_pdgId);
  p->set_status(persPart.m_status);
  // Note: do the E calculation in extended (long double) precision.
  // That happens implicitly on x86 with optimization on; saying it
  // explicitly ensures that we get the same results with and without
  // optimization.  (If this is a performance issue for platforms
  // other than x86, one could change to double for those platforms.)
  double temp_e=0.0;
  if ( 0 == persPart.m_recoMethod ) {
    temp_e = std::sqrt( (long double)(persPart.m_px)*persPart.m_px +
                          (long double)(persPart.m_py)*persPart.m_py +
                          (long double)(persPart.m_pz)*persPart.m_pz +
                          (long double)(persPart.m_m) *persPart.m_m );
  } else {
    const int signM2 = ( persPart.m_m >= 0. ? 1 : -1 );
    const double persPart_ene =
      std::sqrt( std::abs((long double)(persPart.m_px)*persPart.m_px +
                (long double)(persPart.m_py)*persPart.m_py +
                (long double)(persPart.m_pz)*persPart.m_pz +
                signM2* (long double)(persPart.m_m)* persPart.m_m));
    const int signEne = ( persPart.m_recoMethod == 1 ? 1 : -1 );
    temp_e=signEne * persPart_ene;
  }
  p->set_momentum(HepMC::FourVector(persPart.m_px,persPart.m_py,persPart.m_pz,temp_e));
  // setup flow
  // fillin' the flow
  std::vector<int> flows;
  const unsigned int nFlow = persPart.m_flow.size();
  for ( unsigned int iFlow= 0; iFlow != nFlow; ++iFlow ) {
  flows.push_back(persPart.m_flow[iFlow].second );
  }
  //We construct it here as vector w/o gaps.
  p->add_attribute(HepMCStr::flows, std::make_shared<HepMC3::VectorIntAttribute>(flows));
  HepMC::suggest_barcode(p,persPart.m_barcode);

  if ( persPart.m_endVtx != 0 ) {
    partToEndVtx[p] = persPart.m_endVtx;
  }

  return p;
}
