/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNEVENT_SEGMENT__H
#define MUONR4_MUONPATTERNEVENT_SEGMENT__H

#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"

namespace MuonR4{

    /// @brief Placeholder for what will later be the muon segment EDM representation. 
    /// For now, just a plain storage for the dummy fit result, to test the 
    /// implementation of residuals 
    class Segment{
        public: 
            
            /** @brief Calibrated space point type */
            using MeasType = std::unique_ptr<CalibratedSpacePoint>;
            using MeasVec = std::vector<MeasType>;
            /** @brief Segment constructor
             *  @param globPos: Global position of the segment expressed at the associated chamber centre
             *  @param globDir: Global direction of the segment
             *  @param parent: Seed out of which the segment has been built
             *  @param constMeas: Measurements building up the segment
             *  @param chi2: Chi2 of the segment fit
             *  @param nDoF: Degrees of freedom */
            Segment(Amg::Vector3D&& globPos,
                    Amg::Vector3D&& globDir,
                    const SegmentSeed* parent,
                    MeasVec&& constMeas,
                    double chi2,
                    unsigned int nDoF);
            /** @brief Returns the associated MS sector */
            const MuonGMR4::SpectrometerSector* msSector() const { return m_parent->msSector(); }
            /** @brief Returns the global segment position */
            const Amg::Vector3D& position() const { return m_globPos; }
            /** @brief Returns the global segment direction */
            const Amg::Vector3D& direction() const { return m_globDir; }
            /** @brief Returns the chi2 of the segment fit */
            double chi2() const { return m_chi2; }
            /** @brief Returns the number of degrees of freedom */
            unsigned int nDoF() const { return m_nDoF; }
            /** @brief Returns the associated measurements */
            const MeasVec& measurements() const { return m_measurements; }
            /** @brief Returns the seed out of which the segment was built */
            const SegmentSeed* parent() const { return m_parent; }
            /** @brief Returns the uncertainties of the defining parameters */
            const SegmentFit::Covariance& covariance() const { return m_cov; }
            /** @brief Returns how many iterations the fitter needed to make the segment converge */ 
            unsigned int nFitIterations() const { return m_nCalls; }
            /** @brief has the time been fitted */
            bool hasTimeFit() const { return m_t0 != std::nullopt; }
            /** @brief Returns the fitted segment time, if there's any */
            double segementT0() const { return m_t0.value_or(0); }
            /** @brief Helper struct to summarize the hit count  */
            struct HitSummary{
                /** @brief Number of good Mdt / Mm / sTgc eta hits */
                unsigned nPrecHits{0};
                /** @brief Number of good Rpc / Tgc eta hits */
                unsigned nEtaTrigHits{0};
                /** @brief Number of good Rpc / Tgc / sTgc phi hits */
                unsigned nPhiHits{0};
                /** @brief Number of Mdt / Mm / sTGC eta outliers */
                unsigned nPrecOutlier{0};
                /** @brief Precision technology */
                xAOD::UncalibMeasType tech{xAOD::UncalibMeasType::Other};
            };   
            /** @brief Returns the hit summary */
            const HitSummary& summary() const { return m_summary; }
            
            /** @brief Sets the fitted segment time */
            void setSegmentT0(double t0);
            /** @brief Set how many iteration the fitter needed to reach convergence */
            void setCallsToConverge(unsigned int nCalls);
            /** @brief Set the uncertainties from the fit */
            void setParUncertainties(SegmentFit::Covariance&& cov);
        private: 
            /** @brief Global position of the segment at the chamber centre */
            Amg::Vector3D m_globPos{Amg::Vector3D::Zero()};
            /** @brief Global direction of the segment */
            Amg::Vector3D m_globDir{Amg::Vector3D::Zero()};
            /** @brief Seed from which the segment is stemming */
            const SegmentSeed* m_parent{nullptr};
            /** @brief List of associated measurements */
            MeasVec m_measurements{};
            /*** @brief Chi2 of the segment fit */
            double m_chi2{0.};
            /** @brief Number of degrees of freedom in the fit */
            unsigned int m_nDoF{0};
            /** @brief Fitted time of arrival at the chamber centre */
            std::optional<double> m_t0{std::nullopt};
            /** @brief Number of calls to reach the minimum */
            unsigned int m_nCalls{0};
            /** @brief Covariance matrix of the fit  */
            SegmentFit::Covariance m_cov{SegmentFit::Covariance::Identity()};
            /** @brief Calculate the hit summary */
            HitSummary m_summary{};
    };
}

#endif
