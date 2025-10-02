/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_BLINEPAR_H
#define MUONALIGNMENTDATA_BLINEPAR_H

#include "MuonAlignmentData/MuonAlignmentPar.h"
#include <array>
#include <limits>
#include <iostream>
#include <algorithm>

class BLinePar : public MuonAlignmentPar {
public:
    // Default constructor
    BLinePar() = default;
    // destructor
    virtual ~BLinePar() override = default;

    enum class Parameter{
       bz=0,  // tube bow in plane,
       bp,    // tube bow out of plane,
       bn,    // tube bow out of plane
       sp,    // cross plate sag out of plane
       sn,    // cross plate sag out of plane
       tw,    // twist
       pg,    // parallelogram
       tr,    // trapezoid
       eg,    // global expansion
       ep,    // local expansion
       en,    // local expansion
       numPars
    };
    static constexpr double expansionScale = 1.e-3;
    /// Cast the parameter to an unsigned int    
    void setParameters(float bz, float bp, float bn, float sp, float sn, float tw, float pg, float tr, float eg, float ep, float en);

    /// @brief Returns a given parameter
    float getParameter(const Parameter p) const {
       return m_payload[static_cast<unsigned int>(p)];
    }    
    /// @brief  Returns true if at least one of the payload parameters is set
    operator bool () const {
      return std::find_if(m_payload.begin(),
                          m_payload.end(),[](const float par){
                            return std::abs(par) > std::numeric_limits<float>::epsilon();
                          }) != m_payload.end();
    }

private:
    std::array<float, static_cast<unsigned int>(Parameter::numPars)> m_payload{};
};

std::ostream& operator<<(std::ostream& ostr, const BLinePar& par);


#endif  // MUONALIGNMENTDATA_BLINEPAR_H
