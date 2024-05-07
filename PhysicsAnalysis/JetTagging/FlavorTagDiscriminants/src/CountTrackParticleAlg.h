#ifndef COUNT_TRACK_PARTICLE_ALG_H
#define COUNT_TRACK_PARTICLE_ALG_H

#include "LinkCounterAlg.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODBTagging/BTaggingContainer.h"

namespace detail {
  using CountTrackParticle_t = FlavorTagDiscriminants::LinkCounterAlg<
    xAOD::TrackParticleContainer, xAOD::BTaggingContainer>;
}

namespace FlavorTagDiscriminants {
  class CountTrackParticleAlg: public detail::CountTrackParticle_t
  {
  public:
    CountTrackParticleAlg(const std::string& name, ISvcLocator* loc);
  };
}

#endif
