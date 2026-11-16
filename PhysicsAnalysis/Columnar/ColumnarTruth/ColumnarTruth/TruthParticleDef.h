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
  using TruthParticle0Def = TruthParticleDef;

  /// @brief additional truth particle container ids, for tools that use
  /// multiple truth particle containers at once (e.g. via variant links)
  struct TruthParticle1Def : TruthParticleDef
  {
    static constexpr std::string_view idName = "truthparticle1";
  };

  struct TruthParticle2Def : TruthParticleDef
  {
    static constexpr std::string_view idName = "truthparticle2";
  };

  using TruthParticleRange = ObjectRange<TruthParticleDef>;
  using TruthParticleId = ObjectId<TruthParticleDef>;
  using OptTruthParticleId = OptObjectId<TruthParticleDef>;
  template<typename CT,ColumnarModeConcept CM=ColumnarMode> using TruthParticleAccessor  = AccessorTemplate<TruthParticleDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarModeConcept CM=ColumnarMode> using TruthParticleDecorator = AccessorTemplate<TruthParticleDef,CT,ColumnAccessMode::output,CM>;
}

#endif
