/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKMEASUREMENT_V1_H
#define XAODTRACKING_VERSIONS_TRACKMEASUREMENT_V1_H
#include <cstdint>
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "EventPrimitives/EventPrimitives.h"



namespace xAOD {
    /**
     * @brief Track Measurements for Acts MultiTrajectory
     **/

    class TrackMeasurement_v1 : public SG::AuxElement {
    public:
        TrackMeasurement_v1() = default;
        /**
         * access track Measurements vector of const element
         **/
        template<std::size_t measdim = 6>
          Eigen::Map<const Eigen::Matrix<double, measdim, 1>> measEigen() const {
          return Eigen::Map<const Eigen::Matrix<double, measdim, 1>>{s_measAcc(*this).data()};
        }

        /**
        * access Measurements of non const element
        **/
        template<std::size_t measdim = 6>
          Eigen::Map<Eigen::Matrix<double, measdim, 1>> measEigen() {
          return Eigen::Map<Eigen::Matrix<double, measdim, 1>>{s_measAcc(*this).data()};
        }

        /**
        * access track Measurements as plain vector
        **/
        const std::vector<double>& meas() const;

        /**
        * access set Measurements from plain vector
        **/
        void setMeas( const std::vector<double>& m);

        /**
        * access track covariance matrix (flattened, rows layout) of const element
        **/
        template<std::size_t measdim = 6>
          Eigen::Map<const Eigen::Matrix<double, measdim, measdim>> covMatrixEigen() const {
          return Eigen::Map<const Eigen::Matrix<double, measdim, measdim>>{s_covMatrixAcc(*this).data()};
        }

        /**
        * access track covariance matrix (flattened, rows layout)
        **/
        template<std::size_t measdim = 6>
          Eigen::Map<Eigen::Matrix<double, measdim, measdim>> covMatrixEigen() {
          return Eigen::Map<Eigen::Matrix<double, measdim, measdim>> {s_covMatrixAcc(*this).data()};
        }

        /**
         * access track covariance as plain vector
         **/
        const std::vector<double>& covMatrix() const;

        /**
         * access set covariance from plain vector
         **/
        void setCovMatrix( const std::vector<double>& m);


        /**
         * @brief access the uncalibrated measurement
         */
        const xAOD::UncalibratedMeasurement *uncalibratedMeasurement() const;


        /**
         * @brief set uncalibrated measurement
         */
        void setUncalibratedMeasurement(const xAOD::UncalibratedMeasurement *uncalibrated_measurement);


        /**
        The quantities measured by detector, are functions of the state vector, corrupted by a measurement noise.
        However the state vector is normally not observed directly. The projector is mapping from the  state
        vector to the mesured quantities. In our case the projector is linear, i.e. represented by a matrix of ‘ones’.
        The projector matrix is coded by the bits of "unsigned long long" variable and the conversion to/from
        matrix is done in Acts.

         * Define projector access
         **/
        const std::uint64_t& projector() const;
        const std::uint64_t* projectorPtr() const;
				std::uint64_t* projectorPtr();
        void setProjector( const std::uint64_t& m);

        /**
         * @brief expands sizes of internal vectors for the data storage
         * ( by default this is 6 for Measurements and 6x6 for Covariance)
         * typically only 5 are used
         */
        void resize(size_t sz = 6);

        /**
        * @brief retrieve the size of the internal vectors for the data storage
        */
        size_t size() const;
      private:
        static const SG::Accessor<std::vector<double>> s_measAcc;
        static const SG::Accessor<std::vector<double>> s_covMatrixAcc;
    };
}
#endif
