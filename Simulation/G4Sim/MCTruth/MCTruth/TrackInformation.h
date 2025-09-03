/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_TRACKINFORMATION_H
#define MCTRUTH_TRACKINFORMATION_H

#include "VTrackInformation.h"
#include "TruthUtils/MagicNumbers.h" //HepMC::INVALID_PARTICLE_BARCODE
#include "CxxUtils/checker_macros.h"

namespace ISF {
  class ISFParticle;
}

/** @class TrackInformation

 * @brief Implementation of VTrackInformation. Instances of this class
 * are attached as UserInformation to G4Tracks.

 * The GenParticlePtr m_currentGenParticle held by the TrackInformation
 * object points to the current GenParticle corresponding to the
 * current G4Track and can change during simulation (i.e. each time the
 * track undergoes a non-destructive interaction). The
 * m_generationZeroGenParticle member keeps the pre-regeneration truth
 * particle stable.

 * The other member variables are: m_theBaseISFParticle - a pointer to
 * the ISFParticle corresponding to the current G4Track,
 * m_returnedToISF - a flag indicating whether the ISFParticle
 * corresponding to the current G4Track scheduled to be returned to
 * the ISF, m_regenerationNr - the number of times the particle
 * represented by the G4Track has undergone a non-destructive
 * interaction that was recorded in the HepMC::GenEvent,
 * m_generationZeroGenParticle - the truth particle before any such
 * regeneration, m_uniqueID - convenience variable
 * corresponding to the id of m_currentGenParticle.
 * The member variables are m_classify: a classification of the
 * current G4Track (Primary, Regenerated Primary, Registered
 * Secondary, Secondary) and m_primaryGenParticle: a pointer to the
 * GenParticle used to create the initial G4PrimaryParticle from which
 * the current G4Track decends.
 */
class TrackInformation: public VTrackInformation {
public:
  TrackInformation();
  TrackInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* baseIsp=nullptr);
  /**
   * @brief return a pointer to the GenParticle corresponding to the
   * current G4Track (if there is one).
   */
  virtual HepMC::ConstGenParticlePtr GetCurrentGenParticle() const override {return m_currentGenParticle;}
  virtual HepMC::GenParticlePtr GetCurrentGenParticle() override {return m_currentGenParticle;}
  /**
   * @brief set m_currentGenParticle, the pointer to the GenParticle
   * corresponding to the current G4Track. This will be updated each
   * time an interaction of the G4Track is recorded to the
   * HepMC::GenEvent. Also invalidates previously cached values in
   * m_barcode and m_uniqueID.
   */
  virtual void SetCurrentGenParticle(HepMC::GenParticlePtr) override;

  /**
   * @brief return a pointer to the simulation truth particle before any
   * regeneration happened.
   */
  virtual HepMC::ConstGenParticlePtr GetGenerationZeroGenParticle() const override {return m_generationZeroGenParticle;}
  virtual HepMC::GenParticlePtr GetGenerationZeroGenParticle() override {return m_generationZeroGenParticle;}
  /**
   * @brief set m_generationZeroGenParticle, the pointer to the
   * simulation truth particle before any regeneration happened.
   */
  virtual void SetGenerationZeroGenParticle(HepMC::GenParticlePtr) override;

  /**
   * @brief return a pointer to the ISFParticle corresponding to the
   * current G4Track.
   */
  virtual const ISF::ISFParticle *GetBaseISFParticle() const override {return m_theBaseISFParticle;}
  virtual ISF::ISFParticle *GetBaseISFParticle() override {return m_theBaseISFParticle;}
  /**
   * @brief set the pointer to the ISFParticle corresponding to the
   * current G4Track. (Only used to replace the ISFParticle by a copy
   * of itself when the G4Track is killed and the copy is scheduled to
   * be returned to the ISF.)
   */
  virtual void SetBaseISFParticle(ISF::ISFParticle*) override;

  /**
   * @brief Is the ISFParticle corresponding to the current G4Track
   * scheduled to be returned to the ISF?
   */
  virtual bool GetReturnedToISF() const override {return m_returnedToISF;}
  /**
   * @brief Flag whether the ISFParticle corresponding to the current G4Track
   * scheduled to be returned to the ISF. Only called in TrackProcessorUserActionPassBack::returnParticleToISF
   */
  virtual void SetReturnedToISF(bool returned) override {m_returnedToISF=returned;}

  /**
   * @brief return the number of times the particle represented by the
   * G4Track has undergone a non-destructive interaction that was
   * recorded in the HepMC::GenEvent.
   */
  int GetRegenerationNr() const {return m_regenerationNr;}
  /**
   * @brief update the number of times the particle represented by the
   * G4Track has undergone a non-destructive interaction that was
   * recorded in the HepMC::GenEvent.
   */
  void SetRegenerationNr(int i) {m_regenerationNr=i;}

  virtual int GetParticleUniqueID() const override;
  virtual int GetParticleStatus() const override;
private:
  int m_regenerationNr{0};
  HepMC::GenParticlePtr m_currentGenParticle{};
  HepMC::GenParticlePtr m_generationZeroGenParticle{};
  mutable int m_uniqueID ATLAS_THREAD_SAFE = HepMC::INVALID_PARTICLE_BARCODE;
  ISF::ISFParticle *m_theBaseISFParticle{};
  bool m_returnedToISF{false};
};

#endif // MCTRUTH_TRACKINFORMATION_H
