/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 23.03.2005, AUTHOR: OLIVER KORTNER
// Modified: 31.05.2006 by O. Kortner: major redesign:
//                                     driftvelocity implemented,
//                                     resolution has been removed,
//                                     set-method have been removed.
//           04.06.2006 by O. Kortner: bug in constructor fixed.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include "MdtCalibData/RtChebyshev.h"

#include <TString.h>  // for Form

using namespace MuonCalib;

//*****************************************************************************

//////////////////
// METHOD _init //
//////////////////

void RtChebyshev::_init(void) {
    // check for consistency //
    if (nPar() < 3) {
        throw std::runtime_error(Form("File: %s, Line: %d\nRtChebyshev::_init() - Not enough parameters!", __FILE__, __LINE__));
    }
    if (parameters()[0] >= parameters()[1]) {
        throw std::runtime_error(
            Form("File: %s, Line: %d\nRtChebyshev::_init() - Lower time boundary >= upper time boundary!", __FILE__, __LINE__));
    }

    // pointer to the chebyshev service //
    m_Chebyshev = Tschebyscheff_polynomial::get_Tschebyscheff_polynomial();

    return;
}  // end RtChebyshev::_init

//*****************************************************************************

/////////////////
// METHOD name //
/////////////////
std::string RtChebyshev::name(void) const { return std::string("RtChebyshev"); }

//*****************************************************************************

///////////////////
// METHOD radius //
///////////////////
double RtChebyshev::radius(double t) const {
    ////////////////////////
    // INITIAL TIME CHECK //
    ////////////////////////
    if (t < parameters()[0]) return 0.0;
    if (t > parameters()[1]) return 14.6;
    // if x is out of bounds, return 99999 //
    //	if (t<parameters()[0] || t>parameters()[1]) {
    //		return 99999.0;
    //	}

    ///////////////
    // VARIABLES //
    ///////////////
    // argument of the Chebyshev polynomials
    double x(2 * (t - 0.5 * (parameters()[1] + parameters()[0])) / (parameters()[1] - parameters()[0]));
    double rad(0.0);  // auxiliary radius

    ////////////////////
    // CALCULATE r(t) //
    ////////////////////
    for (unsigned int k = 0; k < nPar() - 2; k++) { rad = rad + parameters()[k + 2] * m_Chebyshev->value(k, x); }

    return rad >= 0 ? rad : 0;
}

double RtChebyshev::drdt(double t) const {
    // Set derivative to 0 outside of the bounds
    if (t < parameters()[0]) return 0.0;
    if (t > parameters()[1]) return 0.0;

    // Argument of the Chebyshev polynomials
    double x = 2 * (t - 0.5 * (parameters()[1] + parameters()[0])) / (parameters()[1] - parameters()[0]);
    // Chain rule
    double dx_dt = 2 / (parameters()[1] - parameters()[0]);
    double drdt = 0.0;

    // Compute U_{k-1}(x) using recurrence relation
    double U_k_minus_2 = 1.0;  // U_0(x)
    double U_k_minus_1 = 2 * x;  // U_1(x)
    double U_k;

    for (unsigned int k = 1; k < nPar() - 2; ++k) {
        if (k == 1) {
            U_k = U_k_minus_1;
        } else {
            //recurrence relation is U_{k}(x) = 2*x*U_{k-1}(x) - U_{k-2}(x)
            U_k = 2 * x * U_k_minus_1 - U_k_minus_2;
            U_k_minus_2 = U_k_minus_1;
            U_k_minus_1 = U_k;
        }
        // Calculate the contribution to dr/dt using k * U_{k-1}(x) * dx/dt
        drdt += parameters()[k + 2] * k * U_k_minus_1 * dx_dt;
    }
    return drdt;
}
//*****************************************************************************

//////////////////////////
// METHOD driftvelocity //
//////////////////////////
double RtChebyshev::driftvelocity(double t) const { return (radius(t + 1.0) - radius(t)); }

//*****************************************************************************

///////////////////
// METHOD tLower //
///////////////////
double RtChebyshev::tLower(void) const { return parameters()[0]; }

//*****************************************************************************

///////////////////
// METHOD tUpper //
///////////////////
double RtChebyshev::tUpper(void) const { return parameters()[1]; }

//*****************************************************************************

/////////////////////////////////
// METHOD numberOfRtParameters //
/////////////////////////////////
unsigned int RtChebyshev::numberOfRtParameters(void) const { return nPar() - 2; }

//*****************************************************************************

/////////////////////////
// METHOD rtParameters //
/////////////////////////
std::vector<double> RtChebyshev::rtParameters(void) const {
    std::vector<double> alpha(nPar() - 2);
    for (unsigned int k = 0; k < alpha.size(); k++) { alpha[k] = parameters()[k + 2]; }

    return alpha;
}

//*****************************************************************************

/////////////////////////////
// METHOD get_reduced_time //
/////////////////////////////
double RtChebyshev::get_reduced_time(const double& t) const {
    return 2 * (t - 0.5 * (parameters()[1] + parameters()[0])) / (parameters()[1] - parameters()[0]);
}
