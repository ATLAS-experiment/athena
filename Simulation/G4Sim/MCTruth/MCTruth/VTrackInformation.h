/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_VTRACKINFORMATION_H
#define MCTRUTH_VTRACKINFORMATION_H

#include "G4VUserTrackInformation.hh"
#include "AtlasHepMC/GenParticle_fwd.h"

namespace ISF {
  class ISFParticle;
}

/** @class VTrackInformation

 * @brief Instances of classes derived from this class are attached as
 * UserInformation to G4Tracks. It extends G4VUserTrackInformation.
 * The current GenParticlePtr held by derived VTrackInformation objects
 * can change during simulation (i.e. each time the track undergoes a
 * non-destructive interaction).
 * The member variables are m_classify: a classification of the current
 * G4Track (Primary, Regenerated Primary, Registered Secondary,
 * Secondary) and m_primaryGenParticle: a pointer to the GenParticle
 * used to create the initial G4PrimaryParticle from which the current
 * G4Track descends.

 * Dervived classes also provide access to the GenParticle corresponding to the current G4Track (if one exists)
 */
class VTrackInformation: public G4VUserTrackInformation {
public:
  enum TrackClassification { Primary, RegeneratedPrimary, RegisteredSecondary, Secondary, BarcodeOnly } ;
  VTrackInformation(TrackClassification tc=Primary);

  /**
   * @brief return the classification of the current G4Ttrack
   *  (Primary, Regenerated Primary, Registered Secondary, Secondary)
   */
  TrackClassification GetClassification() const {return m_classify;}
  /**
   * @brief update the classification of the currently tracked
   * particle, usually called when a new G4Track is created or after
   * it survives an interaction.
   */
  void SetClassification(TrackClassification tc) {m_classify=tc;}

  /**
   * @brief return a pointer to the GenParticle used to create the
   * initial G4PrimaryParticle from which the current G4Track decends.
   * Should match the return value of
   * ISF::TruthBinding::getPrimaryGenParticle().
   * TODO Check that this is required.
   */
  HepMC::ConstGenParticlePtr GetPrimaryGenParticle() const {return m_primaryGenParticle;}
  HepMC::GenParticlePtr GetPrimaryGenParticle() {return m_primaryGenParticle;}
  /**
   * @brief set the pointer to the GenParticle used to create the
   * initial G4PrimaryParticle from which the current G4Track decends.
   * TODO Check that this is required - if so, ensure it is set consistently.
   */
  void  SetPrimaryGenParticle(HepMC::GenParticlePtr);

  /**
   * @brief return a pointer to the GenParticle corresponding to the
   * current G4Track (if there is one).
   */
  virtual HepMC::ConstGenParticlePtr GetCurrentGenParticle() const {return nullptr;}
  virtual HepMC::GenParticlePtr GetCurrentGenParticle() {return nullptr;}
  /**
   * @brief set the pointer to the GenParticle corresponding to the
   * current G4Track. This will be updated each time an interaction of
   * the G4Track is recorded to the HepMC::GenEvent.
   */
  virtual void SetCurrentGenParticle(HepMC::GenParticlePtr);

  /**
   * @brief return a pointer to the simulation truth particle before any
   * regeneration happened.
   */
  virtual HepMC::ConstGenParticlePtr GetGenerationZeroGenParticle() const {return nullptr;}
  virtual HepMC::GenParticlePtr GetGenerationZeroGenParticle() {return nullptr;}
  /**
   * @brief set the pointer to the simulation truth particle before any
   * regeneration happened.
   */
  virtual void SetGenerationZeroGenParticle(HepMC::GenParticlePtr);

  /**
   * @brief return a pointer to the ISFParticle corresponding to the
   * current G4Track.
   */
  virtual const ISF::ISFParticle *GetBaseISFParticle() const {return nullptr;}
  virtual ISF::ISFParticle *GetBaseISFParticle() {return nullptr;}
  /**
   * @brief set the pointer to the ISFParticle corresponding to the
   * current G4Track. (Only used to replace the ISFParticle by a copy
   * of itself when the G4Track is killed and the copy is scheduled to
   * be returned to the ISF.)
   */
  virtual void SetBaseISFParticle(ISF::ISFParticle*);

  /**
   * @brief Is the ISFParticle corresponding to the current G4Track
   * scheduled to be returned to the ISF?
   */
  virtual bool GetReturnedToISF() const;
  /**
   * @brief Flag whether the ISFParticle corresponding to the current G4Track
   * scheduled to be returned to the ISF. Only called in TrackProcessorUserActionPassBack::returnParticleToISF
   */
  virtual void SetReturnedToISF(bool) ;

  virtual int GetParticleUniqueID() const = 0;
  virtual int GetParticleStatus() const = 0;

  virtual void Print() const {}
private:
  TrackClassification m_classify;
  HepMC::GenParticlePtr m_primaryGenParticle{};
};

#endif // MCTRUTH_VTRACKINFORMATION_H
