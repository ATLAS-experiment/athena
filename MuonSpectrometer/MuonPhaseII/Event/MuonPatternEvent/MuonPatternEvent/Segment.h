/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNEVENT_SEGMENT__H
#define MUONR4_MUONPATTERNEVENT_SEGMENT__H

#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"

namespace MuonR4{

    /// @brief Placeholder for what will later be the muon segment EDM representation. 
    /// For now, just a plain storage for the dummy fit result, to test the 
    /// implementation of residuals 
    class Segment{
        public: 
            
            using MeasType = std::unique_ptr<CalibratedSpacePoint>;
            using MeasVec = std::vector<MeasType>;
            /** @brief Segment constructor
             *  @param globPos: Global position of the segment expressed at the associated chamber centre
             *  @param globDir: Global direction of the segment
             *  @param parent: Seed out of which the segment has been built
             *  @param constMeas: Measurements building up the segment
             *  @param chi2: Chi2 of the segment fit
             *  @param nDoF: Degrees of freedom
             */
            Segment(Amg::Vector3D&& globPos,
                    Amg::Vector3D&& globDir,
                    const SegmentSeed* parent,
                    MeasVec&& constMeas,
                    double chi2,
                    unsigned int nDoF):
                m_globPos{std::move(globPos)},
                m_globDir{std::move(globDir)},
                m_parent{parent},
                m_measurements{std::move(constMeas)},
                m_chi2{chi2}, 
                m_nDoF{nDoF}{}


            void setChi2PerMeasurement(std::vector<double> && chi2vals){
                m_chi2PerMeasurement = chi2vals; 
            }
            void setCallsToConverge(unsigned int nCalls) {
                m_nCalls = nCalls;
            }
            void setParUncertainties(SegmentFit::Covariance&& cov){
                m_cov = std::move(cov);
            }

            /** @brief Returns the associated chamber */
            const MuonGMR4::MuonChamber* chamber() const { return m_chamber; }
            /** @brief Returns the global segment position */
            const Amg::Vector3D& position() const { return m_globPos; }
            /** @brief Returns the global segment direction */
            const Amg::Vector3D& direction() const { return m_globDir; }
            /** @brief Returns the chi2 of the segment fit */
            double chi2() const { return m_chi2; }
            /** @brief Returns the number of degrees of freedom */
            unsigned int nDoF() const { return m_nDoF; }
            /** @brief Returns the associated measurements */
            const MeasVec& measurements() const {
                return m_measurements;
            }
            /** @brief Returns the seed out of which the segment was built */
            const SegmentSeed* parent() const { return m_parent; }
            /** @brief Returns the uncertainties of the defining parameters */
            const SegmentFit::Covariance& covariance() const { return m_cov; }

            const std::vector<double>& chi2PerMeasurement() const{
                return m_chi2PerMeasurement;
            } 
            void setSegmentT0(double t0) {
                m_t0 = t0;             
            }

        private: 
            Amg::Vector3D m_globPos{Amg::Vector3D::Zero()};
            Amg::Vector3D m_globDir{Amg::Vector3D::Zero()};
            const SegmentSeed* m_parent{nullptr};
            MeasVec m_measurements{};
            double m_chi2{0.};
            unsigned int m_nDoF{0};

            double m_t0{0.};

            const MuonGMR4::MuonChamber* m_chamber{m_parent->chamber()};
            std::vector<double> m_chi2PerMeasurement{};
            unsigned int m_nCalls{0};
            SegmentFit::Covariance m_cov{};
    };
    // placeholder - later will be xAOD EDM 
}

#endif
