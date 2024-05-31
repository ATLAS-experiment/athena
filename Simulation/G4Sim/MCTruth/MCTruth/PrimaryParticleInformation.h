/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_PRIMARYPARTICLEINFORMATION_H
#define MCTRUTH_PRIMARYPARTICLEINFORMATION_H

#include "G4VUserPrimaryParticleInformation.hh"
#include "AtlasHepMC/GenParticle_fwd.h" 
#include "TruthUtils/MagicNumbers.h" //HepMC::INVALID_PARTICLE_BARCODE
#include "CxxUtils/checker_macros.h"

namespace ISF {
  class ISFParticle;
}
/** @class PrimaryParticleInformation

 * @brief This class is attached to G4PrimaryParticle objects as
 * UserInformation. The member variable m_theParticle holds a pointer
 * to the HepMC::GenParticle which was used to create the
 * G4PrimaryParticle. Tthe member variable m_theISFParticle holds a
 * pointer to the ISFParticle used to create the
 * G4PrimaryParticle. (See
 * ISF::InputConverter::getG4PrimaryParticle().)
 * The behaviour is slightly different inside and outside ISF:
 * - outside of ISF: m_theParticle is used to build the initial
 * VTrackInformation attached as UserInformation to G4Tracks created
 * from G4PrimaryParticles.
 * (See AthenaStackingAction::ClassifyNewTrack(...))
 * - inside ISF: The TruthBinding of m_theISFParticle is used to build
 * the VTrackInformation object attached as UserInformation to
 * G4Tracks created from G4PrimaryParticles.
 * (See TrackProcessorUserActionBase::setupPrimary(...))
 * NB While the GenParticlePtr held by the
 * VTrackInformation object can change during simulation (i.e. each
 * time the track undergoes a non-destructive interaction). The
 * GenParticlePtr held by PrimaryParticleInformation never changes.
 */
class PrimaryParticleInformation: public G4VUserPrimaryParticleInformation {
public:
  PrimaryParticleInformation();
  PrimaryParticleInformation(HepMC::GenParticlePtr p, ISF::ISFParticle *isp=nullptr);

  /**
   * @brief return a pointer to the GenParticle used to create the G4PrimaryParticle
   */
  HepMC::ConstGenParticlePtr GetHepMCParticle() const { return m_theParticle; }
  HepMC::GenParticlePtr GetHepMCParticle() { return m_theParticle; }

  /**
   * @brief return the number of times the particle represented by the
   * G4PrimaryParticle has undergone a non-destructive interaction that was
   * recorded in the HepMC::GenEvent.
   */
  int GetRegenerationNr() {return  m_regenerationNr;}
 /**
   * @brief update the number of times the particle represented by the
   * G4PrimaryParticle has undergone a non-destructive interaction that was
   * recorded in the HepMC::GenEvent.
   */
  void SetRegenerationNr(int i) {m_regenerationNr=i;}

  /**
   * @brief return a pointer to the ISFParticle used to create the G4PrimaryParticle
   */
  const ISF::ISFParticle* GetISFParticle() const { return m_theISFParticle; }
  ISF::ISFParticle* GetISFParticle() { return m_theISFParticle; }
  void SetISFParticle(ISF::ISFParticle* isp);

  int GetParticleBarcode() const;
  int GetParticleUniqueID() const;
  void Print() const {}
private:
  HepMC::GenParticlePtr m_theParticle{};
  ISF::ISFParticle* m_theISFParticle{};

  int m_regenerationNr{0};
  mutable int m_barcode ATLAS_THREAD_SAFE = HepMC::INVALID_PARTICLE_BARCODE;
  mutable int m_uniqueID ATLAS_THREAD_SAFE = HepMC::INVALID_PARTICLE_BARCODE;
};

#endif // MCTRUTH_PRIMARYPARTICLEINFORMATION_H
