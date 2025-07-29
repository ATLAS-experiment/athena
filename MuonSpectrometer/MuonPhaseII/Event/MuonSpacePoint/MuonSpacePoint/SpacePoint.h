/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_SPACEPOINT_H
#define MUONSPACEPOINT_SPACEPOINT_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/UtilFunctions.h"

namespace MuonR4 {
    /**
     *  @brief The muon space point is the combination of two uncalibrated measurements one of them 
     *          measures the eta and the other the phi coordinate. In cases, without a complementary measurment
     *          the spacepoint just represents the single measurement and hence has a uncertainty into the other
     *          direction corresponding to the half-length of the measurement channel
    */
    class SpacePoint {
        public:
            

            /*** @brief: Constructor of the SpacePoint
             *   @param primaryMeas: Primary measurement of the spacepoint by convention that shall be the eta one
             *                       if both measurements are available
             *   @param secondaryMeas: The complementary phi measurement if availbe
            */
            SpacePoint(const xAOD::UncalibratedMeasurement* primMeas,
                       const xAOD::UncalibratedMeasurement* secondMeas = nullptr);

            /*** @brief  Setter for the measurement covariance */
            void setCovariance(AmgSymMatrix(2)&& cov){ m_measCovariance = std::move(cov); };
            /*** @brief  Setter for the direction of the measurement channel in the sector frame */
            void setDirection(Amg::Vector3D&& dir){ m_dir = std::move(dir); };
            /*** @brief  Setter for the direction of the precision axis of the measurement in the sector frame */
            void setNormal(Amg::Vector3D&& normal){ m_normal = std::move(normal); };
            /*** @brief  Setter for the position of the uncalibrated muon measurement in the sector frame */
            void setPosition(Amg::Vector3D&& pos){ m_pos = std::move(pos); };
            
            /*** @brief: Pointer to the primary measurement */
            const xAOD::UncalibratedMeasurement* primaryMeasurement() const;
            /*** @brief: Pointer to the secondary measurement */
            const xAOD::UncalibratedMeasurement* secondaryMeasurement() const;
            /*** @brief: Pointer to the associated ms sector */
            const MuonGMR4::SpectrometerSector* msSector() const;
            /** @brief: Pointer to the associated chamber */
            const MuonGMR4::Chamber* chamber() const;
            /*** @brief: Position of the space point inside the chamber */
            const Amg::Vector3D& positionInChamber() const;
            /*** @brief: Returns the direction parallel to the primary channel, i.e. the strip or the wire */
            const Amg::Vector3D& directionInChamber() const;
            /*** @brief: Returns the vector pointing to the adjacent channel in the chamber */
            const Amg::Vector3D& normalInChamber() const;
            /** @brief Returns the vector pointing out of the measurement plane */
            Amg::Vector3D planeNormal() const;
            /*** @brief: Returns the measurement type of the primary measurement */
            xAOD::UncalibMeasType type() const;
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
            bool operator==(const SpacePoint& other) const {
                return primaryMeasurement() == other.primaryMeasurement() &&
                       secondaryMeasurement() == other.secondaryMeasurement();
            }
            /** @brief Set the number of space points built with the same eta / phi prd. */
            void setInstanceCounts(std::shared_ptr<unsigned> etaCounts,
                                   std::shared_ptr<unsigned> phiCounts);
            /** @brief How many space points have been built in total with the same eta prd */
            unsigned int nEtaInstanceCounts() const;
            /** @brief How many space points have been built in total with the same phi prd  */
            unsigned int nPhiInstanceCounts() const;
            /** @brief Is the space point a 1D or combined 2D measurement */
            unsigned int dimension() const;
        private:
            const xAOD::UncalibratedMeasurement* m_primaryMeas{nullptr};
            const xAOD::UncalibratedMeasurement* m_secondaryMeas{nullptr};

            const MuonGMR4::Chamber* m_chamber{xAOD::muonReadoutElement(m_primaryMeas)->chamber()};
            const MuonGMR4::SpectrometerSector* m_msSector{m_chamber->parent()};
            /** @brief Flag indicating that the measurement is an eta measurement */
            bool m_measEta{!m_msSector->idHelperSvc()->measuresPhi(identify())};
            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            Amg::Vector3D m_dir{Amg::Vector3D::Zero()};
            Amg::Vector3D m_normal{Amg::Vector3D::Zero()};
            /** @brief: Measurement covariance 
             *          If the spacePoint represents an 1D measurement the second coordinate is the length of the
             *          channel (e.g halfLength of the wire or of the associated strip)
             *          the uncertainty of the other coordinate, otherwise
            */
            AmgSymMatrix(2) m_measCovariance{AmgSymMatrix(2)::Identity()}; 
            /// In how many space points is the eta measurement used
            std::shared_ptr<const unsigned> m_etaInstances{std::make_shared<unsigned>(1)};
            /// In how many space points is the phi measurement used
            std::shared_ptr<const unsigned> m_phiInstances{std::make_shared<unsigned>(1)};
    };
}


#endif
