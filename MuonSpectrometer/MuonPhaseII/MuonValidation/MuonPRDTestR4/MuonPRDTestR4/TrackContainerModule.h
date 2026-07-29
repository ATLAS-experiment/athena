/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_TRACKCONTAINERMODULE_H
#define PRDTESTERR4_TRACKCONTAINERMODULE_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

namespace MuonValR4{
 /** @brief  Store the number of iterations of the global chi2 fitter
  *          to arrive at the minimum */
    class TrackFitIterBranch : public MuonVal::VectorBranch<std::uint16_t>,
                               virtual public MuonVal::IParticleDecorationBranch {
        public:
            TrackFitIterBranch(IParticleFourMomBranch& parent);

            using VectorBranch<std::uint16_t>::push_back;
            virtual void push_back(const xAOD::IParticle* p) override;
            virtual void push_back(const xAOD::IParticle& p) override;
            virtual void operator+=(const xAOD::IParticle* p) override;
            virtual void operator+=(const xAOD::IParticle& p) override;

    };
    /** @brief Record the L0, X0 and the number of material states on
     *         the track. */
    class MaterialRecorderBranch : public MuonVal::VectorBranch<float>,
                                   virtual public MuonVal::IParticleDecorationBranch {
        public:
            
            MaterialRecorderBranch(MuonVal::IParticleFourMomBranch& parent);

            using MuonVal::VectorBranch<float>::push_back;
            virtual void push_back(const xAOD::IParticle* p) override;
            virtual void push_back(const xAOD::IParticle& p) override;
            virtual void operator+=(const xAOD::IParticle* p) override;
            virtual void operator+=(const xAOD::IParticle& p) override;
            virtual bool init() override final;

        private:
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{"ActsAlignment"};
            std::shared_ptr<MuonVal::VectorBranch<float>> m_thickX0{};
            std::shared_ptr<MuonVal::VectorBranch<std::uint8_t>> m_nStates{};
    };  
    /** @brief Store the energy loss by comparing the first and last track
     *         state momentum */
    class EnergyLossBranch: public MuonVal::VectorBranch<float>,
                            virtual public MuonVal::IParticleDecorationBranch {
        public:
            EnergyLossBranch(MuonVal::IParticleFourMomBranch& parent);

            using MuonVal::VectorBranch<float>::push_back;
            virtual void push_back(const xAOD::IParticle* p) override;
            virtual void push_back(const xAOD::IParticle& p) override;
            virtual void operator+=(const xAOD::IParticle* p) override;
            virtual void operator+=(const xAOD::IParticle& p) override;  
    };                
}

#endif