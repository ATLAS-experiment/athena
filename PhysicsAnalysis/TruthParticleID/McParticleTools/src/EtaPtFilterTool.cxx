/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// EtaPtFilterTool.cxx 
// Implementation file for class EtaPtFilterTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

// STL includes
#include <cmath>

// CLHEP includes
#include "CLHEP/Units/SystemOfUnits.h"

// McParticleKernel includes
#include "McParticleKernel/IMcVtxFilterTool.h"

// McParticleTools includes
#include "EtaPtFilterTool.h"
#include "copyBeamParticles.h"

#include "AtlasHepMC/Flow.h"
#include "TruthUtils/HepMCHelpers.h"

using CLHEP::GeV;

/// Constructors
////////////////
EtaPtFilterTool::EtaPtFilterTool( const std::string& type, 
				  const std::string& name, 
				  const IInterface* parent ) : 
  TruthParticleFilterBaseTool( type, name, parent )
{
  //
  // Property declaration
  // 
  //declareProperty( "Property", m_nProperty );

  declareProperty( "InnerEtaRegionCuts", 
		   m_innerEtaRegionCuts,
		   "Vector of cuts parameters for the inner region.\n"
		   "Two first elements are minimum and maximum |eta| values "
		   "for this inner region.\n"
		   "Third element is the minimum pT for the stable particles "
		   "to be accepted." );
  // defaults
  std::vector<double> innerEtaRegionCuts;
  innerEtaRegionCuts.push_back( 0.0     ); // minimum |eta|
  innerEtaRegionCuts.push_back( 3.5     ); // maximum |eta|
  innerEtaRegionCuts.push_back( 0.3*GeV ); // minimum pT of particles
  m_innerEtaRegionCuts.set( innerEtaRegionCuts );

  m_innerEtaRegionCuts.declareUpdateHandler( &EtaPtFilterTool::setupInnerEtaRegionCuts,
					     this );
  
  declareProperty( "OuterEtaRegionCuts", 
		   m_outerEtaRegionCuts,
		   "Vector of cuts parameters for the outer region.\n"
		   "Two first elements are minimum and maximum |eta| values "
		   "for this outer region.\n"
		   "Third element is the minimum pT for the stable particles "
		   "to be accepted." );
  // defaults
  std::vector<double> outerEtaRegionCuts;
  outerEtaRegionCuts.push_back( 3.5     ); // minimum |eta|
  outerEtaRegionCuts.push_back( 5.5     ); // maximum |eta|
  outerEtaRegionCuts.push_back( 1.0*GeV ); // minimum pT of particles
  m_outerEtaRegionCuts.set( outerEtaRegionCuts );

  m_outerEtaRegionCuts.declareUpdateHandler( &EtaPtFilterTool::setupOuterEtaRegionCuts,
					     this );

  // switches
  declareProperty( "OnlyGenerator",      
		   m_onlyGenerator = false,
		   "Switch to only include particles from generation and "
		   "reject particles from detector simulation (Geant4)" );


  declareProperty( "ButKeepAllGeneratorStable",      
		   m_butKeepAllGeneratorStable = true,
		   "Switch to keep all generated stable particles (they sum to "
		   "14 TeV) regardless of the eta and pt cuts defined for the "
		   "remaining particles" );


  declareProperty( "KeepDocumentaries",      
		   m_keepDocumentaries = true,
		   "Switch to keep *all* generator particles which are "
		   "documentaries (statuscode == 3)" );

  declareProperty( "KeepAllLeptons",
       m_keepAllLeptons = true,
		   "Switch to keep *all* leptons - i.e. do not apply kinematic cuts on them");

}

/// Destructor
///////////////
EtaPtFilterTool::~EtaPtFilterTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");
}


StatusCode EtaPtFilterTool::buildMcAod( const McEventCollection* in,McEventCollection* out )
{
  if ( !in || !out ) {
    ATH_MSG_ERROR("Invalid pointer to McEventCollection !" << endmsg
		  << "  in: " << in << endmsg
		  << " out: " << out);
    return StatusCode::FAILURE;
  }

  for ( unsigned int iEvt = 0; iEvt != in->size(); ++iEvt ) {
    const HepMC::GenEvent * inEvt = (*in)[iEvt];
    if ( 0 == inEvt ) {
      ATH_MSG_WARNING
        ("Could not launch filtering procedure for GenEvent number ["
         << iEvt << "] from McEventCollection ["
         << m_mcEventsReadHandleKey.key() << " !!"
         << endmsg
         << "  inEvt: " << inEvt);
      continue;
    }
	HepMC::GenEvent* outEvt = HepMC::copyemptyGenEvent( inEvt);

    if ( buildGenEvent( inEvt, outEvt ).isFailure() ) {
      ATH_MSG_ERROR("Could filter GenEvent number [" << iEvt 
		    << "] from McEventCollection ["     << m_mcEventsReadHandleKey.key() 
		    << "] !!");
      delete outEvt;
      outEvt = 0;
      continue;
    }

    TruthHelper::copyBeamParticles (*inEvt, *outEvt);

    out->push_back( outEvt );
  }

  return StatusCode::SUCCESS;
}


/////////////////////////////////////////////////////////////////// 
/// Const methods: 
///////////////////////////////////////////////////////////////////

StatusCode EtaPtFilterTool::buildGenEvent( const HepMC::GenEvent* in, HepMC::GenEvent* out )
{
  if (  nullptr == in || nullptr == out ) {
    ATH_MSG_ERROR("Invalid pointer to GenEvent !!" << endmsg
		  << "  in: " << in << endmsg
		  << " out: " << out);
    return StatusCode::FAILURE;
  }

  // loop over vertices
  VertexMap_t vmap;
  ParticleMap_t pmap;
  auto spv =  HepMC::signal_process_vertex(in); //AV To be removed in the future
  for ( auto vtx: in->vertices() ) {
    bool isSignalVertex = (vtx == spv);
    if ( !isAccepted(vtx) && !isSignalVertex ) {
      // no in-going nor out-going particles at this vertex matches 
      // the requirements nor it is a signal process vertex : ==> Skip it
      continue;
    }
    
    if ( addVertex( vtx, out, vmap, pmap, isSignalVertex ).isFailure() )
    {
      ATH_MSG_WARNING("Could not add vertex " << vtx );
    }

  } //> end loop over vertices
  
  return StatusCode::SUCCESS;
}

bool EtaPtFilterTool::isAccepted( const HepMC::ConstGenParticlePtr& mc ) const
{
  if ( ! mc ) {
    return false;
  }

  if ( m_butKeepAllGeneratorStable.value() ) {
    // static IsGenStable isStable;
    if ( MC::isStable(mc) ) {
      return true;
    }
  }

  if ( m_onlyGenerator.value() ) {
    // helper class to know if a GenParticle has been produced at Generator 
    // level. ie: not at simulation level (Geant4)

    if ( HepMC::is_simulation_particle(mc) ) {
      return false;
    }
  }

  if ( m_keepDocumentaries.value() ) {
    if ( !MC::isPhysical(mc) ) {
      return true;
    }
  }

  // Skip kinematic cuts for leptons
  if ( m_keepAllLeptons.value() ) {
    if ( MC::isLepton(mc) ) {
      return true;
    }
  }

  const HepMC::FourVector hlv = mc->momentum();
  const double pt             = hlv.perp();
  const double eta            = hlv.pseudoRapidity();
  if ( ( std::abs( eta )  >=  m_innerEtaRegionCuts.value()[0] && 
	 std::abs( eta )  <   m_innerEtaRegionCuts.value()[1] &&
	 pt               >   m_innerEtaRegionCuts.value()[2] ) 
       
       ||
       
       ( std::abs( eta )  >=  m_outerEtaRegionCuts.value()[0] && 
	 std::abs( eta )  <   m_outerEtaRegionCuts.value()[1] &&
	 pt               >   m_outerEtaRegionCuts.value()[2] ) ) {
    return true;
  } else {
    return false;
  }
}

bool EtaPtFilterTool::isAccepted( const HepMC::ConstGenVertexPtr& vtx ) const
{
  if ( !vtx ) {
    return false;
  }

  // check if this vertex looks like a vertex from the hard-scattering
  if ( isFromHardScattering(vtx) ) {
    return true;
  }

  // check if this vertex matches some decay pattern given 
  // to the McVtxFilterTool
  if ( m_mcVtxFilterTool->isAccepted(vtx) ) {
    return true;
  }

  // ///////////////////////////////////////////////////////////////////////
  // Now we check if at least one in- or out-going particle can be accepted.
  // If yes, then we accept the entire vertex
  //
  // check the parent branch
  for ( const auto& p: vtx->particles_in() ) {
    if ( isAccepted(p) ) {
      return true;
    }
  }//> end loop over parents
  
  // check the child branch
  for ( auto p :  *vtx) {
    if ( isAccepted(p) ) {
      return true;
    }
  }//> end loop over children

  return false;
}

StatusCode EtaPtFilterTool::addVertex( const HepMC::ConstGenVertexPtr& srcVtx, HepMC::GenEvent* evt,
                                       VertexMap_t& vmap,
                                       ParticleMap_t& pmap,
				       bool isSignalVertex) const
{
  if ( 0 == srcVtx || 0 == evt ) {
    ATH_MSG_ERROR("In addVertex(vtx,evt) : INVALID pointer given !!" << endmsg  << " vtx: " << srcVtx << endmsg << " evt: " << evt);
    return StatusCode::FAILURE;
  }
  HepMC::GenVertexPtr& vtx = vmap[srcVtx.get()];
  if ( !vtx ) {
    vtx = HepMC::newGenVertexPtr();
    evt->add_vertex(vtx); 
    vtx->set_position( srcVtx->position() );
    vtx->set_status( srcVtx->status() );
    HepMC::suggest_barcode(vtx, HepMC::barcode(srcVtx));
    vtx->add_attribute(HepMCStr::weights,srcVtx->attribute<HepMC3::VectorDoubleAttribute> (HepMCStr::weights));
    if (isSignalVertex) HepMC::set_signal_process_vertex(evt,vtx);
  }
  ////////////////////////////
  /// Fill the parent branch
  for ( auto  parent:  srcVtx->particles_in()) {
    HepMC::GenParticlePtr& p = pmap[parent.get()];
    if ( !p ) {
      p = HepMC::newGenParticlePtr();
      vtx->add_particle_in( p );
      p->set_momentum( parent->momentum() );
      p->set_generated_mass( parent->generated_mass() );
      p->set_pdg_id( parent->pdg_id() );
      p->set_status( parent->status() );
      HepMC::set_flow(p, HepMC::flow(parent) );
      HepMC::set_polarization(p, HepMC::polarization( parent) );
      HepMC::suggest_barcode(p, HepMC::barcode(parent));
    }
    // set the mother's decay to our (new) vertex
    vtx->add_particle_in( p );
    
  }//> loop over ingoing particles
  
  //////////////////////////////
  /// Fill the children branch
  for (auto child: srcVtx->particles_out()) {
    HepMC::GenParticlePtr& p = pmap[child.get()];
    if ( !p ) {
      p = HepMC::newGenParticlePtr();
      vtx->add_particle_out( p );
      p->set_momentum( child->momentum() );
      p->set_generated_mass( child->generated_mass() );
      p->set_pdg_id( child->pdg_id() );
      p->set_status( child->status() );
      HepMC::set_flow(p, HepMC::flow(child) );
      HepMC::set_polarization(p, HepMC::polarization(child) );
      HepMC::suggest_barcode(p, HepMC::barcode(child) );
    }
    // set the daughter's production vertex to our new vertex
    vtx->add_particle_out( p );
  }//> loop over outgoing particles


  return StatusCode::SUCCESS;
}

bool EtaPtFilterTool::isPartonVertex( const HepMC::ConstGenVertexPtr& vtx ) const
{
 if (!vtx) return false;
 for (auto& p: vtx->particles_in()) {
   if (MC::isHadron(p)&&!MC::isBeam(p)) return false;
   auto pv = p->production_vertex();
   if (pv && !isPartonVertex(pv)) return false;
 }
 for (auto& p: vtx->particles_out()) {
   if (MC::isHadron(p)&&!MC::isBeam(p)) return false;
 }
 return true;
}
bool EtaPtFilterTool::isFromHardScattering( const HepMC::ConstGenVertexPtr& vtx ) const
{
  int partonsin = 0;
  int showerout = 0;
  for (auto& p: vtx->particles_in()) if (MC::isQuark(p) || MC::isGluon(p)) partonsin++;
  for (auto& p: *vtx) if (p->pdg_id() == 91||p->pdg_id() == 92||p->pdg_id() == 94)  showerout++;
  
  return isPartonVertex(vtx) && (partonsin >= 2) &&  (showerout == 0);
}

/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode EtaPtFilterTool::initializeTool()
{
  ATH_MSG_INFO("Initializing " << name() << "...");

  // make sure the properties are synchronised
  setupInnerEtaRegionCuts(m_innerEtaRegionCuts);
  setupOuterEtaRegionCuts(m_outerEtaRegionCuts);

  if ( m_innerEtaRegionCuts.value().size() != 3 ||
       m_outerEtaRegionCuts.value().size() != 3 ) {
    ATH_MSG_ERROR
      ("Wrong size for eta regions cut :" << endmsg
       << "\tinner region: " << m_innerEtaRegionCuts.value().size()
       << endmsg
       << "\touter region: " << m_outerEtaRegionCuts.value().size()
       << endmsg
       << "You have to provide a list of cuts of the form : " << endmsg
       << " |etaMin| |etaMax| ptMin");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO
    ("Inner Eta region cuts : nCuts = " 
     << m_innerEtaRegionCuts.value().size()
     << endmsg
     << "\tetaMin = " << m_innerEtaRegionCuts.value()[0] << endmsg
     << "\tetaMax = " << m_innerEtaRegionCuts.value()[1] << endmsg
     << "\tPtMin  = " << m_innerEtaRegionCuts.value()[2] << endmsg
     << "Outer Eta region cuts : nCuts = " 
     << m_outerEtaRegionCuts.value().size()
     << endmsg
     << "\tetaMin = " << m_outerEtaRegionCuts.value()[0] << endmsg
     << "\tetaMax = " << m_outerEtaRegionCuts.value()[1] << endmsg
     << "\tPtMin  = " << m_outerEtaRegionCuts.value()[2]);

  return StatusCode::SUCCESS;
}

void EtaPtFilterTool::setupInnerEtaRegionCuts( Gaudi::Details::PropertyBase& /*innerRegion*/ )
{
  return;
}

void EtaPtFilterTool::setupOuterEtaRegionCuts( Gaudi::Details::PropertyBase& /*outerRegion*/ )
{
  return;
}
