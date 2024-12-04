/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// c- c++
#include "iostream"

// this
#include "MdtCalibData/RtSpline.h"

// root
#include "GeoModelKernel/throwExcept.h"
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "TSpline.h"

namespace MuonCalib {
    RtSpline::~RtSpline() = default;
    RtSpline::RtSpline(const ParVec &vec) : 
        IRtRelation(vec) {
        // check for minimum number of parameters
        if (nPar() < 6) {
            THROW_EXCEPTION("Not enough parameters!" << std::endl<< "Minimum number of parameters is 6!");
        }
        // check if the number of parameters is even
        if ((nPar() % 2) != 0) {
            THROW_EXCEPTION("RtSpline::_init(): Odd number of parameters!");
        }
        // create spline
        std::vector<double> x(nPar() /2);
        std::vector<double> y(nPar() /2);
        for (unsigned int i = 0; i < nPar() / 2; i++) {
            x[i] = par(2 * i);
            y[i] = par(2 * i + 1);
        }
        m_sp3 = std::make_unique<TSpline3>("Rt Relation", x.data(), y.data(), nPar() / 2, "b2e2", 0, 0);
    }  // end RtSpline::_init
    double RtSpline::tBinWidth() const {
        return m_sp3->GetDelta();
    }
    double RtSpline::radius(double t) const {
        // check for t_min and t_max
        if (t > m_sp3->GetXmax()) return m_sp3->Eval(m_sp3->GetXmax());
        if (t < m_sp3->GetXmin()) return m_sp3->Eval(m_sp3->GetXmin());
        double r = m_sp3->Eval(t);
        return r >= 0 ? r : 0;
    }

    double RtSpline::driftVelocity(double t) const { return m_sp3->Derivative(t); }
    double RtSpline::driftAcceleration(double t) const { 
        constexpr double h = 1.e-7;
        return (driftVelocity(t + h) - driftVelocity(t-h))/ (2.*h);
    }
    double RtSpline::tLower() const { return par(0); }

    double RtSpline::tUpper() const { return par(nPar() - 2); }

}  // namespace MuonCalib
