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
  struct ParticleDef : RegularContainerId<xAOD::IParticle,xAOD::IParticleContainer>
  {
    static constexpr std::string_view idName = "particle";
  };
  using Particle0Def = ParticleDef;

  struct Particle1Def : ParticleDef
  {
    static constexpr std::string_view idName = "particle1";
  };

  struct Particle2Def : ParticleDef
  {
    static constexpr std::string_view idName = "particle2";
  };

  using ParticleRange = ObjectRange<ParticleDef>;
  using ParticleId = ObjectId<ParticleDef>;
  using OptParticleId = OptObjectId<ParticleDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using ParticleAccessor  = AccessorTemplate<ParticleDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using ParticleDecorator = AccessorTemplate<ParticleDef,CT,ColumnAccessMode::output,CM>;

  using Particle0Range = ObjectRange<Particle0Def>;
  using Particle0Id = ObjectId<Particle0Def>;
  using OptParticle0Id = OptObjectId<Particle0Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle0Accessor  = AccessorTemplate<Particle0Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle0Decorator = AccessorTemplate<Particle0Def,CT,ColumnAccessMode::output,CM>;

  using Particle1Range = ObjectRange<Particle1Def>;
  using Particle1Id = ObjectId<Particle1Def>;
  using OptParticle1Id = OptObjectId<Particle1Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle1Accessor  = AccessorTemplate<Particle1Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle1Decorator = AccessorTemplate<Particle1Def,CT,ColumnAccessMode::output,CM>;

  using Particle2Range = ObjectRange<Particle2Def>;
  using Particle2Id = ObjectId<Particle2Def>;
  using OptParticle2Id = OptObjectId<Particle2Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle2Accessor  = AccessorTemplate<Particle2Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Particle2Decorator = AccessorTemplate<Particle2Def,CT,ColumnAccessMode::output,CM>;
}

#endif
