/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_TRACKHELPER_H
#define MCTRUTH_TRACKHELPER_H


#include "GeneratorObjects/HepMcParticleLink.h"

class G4Track;
class TrackInformation;

class TrackHelper {
public:
  TrackHelper(const G4Track* t);
  bool IsPrimary() const ;
  bool IsRegeneratedPrimary() const;
  bool IsRegisteredSecondary() const ;
  bool IsSecondary() const ;
  int GetBarcode() const ; // TODO Drop this once UniqueID and Status are used instead
  int GetUniqueID() const;
  int GetStatus() const ;
  TrackInformation * GetTrackInformation() {return m_trackInfo;}
  /**
   * @brief Generates a creates new HepMcParticleLink object on the
   * stack based on GetUniqueID(), assuming that the link should point
   * at the first GenEvent in the McEventCollection.
   */
  inline HepMcParticleLink GenerateParticleLink();
 private:
  TrackInformation *m_trackInfo;
};

HepMcParticleLink TrackHelper::GenerateParticleLink()
{
#if defined(HEPMC3)
  return HepMcParticleLink(this->GetUniqueID(), 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_ID);
#else
  return HepMcParticleLink(this->GetBarcode(), 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
#endif
}

#endif // MCTRUTH_TRACKHELPER_H
