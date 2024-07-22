/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_TRACKCHI2BRANCH_H
#define MUONTESTERTREE_TRACKCHI2BRANCH_H
#include <MuonTesterTree/VectorBranch.h>
#include <MuonTesterTree/IParticleFourMomBranch.h>
namespace MuonVal{
    /** @brief Dump the chi2 / nDof of a muon or a track particle */
    class TrackChi2Branch : public VectorBranch<float>,
                            virtual public IParticleDecorationBranch {
        public:
            TrackChi2Branch(IParticleFourMomBranch& parent);

            using VectorBranch<float>::push_back;
            void push_back(const xAOD::IParticle* p) override;
            void push_back(const xAOD::IParticle& p) override;
            void operator+=(const xAOD::IParticle* p) override;
            void operator+=(const xAOD::IParticle& p) override;

        private:
            std::shared_ptr<VectorBranch<unsigned int>> m_nDoF{nullptr};
    };
}
#endif
