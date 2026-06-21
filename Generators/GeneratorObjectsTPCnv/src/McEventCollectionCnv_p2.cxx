///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv_p2.cxx
// Implementation file for class McEventCollectionCnv_p2
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////


// STL includes
#include <utility>

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p2.h"
#include "HepMcDataPool.h"

///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////

McEventCollectionCnv_p2::McEventCollectionCnv_p2() :
  Base_t( )
{}

McEventCollectionCnv_p2::McEventCollectionCnv_p2( const McEventCollectionCnv_p2& rhs ) 
  
= default;

McEventCollectionCnv_p2&
McEventCollectionCnv_p2::operator=( const McEventCollectionCnv_p2& rhs )
{
  if ( this != &rhs ) {
    Base_t::operator=( rhs );
  }
  return *this;
}

///////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////

McEventCollectionCnv_p2::~McEventCollectionCnv_p2()
= default;


void McEventCollectionCnv_p2::persToTrans( const McEventCollection_p2* persObj,
                                           McEventCollection* transObj,
                                           MsgStream& msg )
{
  msg << MSG::DEBUG << "Loading McEventCollection from persistent state..."
      << endmsg;

  // elements are managed by DataPool
  transObj->clear(SG::VIEW_ELEMENTS);
  HepMC::DataPool datapools;
  const unsigned int nVertices = persObj->m_genVertices.size();
  if ( datapools.vtx.capacity() - datapools.vtx.allocated() < nVertices ) {
    datapools.vtx.reserve( datapools.vtx.allocated() + nVertices );
  }
  const unsigned int nParts = persObj->m_genParticles.size();
  if ( datapools.part.capacity() - datapools.part.allocated() < nParts ) {
    datapools.part.reserve( datapools.part.allocated() + nParts );
  }
  const unsigned int nEvts = persObj->m_genEvents.size();
  if ( datapools.evt.capacity() - datapools.evt.allocated() < nEvts ) {
    datapools.evt.reserve( datapools.evt.allocated() + nEvts );
  }

  transObj->reserve( nEvts );
  const std::vector<GenEvent_p2>::const_iterator itrEnd = persObj->m_genEvents.end();
  for ( std::vector<GenEvent_p2>::const_iterator itr = persObj->m_genEvents.begin();
        itr != itrEnd;
        ++itr ) {
    const GenEvent_p2& persEvt = *itr;
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

    ParticlesMap_t partToEndVtx( (persEvt.m_particlesEnd-persEvt.m_particlesBegin)/2 );

    // create the vertices
    const unsigned int endVtx = persEvt.m_verticesEnd;
    for ( unsigned int iVtx= persEvt.m_verticesBegin; iVtx != endVtx; ++iVtx ) {
      genEvt->add_vertex( createGenVertex( *persObj,
                                           persObj->m_genVertices[iVtx],
                                           partToEndVtx,
                                           datapools ) );
    } //> end loop over vertices

    // set the signal process vertex
    const int sigProcVtx = persEvt.m_signalProcessVtx;
    if ( sigProcVtx ) {
      auto Vtx=HepMC::barcode_to_vertex(genEvt, sigProcVtx );
      HepMC::set_signal_process_vertex(genEvt, Vtx );
    }
    // connect particles to their end vertices
    for ( const auto& p:  partToEndVtx) {
      auto decayVtx = HepMC::barcode_to_vertex(genEvt, p.second );
      if ( decayVtx ) {
        decayVtx->add_particle_in( p.first );
      } else {
        msg << MSG::ERROR<< "GenParticle points to null end vertex !!"<< endmsg;
      }
    }
  } //> end loop over m_genEvents

  msg << MSG::DEBUG << "Loaded McEventCollection from persistent state [OK]"
      << endmsg;
}

void McEventCollectionCnv_p2::transToPers( const McEventCollection*,
                                           McEventCollection_p2*,
                                           MsgStream& msg )
{
  msg << MSG::DEBUG << "Creating persistent state of McEventCollection..."
      << endmsg;

  msg << MSG::ERROR
      << "This transient-to-persistent converter method has been RETIRED !!"
      << endmsg
      << "You are not supposed to end-up here ! Go away !"
      << endmsg;

  throw std::runtime_error( "Retired McEventCollectionCnv_p2::transToPers() !!" );
}


HepMC::GenVertexPtr
McEventCollectionCnv_p2::createGenVertex( const McEventCollection_p2& persEvt,
                                          const GenVertex_p2& persVtx,
                                          ParticlesMap_t& partToEndVtx, HepMC::DataPool& datapools, HepMC::GenEvent* parent ) 
{
  HepMC::GenVertexPtr vtx = datapools.getGenVertex();
  if (parent) parent->add_vertex(vtx);
  vtx->set_position( HepMC::FourVector(persVtx.m_x,persVtx.m_y, persVtx.m_z, persVtx.m_t) );
  vtx->set_status(persVtx.m_id);
  // cast from std::vector<float> to std::vector<double>
  std::vector<double> weights( persVtx.m_weights.begin(), persVtx.m_weights.end() );
  vtx->add_attribute(HepMCStr::weights,std::make_shared<HepMC3::VectorDoubleAttribute>(weights));
  HepMC::suggest_barcode(vtx,persVtx.m_barcode);

  // handle the in-going (orphans) particles
  const unsigned int nPartsIn = persVtx.m_particlesIn.size();
  for ( unsigned int i = 0; i != nPartsIn; ++i ) {
    createGenParticle( persEvt.m_genParticles[persVtx.m_particlesIn[i]], partToEndVtx, datapools );
  }

  // now handle the out-going particles
  const unsigned int nPartsOut = persVtx.m_particlesOut.size();
  for ( unsigned int i = 0; i != nPartsOut; ++i ) {
     createGenParticle( persEvt.m_genParticles[persVtx.m_particlesOut[i]], partToEndVtx, datapools, vtx );
  }

  return vtx;
}

HepMC::GenParticlePtr
McEventCollectionCnv_p2::createGenParticle( const GenParticle_p2& persPart,
                                            ParticlesMap_t& partToEndVtx, HepMC::DataPool& datapools, const HepMC::GenVertexPtr& parent ) 
{
  HepMC::GenParticlePtr p    = datapools.getGenParticle();

  if (parent) parent->add_particle_out(p);
  p->set_momentum( HepMC::FourVector(persPart.m_px,persPart.m_py,persPart.m_pz, persPart.m_ene ));
  p->set_pdg_id(               persPart.m_pdgId);
  p->set_status(               persPart.m_status);
  p->add_attribute(HepMCStr::phi,std::make_shared<HepMC3::DoubleAttribute>(persPart.m_phiPolarization));
  p->add_attribute(HepMCStr::theta,std::make_shared<HepMC3::DoubleAttribute>(persPart.m_thetaPolarization));
  HepMC::suggest_barcode(p,persPart.m_barcode);

  // fillin' the flow
  std::vector<int> flows;
  const unsigned int nFlow = persPart.m_flow.size();
  for ( unsigned int iFlow= 0; iFlow != nFlow; ++iFlow ) {
  flows.push_back(persPart.m_flow[iFlow].second );
  }
  //We construct it here as vector w/o gaps.
  p->add_attribute(HepMCStr::flows, std::make_shared<HepMC3::VectorIntAttribute>(flows));

  if ( persPart.m_endVtx != 0 ) {
    partToEndVtx[p] = persPart.m_endVtx;
  }

  return p;
}
