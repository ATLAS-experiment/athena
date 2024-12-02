/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <stdlib.h>
#include <MuonPatternHelpers/MdtSegmentFitter.h>

#include <TRandom3.h>

/*** @brief unit test to ensure that the partial derivatives of the segment fit line are conserved
 *          and actually correct. As refernce, the numerical derivative approach is chosen.
 */
using namespace MuonR4;
using namespace SegmentFit;

Parameters makeTrack(TRandom3& rndEngine){
    Parameters pars{};
    pars[toInt(ParamDefs::theta)] = rndEngine.Uniform(0., 180. * Gaudi::Units::deg);
    pars[toInt(ParamDefs::phi)] = rndEngine.Uniform(-120.* Gaudi::Units::deg, 120.* Gaudi::Units::deg);

    pars[toInt(ParamDefs::y0)] = rndEngine.Uniform(-Gaudi::Units::m, Gaudi::Units::m);
    pars[toInt(ParamDefs::x0)] = rndEngine.Uniform(-Gaudi::Units::m, Gaudi::Units::m);
    return pars;
}

int main (){
    constexpr double h = 1.e-8;
    constexpr double tolerance = 1.e-7;
    TRandom3 rnd{123456};
    for (unsigned trial = 0; trial < 1000; ++trial){
        const Parameters pars{makeTrack(rnd)};

        std::cout<<__FILE__<<":"<<__LINE__<<" --  Generated parameters: "<<toString(pars)<<std::endl;
        MdtSegmentFitter::LineWithPartials segLine{};
        MdtSegmentFitter::updateLinePartials(pars, segLine);
        for (const int param : {toInt(ParamDefs::theta) , toInt(ParamDefs::phi)} ){
            Parameters parsUp{pars}, parsDn{pars};
            parsUp[param]+=h;
            parsDn[param]-=h;
            MdtSegmentFitter::LineWithPartials segLineUp{}, segLineDn{};
            constexpr unsigned int nPars{MdtSegmentFitter::LineWithPartials::nPars};
            MdtSegmentFitter::updateLinePartials(parsUp, segLineUp);
            MdtSegmentFitter::updateLinePartials(parsDn, segLineDn);

            
            const Amg::Vector3D numDeriv{(segLineUp.dir - segLineDn.dir)/ (2.*h)};
            if ((numDeriv - segLine.gradient[param]).mag() > tolerance){
                std::cout<<__FILE__<<":"<<__LINE__<<" -- Parameter:"<< toString(static_cast<ParamDefs>(param))
                         <<" Analytic derivative: "<<Amg::toString(segLine.gradient[param])<<" numeric derivative: "
                         <<Amg::toString(numDeriv)<<std::endl;
                std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives are deviating too much from each other "
                         <<(numDeriv - segLine.gradient[param]).mag()<<std::endl;
                return EXIT_FAILURE;
            }
            /** Calculate the second order derivatives of the line partials */
            for (const int param1 : {toInt(ParamDefs::theta), toInt(ParamDefs::phi)}){
                Parameters parsUp1{pars}, parsDn1{pars};
                parsUp1[param1]+=h;
                parsDn1[param1]-=h;

                MdtSegmentFitter::updateLinePartials(parsUp1, segLineUp);
                MdtSegmentFitter::updateLinePartials(parsDn1, segLineDn);

                const Amg::Vector3D numDeriv1{ (segLineUp.gradient[param] - segLineDn.gradient[param]) / (2. *h) };
                const Amg::Vector3D& analyticDeriv = segLine.hessian[vecIdxFromSymMat<nPars>(param, param1)];
                if ( (analyticDeriv - numDeriv1).mag() > tolerance) {
                    std::cout<<__FILE__<<":"<<__LINE__<<" -- second Parameter:"<< toString(static_cast<ParamDefs>(param1))
                             <<" Analytic derivative: "<<Amg::toString(analyticDeriv)<<" numeric derivative: "
                             <<Amg::toString(numDeriv1)<<std::endl;
                    std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives are deviating too much from each other "<<std::endl;
                    return EXIT_FAILURE;
                }
            }
        }
    }
    std::cout<<"Test terminated successfully"<<std::endl;
    return EXIT_SUCCESS;
}