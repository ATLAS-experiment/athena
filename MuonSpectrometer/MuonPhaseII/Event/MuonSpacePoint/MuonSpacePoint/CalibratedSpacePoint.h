/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_CALIBSPACEPOINT_H
#define MUONSPACEPOINT_CALIBSPACEPOINT_H

#include <MuonSpacePoint/SpacePoint.h>
#include <variant>

namespace MuonR4{
    /** @brief The calibrated Space point is created during the calibration process.
     *         It usually exploits the information of the external tracking seed. Calibrated
     *         space points may also be created without a link to a measurement space point. 
     *         In this case, they serve in an analogous way as the Trk::PseudoMeasurement */
    class CalibratedSpacePoint {
        public:
            
            /** @brief State flag to distinguish different space point states
             *      - Valid: Calibration of the space point was successful and it should be used in the fit
             *      - FailedCalib: The calibration procedure produced invalid constants and the space point shall not be included
             *                      in the current chi2 iteration, but may tried in the next cycle
             *      - Outlier: The Space point is an outlier and shall never be included in the fit. It's kept for the hit counting
             *                 purpose but nothing else */
            enum class State : uint8_t {
                Valid = 0,
                FailedCalib = 1,
                Outlier = 2,
            };
            
            /** @brief Standard constructor
             *  @param uncalibSpacePoint: Pointer to the underyling uncalibrated space point
             *  @param posInChamber: Calibrated position of the space point inside the chamber
             *  @param dirInChamber: Direction of the space point in chamber */
            CalibratedSpacePoint(const SpacePoint* uncalibSpacePoint,
                                 Amg::Vector3D&& posInChamber,
                                 Amg::Vector3D&& dirInChamber,
                                 State st = State::Valid);

            ~CalibratedSpacePoint() = default;
            /** @brief The position of the calibrated space point inside the chamber */
            const Amg::Vector3D& positionInChamber() const;
            /** @brief The direction of the calibrated space point inside the chamber */
            const Amg::Vector3D& directionInChamber() const;
            /** @brief The drift radius of the calibrated space point. Needs to be set externally */
            double driftRadius() const;
            /** @brief Set the drift radius of the calibrated space point after the calibration procedure */
            void setDriftRadius(const double r);
            /** @brief The spatial covariance matrix of the calibrated space point */
            using Covariance_t = std::variant<AmgSymMatrix(2), AmgSymMatrix(3)>;
            const Covariance_t& covariance() const;
            /** @brief Set the covariance matrix of the calibrated space pooint */
            template <unsigned k>
                void setCovariance(const AmgSymMatrix(k)& cov){
                    static_assert(k==2 || k==3, "Covariance dimension needs to be 2 or 3");
                    m_cov = cov;
                }
            /** @brief The pointer to the space point out of which this space point has been built */
            const SpacePoint* spacePoint() const;
            /** @brief Returns the space point type. If the calibrated space point is built without 
             *         a valid point to a spacePoint, e.g. external beamspot constraint, Other is returned */
            xAOD::UncalibMeasType type() const;
            /** @brief Current time of the calibrated space point */
            double time() const;
            /** @brief Set the time measurement
             *  @param t: Time of arrival */
            void setTimeMeasurement(double t);
            /** @brief Returns whether the calibrated space point measures time*/
            bool measuresTime() const;
            /** @brief Returns whether the calibrated space point measures phi */
            bool measuresPhi() const;
            /** @brief Returns whether the calibrated space point measures eta */
            bool measuresEta() const;
            /** @brief Returns the state of the calibrated space point */
            State fitState() const;
            /** @brief Set the state of the calibrated space point */
            void setFitState(State st);
            /** @brief Returns the local dimension of the measurement */
            unsigned dimension() const;
        private:
            const SpacePoint* m_parent{nullptr};
            Amg::Vector3D m_posInChamber{Amg::Vector3D::Zero()};
            Amg::Vector3D m_dirInChamber{Amg::Vector3D::Zero()};
            
            double m_driftRadius{0.};
            Covariance_t m_cov{};

            double m_time{0.};
            /// By default the Mdt may measure time
            bool m_measuresTime{type() == xAOD::UncalibMeasType::MdtDriftCircleType};
            State m_state{State::Valid};
    };

}

#endif