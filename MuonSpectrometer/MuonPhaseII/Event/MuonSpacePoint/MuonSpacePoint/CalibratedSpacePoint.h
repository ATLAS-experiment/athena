/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
            using Cov_t = SpacePoint::Cov_t;
            using CovIdx = SpacePoint::CovIdx;
            /** @brief State flag to distinguish different space point states
             *      - Valid: Calibration of the space point was successful and it should be used in the fit
             *      - FailedCalib: The calibration procedure produced invalid constants and the space point shall not be included
             *                      in the current chi2 iteration, but may tried in the next cycle
             *      - Outlier: The Space point is an outlier and shall never be included in the fit. It's kept for the hit counting
             *                 purpose but nothing else */
            enum class State : std::uint8_t {
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
                                 State st = State::Valid);

            ~CalibratedSpacePoint() = default;
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

            /** @brief Set the covariance matrix of the calibrated space point */
            void setCovariance(const Cov_t& cov);
            /** @brief Update the drift radius of the space point measurement
             *  @param r: Radius to set */
            void setDriftRadius(const double r);
            
            /** @brief The pointer to the space point out of which this space point has been built */
            const SpacePoint* spacePoint() const;
            /** @brief Returns the space point type. If the calibrated space point is built without 
             *         a valid point to a spacePoint, e.g. external beamspot constraint, Other is returned */
            xAOD::UncalibMeasType type() const;
            /** @brief Set the time measurement
             *  @param t: Time of Record */
            void setTimeMeasurement(double t);
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
            /** @brief Sets the beamline direction */
            void setBeamDirection(Amg::Vector3D&& beamDir);

            friend std::ostream& operator<<(std::ostream& ostr, const CalibratedSpacePoint& sp) {
                    sp.print(ostr);
                    return ostr;
            }
        private:
            /** @brief Print function */
            void print(std::ostream& ostr) const;
            /** @brief Calibrated position */
            Amg::Vector3D m_posInChamber{Amg::Vector3D::Zero()};
            /** @brief Covariance array */
            Cov_t m_cov{Acts::filledArray<double, 3>(0.)};
            /** @brief Calibrated drift radius */
            double m_driftRadius{0.};
            /** @brief Calibrated time (Acts units) */
            double m_time{0.};
            /** @brief Uncalibrated space point from which this 
             *         space point is constructed */
            const SpacePoint* m_parent{nullptr};
            /** @brief Direction of the beamline (Beamspot constraint) */
            std::shared_ptr<Amg::Vector3D> m_beamLine{};
            /** @brief Calibration state */
            State m_state{State::Valid};
            /** @brief time flag (By default true for Mdt detectors) */
            bool m_measuresTime{type() == xAOD::UncalibMeasType::MdtDriftCircleType};
        
        };
        static_assert(Acts::Experimental::CompositeSpacePoint<CalibratedSpacePoint>);


}

#endif