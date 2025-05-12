/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// GenAodValidationTool.cxx 
// Implementation file for class GenAodValidationTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <sstream>

// FrameWork includes
#include "Gaudi/Property.h"

// StoreGate
#include "StoreGate/StoreGateSvc.h"

// CLHEP/HepMC includes
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/Flow.h"
#include "AtlasHepMC/Polarization.h"

#include "TruthUtils/HepMCHelpers.h"


// McParticleTools includes
#include "GenAodValidationTool.h"

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////
GenAodValidationTool::GenAodValidationTool( const std::string& type, 
					    const std::string& name, 
					    const IInterface* parent ) : 
  TruthParticleValidationBaseTool( type, name, parent ),
  m_outFile( nullptr )
{
  //
  // Property declaration
  // 
  
  declareProperty( "RefMcEvents",   
		   m_refMcEventsName = "TruthEvent",
		   "StoreGate location of the reference McEventCollection" );

  declareProperty( "CheckMcEvents", 
		   m_checkMcEventsName = "GEN_AOD",
		   "Location of the 'to-be-validated' McEventCollection" );

  declareProperty( "HardScatteringVtxOutputFile",
		   m_hardVtxOutFileName = "hepmc.hard.vtx.txt",
		   "Name of the output text file which will contain the " 
		   "hard-scattering vertices" );

  declareProperty( "RefHepMcWriterTool",
		   m_refMcEventWriter = HepMcTool_t( "HepMcWriterTool", 
						     this ),
		   "Tool to write the reference HepMC::GenEvent into a dedicated file" );

  declareProperty( "CheckHepMcWriterTool",
		   m_checkMcEventWriter = HepMcTool_t( "HepMcWriterTool", 
						       this ),
		   "Tool to write the \"to-be-checked\" HepMC::GenEvent "
		   "into a dedicated file" );

}

/// Destructor
///////////////
GenAodValidationTool::~GenAodValidationTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");

  if ( m_outFile ) {
    m_outFile->close();
    delete m_outFile;
    m_outFile = nullptr;
  }

}


/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode GenAodValidationTool::initializeTool()
{
  ATH_MSG_INFO("Initializing " << name() << "...");

  if ( m_outFile ) {
    delete m_outFile;
    m_outFile = nullptr;
  }
  m_outFile = new std::ofstream( m_hardVtxOutFileName.value().c_str(),
				 std::ios::trunc );
  if ( !m_outFile || !m_outFile->is_open() || m_outFile->bad() ) {
    ATH_MSG_ERROR("Could not open output Hard-Scattering vertices file at : " 
		  << m_hardVtxOutFileName.value());
    if ( m_outFile ) {
      delete m_outFile;
      m_outFile = nullptr;
    }
    return StatusCode::FAILURE;
  }

  // retrieve and configure HepMC writer tools
  if ( setupHepMcWriterTools().isFailure() ) {
    ATH_MSG_ERROR("Could not configure the HepMC::GenEvent writer tools !!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode GenAodValidationTool::finalizeTool()
{
  ATH_MSG_INFO("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode GenAodValidationTool::executeTool()
{
  // retrieve reference McEventCollection
  const McEventCollection * refMcEvents = nullptr;
  if ( evtStore()->retrieve( refMcEvents, m_refMcEventsName ).isFailure() ||
       nullptr == refMcEvents ) {
    ATH_MSG_ERROR("Could not retrieve reference McEventCollection at ["
		  << m_refMcEventsName << "] !!");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_VERBOSE("Successfully retrieved reference McEventCollection at ["
		    << m_refMcEventsName << "]");

    if ( !m_refMcEventWriter || 
	 m_refMcEventWriter->execute().isFailure() ) {
      ATH_MSG_WARNING("Failed to write the reference GenEvent !!");
    }
  }

  // retrieve checking McEventCollection
  const McEventCollection * checkMcEvents = nullptr;
  if ( evtStore()->retrieve(checkMcEvents, m_checkMcEventsName).isFailure() ||
       nullptr == checkMcEvents ) {
    ATH_MSG_ERROR("Could not retrieve 'to-be-validated' McEventCollection at ["
		  << m_checkMcEventsName << "] !!");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_VERBOSE
      ("Successfully retrieved 'to-be-validated' McEventCollection at ["
       << m_checkMcEventsName << "]");

    if ( !m_checkMcEventWriter ||
	 m_checkMcEventWriter->execute().isFailure() ) {
      ATH_MSG_WARNING("Failed to write the reference GenEvent !!");
    }
  }

  return executeTool( refMcEvents, checkMcEvents );
}

StatusCode 
GenAodValidationTool::executeTool( const McEventCollection* refMcEvents,
				   const McEventCollection* checkMcEvents )
{
  if ( nullptr == refMcEvents ) {
    ATH_MSG_ERROR("NULL pointer to reference McEventCollection !!");
    return StatusCode::FAILURE;
  }

  if ( nullptr == checkMcEvents ) {
    ATH_MSG_ERROR("NULL pointer to the 'to-be-validated' McEventCollection !!");
    return StatusCode::FAILURE;
  }

  if ( refMcEvents->size() != checkMcEvents->size() ) {
    ATH_MSG_ERROR("McEventCollections to be compared don't have the same size !"
		  << endmsg
		  << "Ref:     " << refMcEvents->size() << endmsg
		  << "Current: " << checkMcEvents->size());
  }

  const unsigned int nMaxEvts = std::min( refMcEvents->size(),
					  checkMcEvents->size() );
  for ( unsigned int iEvt = 0; iEvt != nMaxEvts; ++iEvt ) {

    if ( executeTool( (*refMcEvents)[iEvt], 
		      (*checkMcEvents)[iEvt] ).isFailure() ) {
      ATH_MSG_WARNING("Could not VALIDATE this event ["
		      << (*refMcEvents)[iEvt]->event_number()
		      << "] !!");
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode 
GenAodValidationTool::executeTool( const HepMC::GenEvent* refMcEvts,
				   const HepMC::GenEvent* checkMcEvts )
{
  if ( nullptr == refMcEvts ) {
    ATH_MSG_ERROR("NULL pointer to reference HepMC::GenEvent !!");
    return StatusCode::FAILURE;
  }

  if ( nullptr == checkMcEvts ) {
    ATH_MSG_ERROR("NULL pointer to the 'to-be-validated' HepMC::GenEvent !!");
    return StatusCode::FAILURE;
  }

  const unsigned int evtNbr = refMcEvts->event_number();
  (*m_outFile) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	       << std::endl
	       << "Event: " << evtNbr
	       << std::endl;

#ifdef HEPMC3   
  std::map<int,int> ref_bc_to_id;
  const auto& refvertices = refMcEvts->vertices();
  for ( const auto&  vtx: refvertices) {
    // AV: this algorthm is not very correct
    int partonsin = 0;
    int showerout = 0;
    for (auto& p: vtx->particles_in()) if (MC::isQuark(p) || MC::isGluon(p)) partonsin++;
    for (auto& p: vtx->particles_out()) if (p->pdg_id() == 91||p->pdg_id() == 92||p->pdg_id() == 94)  showerout++;
    if ( partonsin >= 2 &&  showerout == 0 ) {
      ref_bc_to_id[HepMC::barcode(vtx)] = vtx->id();
   }
  }
  std::map<int,int> che_bc_to_id;
  const auto& chevertices = checkMcEvts->vertices();
  for (const auto& checkVtx: chevertices) {
    che_bc_to_id[HepMC::barcode(checkVtx)] = checkVtx->id();
  }
  std::set<int> allbarcodes;
  for (const auto &[bc,id]: ref_bc_to_id) allbarcodes.insert(bc);
  for (const auto &[bc,id]: che_bc_to_id) allbarcodes.insert(bc);
  for (const auto &bc: allbarcodes){
   auto ref_it = ref_bc_to_id.find(bc);
   auto che_it = che_bc_to_id.find(bc);
   if (ref_it == ref_bc_to_id.end()) {ATH_MSG_WARNING("In Event [" << evtNbr << "]: got null ref-vertex (barcode: " << bc << ")");  continue; }
   if (che_it == che_bc_to_id.end()) {ATH_MSG_WARNING("Output GenEvent is missing the selected HardScattering Vtx !!"<< " (" << bc << ")"); continue; }
   if (-ref_it->second > int(refvertices.size()) ) { continue; } 
   if (-che_it->second > int(chevertices.size()) ) { continue; } 
   const auto& refvtx = refvertices[-ref_it->second-1];
   const auto& chevtx = chevertices[-che_it->second-1];
   (*m_outFile) << refvtx << std::endl;
   (*m_outFile) << chevtx << std::endl;
   (*m_outFile) << "---------" << std::endl;
   if ( !compareVtx( refvtx, chevtx ) ) {
        ATH_MSG_WARNING("Selected HardScattering vertices are NOT the same !!"<< " at Event [" << evtNbr << "]"<< " refVtx = " << refvtx<<" chevtx = " << chevtx);
   }
  }
  (*m_outFile) << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;  
#else
  std::set<int> ref_bc;
  for ( auto vtxIt = refMcEvts->vertices_begin(); vtxIt != refMcEvts->vertices_end(); ++vtxIt ) {
    auto vtx =*vtxIt;
    int partonsin = 0;
    int showerout = 0;
#ifdef HEPMC3
    for (auto p = vtx->particles_in_begin();p!=vtx->particles_in_end();++p ) if (MC::isQuark(*p) || MC::isGluon(*p)) partonsin++;
#else
    for (auto p = vtx->particles_in_const_begin();p!=vtx->particles_in_const_end();++p ) if (MC::isQuark(*p) || MC::isGluon(*p)) partonsin++;
#endif
    for (auto& p : *vtx) if (p->pdg_id() == 91||p->pdg_id() == 92||p->pdg_id() == 94)  showerout++;
    if ( partonsin >= 2 &&  showerout == 0 ) {
      ref_bc.insert(vtx->barcode());
   }
  }
  std::set<int> che_bc;
  for ( auto vtxIt = checkMcEvts->vertices_begin(); vtxIt != checkMcEvts->vertices_end(); ++vtxIt ) {
    auto vtx =*vtxIt;
    che_bc.insert(vtx->barcode());
  }
  std::set<int> allbarcodes;
  for (const auto &bc: ref_bc) allbarcodes.insert(bc);
  for (const auto &bc: che_bc) allbarcodes.insert(bc);
  for (const auto &bc: allbarcodes){
   auto ref_it = ref_bc.find(bc);
   auto che_it = che_bc.find(bc);
   if (ref_it == ref_bc.end()) {ATH_MSG_WARNING("In Event [" << evtNbr << "]: got null ref-vertex (barcode: " << bc << ")"); continue; }
   if (che_it == che_bc.end()) {ATH_MSG_WARNING("Output GenEvent is missing the selected HardScattering Vtx !!"<< " (" << bc << ")"); continue; }
   const auto refvtx = refMcEvts->barcode_to_vertex(bc);
   const auto chevtx = checkMcEvts->barcode_to_vertex(bc);
   (*m_outFile) << refvtx << std::endl;
   (*m_outFile) << chevtx << std::endl;
   (*m_outFile) << "---------" << std::endl;
   if ( !compareVtx( refvtx, chevtx ) ) {
     ATH_MSG_WARNING("Selected HardScattering vertices are NOT the same !!"<< " at Event [" << evtNbr << "]"<< " refVtx = " << refvtx<<" chevtx = " << chevtx);
   }
  }
  (*m_outFile) << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
#endif

  return StatusCode::SUCCESS;
}

bool GenAodValidationTool::compareVtx( const HepMC::ConstGenVertexPtr& vtx1, const HepMC::ConstGenVertexPtr& vtx2 ) const
{
  if ( !vtx1 || !vtx2 ) {
    ATH_MSG_ERROR("One of vertices is a NULL pointer !!" << endmsg
		  << " vtx1: " << vtx1 << endmsg
		  << " vtx2: " << vtx2);
    return false;
  }

#ifdef HEPMC3 
  auto inVtx1 = vtx1->particles_in();
  auto inVtx2 = vtx2->particles_in();
  if (inVtx1.size() !=  inVtx2.size()) {
    ATH_MSG_ERROR("Not the same number of branches !!" << endmsg  << " in:  " << inVtx1.size()  << "\t" << inVtx2.size());
    return false;
  }
  std::sort(inVtx1.begin(), inVtx1.end(), [](auto& a, auto& b) -> bool{return a->momentum().pz() > b->momentum().pz(); });
  std::sort(inVtx2.begin(), inVtx2.end(), [](auto& a, auto& b) -> bool{return a->momentum().pz() > b->momentum().pz(); });
  for (size_t i=0; i<inVtx1.size(); ++i){
     if ( compareParts( inVtx1.at(i), inVtx2.at(i) ) ) continue;
     ATH_MSG_ERROR("In-going particles are NOT matching !!");
     return false;
  }
  auto outVtx1 = vtx1->particles_out();
  auto outVtx2 = vtx2->particles_out();
  if (outVtx1.size() !=  outVtx2.size()) {
    ATH_MSG_ERROR("Not the same number of branches !!" << endmsg  << " out:  " << outVtx1.size()  << "\t" << outVtx2.size());
    return false;
  }
  std::sort(outVtx1.begin(), outVtx1.end(), [](auto& a, auto& b) -> bool{return a->momentum().pz() > b->momentum().pz(); });
  std::sort(outVtx2.begin(), outVtx2.end(), [](auto& a, auto& b) -> bool{return a->momentum().pz() > b->momentum().pz(); });
  for (size_t i=0; i<outVtx1.size(); ++i){
     if ( compareParts( outVtx1.at(i), outVtx2.at(i) ) ) continue;
     ATH_MSG_ERROR("Out-going particles are NOT matchiutg !!");
     return false;
  }
#else
  const int inVtx1 = vtx1->particles_in_size();
  const int inVtx2 = vtx2->particles_in_size();

  const int outVtx1 = vtx1->particles_out_size();
  const int outVtx2 = vtx2->particles_out_size();
  
  if (  inVtx1 !=  inVtx2 || outVtx1 != outVtx2 ) {
    ATH_MSG_ERROR("Not the same number of branches !!" << endmsg
		  << " in:  " << inVtx1  << "\t" << inVtx2  << endmsg
		  << " out: " << outVtx1 << "\t" << outVtx2);
    return false;
  }

  for ( HepMC::GenVertex::particles_in_const_iterator inPart1 = vtx1->particles_in_const_begin();
	inPart1 != vtx1->particles_in_const_end();
	++inPart1 ) {
    bool inParticleOK = false;
    for ( HepMC::GenVertex::particles_in_const_iterator inPart2 = vtx2->particles_in_const_begin();
	inPart2 != vtx2->particles_in_const_end();
	++inPart2 ) {
      if ( compareParts( *inPart1, *inPart2 ) ) {
	inParticleOK = true;
	break;
      }
    } //> end loop over in-part2

    if ( !inParticleOK ) {
      ATH_MSG_ERROR("In-going particles are NOT matching !!");
      return false;
    }

  }//> end loop over in-part1


  for ( HepMC::GenVertex::particles_out_const_iterator outPart1 = vtx1->particles_out_const_begin();
	outPart1 != vtx1->particles_out_const_end();
	++outPart1 ) {
    bool outParticleOK = false;
    for ( HepMC::GenVertex::particles_out_const_iterator outPart2 = vtx2->particles_out_const_begin();
	outPart2 != vtx2->particles_out_const_end();
	++outPart2 ) {
      if ( compareParts( *outPart1, *outPart2 ) ) {
	outParticleOK = true;
	break;
      }
    } //> end loop over out-part2

    if ( !outParticleOK ) {
      ATH_MSG_ERROR("Out-going particles are NOT matching !!");
      return false;
    }

  }//> end loop over out-part1
#endif

  return true;
}

bool 
GenAodValidationTool::compareParts( const HepMC::ConstGenParticlePtr& p1, const HepMC::ConstGenParticlePtr& p2 ) const
{
  if ( !p1 || !p2 ) {
    ATH_MSG_ERROR("One of particles is a NULL pointer !!" << endmsg
		  << " p1: " << p1 << endmsg
		  << " p2: " << p2);
    return false;
  }

  if ( HepMC::barcode(p1) != HepMC::barcode(p2) ) {
    return false;
  }

  const HepMC::FourVector hlv1   = p1->momentum();
  const int id1                  = p1->pdg_id();
  const int status1              = p1->status();
  auto flow1                     = HepMC::flow(p1);
  auto pol1                      = HepMC::polarization(p1);

  const HepMC::FourVector hlv2   = p2->momentum();
  const int id2                  = p2->pdg_id();
  const int status2              = p2->status();
  auto flow2                     = HepMC::flow(p2);
  auto pol2                      = HepMC::polarization(p2);

  const bool isOK = ( hlv1    == hlv2    &&
		      id1     == id2     && 
		      status1 == status2 &&
		      flow1   == flow2   &&
		      pol1    == pol2    );


  if ( !isOK ) {
    ATH_MSG_WARNING 
      ("=============================" << endmsg
       << "\thlv:    " << std::boolalpha << (hlv1    == hlv2   ) << endmsg
       << "\tid:     " << std::boolalpha << (id1     == id2    ) << endmsg
       << "\tstatus: " << std::boolalpha << (status1 == status2) << endmsg
       << "\tflow:   " << std::boolalpha << (flow1   == flow2  ) << endmsg
       << "\tpolar:  " << std::boolalpha << (pol1    == pol2   ) << endmsg
       << "=============================");
  }

  return isOK;
}

StatusCode GenAodValidationTool::setupHepMcWriterTools()
{
  ATH_CHECK( m_refMcEventWriter.retrieve() );
  ATH_CHECK( m_checkMcEventWriter.retrieve() );

  // now we configure the tools
  SmartIF<IProperty> prop{m_refMcEventWriter.get()};
  ATH_CHECK( prop.isValid() );
  ATH_CHECK( prop->setProperty("McEvents", m_refMcEventsName.value()) );

  prop = m_checkMcEventWriter.get();
  ATH_CHECK( prop.isValid() );
  ATH_CHECK( prop->setProperty("McEvents", m_checkMcEventsName.value()) );

  return StatusCode::SUCCESS;
}
