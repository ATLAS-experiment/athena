/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_PARTICLE_DEF_H
#define COLUMNAR_CORE_PARTICLE_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODBase/IParticleContainer.h>

namespace columnar
{
  namespace ContainerId
  {
    struct particle : regularCIBase<xAOD::IParticle,xAOD::IParticleContainer>
    {
      static constexpr std::string_view idName = "particle";
    };
    using particle0 = particle;

    struct particle1 : particle
    {
      static constexpr std::string_view idName = "particle1";
    };

    struct particle2 : particle
    {
      static constexpr std::string_view idName = "particle2";
    };
  }

  using ParticleRange = ObjectRange<ContainerId::particle>;
  using ParticleId = ObjectId<ContainerId::particle>;
  using OptParticleId = OptObjectId<ContainerId::particle>;
  template<typename CT,typename CM=ColumnarModeDefault> using ParticleAccessor  = AccessorTemplate<ContainerId::particle,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using ParticleDecorator = AccessorTemplate<ContainerId::particle,CT,ColumnAccessMode::output,CM>;

  using Particle0Range = ObjectRange<ContainerId::particle0>;
  using Particle0Id = ObjectId<ContainerId::particle0>;
  using OptParticle0Id = OptObjectId<ContainerId::particle0>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle0Accessor  = AccessorTemplate<ContainerId::particle0,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle0Decorator = AccessorTemplate<ContainerId::particle0,CT,ColumnAccessMode::output,CM>;

  using Particle1Range = ObjectRange<ContainerId::particle1>;
  using Particle1Id = ObjectId<ContainerId::particle1>;
  using OptParticle1Id = OptObjectId<ContainerId::particle1>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle1Accessor  = AccessorTemplate<ContainerId::particle1,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle1Decorator = AccessorTemplate<ContainerId::particle1,CT,ColumnAccessMode::output,CM>;

  using Particle2Range = ObjectRange<ContainerId::particle2>;
  using Particle2Id = ObjectId<ContainerId::particle2>;
  using OptParticle2Id = OptObjectId<ContainerId::particle2>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle2Accessor  = AccessorTemplate<ContainerId::particle2,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle2Decorator = AccessorTemplate<ContainerId::particle2,CT,ColumnAccessMode::output,CM>;
}

#endif
