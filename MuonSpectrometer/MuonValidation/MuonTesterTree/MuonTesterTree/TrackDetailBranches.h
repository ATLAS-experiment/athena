/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_TRACKDETAILBRANCHES_H
#define MUONTESTERTREE_TRACKDETAILBRANCHES_H
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

    /** @brief Utility class to calculate the energyloss from the track
     *         The energy loss is defined as the momentum of the last
     *         track state vs. the momentum of the first track state with
     *         measurement */
    class EnergylossBranch: public VectorBranch<float>,
                            virtual public IParticleDecorationBranch {
        public:
            EnergylossBranch(IParticleFourMomBranch& parent);

            using VectorBranch<float>::push_back;
            void push_back(const xAOD::IParticle* p) override;
            void push_back(const xAOD::IParticle& p) override;
            void operator+=(const xAOD::IParticle* p) override;
            void operator+=(const xAOD::IParticle& p) override;
    };
    /** @brief Utility branch to collect all the scatteres from the 
     *         associated Trk::Track */
    class ScatteringBranch : public MatrixBranch<float>,
                             virtual public IParticleDecorationBranch {
        public:
            using Base_t = MatrixBranch<float>;
            ScatteringBranch(IParticleFourMomBranch& parent);

            using Base_t::push_back;
            void push_back(const xAOD::IParticle* p) override;
            void push_back(const xAOD::IParticle& p) override;
            void operator+=(const xAOD::IParticle* p) override;
            void operator+=(const xAOD::IParticle& p) override;

        private:
          std::shared_ptr<Base_t> m_deltaTheta{};
          std::shared_ptr<Base_t> m_sigmaPhi{};
          std::shared_ptr<Base_t> m_sigmaTheta{};
          std::shared_ptr<VectorBranch<std::uint8_t>> m_nScat{};

    };

}
#endif
