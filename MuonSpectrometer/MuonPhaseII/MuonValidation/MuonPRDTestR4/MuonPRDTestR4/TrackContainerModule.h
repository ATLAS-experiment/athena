/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_TRACKCONTAINERMODULE_H
#define PRDTESTERR4_TRACKCONTAINERMODULE_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "ActsEvent/TrackContainer.h"


namespace MuonValR4{
 /** @brief  Dump the number of iterations to construct an Acts track */
    class TrackFitIterBranch : public MuonVal::VectorBranch<std::uint16_t>,
                               virtual public MuonVal::IParticleDecorationBranch {
        public:
            TrackFitIterBranch(IParticleFourMomBranch& parent);

            using VectorBranch<std::uint16_t>::push_back;
            void push_back(const xAOD::IParticle* p) override;
            void push_back(const xAOD::IParticle& p) override;
            void operator+=(const xAOD::IParticle* p) override;
            void operator+=(const xAOD::IParticle& p) override;

    };
}

#endif