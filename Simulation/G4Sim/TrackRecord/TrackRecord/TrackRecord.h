/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKRECORD_TRACKRECORD_H
#define TRACKRECORD_TRACKRECORD_H

#include "CLHEP/Vector/ThreeVector.h"

class TrackRecord {
public:
  /** @brief Default constructor */
  TrackRecord() = default;

  /** @brief Default destructor */
  virtual ~TrackRecord() = default;

  /** @brief Constructor */
  TrackRecord(
              int pdg,
              double energy,
              const CLHEP::Hep3Vector& momentum,
              const CLHEP::Hep3Vector& postition,
              double time,
              int barcode,
              const std::string& volumeName)
    : m_pdgCode(pdg)
    , m_energy(energy)
    , m_momentum(momentum)
    , m_position(postition)
    , m_time(time)
    , m_barcode(barcode)
    , m_volName(volumeName) {}

  /** @brief Constructor */
  TrackRecord(const TrackRecord& trc)
    : m_pdgCode(trc.m_pdgCode)
    , m_energy(trc.m_energy)
    , m_momentum(trc.m_momentum)
    , m_position(trc.m_position)
    , m_time(trc.m_time)
    , m_barcode(trc.m_barcode)
    , m_volName(trc.m_volName){}

  /** @brief Assignement Operator */
  TrackRecord &operator=(const TrackRecord& trc) {
    if (this != &trc) {
      m_pdgCode = trc.m_pdgCode;
      m_energy = trc.m_energy;
      m_momentum = trc.m_momentum;
      m_position = trc.m_position;
      m_time = trc.m_time;
      m_barcode = trc.m_barcode;
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

  /** @brief bar code. Alias function. */
  int barcode() const {return m_barcode;}

  /** @brief Set barcode */
  void SetBarcode(int barcode){m_barcode = barcode;}

private:
  int m_pdgCode{0};
  double m_energy{0};
  CLHEP::Hep3Vector m_momentum{0,0,0};
  CLHEP::Hep3Vector m_position{0,0,0};
  double m_time{0.};
  int m_barcode{0};
  std::string m_volName{""};
};

#endif // TRACKRECORD_TRACKRECORD_H
