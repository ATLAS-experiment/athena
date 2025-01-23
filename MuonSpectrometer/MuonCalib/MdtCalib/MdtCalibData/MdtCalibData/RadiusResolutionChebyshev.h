/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MdtCalibData_RadiusResoChebyshev_H
#define MdtCalibData_RadiusResoChebyshev_H

// standard C++ //
#include <cstdlib>
#include <iostream>

// STL //
#include <vector>

// MDT calibration //
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/IRtRelation.h"

namespace MuonCalib {
    /** @brief RadiusResolutionChebyshev parametrizes the uncertainty on the drift-radius as a function of the 
     *         drift radius itself and not of the drift time. It has a pointer to the corresponding 
     *         rt-relation which is used to translate the drift time into a radius before parsed through
     *         the parametrized chebyChev polynomial */

    class RadiusResolutionChebyshev : public IRtResolution {
    public:
        // Constructors
        /** initialization constructor,

        size of ParVec - 2 = order of the r(t) polynomial,

        ParVec[0] = t_low (smallest allowed drift time),
        ParVec[1] = t_up (largest allowed drift time).
        ParVec[2...] = parameters of the Chebyshev polynomial

        */
        RadiusResolutionChebyshev(const ParVec& vec, const IRtRelationPtr& rtRel);
        /** @brief Initialization from a  */

        // Methods //
        // methods required by the base classes //
        virtual std::string name() const override final;  //!< get the class name

        //!< get the resolution corresponding to the drift time t;
        //!< if t is not within [t_low, t_up] an unphysical radius of 99999 is
        //!< returned; the background rate is ignored in present implementation
        virtual double resolution(double t, double bgRate = 0.0) const override final;

        //!< get the number of parameters used to describe the resolution
        virtual unsigned int nDoF() const override final;

        //!< get the coefficients of the r(t) polynomial
        std::vector<double> resParameters() const;
      private:
          IRtRelationPtr m_rtRel{};
          double m_r_max{m_rtRel->radius(m_rtRel->tUpper())};
          double m_r_min{m_rtRel->radius(m_rtRel->tLower())};

    };
}  // namespace MuonCalib

#endif
