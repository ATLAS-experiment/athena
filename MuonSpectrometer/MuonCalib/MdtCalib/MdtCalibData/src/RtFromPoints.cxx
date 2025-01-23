/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/RtFromPoints.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "MdtCalibData/RtSpline.h"
#include "MuonCalibMath/BaseFunctionFitter.h"

#include "MuonCalibMath/ChebyshevPolynomial.h"
#include "MuonCalibMath/LegendrePolynomial.h"
#include "MuonCalibMath/SimplePolynomial.h"

#include "MuonCalibMath/PolygonBase.h"
#include "MdtCalibData/SamplePointUtils.h"

#include "MdtCalibData/RtChebyshev.h"
#include "MdtCalibData/TrChebyshev.h"
#include "MdtCalibData/RadiusResolutionChebyshev.h"
#include "MdtCalibData/RtResolutionChebyshev.h"

#include "MdtCalibData/RtLegendre.h"
#include "MdtCalibData/TrLegendre.h"

#include "MdtCalibData/RtSimplePolynomial.h"
#include "MdtCalibData/TrSimplePolynomial.h"

#include "MdtCalibData/RtRelationLookUp.h"


using namespace MuonCalib;



CalibFunc::ParVec RtFromPoints::legendreFit(const std::vector<SamplePoint>& dataPoints,
                                            const unsigned order) {
    const auto [minT, maxT] = interval(dataPoints);

    CalibFunc::ParVec pars{minT, maxT};
    BaseFunctionFitter fitter{order};
    LegendrePolynomial legendre{};
    fitter.fit_parameters(normalizeDomain(dataPoints), 1, dataPoints.size(), legendre);
    for (unsigned k = 0; k < fitter.coefficients().size(); k++) { 
        pars.emplace_back(fitter.coefficients()[k]); 
    }
    return pars;
}

CalibFunc::ParVec RtFromPoints::chebyFit(const std::vector<SamplePoint>& dataPoints,
                                         const unsigned order) {

    const auto [minT, maxT] = interval(dataPoints);
    CalibFunc::ParVec pars{minT, maxT};
    BaseFunctionFitter fitter{order};
    ChebyshevPolynomial chebyshev{};                      // Chebyshev polynomial

 
    fitter.fit_parameters(normalizeDomain(dataPoints), 1, dataPoints.size(), chebyshev);
    for (unsigned k = 0; k < fitter.coefficients().size(); k++) { 
        pars.emplace_back(fitter.coefficients()[k]); 
    }
    return pars;
}
CalibFunc::ParVec RtFromPoints::simplePolyFit(const std::vector<SamplePoint>& dataPoints,
                                              const unsigned order) {
    const auto [minT, maxT] = interval(dataPoints);
    CalibFunc::ParVec pars{minT, maxT};
    BaseFunctionFitter fitter{order};
    SimplePolynomial simplePoly{};
    fitter.fit_parameters(normalizeDomain(dataPoints), 1, dataPoints.size(), simplePoly);
    for (unsigned k = 0; k < fitter.coefficients().size(); k++) { 
        pars.emplace_back(fitter.coefficients()[k]); 
    }
    return pars;

}
std::unique_ptr<IRtRelation> RtFromPoints::getRtChebyshev(const std::vector<SamplePoint> &dataPoints, const unsigned order) {
    return std::make_unique<RtChebyshev>(chebyFit(dataPoints, order));
}  // end RtFromPoints::getRtChebyshev
std::unique_ptr<ITrRelation> RtFromPoints::getTrChebyshev(const std::vector<SamplePoint>& dataPoints, const unsigned order) {
    return std::make_unique<TrChebyshev>(chebyFit(dataPoints, order));
}
std::unique_ptr<IRtResolution> RtFromPoints::getResoChebyshev(const std::vector<SamplePoint>& dataPoints, const unsigned order){
    return std::make_unique<RtResolutionChebyshev>(chebyFit(dataPoints, order));
}
std::unique_ptr<IRtResolution> RtFromPoints::getResoChebyshev(const std::vector<SamplePoint>& dataPoints, const IRtRelationPtr rtRelPtr, 
                                                              const double relUnc, const unsigned order) {    
    std::vector<double> chebyCoeff = chebyFit(resoFromRadius(dataPoints, relUnc), order);
    chebyCoeff.erase(chebyCoeff.begin(), chebyCoeff.begin() +2);
    return std::make_unique<RadiusResolutionChebyshev>(chebyCoeff, rtRelPtr);
}

std::unique_ptr<IRtRelation> RtFromPoints::getRtLegendre(const std::vector<SamplePoint>& dataPoints, const unsigned order) {
    return std::make_unique<RtLegendre>(legendreFit(dataPoints, order));
}
std::unique_ptr<ITrRelation> RtFromPoints::getTrLegendre(const std::vector<SamplePoint>& dataPoints, const unsigned order) {
    return std::make_unique<TrLegendre>(legendreFit(dataPoints, order));
}
std::unique_ptr<IRtRelation> RtFromPoints::getRtSimplePoly(const std::vector<SamplePoint>& dataPoints, const unsigned order) {
    return std::make_unique<RtSimplePolynomial>(simplePolyFit(dataPoints, order));
}
std::unique_ptr<ITrRelation> RtFromPoints::getTrSimplePoly(const std::vector<SamplePoint>& dataPoints, const unsigned order) {
    return std::make_unique<TrSimplePolynomial>(simplePolyFit(dataPoints, order));
}
//*****************************************************************************

//::::::::::::::::::::::::::::::::
//:: METHOD getRtRelationLookUp ::
//::::::::::::::::::::::::::::::::
std::unique_ptr<IRtRelation> RtFromPoints::getRtRelationLookUp(const std::vector<SamplePoint> &dataPoints) {
    // create spline rt relation
    CalibFunc ::ParVec pars(2 * dataPoints.size());
    for (unsigned i = 0; i < dataPoints.size(); i++) {
        pars[2 * i] = dataPoints[i].x1();
        pars[2 * i + 1] = dataPoints[i].x2();
    }
    RtSpline rt(pars);

    // variables
    unsigned nb_points(100);                                                         // number of (r, t) points
    double bin_width((rt.tUpper() - rt.tLower()) / static_cast<double>(nb_points - 1));  // step size
    std::vector<double> rt_param(nb_points + 2);                                         // r-t parameters

    ///////////////////////////////////////////////////////////////////
    // CREATE AN RtRelationLookUp OBJECT WITH THE CORRECT PARAMETERS //
    ///////////////////////////////////////////////////////////////////
    rt_param[0] = rt.tLower();
    rt_param[1] = bin_width;
    for (unsigned k = 0; k < nb_points; k++) { rt_param[k + 2] = rt.radius(rt.tLower() + k * bin_width); }
    return std::make_unique<RtRelationLookUp>(rt_param);
}
