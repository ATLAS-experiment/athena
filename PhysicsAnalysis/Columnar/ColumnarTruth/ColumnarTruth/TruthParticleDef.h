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

  template<ColumnarMode CM> using TruthParticleRange = ObjectRange<TruthParticleDef,CM>;
  template<ColumnarMode CM> using TruthParticleId = ObjectId<TruthParticleDef,CM>;
  template<ColumnarMode CM> using OptTruthParticleId = OptObjectId<TruthParticleDef,CM>;
  template<typename CT,ColumnarMode CM> using TruthParticleAccessor  = AccessorTemplate<TruthParticleDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using TruthParticleDecorator = AccessorTemplate<TruthParticleDef,CT,ColumnAccessMode::output,CM>;
}

#endif
