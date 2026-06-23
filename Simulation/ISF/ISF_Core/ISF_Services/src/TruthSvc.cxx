/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "TruthSvc.h"
// other ISF_HepMC includes
// ISF includes
#include "ISF_Event/ITruthIncident.h"
// Framework
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/SystemOfUnits.h"
// HepMC includes
#include "AtlasHepMC/SimpleVector.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/Relatives.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

// CLHEP includes
#include "CLHEP/Geometry/Point3D.h"

// DetectorDescription
#include "AtlasDetDescr/AtlasRegionHelper.h"

#include <sstream>


std::vector<HepMC::GenParticlePtr> findChildren(const HepMC::GenParticlePtr& p) {
  if (!p) return std::vector<HepMC::GenParticlePtr>();
  const auto& v = p->end_vertex();
  if (!v) return std::vector<HepMC::GenParticlePtr>();
#ifdef HEPMC3
  std::vector<HepMC::GenParticlePtr> ret = v->particles_out();
#else
  std::vector<HepMC::GenParticlePtr> ret;
  for (auto pp=v->particles_out_const_begin();pp!=v->particles_out_const_end();++pp) ret.push_back(*pp);
#endif
  if (ret.size()==1) if (ret.at(0)->pdg_id()==p->pdg_id()) ret = findChildren(ret.at(0));
  return ret;
}

#undef DEBUG_TRUTHSVC

/** Constructor **/
ISF::TruthSvc::TruthSvc(const std::string& name,ISvcLocator* svc) :
  base_class(name,svc),
  m_geoStrategies(),
  m_numStrategies()
{
}

/** framework methods */
StatusCode ISF::TruthSvc::initialize()
{
  ATH_MSG_VERBOSE( "initialize()" );

  // Screen output
  ATH_MSG_DEBUG("--------------------------------------------------------");

  // retrieve BarcodeSvc
  if ( m_barcodeSvc.retrieve().isFailure() ) {
    ATH_MSG_FATAL("Could not retrieve BarcodeService. Abort.");
    return StatusCode::FAILURE;
  }

  // copy a pointer to the strategy instance to the local
  // array of pointers (for faster access)
  ATH_CHECK(m_truthStrategies.retrieve());
  // Would be nicer to make m_geoStrategies a vector of vectors
  for ( unsigned short geoID=AtlasDetDescr::fFirstAtlasRegion; geoID<AtlasDetDescr::fNumAtlasRegions; ++geoID) {
    m_numStrategies[geoID] = 0;
    for ( const auto &truthStrategy : m_truthStrategies) {
      if(truthStrategy->appliesToRegion(geoID)) {
        ++m_numStrategies[geoID];
      }
    }
  }
  for ( unsigned short geoID=AtlasDetDescr::fFirstAtlasRegion; geoID<AtlasDetDescr::fNumAtlasRegions; ++geoID) {
    m_geoStrategies[geoID] = new ISF::ITruthStrategy*[m_numStrategies[geoID]];
    unsigned short curNumStrategies = m_truthStrategies.size();
    unsigned short nStrat(0);
    for ( unsigned short i = 0; i < curNumStrategies; ++i) {
      if(m_truthStrategies[i]->appliesToRegion(geoID)) {
        m_geoStrategies[geoID][nStrat++] = &(*m_truthStrategies[i]);
      }
    }

    // setup whether we want to write end-vertices in this region whenever a truth particle dies
    // create an end-vertex for all truth particles ending in the current AtlasRegion?
    bool forceEndVtx = std::find( m_forceEndVtxRegionsVec.begin(),
                                  m_forceEndVtxRegionsVec.end(),
                                  geoID ) != m_forceEndVtxRegionsVec.end();
    m_forceEndVtx[geoID] = forceEndVtx;
  }
  ATH_MSG_VERBOSE("initialize() successful");
  return StatusCode::SUCCESS;
}


/** framework methods */
StatusCode ISF::TruthSvc::finalize()
{
  ATH_MSG_VERBOSE("Finalizing ...");
  return StatusCode::SUCCESS;
}


/** Initialize the TruthSvc and the truthSvc */
StatusCode ISF::TruthSvc::initializeTruthCollection(int largestGeneratedParticleBC, int largestGeneratedVertexBC)
{
  ATH_CHECK( m_barcodeSvc->initializeBarcodes(largestGeneratedParticleBC, largestGeneratedVertexBC) );
  return StatusCode::SUCCESS;
}

StatusCode ISF::TruthSvc::releaseEvent() {
  return StatusCode::SUCCESS;
}


/** Register a truth incident */
void ISF::TruthSvc::registerTruthIncident( ISF::ITruthIncident& ti, bool saveAllChildren) const {

  const bool passWholeVertex = m_passWholeVertex || saveAllChildren;
  // pass whole vertex or individual child particles
  ti.setPassWholeVertices(passWholeVertex);

  // the GeoID
  AtlasDetDescr::AtlasRegion geoID = ti.geoID();

  // check geoID assigned to the TruthIncident
  if ( !validAtlasRegion(geoID) ) {
    const auto& position = ti.position();
    ATH_MSG_ERROR("Unable to register truth incident with unknown SimGeoID="<< geoID
                  << " at position z=" << position.z() << " r=" << position.perp());
    return;
  }

  ATH_MSG_VERBOSE( "Registering TruthIncident for SimGeoID="
                   << AtlasDetDescr::AtlasRegionHelper::getName(geoID) );

  // number of child particles
  const unsigned short numSec = ti.numberOfChildren();
  if ( m_skipIfNoChildren && (numSec==0) ) {
    ATH_MSG_VERBOSE( "No child particles present in the TruthIncident,"
                     << " will not record this TruthIncident.");
    return;
  }

  // the parent particle -> get its id
  const int parentID = ti.parentUniqueID();
  if ( m_skipIfNoParentId && (parentID == HepMC::UNDEFINED_ID) ) {
    ATH_MSG_VERBOSE( "Parent particle in TruthIncident does not have an id,"
                     << " will not record this TruthIncident.");
    return;
  }

  // loop over registered truth strategies for given geoID
  bool pass = false;
  for ( unsigned short stratID=0; (!pass) && (stratID<m_numStrategies[geoID]); stratID++) {
    // (*) test if given TruthIncident passes current strategy
    pass = m_geoStrategies[geoID][stratID]->pass(ti);
  }

  if (pass) {
    ATH_MSG_VERBOSE("At least one TruthStrategy passed.");
    // at least one truth strategy returned true
    //  -> record incident
    recordIncidentToMCTruth(ti, passWholeVertex);

  } else {
    // none of the truth strategies returned true
    //  -> child particles will NOT be added to the TruthEvent collection
    // attach parent particle end vertex if it gets killed by this interaction
    if ( m_forceEndVtx[geoID] && !ti.parentSurvivesIncident() ) {
      ATH_MSG_VERBOSE("No TruthStrategies passed and parent destroyed - create end vertex.");
      this->createGenVertexFromTruthIncident( ti );

#ifdef DEBUG_TRUTHSVC
      const std::string survival = (ti.parentSurvivesIncident()) ? "parent survives" : "parent destroyed";
      const std::string vtxType = (ti.interactionClassification()==ISF::STD_VTX) ? "Normal" : "Quasi-stable";
      ATH_MSG_INFO("TruthSvc: " << vtxType << " vertex + " << survival
                   << ", TI Class: " << ti.interactionClassification()
                   << ", ProcessType: " << ti.physicsProcessCategory()
                   << ", ProcessSubType: " << ti.physicsProcessCode());
#endif

    }

  }

  return;
}

/** Record the given truth incident to the MC Truth */
void ISF::TruthSvc::recordIncidentToMCTruth( ISF::ITruthIncident& ti, bool passWholeVertex) const {
#ifdef  DEBUG_TRUTHSVC
  ATH_MSG_INFO("Starting recordIncidentToMCTruth(...)");
#endif
  int       parentBC = ti.parentBarcode();

  if (ti.parentParticle()->end_vertex()) {
    ATH_MSG_WARNING ("Attempting to record a TruthIncident for a particle which has already decayed!");
    ATH_MSG_WARNING ("No action will be taken - please fix client code.");
    return;
  }

  // record the GenVertex
  HepMC::GenVertexPtr  vtxFromTI = createGenVertexFromTruthIncident( ti );
  const ISF::InteractionClass_t classification = ti.interactionClassification();
#ifdef DEBUG_TRUTHSVC
  const std::string survival = (ti.parentSurvivesIncident()) ? "parent survives" : "parent destroyed";
  const std::string vtxType = (ti.interactionClassification()==ISF::STD_VTX) ? "Normal" : "Quasi-stable";
  ATH_MSG_INFO("TruthSvc: " << vtxType << " vertex + " << survival
               << ", TI Class: " << ti.interactionClassification()
               << ", ProcessType: " << ti.physicsProcessCategory()
               << ", ProcessSubType: " << ti.physicsProcessCode());
#endif

  ATH_MSG_VERBOSE ( "Outgoing particles:" );
  // update parent barcode and add it to the vertex as outgoing particle
  int newPrimaryBC = HepMC::UNDEFINED_ID;
  if (classification == ISF::QS_SURV_VTX) {
    // Special case when a particle with a pre-defined decay interacts
    // and survives.
    // Set the barcode to the next available value below the simulation
    // barcode offset.
    newPrimaryBC = m_barcodeSvc->newGeneratedParticle(parentBC);
  }
  else {
    newPrimaryBC = parentBC + HepMC::SIM_REGENERATION_INCREMENT;
  }

  HepMC::GenParticlePtr  parentAfterIncident = ti.parentParticleAfterIncident( newPrimaryBC ); // This call changes ti.parentParticle() output
  if(parentAfterIncident) {
    if (classification==ISF::QS_SURV_VTX) {
      // Special case when a particle with a pre-defined decay
      // interacts and survives.
      // As the parentParticleAfterIncident has a pre-defined decay
      // its status should end in 2, but we should flag that it has
      // survived an interaction.
      if (!MC::isDecayed(parentAfterIncident)) {
        ATH_MSG_WARNING ( "recordIncidentToMCTruth - check parentAfterIncident: " << parentAfterIncident );
      }
    }
    vtxFromTI->add_particle_out( parentAfterIncident );
#ifdef HEPMC3
    HepMC::suggest_barcode( parentAfterIncident, newPrimaryBC ); // TODO check this works correctly
#endif
    // NB For ISFTruthIncident the m_parent ISFParticle still needs
    // its id and particleLink properties to be properly updated at
    // this point.
    ATH_MSG_VERBOSE ( "Parent After Incident: " << parentAfterIncident << ", barcode: " << HepMC::barcode(parentAfterIncident));
  }

  const bool isQuasiStableVertex = (classification == ISF::QS_PREDEF_VTX); // QS_DEST_VTX and QS_SURV_VTX should be treated as normal from now on.
  // add child particles to the vertex
  const unsigned short numSec = ti.numberOfChildren();
  for ( unsigned short i=0; i<numSec; ++i) {
    bool writeOutChild = isQuasiStableVertex || passWholeVertex || ti.childPassedFilters(i);
    if (writeOutChild) {
      HepMC::GenParticlePtr  p = nullptr;
      // generate a new barcode for the child particle
      int secondaryParticleBC = (isQuasiStableVertex) ?
        m_barcodeSvc->newGeneratedParticle(parentBC) : m_barcodeSvc->newSecondaryParticle(parentBC); // TODO replace m_barcodeSvc
      if ( secondaryParticleBC == HepMC::UNDEFINED_ID) {
        if (m_ignoreUndefinedBarcodes)
          ATH_MSG_WARNING("Unable to generate new Secondary Particle Barcode. Continuing due to 'IgnoreUndefinedBarcodes'==True");
        else {
          ATH_MSG_ERROR("Unable to generate new Secondary Particle Barcode. Aborting");
          abort();
        }
      }
      p = ti.childParticle( i, secondaryParticleBC ); // potentially overrides secondaryParticleBC
      if (p) {
        // add particle to vertex
        vtxFromTI->add_particle_out( p);
#ifdef HEPMC3
        int secondaryParticleBCFromTI = ti.childBarcode(i);
        HepMC::suggest_barcode( p, secondaryParticleBCFromTI ? secondaryParticleBCFromTI : secondaryParticleBC );
        // NB For ISFTruthIncident the current child ISFParticle still needs
        // its id and particleLink properties to be properly updated at
        // this point.
#endif
      }
      ATH_MSG_VERBOSE ( "Writing out " << i << "th child particle: " << p << ", barcode: " << HepMC::barcode(p));
    } // <-- if write out child particle
    else {
      ATH_MSG_VERBOSE ( "Not writing out " << i << "th child particle." );
    }

  } // <-- loop over all child particles
  ATH_MSG_VERBOSE("--------------------------------------------------------");
}

/** Record the given truth incident to the MC Truth */
HepMC::GenVertexPtr  ISF::TruthSvc::createGenVertexFromTruthIncident( ISF::ITruthIncident& ti ) const {

  int processCode = ti.physicsProcessCode();
  int       parentBC = ti.parentBarcode();

  std::vector<double> weights(1);
  int primaryBC = parentBC % HepMC::SIM_REGENERATION_INCREMENT;
  weights[0] = static_cast<double>( primaryBC ); // FIXME vertex weights should not be used to encode other info.

  // Check for a previous end vertex on this particle.  If one existed, then we should put down next to this
  //  a new copy of the particle.  This is the agreed upon version of the quasi-stable particle truth, where
  //  the vertex at which we start Q-S simulation no longer conserves energy, but we keep both copies of the
  //  truth particles
  HepMC::GenParticlePtr  parent = ti.parentParticle();
  if (!parent) {
    ATH_MSG_ERROR("Unable to write particle interaction to MC truth due to missing parent HepMC::GenParticle instance");
    abort();
  }
  HepMC::GenEvent *mcEvent = parent->parent_event();
  if (!mcEvent) {
    ATH_MSG_ERROR("Unable to write particle interaction to MC truth due to missing parent HepMC::GenEvent instance");
    abort();
  }

  // generate vertex
  int vtxbcode = m_barcodeSvc->newSimulationVertex(); // TODO replace barcodeSvc
  if ( vtxbcode == HepMC::UNDEFINED_ID) {
    if (m_ignoreUndefinedBarcodes) {
      ATH_MSG_WARNING("Unable to generate new Truth Vertex Barcode. Continuing due to 'IgnoreUndefinedBarcodes'==True");
    } else {
      ATH_MSG_ERROR("Unable to generate new Truth Vertex Barcode. Aborting");
      abort();
    }
  }
  const int vtxStatus = 1000 + static_cast<int>(processCode) + HepMC::SIM_STATUS_THRESHOLD;
#ifdef HEPMC3
  auto newVtx = HepMC::newGenVertexPtr( ti.position(),vtxStatus);
#else
  // NB In HepMC2 there is no GenVertex status, so we set the GenVertex ID.
  std::unique_ptr<HepMC::GenVertex> newVtx = std::make_unique<HepMC::GenVertex>( ti.position(), vtxStatus, weights );
  HepMC::suggest_barcode( newVtx.get(), vtxbcode );
#endif

  if (parent->end_vertex()){
      ATH_MSG_ERROR ("createGVfromTI: Parent particle found with an end vertex attached.  This should not happen!");
      ATH_MSG_ERROR ("createGVfromTI: Parent 1: " << parent << ", barcode: " << HepMC::barcode(parent));
      ATH_MSG_ERROR ( "createGVfromTI: parent->end_vertex(): " << parent->end_vertex() << ", barcode: " << HepMC::barcode(parent->end_vertex()) );
     abort();
  } else { // Normal simulation
#ifdef DEBUG_TRUTHSVC
    ATH_MSG_VERBOSE ("createGVfromTI Parent 1: " << parent << ", barcode: " << HepMC::barcode(parent));
#endif
    // add parent particle to newVtx
    newVtx->add_particle_in( parent );
#ifdef DEBUG_TRUTHSVC
    ATH_MSG_VERBOSE ( "createGVfromTI End Vertex representing process: " << processCode << ", for parent with barcode "<<parentBC<<". Creating." );
    ATH_MSG_VERBOSE ( "createGVfromTI Parent 2: " << parent << ", barcode: " << HepMC::barcode(parent));
#endif
#ifdef HEPMC3
    mcEvent->add_vertex(newVtx);
    HepMC::suggest_barcode( newVtx, vtxbcode );
    newVtx->add_attribute(HepMCStr::weights,std::make_shared<HepMC3::VectorDoubleAttribute>(weights));
#else
    mcEvent->add_vertex( newVtx.release() );
#endif
  }

  return parent->end_vertex();
}
