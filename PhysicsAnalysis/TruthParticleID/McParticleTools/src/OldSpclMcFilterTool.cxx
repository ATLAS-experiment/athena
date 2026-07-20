/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// OldSpclMcFilterTool.cxx 
// Implementation file for class OldSpclMcFilterTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <cmath>

// CLHEP/HepMC includes
#include "TruthUtils/MagicNumbers.h"
#include "GenAccessIO.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/Relatives.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TruthUtils/HepMCHelpers.h"


// McParticleKernel includes
#include "McParticleKernel/IMcVtxFilterTool.h"

// McParticleTools includes
#include "OldSpclMcFilterTool.h"

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

using namespace TruthHelper;
using CLHEP::GeV;

// Constructors
////////////////
OldSpclMcFilterTool::OldSpclMcFilterTool( const std::string& type, 
					  const std::string& name, 
					  const IInterface* parent ) : 
  TruthParticleFilterBaseTool( type, name, parent ),
  m_tesIO( nullptr )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty( "rIsolation",
		   m_rIsol = 0.45,
		   "DeltaR isolation energy cut for electrons, muons, "
		   "taus and photons" );

  declareProperty( "ptGammaMin",
		   m_ptGamMin = 0.5*GeV,
		   "Minimum threshold for transverse momentum of photons" );

  declareProperty( "ptMin",             
		   m_ptMin = 0.5*GeV,
		   "Minimum threshold for transverse momentum for all particles.\n"
		   "Warning: this cut is applied *before* Pt photon cut !" );

  declareProperty( "etaRange", 
		   m_etaRange = 5.0,
		   "Eta acceptance cut applied on all stable particles" );

  // switches

  declareProperty( "IncludeSimul",         
		   m_includeSimul = true,
		   "Switch to include or not particles from detector simulation "
		   "(Geant 4)" );

  declareProperty( "IncludePartonShowers", 
		   m_includePartonShowers = true,
		   "Switch to include or not parton showers" );

  declareProperty( "RemoveDecayToSelf",    
		   m_removeDecayToSelf = true,
		   "Switch to remove particles which decay into themselves (t->tg) "
		   "*but* only for generated particles, not the ones from the " 
		   "Geant4 interactions" );

}

// Destructor
///////////////
OldSpclMcFilterTool::~OldSpclMcFilterTool()
{ 
  delete m_tesIO; m_tesIO = nullptr;
  ATH_MSG_DEBUG("Calling destructor");
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode OldSpclMcFilterTool::buildMcAod( const McEventCollection* in,
					    McEventCollection* out )
{
  if ( nullptr == in || nullptr == out ) {
    ATH_MSG_ERROR("Invalid pointer to McEventCollection !" << endmsg
		  << "  in: " << in << endmsg
		  << " out: " << out);
    return StatusCode::FAILURE;
  }

  // we just copy the input McEventCollection and put it into the output one
  out->operator=( *in );

  // select the barcodes of the "special" particles from the input GenEvent
  if ( selectSpclMcBarcodes().isFailure() ) {
    ATH_MSG_ERROR("Could not select the \"special\" barcodes !!");
    return StatusCode::FAILURE;
  }

  // remove the not "special" particles from the filtered McEventCollection
  if ( shapeGenEvent(out).isFailure() ) {
    ATH_MSG_ERROR("Could not remove the not \"special\" particles from the "\
		  "filtered McEventCollection !!");
    return StatusCode::FAILURE;
  }

  // reconnect the particles
  if ( reconnectParticles(in, out).isFailure() ) {
    ATH_MSG_ERROR("Could not reconnect the particles in the filtered "\
		  "McEventCollection !!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////// 
// Non-const methods: 
/////////////////////////////////////////////////////////////////// 
StatusCode OldSpclMcFilterTool::selectSpclMcBarcodes()
{
  StatusCode sc = StatusCode::SUCCESS;

  // Get all of the generated particles (does not have Geant secondaries)

  std::vector<HepMC::ConstGenParticlePtr> particles;
  if ( m_includeSimul ) {
    sc = m_tesIO->getMC(particles, false,  m_mcEventsReadHandleKey.key());
  } else {
    sc = m_tesIO->getMC(particles, true, m_mcEventsReadHandleKey.key());
  }
  if ( sc.isFailure() ) {
    ATH_MSG_ERROR("Could not get Monte Carlo particles from TDS at : "
		  << m_mcEventsReadHandleKey.key());
    return StatusCode::FAILURE;
  }

  m_barcodes.clear();

  // Loop over all particles, selecting special ones
  // keep track of them using their barcodes

  for (const auto& part : particles) {

    const int id      = part->pdg_id();
    const int ida     = std::abs(id);
    const HepMC::FourVector hlv = part->momentum();
    const double pt   = hlv.perp();
    const double eta  = hlv.pseudoRapidity();
    const double mass = hlv.m();
    const int barcode = HepMC::barcode(part);

    HepMC::ConstGenVertexPtr  decayVtx = part->end_vertex();
    HepMC::ConstGenVertexPtr  prodVtx  = part->production_vertex();

    bool isSpcl = false;

    /// skip stuff with no end-vertex
    if( !MC::isStable(part) && !decayVtx ) continue;

    const bool accept = pt > m_ptMin;

    ///////////////////////////
    // Select special particles
    ///////////////////////////

    /// stable particles
    if (m_includeSimul ) {
        if ( MC::isSimStable(part) && accept && std::abs(eta) < m_etaRange ) isSpcl = true;
    } else {
        if ( MC::isGenStable(part) && accept && std::abs(eta) < m_etaRange ) isSpcl = true;
    }
    // e, mu, tau, neutrino 
    // Hard coded cut pt>2GeV to remove beam-jet Dalitz decays, etc.
    if( MC::isSMLepton(part) && pt>2.*GeV && std::abs(eta) < m_etaRange ) isSpcl = true;

   /// Save photons
   ///
   if ( MC::isPhoton(part) && pt>m_ptGamMin && std::abs(eta) < m_etaRange ) {
     /// Save photon only if it does not decay (ie no end_vertex)
     if ( nullptr == part->end_vertex() ) {
        isSpcl=true;
     } 
   } 

    // Any long-lived B meson or baryon 5x1 or 5xx2
    int ifl=0;
    if( ida>100 && ida<1000) ifl=ida/100;
    if( ida>1000 && ida<10000) ifl=ida/1000;
    int jj1 = ida%10;
    if( (ifl==5 && jj1<3 && mass<9.0*GeV) || ( ifl==4 && jj1<3 && mass<2.4*GeV) ) {
      if (accept) isSpcl = true;
    } 

    // Any heavy particle other than quarks or gluons.
    // M > 9.0GeV excludes B hadrons but includes Upsilon, W, Z, t, ...
    if( ida>5 && ida!=21 && ida!=9 && mass>9.*GeV && accept ) isSpcl=true;

    // save the quarks
    if (ida >= 1 && ida <= 5 && accept ) isSpcl = true;

    // save the gluons
    if ( ida == 21 && accept ) isSpcl = true;

    // save the double charged Higgs
    if ( (ida == 9900041 || ida == 9900042) && accept ) isSpcl = true; 

    // SUSY particles -- relies on PDG codes
    if( ida>1000000 && ida<3000000 && accept ) isSpcl = true;

    ////////////////////////////////////
    // Exclude invalid special particles
    ////////////////////////////////////
    
    // Monte Carlo internal particles per PDG
    if( ida>80 && ida<101 ) isSpcl=false;

    // Particles decaying into itself
    // This prevents documentaries from being reported twice and
    // eliminates (if asked to) parton showers like q -> qg.
    // But should not delete particle if child is from GEANT.
    if( isSpcl && decayVtx ) {
        auto dcyVtx = part->end_vertex();
        for(const auto& child: *dcyVtx) {
	if( child->pdg_id()==id && //> looking for parton showers or documentaries
	    HepMC::barcode(child) !=barcode  && //> just to be sure that merging GEN_EVENT/G4Truth is OK
	    !HepMC::is_simulation_particle(child)  //> child is not from GEANT
	    ) {
	  if ( m_includePartonShowers ) {
	    // we keep the parent particle
	    // this could be vertices of the form :
	    // q -> q + g + b + bbar(hence the outgoing quark isn't decaying:documentary)
	    // t -> t + W + b (with outgoing top being a documentary)
	    // q -> q + g (real parton shower)
	    isSpcl = true;
	  } else {
	    isSpcl = false;
	  }
	}
      }
    }

    /////////////////////////////////////////
    /// ask McVtxFilterTool for its decision
    /// ie: final word is given to the user
    ////////////////////////////////////////
    if ( ( decayVtx && m_mcVtxFilterTool->isAccepted(decayVtx) ) || 
	 ( prodVtx  && m_mcVtxFilterTool->isAccepted(prodVtx) ) ) {
      isSpcl = true;
    }

    //////////////////////////////////////
    // Save special particles and children
    //////////////////////////////////////
    if( !isSpcl ) continue;
    m_barcodes.insert(barcode); // add it to list

    // Children
    if( isSpcl && decayVtx ) {
      for(const auto& child: *(part->end_vertex())) {
        if( HepMC::is_simulation_particle(child) && !m_removeDecayToSelf) { 
	  m_barcodes.insert(HepMC::barcode(child));// its not there already
        }
      }
    }

  }
  ATH_MSG_DEBUG("Read " << particles.size() 
		<< " and selected  " <<  m_barcodes.size() << " particles");

  return StatusCode::SUCCESS;
}

StatusCode OldSpclMcFilterTool::shapeGenEvent( McEventCollection* genAod )
{
  //now remove all the particles except those whose barcodes are marked
  for ( McEventCollection::iterator evt = genAod->begin(); evt != genAod->end();++evt) {
    std::vector<HepMC::GenParticlePtr> going_out;
    for ( auto & p :  ((HepMC::GenEvent*)(*evt))->particles()) {
//AV: We modify event      
      int pBC = HepMC::barcode(p);
      ATH_MSG_DEBUG("[pdg,bc]= " << p->pdg_id() << ", " << pBC);
      if ( m_barcodes.count(pBC) != 0 ) continue; 
      going_out.push_back(p); // list of useless particles
      auto pvtx = p->production_vertex();
      auto evtx = p->end_vertex();
      if (pvtx) pvtx->remove_particle_out(p); //remove from production vertex from useless partilcle
      if (evtx) { // if it has end vertex, may need to move the out partilces
        if(pvtx){ // move the partilces back
          for (auto& pp: evtx->particles_out()) {
            pvtx->add_particle_out(pp);
          }
        }
        evtx->remove_particle_out(p); // disconnect from end vertex
      }
    }//> loop over particles

    // there may be a bunch of vertices with no particles connected to them:
    // ==> Get rid of them //AV: Not sure if this is needed
    std::vector<HepMC::ConstGenVertexPtr> going_out_again;
    for ( auto& v: (*evt)->vertices()) {
      if ( v->particles_in().empty() && v->particles_out().empty() ){
        going_out_again.push_back(v);
      }
    }//> loop over vertices
//HepMC3 uses smart pointers

    
  }//> loop over GenEvents in McEventCollection
  
  // Set the signal_process_vertex to NULL if not to be recorded 
  for ( McEventCollection::iterator evt = genAod->begin();  evt != genAod->end(); ++evt) {
    auto sigProcVtx = HepMC::signal_process_vertex(*evt); 
    if (!sigProcVtx) continue;
      const int sigProcBC = HepMC::barcode(sigProcVtx); 
      bool isInColl = false; 
      for ( const auto& itrVtx: (*evt)->vertices() ) { 
        if ( sigProcBC == HepMC::barcode(itrVtx) ) { 
          isInColl = true; 
          break; 
        } 
      }  //> loop over vertices 
//AV: We don't set nullptr as signal vertex in HepMC3
    if ( !isInColl ) { 
       (*evt)->remove_attribute("signal_process_vertex");
    }
  }//> loop over GenEvent's 

  return StatusCode::SUCCESS;
}

StatusCode OldSpclMcFilterTool::reconnectParticles( const McEventCollection* in,
						    McEventCollection* out )
{
  if ( nullptr == in || nullptr == out ) {
    ATH_MSG_ERROR("Invalid pointer to McEventCollection !!" << endmsg
		  << "  in: " << in << endmsg
		  << " out: " << out);
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode OldSpclMcFilterTool::rebuildLinks( const HepMC::GenEvent * mcEvt,
					      HepMC::GenEvent * outEvt,
					      const HepMC::GenParticlePtr& mcPart )
{

  if ( !mcPart ) {
    ATH_MSG_WARNING("Null GenParticle: can not rebuildLinks");
    return StatusCode::FAILURE;
  }

  if ( mcPart->end_vertex() ) {
    ATH_MSG_VERBOSE("GenParticle has already a decay vertex : nothing to do");
    return StatusCode::SUCCESS;
  }

  if ( !mcEvt ) {
    ATH_MSG_WARNING("Null input HepMC::GenEvent : can not rebuildLinks");
    return StatusCode::FAILURE;
  }

  if ( !outEvt ) {
    ATH_MSG_WARNING("Null output HepMC::GenEvent: can not rebuildLinks");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode OldSpclMcFilterTool::initializeTool() 
{
  ATH_MSG_DEBUG("Calling initializeTool");
  delete m_tesIO; m_tesIO = nullptr;
  // accessor for particles  
  m_tesIO = new GenAccessIO();
  if( m_tesIO == nullptr ) {
    ATH_MSG_ERROR("Unable to retrieve GenAccessIO pointer");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

