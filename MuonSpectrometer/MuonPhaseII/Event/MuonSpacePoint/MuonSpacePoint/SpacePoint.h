/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_SPACEPOINT_H
#define MUONSPACEPOINT_SPACEPOINT_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "Acts/EventData/CompositeSpacePoint.hpp"
#include "Acts/Seeding/detail/CompSpacePointAuxiliaries.hpp"
#include "Acts/Utilities/ArrayHelpers.hpp"
#include "Acts/Utilities/Helpers.hpp"
namespace MuonR4 {
    /**
     *  @brief The muon space point is the combination of two uncalibrated measurements one of them 
     *          measures the eta and the other the phi coordinate. In cases, without a complementary measurment
     *          the spacepoint just represents the single measurement and hence has a uncertainty into the other
     *          direction corresponding to the half-length of the measurement channel
    */
   
    class SpacePoint {
        public:
            /** @brief Abrivation of the covariance type */
            using Cov_t = std::array<double, 3>;
            /** @brief Enum to define the components of the covariance array */
            using SeedingAux = Acts::Experimental::detail::CompSpacePointAuxiliaries;
            enum class CovIdx: std::uint8_t {
                phiCov = Acts::toUnderlying(SeedingAux::ResidualIdx::nonBending),
                etaCov = Acts::toUnderlying(SeedingAux::ResidualIdx::bending),
                timeCov = Acts::toUnderlying(SeedingAux::ResidualIdx::time)
            };
            /*** @brief: Constructor of the SpacePoint
             *   @param primaryMeas: Primary measurement of the spacepoint by convention that shall be the eta one
             *                       if both measurements are available
             *   @param secondaryMeas: The complementary phi measurement if available */
            SpacePoint(const xAOD::UncalibratedMeasurement* primMeas,
                       const xAOD::UncalibratedMeasurement* secondMeas = nullptr);
            /*** @brief: Position of the space point inside the chamber */
            const Amg::Vector3D& localPosition() const;
            /*** @brief: Returns the direction parallel to the primary channel, i.e. the strip or the wire */
            const Amg::Vector3D& sensorDirection() const;
            /*** @brief: Returns the vector pointing to the adjacent channel in the chamber */
            const Amg::Vector3D& toNextSensor() const;
            /** @brief Returns the vector pointing out of the measurement plane */
            const Amg::Vector3D& planeNormal() const;
            /** @brief Returns the measurement's recorded time */
            double time() const;
            /** @brief Returns whether the measurement is a Mdt */
            bool isStraw() const;
            /** @brief Returns whether the measurement carries time information */
            bool hasTime() const;
            /** @brief Returns whether the measurement constains the non-bending direction*/
            bool measuresLoc0() const;
            /** @brief Returns whether the measurement constains the bending direction */
            bool measuresLoc1() const;
            /** @brief: Returns the size of the drift radius */
            double driftRadius() const;
            /** @brief Returns the covariance array */
            const Cov_t& covariance() const; 
            /*** @brief  Setter for the measurement covariance */


            void setCovariance(Cov_t&& cov);
            /** @brief  Setter for the direction of the measurement channel in the sector frame
             *  @param sensorDir: Direction of the sensor
             *  @param toNextSensor: Vector pointing to the next sensor inside the plane */
            void setDirection(const Amg::Vector3D& sensorDir,
                              const Amg::Vector3D& toNextSensor);
            /*** @brief  Setter for the position of the uncalibrated muon measurement in the sector frame */
            void setPosition(Amg::Vector3D&& pos);

            /*** @brief: Pointer to the primary measurement */
            const xAOD::UncalibratedMeasurement* primaryMeasurement() const;
            /*** @brief: Pointer to the secondary measurement */
            const xAOD::UncalibratedMeasurement* secondaryMeasurement() const;
            /*** @brief: Pointer to the associated ms sector */
            const MuonGMR4::SpectrometerSector* msSector() const;
            /** @brief: Pointer to the associated chamber */
            const MuonGMR4::Chamber* chamber() const;

            /*** @brief: Returns the measurement type of the primary measurement */
            xAOD::UncalibMeasType type() const;
            /** @brief: Does the space point contain a phi measurement */
            bool measuresPhi() const;
            /** @brief: Does the space point contain an eta measurement */
            bool measuresEta() const;
            /** @brief: Identifier of the primary measurement */
            const Identifier& identify() const;
            /** @brief: Equality check by checking the prd pointers */
            bool operator==(const SpacePoint& other) const {
                return primaryMeasurement() == other.primaryMeasurement() &&
                       secondaryMeasurement() == other.secondaryMeasurement();
            }
            /** @brief Set the number of space points built with the same eta / phi prd. */
            void setInstanceCounts(std::shared_ptr<unsigned> etaCounts,
                                   std::shared_ptr<unsigned> phiCounts);
            /** @brief How many space points have been built in total with the same eta prd */
            unsigned nEtaInstanceCounts() const;
            /** @brief How many space points have been built in total with the same phi prd  */
            unsigned nPhiInstanceCounts() const;
            /** @brief Is the space point a 1D or combined 2D measurement */
            unsigned dimension() const;
            /** @brief The print-out operator */
            friend std::ostream& operator<<(std::ostream& ostr, const SpacePoint& sp) {
                    sp.print(ostr);
                    return ostr;
            }
        private:
            void print(std::ostream& ostr) const;
            const xAOD::UncalibratedMeasurement* m_primaryMeas{nullptr};
            const xAOD::UncalibratedMeasurement* m_secondaryMeas{nullptr};

            const MuonGMR4::Chamber* m_chamber{xAOD::muonReadoutElement(m_primaryMeas)->chamber()};
            const MuonGMR4::SpectrometerSector* m_msSector{m_chamber->parent()};
            /** @brief Flag indicating that the measurement is an eta measurement */
            bool m_measEta{!m_msSector->idHelperSvc()->measuresPhi(identify())};
            /** @brief Local position inside the msSector */
            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            /** @brief Local sensor direction */
            Amg::Vector3D m_dir{Amg::Vector3D::Zero()};
            /** @brief Direction to the next sensor */
            Amg::Vector3D m_toNext{Amg::Vector3D::Zero()};
            /** @brief Direction vector pointing outside the sensor plane*/
            Amg::Vector3D m_normal{Amg::Vector3D::Zero()};
            /** @brief Measurement covariance. The first index represents the uncertainty in the 
             *         phi-direction, the second the uncerainty in the precision direction and 
             *         the last component is the time covariance. By convention a pure 1D measurement
             *         has the strip half-length filled in the complementary component */
            Cov_t m_measCovariance{Acts::filledArray<double, 3>(0.)}; 
            /// In how many space points is the eta measurement used
            std::shared_ptr<const unsigned> m_etaInstances{std::make_shared<unsigned>(1)};
            /// In how many space points is the phi measurement used
            std::shared_ptr<const unsigned> m_phiInstances{std::make_shared<unsigned>(1)};
    };
    static_assert(Acts::Experimental::CompositeSpacePoint<SpacePoint>);
}


#endif
