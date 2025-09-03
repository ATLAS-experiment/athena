/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_TRACKHELPER_H
#define MCTRUTH_TRACKHELPER_H


#include <tuple>

#include "AtlasHepMC/GenParticle_fwd.h"
#include "GeneratorObjects/HepMcParticleLink.h"

class IProxyDict;

class G4Track;
class TrackInformation;
class VTrackInformation;

class TrackHelper {
public:
  TrackHelper(const G4Track* t);
  bool IsPrimary() const ;
  bool IsRegeneratedPrimary() const;
  bool IsRegisteredSecondary() const ;
  bool IsSecondary() const ;
  /**
   * @brief Return the truth id/status used for detector output.
   *
   * TrackInformation can keep both the generation-zero particle and the
   * current regenerated particle. Detector hit links and track records use
   * the generation-zero particle to preserve legacy output attribution,
   * falling back to the current particle for older TrackInformation objects.
   * "Barcode-only" track info keeps using its stored id/status values.
   */
  int GetUniqueID() const;
  int GetStatus() const ;
  /**
   * @brief Return the primary truth particle associated with this track.
   *
   * This is the primary-ancestor attribution stored on VTrackInformation.
   * It may differ from the generation-zero/current particle used by the
   * output-link helpers above, and is null when no track information or no
   * primary attribution is attached.
   */
  HepMC::GenParticlePtr GetPrimaryGenParticle();
  HepMC::ConstGenParticlePtr GetPrimaryGenParticle() const;
  /**
   * @brief Return concrete TrackInformation when callers need fields that
   * are not part of the VTrackInformation interface.
   */
  TrackInformation * GetTrackInformation();
  /**
   * @brief Generates a new HepMcParticleLink object on the
   * stack based on the generation-zero unique id, assuming that the
   * link should point at the first GenEvent in the McEventCollection.
   */
  inline HepMcParticleLink GenerateParticleLink();
  inline HepMcParticleLink GenerateParticleLink(IProxyDict*);
 private:
  inline std::tuple<int, HepMcParticleLink::UniqueIDFlag> particleIdentifierAndFlag() const;

  // TrackHelper also handles lightweight TrackBarcodeInfo instances, so the
  // cached pointer intentionally uses the common VTrackInformation base.
  VTrackInformation *m_trackInfo{};
};

inline std::tuple<int, HepMcParticleLink::UniqueIDFlag>
TrackHelper::particleIdentifierAndFlag() const
{
  return {GetUniqueID(), HepMcParticleLink::IS_ID};
}

HepMcParticleLink TrackHelper::GenerateParticleLink()
{
  const auto [identifier, flag] = particleIdentifierAndFlag();
  return HepMcParticleLink(identifier,
                           0,
                           HepMcParticleLink::IS_POSITION,
                           flag);
}

HepMcParticleLink TrackHelper::GenerateParticleLink(IProxyDict* proxy)
{
  const auto [identifier, flag] = particleIdentifierAndFlag();
  if(!proxy) {
    return GenerateParticleLink();
  }
  return HepMcParticleLink(identifier,
                           0,
                           HepMcParticleLink::IS_POSITION,
                           flag,
                           proxy);
}

#endif // MCTRUTH_TRACKHELPER_H
