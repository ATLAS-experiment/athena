/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_TRACKHELPER_H
#define MCTRUTH_TRACKHELPER_H


#include <tuple>

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
  inline HepMcParticleLink GenerateParticleLink(const EventContext&);
 private:
  inline std::tuple<int, HepMcParticleLink::UniqueIDFlag> particleIdentifierAndFlag() const;

  TrackInformation *m_trackInfo;
};

inline std::tuple<int, HepMcParticleLink::UniqueIDFlag>
TrackHelper::particleIdentifierAndFlag() const
{
#if defined(HEPMC3)
  return {GetUniqueID(), HepMcParticleLink::IS_ID};
#else
  return {GetBarcode(), HepMcParticleLink::IS_BARCODE};
#endif
}

HepMcParticleLink TrackHelper::GenerateParticleLink()
{
  const auto [identifier, flag] = particleIdentifierAndFlag();
  return HepMcParticleLink(identifier,
                           0,
                           HepMcParticleLink::IS_POSITION,
                           flag);
}

HepMcParticleLink TrackHelper::GenerateParticleLink(const EventContext& ctx)
{
  const auto [identifier, flag] = particleIdentifierAndFlag();
  return HepMcParticleLink(identifier,
                           0,
                           HepMcParticleLink::IS_POSITION,
                           flag,
                           ctx);
}

#endif // MCTRUTH_TRACKHELPER_H
