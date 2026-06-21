///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// GenEventCnv_p1.cxx 
// Implementation file for class GenEventCnv_p1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// Framework includes
#include "GaudiKernel/MsgStream.h"

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/GenEventCnv_p1.h"
#include "HepMcDataPool.h"

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

GenEventCnv_p1::GenEventCnv_p1( HepMC::DataPool* pool ) :
  m_pool( pool )
{}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 
  
void GenEventCnv_p1::setDataPool( HepMC::DataPool* pool )
{ 
  m_pool = pool; 
}

void GenEventCnv_p1::persToTrans( const GenEvent_p1* persObj, 
				  HepMC::GenEvent* transObj, 
				  MsgStream& msg ) 
{
  msg << MSG::DEBUG << "Loading HepMC::GenEvent from persistent state..."
      << endmsg;

  if ( nullptr == m_pool ) {
    msg << MSG::ERROR
	<< "This instance of GenEventCnv_p1 has a null pointer to "
	<< "HepMC::DataPool !" << endmsg
	<< "This probably means the T/P converter (McEventCollectionCnv_pX) "
	<< "is misconfigured !!"
	<< endmsg;
    throw std::runtime_error("Null pointer to HepMC::DataPool !!");
  }

  const unsigned int nVertices = persObj->m_vertices.size();
  if ( m_pool->vtx.capacity() - m_pool->vtx.allocated() < nVertices ) {
    m_pool->vtx.reserve( m_pool->vtx.allocated() + nVertices );
  }
  const unsigned int nParts = persObj->m_particles.size();
  if ( m_pool->part.capacity() - m_pool->part.allocated() < nParts ) {
    m_pool->part.reserve( m_pool->part.allocated() + nParts );
  }

  transObj->add_attribute (HepMCStr::barcodes, std::make_shared<HepMC::GenEventBarcodes>());
  transObj->add_attribute(HepMCStr::signal_process_id,std::make_shared<HepMC3::IntAttribute>(persObj->m_signalProcessId ));
  transObj->set_event_number(persObj->m_eventNbr);
  transObj->add_attribute(HepMCStr::event_scale,std::make_shared<HepMC3::DoubleAttribute>(persObj->m_eventScale));
  transObj->add_attribute(HepMCStr::alphaQCD,std::make_shared<HepMC3::DoubleAttribute>(persObj->m_alphaQCD));
  transObj->add_attribute(HepMCStr::alphaQED,std::make_shared<HepMC3::DoubleAttribute>(persObj->m_alphaQED));
  transObj->weights()= persObj->m_weights;
  transObj->add_attribute(HepMCStr::random_states,std::make_shared<HepMC3::VectorLongIntAttribute>(persObj->m_randomStates));

  // create a temporary map associating the barcode of an end-vtx to its 
  // particle.
  // As not all particles are stable (d'oh!) we take 50% of the number of
  // particles as an initial size of the hash-map (to prevent re-hash)
  ParticlesMap_t partToEndVtx(nParts/2);

  // create the vertices
  for ( unsigned int iVtx = 0; iVtx != nVertices; ++iVtx ) {
    const GenVertex_p1& persVtx = persObj->m_vertices[iVtx];
    createGenVertex( *persObj, persVtx, partToEndVtx, *m_pool, transObj );
  } //> end loop over vertices

  // set the signal process vertex
  const int sigProcVtx = persObj->m_signalProcessVtx;
  if ( sigProcVtx != 0 ) {
    auto Vtx = HepMC::barcode_to_vertex(transObj,sigProcVtx );
    HepMC::set_signal_process_vertex(transObj, Vtx );
  }

  // connect particles to their end vertices
  const ParticlesMap_t::iterator endItr= partToEndVtx.end();
  for ( ParticlesMap_t::iterator p = partToEndVtx.begin(); 
	p != endItr; 
	++p ) {
    auto decayVtx = HepMC::barcode_to_vertex(transObj, p->second );
    if ( decayVtx ) {
      decayVtx->add_particle_in( p->first );
    } else {
      msg << MSG::ERROR
	  << "GenParticle points to null end vertex !!" 
	  << endmsg;
    }
  }

  msg << MSG::DEBUG << "Loaded HepMC::GenEvent from persistent state [OK]"
      << endmsg;
}

void GenEventCnv_p1::transToPers( const HepMC::GenEvent*, 
				  GenEvent_p1*, 
				  MsgStream& msg ) 
{
  msg << MSG::DEBUG << "Creating persistent state of HepMC::GenEvent..."
      << endmsg;

  msg << MSG::ERROR
      << "This transient-to-persistent converter method has been RETIRED !!"
      << endmsg
      << "You are not supposed to end-up here ! Go away !"
      << endmsg;

  throw std::runtime_error( "Retired GenEventCnv_p1::transToPers() !!" );
}

/////////////////////////////////////////////////////////////////// 
// Protected methods: 
/////////////////////////////////////////////////////////////////// 

HepMC::GenVertexPtr 
GenEventCnv_p1::createGenVertex( const GenEvent_p1& persEvt,
				 const GenVertex_p1& persVtx,
				 ParticlesMap_t& partToEndVtx,
                                 HepMC::DataPool& datapools, HepMC::GenEvent* parent) 
{
  HepMC::GenVertexPtr vtx = datapools.getGenVertex();
  if (parent) parent->add_vertex(vtx);
  vtx->set_position( HepMC::FourVector(persVtx.m_x,persVtx.m_y,persVtx.m_z,persVtx.m_t) );
  vtx->add_attribute(HepMCStr::weights,std::make_shared<HepMC3::VectorDoubleAttribute>(persVtx.m_weights));
  HepMC::suggest_barcode(vtx,persVtx.m_barcode);
  
  // handle the in-going (orphans) particles
  const unsigned int nPartsIn = persVtx.m_particlesIn.size();
  for ( unsigned int i = 0; i != nPartsIn; ++i ) {
     createGenParticle( persEvt.m_particles[persVtx.m_particlesIn[i]], partToEndVtx, datapools);
  }
  // now handle the out-going particles
  const unsigned int nPartsOut = persVtx.m_particlesOut.size();
  for ( unsigned int i = 0; i != nPartsOut; ++i ) {
   createGenParticle( persEvt.m_particles[persVtx.m_particlesOut[i]], partToEndVtx,datapools, vtx);
  }

  return vtx;
}

HepMC::GenParticlePtr 
GenEventCnv_p1::createGenParticle( const GenParticle_p1& persPart,
				   ParticlesMap_t& partToEndVtx,
                                   HepMC::DataPool& datapools, const HepMC::GenVertexPtr& parent) 
{
  HepMC::GenParticlePtr p = datapools.getGenParticle();
  if (parent) parent->add_particle_out(p);
  p->set_momentum( HepMC::FourVector(persPart.m_px,persPart.m_py,persPart.m_pz,persPart.m_ene));
  p->set_pdg_id(persPart.m_pdgId);
  p->set_status(persPart.m_status);
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

