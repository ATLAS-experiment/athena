/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "RootTruthParticleCnvTool.h"
#include "GeneratorObjects/McEventCollection.h"
#include "McParticleEvent/TruthParticle.h"
#include "McParticleEvent/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include <array>
#include "TError.h"



// GeneratorObjects includes
#include "GeneratorObjects/HepMcParticleLink.h"

////////////////////////////////////////////////////////////////////////////////
//
//         Implementation of the ITruthParticleCnvTool function(s)
//

StatusCode RootTruthParticleCnvTool::execute() const { abort(); }
StatusCode RootTruthParticleCnvTool::execute(const EventContext&) const { abort(); }

StatusCode
RootTruthParticleCnvTool::convert(const McEventCollection *mcCollection,
                                  const unsigned int genEventIndex,
                                  TruthParticleContainer * container,
                                  const ITruthParticleVisitor* visitor )
  const 
{
  container->clear();

  if ( 0 == mcCollection ) {
    ::Warning ("RootTruthParticleCnvTool",
               "Null pointer to McEventCollection !");
    return StatusCode::RECOVERABLE;
  }

  if ( mcCollection->size() <= genEventIndex ) {
    ::Warning ("RootTruthParticleCnvTool",
               "McEventCollection size: %ui; Requested element nbr : %ui !!",
               // we could use %zd instead of using %ui and casting size_t to
               // unsigned int, but %zd is not ANSI...
               (unsigned int)mcCollection->size(), 
               genEventIndex);
    return StatusCode::RECOVERABLE;
  }

  const HepMC::GenEvent * evt = (*mcCollection)[genEventIndex];
  container->setGenEvent( mcCollection, genEventIndex );

  // reserve enough space for the container so we don't have to relocate it
  container->reserve( evt->particles_size() );

  /// Create a map to enhance access between GenParticles and TruthParticles
  TruthParticleContainer::Map_t bcToMcPart;

  for ( const auto& hepMcPart:  *evt) {

    TruthParticle * mcPart = new TruthParticle( hepMcPart, container );
    container->push_back( mcPart );

    if ( visitor ) {
      visitor->visit( mcPart );
    }

    mcPart->setCharge( chargeFromPdgId( mcPart->pdgId() ) );
    mcPart->setGenEventIndex( genEventIndex);

    if ( hepMcPart != mcPart->genParticle() ) {
      ::Error ("RootTruthParticleCnvTool",
               "TruthParticle is not wrapping the GenParticle : %d !!",
               HepMC::barcode(hepMcPart));
    }
    HepMcParticleLink mcLink( HepMC::barcode(hepMcPart), genEventIndex, HepMcParticleLink::IS_POSITION );
    bcToMcPart[ mcLink.compress() ] = mcPart;

  }//> end loop over particles

  // at this point the whole GenEvent has been proxied.
  // so we can setup its VectorMap
  container->setParticles( bcToMcPart );

#if 0
  // connect the TruthParticleContainer to the container of TruthEtIsolations
  // if it exists and if we are asked for
  if ( m_doEtIsolation.value() ) {
    const std::string& etIsolName 
      = m_isolationTool->etIsolationsName( container->genEventName() );
    if ( etIsolName.empty() ) {
      m_msg << MSG::WARNING
	    << "Could not retrieve the name of the TruthEtIsolations container"
	    << endmsg;
      return StatusCode::RECOVERABLE;
    }

    const TruthEtIsolationsContainer* etIsols = 0;
    if ( !m_storeGate->retrieve( etIsols, etIsolName ).isSuccess() ) {
      m_msg << MSG::WARNING
	    << "Could not retrieve the TruthEtIsolations container at ["
	    << etIsolName << "] !!"
	    << endmsg;
      return StatusCode::RECOVERABLE;
    }

    // index of HepMC::GenEvent within the McEventCollection is the same
    // than the one of the TruthEtIsolations within the TruthEtIsolationsCont.
    container->setEtIsolations( etIsols, genEventIndex );
  }
#endif

  return StatusCode::SUCCESS;
}

//
////////////////////////////////////////////////////////////////////////////////

double RootTruthParticleCnvTool::chargeFromPdgId (int pdgId) const
{
  return MC::charge(pdgId);
}

////////////////////////////////////////////////////////////////////////////////
//
//              Implementation of the IInterface function(s)

StatusCode RootTruthParticleCnvTool::queryInterface( const InterfaceID&,
                                                     void** ) { abort(); }
unsigned long RootTruthParticleCnvTool::addRef() { abort(); }
unsigned long RootTruthParticleCnvTool::release() { abort(); }
unsigned long RootTruthParticleCnvTool::refCount() const { abort(); }

//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//               Implementation of the IProperty function(s)
//

StatusCode RootTruthParticleCnvTool::setProperty( const std::string& ) {
   abort(); }
StatusCode RootTruthParticleCnvTool::setProperty( const std::string&, const Gaudi::Details::PropertyBase& ) { abort(); }
StatusCode RootTruthParticleCnvTool::setPropertyRepr( const std::string&, const std::string& ){ abort(); }
StatusCode RootTruthParticleCnvTool::getProperty( Gaudi::Details::PropertyBase* ) const { abort(); }
const Gaudi::Details::PropertyBase&
RootTruthParticleCnvTool::getProperty( std::string_view ) const{ abort(); }
StatusCode RootTruthParticleCnvTool::getProperty( std::string_view,
                                                  std::string& ) const {
   abort(); }
const std::vector< Gaudi::Details::PropertyBase* >&
RootTruthParticleCnvTool::getProperties() const { abort(); }
bool RootTruthParticleCnvTool::hasProperty( std::string_view ) const {
   abort(); }

//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//               Implementation of the IAlgTool function(s)
//

const std::string& RootTruthParticleCnvTool::type() const { abort(); }
const IInterface* RootTruthParticleCnvTool::parent() const { abort(); }
StatusCode RootTruthParticleCnvTool::configure() { abort(); }
StatusCode RootTruthParticleCnvTool::initialize() { abort(); }
StatusCode RootTruthParticleCnvTool::sysInitialize() { abort(); }
StatusCode RootTruthParticleCnvTool::reinitialize() { abort(); }
StatusCode RootTruthParticleCnvTool::sysReinitialize() { abort(); }
StatusCode RootTruthParticleCnvTool::start() { abort(); }
StatusCode RootTruthParticleCnvTool::sysStart() { abort(); }
StatusCode RootTruthParticleCnvTool::restart() { abort(); }
StatusCode RootTruthParticleCnvTool::sysRestart() { abort(); }
StatusCode RootTruthParticleCnvTool::stop() { abort(); }
StatusCode RootTruthParticleCnvTool::sysStop() { abort(); }
StatusCode RootTruthParticleCnvTool::finalize() { abort(); }
StatusCode RootTruthParticleCnvTool::sysFinalize() { abort(); }
StatusCode RootTruthParticleCnvTool::terminate() { abort(); }
Gaudi::StateMachine::State RootTruthParticleCnvTool::FSMState() const {
   abort(); }

//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//              Implementation of the INamedInterface function(s)
//

const std::string& RootTruthParticleCnvTool::name() const{abort();}

//
////////////////////////////////////////////////////////////////////////////////
