/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_CALIBSPACEPOINT_H
#define MUONSPACEPOINT_CALIBSPACEPOINT_H

#include <MuonSpacePoint/SpacePoint.h>

namespace MuonR4{
    /** @brief The calibrated Space point is created during the calibration process.
     *         It usually exploits the information of the external tracking seed. Calibrated
     *         space points may also be created without a link to a measurement space point. 
     *         In this case, they serve in an analogous way as the Trk::PseudoMeasurement */
    class CalibratedSpacePoint {
        public:
            CalibratedSpacePoint(const SpacePoint* uncalibSpacePoint,
                                 Amg::Vector3D&& posInChamber,
                                 Amg::Vector3D&& dirInChamber);

            ~CalibratedSpacePoint() = default;

            /** @brief The position of the calibrated space point inside the chamber */
            const Amg::Vector3D& positionInChamber() const;
            /** @brief The direction of the calibrated space point inside the chamber */
            const Amg::Vector3D& directionInChamber() const;
            /** @brief The drift radius of the calibrated space point. Needs to be set externally */
            double driftRadius() const;
            /** @brief Set the drift radius of the calibrated space point after the calibration procedure */
            void setDriftRadius(const double r);
            /** @brief The spatial covariance mattrix of the calibrated space point */
            const AmgSymMatrix(2)& covariance() const;
            /** @brief Set the covariance matrix of the calibrated space pooint */
            void setCovariance(AmgSymMatrix(2)&& cov);
            /** @brief The pointer to the space point out of which this space point has been built */
            const SpacePoint* spacePoint() const;
            /** @brief Returns the space point type. If the calibrated space point is built without 
             *         a valid point to a spacePoint, e.g. external beamspot constraint, Other is returned */
            xAOD::UncalibMeasType type() const;
            /** @brief Current time of the calibrated space point */
            double time() const;
            /** @brief Uncertainty of the time measurement */
            double timeCovariance() const;
            /** @brief Set the time measurement
             *  @param t: Time of arrival
             *  @param errT: Uncertainty on the time measurement */
            void setTimeMeasurement(double t, double errT);
        private:
            const SpacePoint* m_parent{nullptr};
            Amg::Vector3D m_posInChamber{Amg::Vector3D::Zero()};
            Amg::Vector3D m_dirInChamber{Amg::Vector3D::Zero()};
            
            double m_driftRadius{0.};
            AmgSymMatrix(2) m_cov{AmgSymMatrix(2)::Identity()};

            double m_time{0.};
            double m_timeCov{0.};
    };

}

#endif