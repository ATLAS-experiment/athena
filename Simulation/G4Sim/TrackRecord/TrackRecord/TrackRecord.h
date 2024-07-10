/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKRECORD_TRACKRECORD_H
#define TRACKRECORD_TRACKRECORD_H

#include "CLHEP/Vector/ThreeVector.h"
#include "TruthUtils/MagicNumbers.h"
#include "GeneratorObjects/HepMcParticleLink.h"

class TrackRecord {
public:
  /** @brief Default constructor */
  TrackRecord() = default;

  /** @brief Default destructor */
  virtual ~TrackRecord() = default;

  /** @brief Constructor */
  TrackRecord(
              int pdg,
              int status,
              double energy,
              const CLHEP::Hep3Vector& momentum,
              const CLHEP::Hep3Vector& postition,
              double time,
              int barcode,
              int id,
              const std::string& volumeName)
    : m_pdgCode(pdg)
    , m_status(status)
    , m_energy(energy)
    , m_momentum(momentum)
    , m_position(postition)
    , m_time(time)
    , m_barcode(barcode)
    , m_id(id)
    , m_volName(volumeName)
  {
    if (m_id == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
      m_partLink = std::make_unique<HepMcParticleLink>(m_barcode, 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE); // FIXME is barcode-based
    }
  }

  /** @brief Constructor */
  TrackRecord(const TrackRecord& trc)
    : m_pdgCode(trc.m_pdgCode)
    , m_status(trc.m_status)
    , m_energy(trc.m_energy)
    , m_momentum(trc.m_momentum)
    , m_position(trc.m_position)
    , m_time(trc.m_time)
    , m_barcode(trc.m_barcode)
    , m_id(trc.m_id)
    , m_volName(trc.m_volName){
      if (m_id == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
        m_id = trc.id(); // Try a look-up via HepMcParticleLink
    }
}

  /** @brief Assignement Operator */
  TrackRecord &operator=(const TrackRecord& trc) {
    if (this != &trc) {
      m_pdgCode = trc.m_pdgCode;
      m_status = trc.m_status;
      m_energy = trc.m_energy;
      m_momentum = trc.m_momentum;
      m_position = trc.m_position;
      m_time = trc.m_time;
      m_barcode = trc.m_barcode;
      m_id = trc.m_id;
      if (m_id == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
        m_id = trc.id(); // Try a look-up via HepMcParticleLink
      }
      m_volName = trc.m_volName;
    }
    return *this;
  }

  /** @brief Energy */
  double GetEnergy() const {return m_energy;}

  /** @brief Set energy */
  void SetEnergy(double e) {m_energy = e;}

  /** @brief Position */
  CLHEP::Hep3Vector GetPosition() const {return m_position;}

  /** @brief Set position */
  void SetPosition(CLHEP::Hep3Vector p) {m_position = p;}

  /** @brief Momentum */
  CLHEP::Hep3Vector GetMomentum() const {return m_momentum;}

  /** @brief Set momentum */
  void SetMomentum(CLHEP::Hep3Vector e) {m_momentum = e;}

  /** @brief PDG Code */
  int GetPDGCode() const {return m_pdgCode;}

  /** @brief Set PDG code */
  void SetPDGCode(int pcode) {m_pdgCode = pcode;}

  /** @brief  Time */
  double GetTime() const {return m_time;}

  /** @brief Set time */
  void SetTime(double time) {m_time = time;}

  /** @brief Volume name */
  std::string GetVolName() const {return m_volName;}

  /** @brief Set Volume name */
  void SetVolName(const std::string& theName){m_volName = theName;}

  /** @brief status. */
  int status() const {return m_status;}

  /** @brief Set status */
  void SetStatus(int status) {m_status = status;}

  /** @brief unique ID - fall back to HepMcParticeLink if m_id has not been set directly */
  int id() const {if (m_id == HepMC::INVALID_PARTICLE_ID && m_partLink) { return m_partLink->id(); } else { return m_id; } }

  /** @brief Set uniqueID */
  void SetID(int uniqueID){
    m_id = uniqueID;
    if (m_id == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
      m_partLink = std::make_unique<HepMcParticleLink>(m_barcode, 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE); // FIXME is barcode-based
    }
  }

  /** @brief bar code. Alias function. */
  int barcode() const {return m_barcode;}

  /** @brief Set barcode */
  void SetBarcode(int barcode){
    m_barcode = barcode;
    if (m_id == HepMC::INVALID_PARTICLE_ID && m_barcode != HepMC::INVALID_PARTICLE_ID) {
      m_partLink = std::make_unique<HepMcParticleLink>(m_barcode, 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE); // FIXME is barcode-based
    }
  }

private:
  int m_pdgCode{0};
  int m_status{0};
  double m_energy{0};
  CLHEP::Hep3Vector m_momentum{0,0,0};
  CLHEP::Hep3Vector m_position{0,0,0};
  double m_time{0.};
  int m_barcode{HepMC::UNDEFINED_ID};
  int m_id{HepMC::UNDEFINED_ID};
  std::string m_volName{""};
  std::unique_ptr<HepMcParticleLink> m_partLink{}; // nullptr unless object was produced by reading TrackRecord_p1.
};

#endif // TRACKRECORD_TRACKRECORD_H
