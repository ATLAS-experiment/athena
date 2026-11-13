/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSTRK_TRUTHPERIGEEDECORATORALG_H
#define ACTSTRK_TRUTHPERIGEEDECORATORALG_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODTruth/TruthParticleContainer.h"

namespace ActsTrk {
    /** @brief Decorate the perigee d0 and z0 parameters onto a TruthParticle
     *         Charged stable truth particles are extrapolated back to the perigee
     *         surface and then the d0, z0 result is decorated. Further, the algorithm
     *         decorates the production radius and the z production of the charged particle */
    class TruthPerigeeDecorationAlg : public AthReentrantAlgorithm {
        public:
          using AthReentrantAlgorithm::AthReentrantAlgorithm;
          virtual StatusCode initialize() override final;
          virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
          ToolHandle<IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
          /** @brief Dependency on the input truth particle container */
          SG::ReadHandleKey<xAOD::TruthParticleContainer> m_readKey{this, "TruthParticleContainer", "TruthParticles"};
          /** @brief Declare the d0 decoration to the scheduler */
          SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_d0Key{this, "d0Decoration", m_readKey, "d0" };
          /** @brief Declare the z0 decoration to the scheduler */
          SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_z0Key{this, "z0Decoration", m_readKey, "z0"};
          /** @brief Declare the zProd decoration to the scheduler */
          SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_ZProdKey{this, "z0ProdDecoration", m_readKey, "prodZ"};
          /** @brief Declare the rProd decoration to the scheduler */
          SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_RProdKey{this, "rProdDecoration", m_readKey, "prodR"};
          /** @brief Flag indicating whether the decoration happened */
          SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_isDecorKey{this, "isDecorated", m_readKey, "perigeeDecorated"};
    };
}

#endif