/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "ISF_Event/ISFTruthIncident.h"

// ISF includes
#include "ISF_Event/ISFParticle.h"

// HepMC includes
#include "AtlasHepMC/SimpleVector.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include "TruthUtils/MagicNumbers.h"

static HepMC::GenParticlePtr ParticleHelper_convert( const ISF::ISFParticle &particle) {

  const Amg::Vector3D &mom = particle.momentum();
  double mass = particle.mass();
  double energy = sqrt( mom.mag2() + mass*mass);
  HepMC::FourVector fourMomentum( mom.x(), mom.y(), mom.z(), energy);

  auto hepParticle = HepMC::newGenParticlePtr( fourMomentum, particle.pdgCode(), particle.status() );
#ifndef HEPMC3
  HepMC::suggest_barcode(hepParticle, particle.barcode() );
#endif
  // return a newly created GenParticle
  return hepParticle;
}



ISF::ISFTruthIncident::ISFTruthIncident( ISF::ISFParticle &parent,
                                         const ISFParticleVector& children,
                                         int process,
                                         AtlasDetDescr::AtlasRegion geoID,
                                         ISF::KillPrimary killsPrimary,
                                         const HepMC::FourVector *position) :
  ITruthIncident(geoID, children.size()),
  m_parent(parent),
  m_children(children),
  m_process(process),
  m_killsPrimary(killsPrimary),
  m_position(position)
{
  if ( !m_position) {
    // No position was given, so compute it.
    // Default to the parent particle position in the case that there are no child particles.
    const ISF::ISFParticle *particle = (m_children.empty()) ? &m_parent : m_children.front();
    if ( !particle) particle = &m_parent; // protection against nullptrs in m_children ISFParticleVector - this would indicate a bug upstream, better to throw an exception here?
    const Amg::Vector3D &pos = particle->position();

    double time = 0.;  //<! TODO: FIXME
    m_position = new HepMC::FourVector( pos.x(), pos.y(), pos.z(), time );
  }
}

ISF::ISFTruthIncident::~ISFTruthIncident() {
  delete m_position;
}

const HepMC::FourVector& ISF::ISFTruthIncident::position() const {
  return *m_position;
}

int ISF::ISFTruthIncident::physicsProcessCategory() const {
  return -1;
}

int ISF::ISFTruthIncident::physicsProcessCode() const {
  return m_process;
}

double ISF::ISFTruthIncident::parentP2() const {
  return m_parent.momentum().mag2();
}

double ISF::ISFTruthIncident::parentPt2() const {
  return m_parent.momentum().perp2();
}

double ISF::ISFTruthIncident::parentEkin() const {
  return m_parent.ekin();
}

int ISF::ISFTruthIncident::parentPdgCode() const {
  return m_parent.pdgCode();
}

int ISF::ISFTruthIncident::parentStatus() {
  return m_parent.status();
}

HepMC::GenParticlePtr ISF::ISFTruthIncident::parentParticle() {
    return getHepMCTruthParticle(m_parent);
}

int ISF::ISFTruthIncident::parentBarcode() { // TODO Remove this method
  return m_parent.barcode();
}

int ISF::ISFTruthIncident::parentUniqueID() {
  return m_parent.id();
}

bool ISF::ISFTruthIncident::parentSurvivesIncident() const {
  return !(m_killsPrimary == ISF::fKillsPrimary);
}

HepMC::GenParticlePtr ISF::ISFTruthIncident::parentParticleAfterIncident(int newBC) {
  // if parent is killed in the interaction -> return nullptr
  if (m_killsPrimary==ISF::fKillsPrimary) return nullptr;

  // only update the parent particle, if it survived the interaction

  // set a new barcode
  m_parent.setBarcode( newBC );

  // set a new status
  m_parent.setStatus( parentStatus() + HepMC::SIM_STATUS_INCREMENT );

  // FIXME At this point the m_parent ISFParticle's id, truthBinding
  // and particleLink all still need to be updated

  // and update truth info (including the ISFParticle's HMPL)
  return updateHepMCTruthParticle(m_parent, &m_parent);
}

double ISF::ISFTruthIncident::childP2(unsigned short index) const {
  return m_children[index]->momentum().mag2();
}

double ISF::ISFTruthIncident::childPt2(unsigned short index) const {
  return m_children[index]->momentum().perp2();
}

double ISF::ISFTruthIncident::childEkin(unsigned short index) const {
  return m_children[index]->ekin();
}


int ISF::ISFTruthIncident::childPdgCode(unsigned short index) const {
  return m_children[index]->pdgCode();
}

int ISF::ISFTruthIncident::childBarcode(unsigned short index) const {
  return numberOfChildren() > index ? m_children[index]->barcode() : HepMC::UNDEFINED_ID;
}

HepMC::GenParticlePtr ISF::ISFTruthIncident::childParticle(unsigned short index,
                                                           int bc) {
  // the child particle
  ISF::ISFParticle *sec = m_children[index];

  // set particle barcode of the child particle
  if (bc) {
    sec->setBarcode( bc);
  }

  // Enforce that the status is set correctly
  sec->setStatus(1 + HepMC::SIM_STATUS_THRESHOLD);

  // FIXME At this point the sec ISFParticle's id, truthBinding
  // and particleLink all still need to be updated

  // and update truth info (including the ISFParticle's HMPL)
  return updateHepMCTruthParticle( *sec, &m_parent );
}


/** return attached truth particle */
HepMC::GenParticlePtr ISF::ISFTruthIncident::getHepMCTruthParticle( ISF::ISFParticle& particle ) const {
  auto* truthBinding     = particle.getTruthBinding();
  HepMC::GenParticlePtr currentGenParticle = truthBinding ? truthBinding->getCurrentGenParticle() : nullptr;
 
  // We have what we want
  if (currentGenParticle) {
    return currentGenParticle;
  }
  //Otherwise we need to create it
  return updateHepMCTruthParticle(particle,&particle);
}

/** convert ISFParticle to GenParticle and attach to ISFParticle's TruthBinding */
HepMC::GenParticlePtr ISF::ISFTruthIncident::updateHepMCTruthParticle( ISF::ISFParticle& particle,
                                                                       ISF::ISFParticle* parent ) const {
  auto* truthBinding     = particle.getTruthBinding();
  HepMC::GenParticlePtr newGenParticle = ParticleHelper_convert( particle );

  if (truthBinding) {
    truthBinding->setCurrentGenParticle(newGenParticle);
  } else {
    auto parentTruthBinding = parent ? parent->getTruthBinding() : nullptr;
    auto primaryGenParticle = parentTruthBinding ? parentTruthBinding->getPrimaryGenParticle() : nullptr;
    auto generationZeroGenParticle = newGenParticle; // New physical particle so this is also the generation zero particle
    truthBinding = new TruthBinding( newGenParticle, primaryGenParticle, generationZeroGenParticle );
    particle.setTruthBinding(truthBinding);
  }
  // At this point the values returned by particle.getParticleLink()
  // and particle.id() are not consistent with what is stored in the
  // TruthBinding.

  // FIXME Consider deleting the HepMcParticleLink and setting the id to HepMC::UNDEFINED_ID at this point?
  return newGenParticle;
}

/** Update the id and particleLink properties of the parentAfterIncident (to be called after registerTruthIncident) */
void ISF::ISFTruthIncident::updateParentAfterIncidentProperties() {
  // FIXME Check that we correctly deal with the case that the parent
  // particle survives the interaction, but is rejected by
  // registerTruthIncident
  const ISF::TruthBinding *parentAfterIncidentTruthBinding = m_parent.getTruthBinding();
  auto parentAfterIncidentGenParticle = (parentAfterIncidentTruthBinding)  ? parentAfterIncidentTruthBinding->getCurrentGenParticle() : nullptr;
  const int parentAfterIncidentID = (parentAfterIncidentGenParticle) ? HepMC::uniqueID(parentAfterIncidentGenParticle) : HepMC::UNDEFINED_ID;
  HepMcParticleLink* parentAfterIncidentHMPL{};
  const HepMcParticleLink* parentBeforeIncidentHMPL = m_parent.getParticleLink();
  int eventIndex{0};
  if (parentAfterIncidentGenParticle) { eventIndex = parentAfterIncidentGenParticle->parent_event()->event_number(); }
  else if (parentBeforeIncidentHMPL) { eventIndex = parentBeforeIncidentHMPL->eventIndex(); }
  const HepMcParticleLink::PositionFlag idxFlag =
    (eventIndex==0) ? HepMcParticleLink::IS_POSITION: HepMcParticleLink::IS_EVENTNUM;
  if (parentBeforeIncidentHMPL) {
    delete parentBeforeIncidentHMPL;
  }
  if (!parentAfterIncidentGenParticle) {
    parentAfterIncidentHMPL = new HepMcParticleLink(parentAfterIncidentID, eventIndex, idxFlag, HepMcParticleLink::IS_ID);
  }
  else {
    parentAfterIncidentHMPL = new HepMcParticleLink(parentAfterIncidentGenParticle, eventIndex, idxFlag);
  }
  m_parent.setId(parentAfterIncidentID);
  m_parent.setParticleLink(parentAfterIncidentHMPL);
}

/** Update the id and particleLink properties of the child particles (to be called after registerTruthIncident) */
void ISF::ISFTruthIncident::updateChildParticleProperties() {
  unsigned short numSec = numberOfChildren();
  for (unsigned short i=0; i<numSec; i++) {
    // the current particle
    ISF::ISFParticle *child = m_children[i];
    ISF::TruthBinding *childTruthBinding = child->getTruthBinding();
    if (!childTruthBinding) {
      // Child particles which were rejected during
      // registerTruthIncident need a TruthBinding
      auto parentTruthBinding = m_parent.getTruthBinding();
      if  (parentTruthBinding) {
        childTruthBinding = parentTruthBinding->childTruthBinding(nullptr);
      }
      else  {
        // FIXME We really shouldn't end up here, possibly abort if we hit this?
        childTruthBinding = new TruthBinding( nullptr, nullptr, nullptr );
      }
      child->setTruthBinding(childTruthBinding);
    }
    auto childGenParticle = childTruthBinding->getCurrentGenParticle();
    const int childID = (childGenParticle) ? HepMC::uniqueID(childGenParticle) : HepMC::UNDEFINED_ID;
    HepMcParticleLink* childHMPL{};
    const HepMcParticleLink* oldChildHMPL = child->getParticleLink();
    int eventIndex{0};
    if (childGenParticle) { eventIndex = childGenParticle->parent_event()->event_number(); }
    else if (oldChildHMPL) { eventIndex = oldChildHMPL->eventIndex(); }
    const HepMcParticleLink::PositionFlag idxFlag =
      (eventIndex==0) ? HepMcParticleLink::IS_POSITION: HepMcParticleLink::IS_EVENTNUM;
    if (oldChildHMPL) {
      delete oldChildHMPL;
    }
    if (!childGenParticle) {
      childHMPL = new HepMcParticleLink(childID, eventIndex, idxFlag, HepMcParticleLink::IS_ID);
    }
    else {
      childHMPL = new HepMcParticleLink(childGenParticle, eventIndex, idxFlag);
    }
    child->setId(childID);
    child->setParticleLink(childHMPL);
  }
}
