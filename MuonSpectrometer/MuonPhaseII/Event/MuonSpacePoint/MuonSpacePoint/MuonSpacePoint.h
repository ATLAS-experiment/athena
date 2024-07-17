/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_MUONSPACEPOINT_H
#define MUONSPACEPOINT_MUONSPACEPOINT_H

#include "MuonReadoutGeometryR4/MuonChamber.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "CxxUtils/CachedUniquePtr.h"
namespace MuonR4 {
    /**
     *  @brief The muon space point is the combination of two uncalibrated measurements one of them 
     *          measures the eta and the other the phi coordinate. In cases, without a complementary measurment
     *          the spacepoint just represents the single measurement and hence has a uncertainty into the other
     *          direction corresponding to the half-length of the measurement channel
    */
    class MuonSpacePoint {
        public:
            /*** @brief: Constructor of the SpacePoint
             *   @param gctx: Geometry context needed to derive the local positions
             *   @param primaryMeas: Primary measurement of the spacepoint by convention that shall be the eta one
             *                       if both measurements are available
             *   @param secondaryMeas: The complementary phi measurement if availbe
            */
            MuonSpacePoint(const ActsGeometryContext& gctx,
                           const xAOD::UncalibratedMeasurement* primMeas,
                           const xAOD::UncalibratedMeasurement* secondMeas = nullptr);
            
            /*** @brief: Pointer to the primary measurement */
            const xAOD::UncalibratedMeasurement* primaryMeasurement() const;
            /*** @brief: Pointer to the secondary measurement */
            const xAOD::UncalibratedMeasurement* secondaryMeasurement() const;
            /*** @brief: Pointer to the associated muon chamber */
            const MuonGMR4::MuonChamber* muonChamber() const;
            /*** @brief: Position of the space point inside the chamber */
            const Amg::Vector3D& positionInChamber() const;
            /** @brief: Does the space point contain a phi measurement */
            bool measuresPhi() const;
            /** @brief: Does the space point contain an eta measurement */
            bool measuresEta() const;
            /** @brief: Identifier of the primary measurement */
            const Identifier& identify() const;
            /** @brief: Returns the size of the drift radius */
            double driftRadius() const;
            /** @brief: Returns the uncertainties on the space point */
            Amg::Vector2D uncertainty() const;
            const AmgSymMatrix(2)& covariance() const; 
            /** @brief: Equality check by checking the prd pointers */
            bool operator==(const MuonSpacePoint& other) const {
                return primaryMeasurement() == other.primaryMeasurement() &&
                       secondaryMeasurement() == other.secondaryMeasurement();
            }
            /** @brief Set the number of space points built with the same eta / phi prd. */
            void setInstanceCounts(unsigned int etaPrd, unsigned int phiPrd);
            /** @brief How many space points have been built in total with the same eta prd */
            unsigned int nEtaInstanceCounts() const;
            /** @brief How many space points have been built in total with the same phi prd  */
            unsigned int nPhiInstanceCounts() const;
        private:
            const xAOD::UncalibratedMeasurement* m_primaryMeas{nullptr};
            const xAOD::UncalibratedMeasurement* m_secondaryMeas{nullptr};

            Identifier m_id{xAOD::identify(m_primaryMeas)};
            const MuonGMR4::MuonChamber* m_chamber{xAOD::readoutElement(m_primaryMeas)->getChamber()};

            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            /** @brief: Measurement covariance 
             *          If the spacePoint represents an 1D measurement the second coordinate is the length of the
             *          channel (e.g halfLength of the wire or of the associated strip)
             *          the uncertainty of the other coordinate, otherwise
            */
            AmgSymMatrix(2) m_measCovariance{AmgSymMatrix(2)::Identity()}; 
            /// Drift radius of the associated drift circle - if there's any in the space point
            double m_driftR{0.};
            /// In how many space points is the eta measurement used
            unsigned int m_etaInstances{1};
            /// In how many space points is the phi measurement used
            unsigned int m_phiInstances{1};
    };

}


#endif
