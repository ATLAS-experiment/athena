/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TRUTH_TRUTH_PARTICLE_DEF_H
#define COLUMNAR_TRUTH_TRUTH_PARTICLE_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace columnar
{
  struct TruthParticleDef : RegularContainerId<xAOD::TruthParticle,xAOD::TruthParticleContainer>
  {
    static constexpr std::string_view idName = "truthparticle";
  };

  using TruthParticleRange = ObjectRange<TruthParticleDef>;
  using TruthParticleId = ObjectId<TruthParticleDef>;
  using OptTruthParticleId = OptObjectId<TruthParticleDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using TruthParticleAccessor  = AccessorTemplate<TruthParticleDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using TruthParticleDecorator = AccessorTemplate<TruthParticleDef,CT,ColumnAccessMode::output,CM>;
}

#endif
