/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_TRACKBARCODEINFO_H
#define MCTRUTH_TRACKBARCODEINFO_H

#include "VTrackInformation.h"

namespace ISF {
  class ISFParticle;
}

/** @class TrackBarcodeInfo

 * @brief Minimal implementation of VTrackInformation. Instances of
 * this class are attached as UserInformation to G4Tracks. This is
 * used for G4Tracks with no link to the HepMC::GenEvent, but a
 * non-zero barcode/id value. (See
 * AthenaStackingAction::ClassifyNewTrack(...).)
 * No GenParticlePtr is held by the TrackBarcodeInfo class.  The
 * member variables are: m_theBaseISFParticle - a pointer to the
 * ISFParticle corresponding to the current G4Track, m_returnedToISF -
 * a flag indicating whether the ISFParticle corresponding to the
 * current G4Track scheduled to be returned to the ISF, m_barcode,
 * m_uniqueID and m_status - ???
 */
class TrackBarcodeInfo: public VTrackInformation {
public:
  TrackBarcodeInfo(int uid, int bc, ISF::ISFParticle* baseIsp=0);

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
  virtual void SetReturnedToISF(bool returned) override;

  virtual int GetParticleBarcode() const override {return m_barcode;}  // TODO Drop this once UniqueID and Status are used instead
  virtual int GetParticleUniqueID() const override {return m_uniqueID;}
  virtual int GetParticleStatus() const override {return m_status;}
private:
  ISF::ISFParticle *m_theBaseISFParticle{};
  int m_barcode;  // TODO Drop this once UniqueID and Status are used instead
  int m_uniqueID;
  int m_status{0}; //FIXME
  bool m_returnedToISF;
};


#endif // MCTRUTH_TRACKBARCODEINFO_H
