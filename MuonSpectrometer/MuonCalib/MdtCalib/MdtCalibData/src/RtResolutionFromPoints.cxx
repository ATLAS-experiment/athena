/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/RtResolutionFromPoints.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "MdtCalibData/SamplePointUtils.h"
#include "MuonCalibMath/BaseFunctionFitter.h"
#include "MuonCalibMath/ChebyshevPolynomial.h"
#include "MuonCalibMath/PolygonBase.h"

using namespace MuonCalib;


//*****************************************************************************

//:::::::::::::::::::::::::::::::::::::
//:: METHOD getRtResolutionChebyshev ::
//:::::::::::::::::::::::::::::::::::::
RtResolutionChebyshev RtResolutionFromPoints::getRtResolutionChebyshev(const std::vector<SamplePoint> &sample_points,
                                                                       const unsigned int order) {
    ///////////////
    // VARIABLES //
    ///////////////
    MsgStream log(Athena::getMessageSvc(), "RtResolutionChebyshev");

    std::vector<double> res_param(order + 3);           // input parameters of RtChebyshev
    BaseFunctionFitter fitter(order + 1);               // Chebyshev fitter
    ChebyshevPolynomial chebyshev;                      // Chebyshev polynomial
    std::vector<SamplePoint> my_points(sample_points);  // copy of the sample to add reduced times

    ///////////////////////////////////////////////////////////////////
    // GET THE MINIMUM AND MAXIMUM TIMES AND CALCULATE REDUCED TIMES //
    ///////////////////////////////////////////////////////////////////
    const auto [tLow ,tHigh] = interval(sample_points);
    res_param[0] = tLow;
    res_param[1] = tHigh;
    double mean(0.5 * (res_param[1] + res_param[0]));
    double length(res_param[1] - res_param[0]);
    for (unsigned int k = 0; k < my_points.size(); k++) { my_points[k].set_x1(2 * (sample_points[k].x1() - mean) / length); }

    //////////////////////////////////////////////////
    // PERFORM A CHEBYSHEV FIT TO THE SAMPLE POINTS //
    //////////////////////////////////////////////////
    fitter.fit_parameters(my_points, 1, sample_points.size(), chebyshev);
    for (unsigned int k = 0; k < order + 1; k++) { res_param[k + 2] = fitter.coefficients()[k]; }

    //////////////////////////////////////////////////////////////
    // CREATE AN RtChebyshev OBJECT WITH THE CORRECT PARAMETERS //
    //////////////////////////////////////////////////////////////
    return RtResolutionChebyshev{res_param};
}

//*****************************************************************************

//::::::::::::::::::::::::::::::::::
//:: METHOD getRtResolutionLookUp ::
//::::::::::::::::::::::::::::::::::
RtResolutionLookUp RtResolutionFromPoints::getRtResolutionLookUp(const std::vector<SamplePoint> &sample_points) {
    ///////////////
    // VARIABLES //
    ///////////////
    RtResolutionChebyshev res(getRtResolutionChebyshev(sample_points, 8));             // auxiliary resolution-t
    unsigned int nb_points(100);                                                       // number of (r, t) points
    double bin_width((res.tUpper() - res.tLower()) / static_cast<double>(nb_points));  // step size
    std::vector<double> res_param(nb_points + 2);                                      // r-t parameters

    ///////////////////////////////////////////////////////////////////
    // CREATE AN RtRelationLookUp OBJECT WITH THE CORRECT PARAMETERS //
    ///////////////////////////////////////////////////////////////////
    res_param[0] = res.tLower();
    res_param[1] = bin_width;
    for (unsigned int k = 0; k < nb_points; k++) { res_param[k + 2] = res.resolution(res.tLower() + k * bin_width); }
    RtResolutionLookUp rt_res_relation_look_up(res_param);

    return rt_res_relation_look_up;
}
