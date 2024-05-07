#ifndef COUNT_IPARTICLE_ALG_H
#define COUNT_IPARTICLE_ALG_H

#include "LinkCounterAlg.h"

#include "xAODBase/IParticleContainer.h"

namespace detail {
  using CountIParticle_t = FlavorTagDiscriminants::LinkCounterAlg<
    xAOD::IParticleContainer, xAOD::IParticleContainer>;
}

namespace FlavorTagDiscriminants {
  class CountIParticleAlg: public detail::CountIParticle_t
  {
  public:
    CountIParticleAlg(const std::string& name, ISvcLocator* loc);
  };
}

#endif
