/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthConverters/xAODtoHepMCTool.h"
#ifndef XAOD_STANDALONE
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif
#include "TruthUtils/MagicNumbers.h"

xAODtoHepMCTool::xAODtoHepMCTool(const std::string &name)
    : asg::AsgTool(name)
{
}

StatusCode xAODtoHepMCTool::initialize()
{
  ATH_MSG_INFO("Initializing xAODtoHepMCTool " << name() << "...");
  ATH_MSG_INFO("SignalOnly         = " << m_signalOnly);
  return StatusCode::SUCCESS;
}

StatusCode xAODtoHepMCTool ::finalize()
{
  ATH_MSG_INFO("==============================================================");
  ATH_MSG_INFO("==========    xAOD -> HepMC Tool :: Run Summary     ==========");
  ATH_MSG_INFO("==============================================================");
  if (m_noProdVtx)
  {
    ATH_MSG_INFO("Number of events that have a missing production vertex = " << m_noProdVtx);
  }
  else
  {
    ATH_MSG_INFO("No missing production vertices");
  }
  if (m_badBeams)
  {
    ATH_MSG_INFO("Number events with improperly defined beamparticles = " << m_badBeams);
    ATH_MSG_INFO("Inconsistencies with the beam particles storage in the event record!");
    ATH_MSG_INFO("... check xAOD::TruthEvent record ...");
  }
  else
  {
    ATH_MSG_INFO("No events with undefined beams.");
  }
  ATH_MSG_INFO("==============================================================");
  ATH_MSG_INFO("===================    End Run Summary     ===================");
  ATH_MSG_INFO("==============================================================");
  return StatusCode::SUCCESS;
}

std::vector<HepMC::GenEvent> xAODtoHepMCTool ::getHepMCEvents(const xAOD::TruthEventContainer *xTruthEventContainer, const xAOD::EventInfo *eventInfo) const
{
  ++m_evtCount;
  bool doPrint = m_evtCount < m_maxCount;
  // CREATE HEPMC EVENT COLLECTION
  std::vector<HepMC::GenEvent> mcEventCollection;
  // Loop over xAOD truth container
  // Signal event is always first, followed by pileup events
  ATH_MSG_DEBUG("Start event loop");
  for (const xAOD::TruthEvent *xAODEvent : *xTruthEventContainer)
  {
    if (doPrint)
      ATH_MSG_DEBUG("XXX Printing xAOD Event");
    if (doPrint)
      printxAODEvent(xAODEvent, eventInfo);
    // Create GenEvent for each xAOD truth event
    ATH_MSG_DEBUG("Create new GenEvent");
    HepMC::GenEvent&& hepmcEvent = createHepMCEvent(xAODEvent, eventInfo);
    #ifdef HEPMC3
    std::shared_ptr<HepMC3::GenRunInfo> runinfo = std::make_shared<HepMC3::GenRunInfo>(*(hepmcEvent.run_info().get()));
    #endif
    //possibly print info, before moving the object
    if (doPrint){
      ATH_MSG_DEBUG("XXX Printing HepMC Event");
      HepMC::Print::line(std::cout, hepmcEvent);
    }
    // Insert into McEventCollection
    mcEventCollection.push_back(std::move(hepmcEvent));
    #ifdef HEPMC3
    mcEventCollection[mcEventCollection.size()-1].set_run_info(std::move(runinfo));
    #endif
    // Quit if signal only
    if (m_signalOnly)
      break;
  }
  return mcEventCollection;
}

HepMC::GenEvent xAODtoHepMCTool::createHepMCEvent(const xAOD::TruthEvent *xEvt, const xAOD::EventInfo *eventInfo) const
{

  /// EVENT LEVEL
  HepMC::GenEvent genEvt;
  #ifdef HEPMC3
  genEvt.set_units(HepMC3::Units::MEV, HepMC3::Units::MM);
  #endif

  long long int evtNum = eventInfo->eventNumber();
  genEvt.set_event_number(evtNum);
  ATH_MSG_DEBUG("Start createHepMCEvent for event " << evtNum);

  //Weights
  #ifndef XAOD_STANDALONE
  const std::vector<float> weights = xEvt->weights();
  std::map<std::string, int> weightNameMap;
  if (AthAnalysisHelper::retrieveMetadata("/Generation/Parameters","HepMCWeightNames", weightNameMap).isFailure()) {
    ATH_MSG_DEBUG("Couldn't find meta-data for weight names.");
  }
  #ifdef HEPMC3
  std::shared_ptr<HepMC3::GenRunInfo> runinfo = std::make_shared<HepMC3::GenRunInfo>();
  genEvt.set_run_info(std::move(runinfo));
  std::vector<std::string> wnames;
  wnames.reserve(weights.size());
  for (int idx = 0; idx < int(weights.size()); ++idx) {
    for (const auto& it : weightNameMap) {
      if (it.second == idx) {
        wnames.push_back(it.first);
        break;
      }
    }
  }
  genEvt.run_info()->set_weight_names(wnames);
  for ( std::vector<float>::const_iterator wgt = weights.begin(); wgt != weights.end(); ++wgt ) {
    genEvt.weights().push_back(*wgt);
  }
  #else
  if (weightNameMap.size()) {
    HepMC::WeightContainer& wc = genEvt.weights();
    wc.clear();
    for (int idx = 0; idx < int(weights.size()); ++idx) {
      for (const auto& it : weightNameMap) {
        if (it.second == idx) {
          wc[ it.first ] = weights[idx];
          break;
        }
      }
    }
  }
  else {
    for ( std::vector<float>::const_iterator wgt = weights.begin(); wgt != weights.end(); ++wgt ) {
      genEvt.weights().push_back(*wgt);
    }
  }
  #endif
  #endif

  // PARTICLES AND VERTICES
  // Map of existing vertices - needed for the tree linking
  std::map<const xAOD::TruthVertex *, HepMC::GenVertexPtr> vertexMap;

  // Loop over all of the particles in the event, call particle builder
  for (auto tlink : xEvt->truthParticleLinks()) {
    if (!tlink.isValid()) { continue; }
    const xAOD::TruthParticle *xPart = *tlink;

    // sanity check
    if (xPart == nullptr) {
      ATH_MSG_WARNING("xAOD TruthParticle is equal to NULL. This should not happen!");
      continue;
    }

    if (!xPart->hasProdVtx() && !xPart->hasDecayVtx()) {
      ATH_MSG_WARNING("xAOD particle with no vertices, uniqueID = " << HepMC::uniqueID(xPart));
      continue;
    }

    // skip particles which are Geant4 secondaries
    if (HepMC::is_simulation_particle(xPart)) { continue; }

      // Create GenParticle
      // presumably the GenEvent takes ownership of this, but creating a unique_ptr here as that will only happen if there's an associated vertex
#ifdef HEPMC3
    auto hepmcParticle = createHepMCParticle(xPart);
#else
    std::unique_ptr<HepMC::GenParticle> hepmcParticle(createHepMCParticle(xPart));
#endif

    // Get the production and decay vertices
    if (xPart->hasProdVtx()) {
      const xAOD::TruthVertex *xAODProdVtx = xPart->prodVtx();
      // skip production vertices which are Geant4 secondaries
      if (HepMC::is_simulation_vertex(xAODProdVtx)) { continue; }
      bool prodVtxSeenBefore(false); // is this new?
      auto hepmcProdVtx = vertexHelper(xAODProdVtx, vertexMap, prodVtxSeenBefore);
      // Set the decay/production links
#ifdef HEPMC3
      hepmcProdVtx->add_particle_out(hepmcParticle);
#else
      hepmcProdVtx->add_particle_out(hepmcParticle.get());
#endif
      // Insert into Event
      if (!prodVtxSeenBefore) {
        genEvt.add_vertex(std::move(hepmcProdVtx));
      }
    }
    else {
      ATH_MSG_VERBOSE("No production vertex found for particle " << HepMC::uniqueID(xPart));
    }

    if (xPart->hasDecayVtx()) {
      const xAOD::TruthVertex *xAODDecayVtx = xPart->decayVtx();
      // skip decay vertices which are Geant4 secondaries
      if (HepMC::is_simulation_vertex(xAODDecayVtx)) {
        /// Avoid double deletion
#ifndef HEPMC3
        if (xPart->hasProdVtx()) { (void)hepmcParticle.release(); }
#endif
        continue;
      }
      bool decayVtxSeenBefore(false); // is this new?
      auto hepmcDecayVtx = vertexHelper(xAODDecayVtx, vertexMap, decayVtxSeenBefore);
      // Set the decay/production links
#ifdef HEPMC3
      hepmcDecayVtx->add_particle_in(std::move(hepmcParticle));
#else
      hepmcDecayVtx->add_particle_in(hepmcParticle.get());
#endif
      // Insert into Event
      if (!decayVtxSeenBefore) {
        genEvt.add_vertex(std::move(hepmcDecayVtx));
      }
    }
#ifndef HEPMC3
    (void)hepmcParticle.release();
#endif

  } // end of particle loop

  return genEvt;
}

// Helper to check whether a vertex exists or not using a map;
// calls createHepMCVertex if not
HepMC::GenVertexPtr xAODtoHepMCTool::vertexHelper(const xAOD::TruthVertex *xaodVertex,
                                                  std::map<const xAOD::TruthVertex *, HepMC::GenVertexPtr> &vertexMap,
                                                  bool &seenBefore) const
{

  HepMC::GenVertexPtr hepmcVertex;
  std::map<const xAOD::TruthVertex *, HepMC::GenVertexPtr>::iterator vMapItr;
  vMapItr = vertexMap.find(xaodVertex);
  // Vertex seen before?
  if (vMapItr != vertexMap.end()) {
    // YES: use the HepMC::Vertex already in the map
    hepmcVertex = (*vMapItr).second;
    seenBefore = true;
  }
  else {
    // NO: create a new HepMC::Vertex
    vertexMap[xaodVertex] = createHepMCVertex(xaodVertex);
    hepmcVertex = vertexMap[xaodVertex];
    seenBefore = false;
  }
  return hepmcVertex;
}

// Create the HepMC GenParticle
HepMC::GenParticlePtr xAODtoHepMCTool::createHepMCParticle(const xAOD::TruthParticle *particle) const
{
  ATH_MSG_VERBOSE("Creating GenParticle for uniqueID " << HepMC::uniqueID(particle));
  const HepMC::FourVector fourVec(m_momFac * particle->px(), m_momFac * particle->py(), m_momFac * particle->pz(), m_momFac * particle->e());
  auto hepmcParticle = HepMC::newGenParticlePtr(fourVec, particle->pdgId(), particle->status());
  hepmcParticle->set_generated_mass(m_momFac * particle->m());
  return hepmcParticle;
}

// Create the HepMC GenVertex
HepMC::GenVertexPtr xAODtoHepMCTool::createHepMCVertex(const xAOD::TruthVertex *vertex) const
{
  ATH_MSG_VERBOSE("Creating GenVertex for uniqueID " << HepMC::uniqueID(vertex));
  HepMC::FourVector prod_pos(m_lenFac * vertex->x(), m_lenFac * vertex->y(), m_lenFac * vertex->z(), m_lenFac * vertex->t());
  auto genVertex = HepMC::newGenVertexPtr(prod_pos);
  return genVertex;
}

// Print xAODTruth Event. The printout is particle oriented, unlike the HepMC particle/vertex printout. Regenerated Geant (i.e. ones surving an interaction) and pileup particles (e.g. suppressed pileup barcodes), with barcode > 1000000 are omitted. Thus we output only the original particles created in Geant4. 

void xAODtoHepMCTool::printxAODEvent(const xAOD::TruthEvent *event, const xAOD::EventInfo *eventInfo) const
{

  std::vector<int> uidPars;
  std::vector<int> uidKids;

  long long int evtNum = eventInfo->eventNumber();

  std::cout << "======================================================================================" << std::endl;
  std::cout << "xAODTruth Event " << evtNum << std::endl;
  std::cout << "   UniqueID      PDG Id  Status   px(GeV)   py(GeV)   pz(GeV)    E(GeV)   Parent: Decay" << std::endl;
  std::cout << "   -----------------------------------------------------------------------------------" << std::endl;

  int nPart = event->nTruthParticles();
  std::ios oldState(nullptr); 
  oldState.copyfmt(std::cout); //save ostream format
  for (int i = 0; i < nPart; ++i)
  {
    const xAOD::TruthParticle *part = event->truthParticle(i);
    if (part == nullptr) continue;
    if (HepMC::generations(part) > 0) continue;
    int id = part->pdgId();
    if (id != 25) continue;
    int stat = part->status();
    float px = part->px() / 1000.;
    float py = part->py() / 1000.;
    float pz = part->pz() / 1000.;
    float e = part->e() / 1000.;
    uidPars.clear();
    uidKids.clear();

    if (part->hasProdVtx())
    {
      const xAOD::TruthVertex *pvtx = part->prodVtx();
      if (pvtx)
        uidPars.push_back(HepMC::uniqueID(pvtx));
    }

    if (part->hasDecayVtx())
    {
      const xAOD::TruthVertex *dvtx = part->decayVtx();
      if (dvtx)
        uidKids.push_back(HepMC::uniqueID(dvtx));
    }
    
    
    std::cout << std::setw(10) << HepMC::uniqueID(part) << std::setw(12) << id
              << std::setw(8) << stat
              << std::setprecision(2) << std::fixed
              << std::setw(10) << px << std::setw(10) << py
              << std::setw(10) << pz << std::setw(10) << e << "   ";
    std::cout << "P: ";
    for (unsigned int k = 0; k < uidPars.size(); ++k)
    {
      std::cout << uidPars[k] << " ";
    }
    std::cout << "  D: ";
    for (unsigned int k = 0; k < uidKids.size(); ++k)
    {
      std::cout << uidKids[k] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "======================================================================================" << std::endl;
  std::cout.copyfmt(oldState);//restore ostream format
}
