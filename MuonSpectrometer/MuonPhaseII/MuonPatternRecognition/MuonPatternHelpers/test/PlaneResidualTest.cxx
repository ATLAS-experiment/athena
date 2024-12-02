/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <stdlib.h>

#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonPatternHelpers/MatrixUtils.h>
#include <TRandom3.h>
using namespace MuonR4;
using namespace SegmentFit;


Parameters makeTrack(TRandom3& rndEngine){
    Parameters pars{};
    pars[toInt(ParamDefs::theta)] = rndEngine.Uniform(-85 * Gaudi::Units::deg, 85. * Gaudi::Units::deg);
    pars[toInt(ParamDefs::phi)] = rndEngine.Uniform(-120.* Gaudi::Units::deg, 120.* Gaudi::Units::deg);

    pars[toInt(ParamDefs::y0)] = rndEngine.Uniform(-Gaudi::Units::m, Gaudi::Units::m);
    pars[toInt(ParamDefs::x0)] = rndEngine.Uniform(-Gaudi::Units::m, Gaudi::Units::m);
    return pars;
}



int main (){

    MdtSegmentFitter::Config cfg{};
    cfg.useSecOrderDeriv = true;
    MdtSegmentFitter fitter{"PlaneIsect", std::move(cfg)};
    fitter.setLevel(MSG::VERBOSE);

    const std::array<CalibratedSpacePoint, 4> measCollection{
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,0, 15.}, Amg::Vector3D::UnitX()},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,161., 83.}, Amg::Vector3D::UnitX()},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,177., 30.}, Amg::dirFromAngles(1.5 * Gaudi::Units::deg, 90.*Gaudi::Units::deg)},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{2727.,75., 0.}, Amg::dirFromAngles(-1.5 * Gaudi::Units::deg, 90.*Gaudi::Units::deg)}};


    TRandom3 rndEngine{12345};

    constexpr double h{0.5e-6};
    constexpr double tolerance{1.e-5};

    MdtSegmentFitter::LineWithPartials segLine{};
    MdtSegmentFitter::ResidualWithPartials measRes{};
    for (unsigned int nEvt = 0; nEvt< 1000; ++nEvt) {
        const Parameters pars = makeTrack(rndEngine);
        /** Calculate the partial derivatives */
        fitter.updateLinePartials(pars, segLine);
        std::cout<<__FILE__<<":"<<__LINE__<<" Test new segment "<<toString(pars)<<" --- "
                           <<Amg::toString(segLine.pos)<<" + "<<Amg::toString(segLine.dir)<<std::endl;
        for (const CalibratedSpacePoint& meas: measCollection) {

            const Amg::Vector3D& hitPos{meas.positionInChamber()};
            fitter.calculateStripResiduals(segLine, meas, measRes);
            
            const Amg::Vector3D planeIsect = segLine.pos + 
                    Amg::intersect<3>(segLine.pos, segLine.dir, Amg::Vector3D::UnitZ(), hitPos.z()).value_or(0) * segLine.dir;
            
            const Amg::Vector3D manualResidual{planeIsect - hitPos};
            if ( (measRes.residual - manualResidual).mag() > tolerance) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" Residual for hit "<<Amg::toString(hitPos)<<" is "
                         <<Amg::toString(manualResidual)<<" while the calculated one is "
                         <<Amg::toString(measRes.residual)<<std::endl;
                return EXIT_FAILURE;
            }
            for (unsigned param = 0; param < segLine.gradient.size(); ++param) {
                Parameters parsUp{pars}, parsDn{pars};
                parsUp[param] +=h;
                parsDn[param] -=h;
                MdtSegmentFitter::LineWithPartials segLineUp{}, segLineDn{};

                fitter.updateLinePartials(parsUp, segLineUp);
                fitter.updateLinePartials(parsDn, segLineDn);

                MdtSegmentFitter::ResidualWithPartials measResUp{}, measResDn{};
                fitter.calculateStripResiduals(segLineUp, meas, measResUp);
                fitter.calculateStripResiduals(segLineDn, meas, measResDn);
                const Amg::Vector3D numDeriv = (measResUp.residual - measResDn.residual) / (2.*h);
                if ( (numDeriv - measRes.gradient[param]).mag() / std::max(1., measRes.gradient[param].mag()) > tolerance) {
                    std::cout<<__FILE__<<":"<<__LINE__<<" -- <"<<toString(static_cast<ParamDefs>(param))
                                 <<"> numerical: "<<Amg::toString(numDeriv)<<", analytical: "
                                 <<Amg::toString(measRes.gradient[param])<<std::endl;
                    std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives deviate: "<<(numDeriv - measRes.gradient[param]).mag()<<std::endl;
                    return EXIT_FAILURE;
                }
                for (unsigned param1 = param; param1<segLine.gradient.size(); ++param1) {
                    Parameters parsSqUp{pars}, parsSqDn{pars};
                    parsSqUp[param1] += h;
                    parsSqDn[param1] -= h;
                    fitter.updateLinePartials(parsSqUp, segLineUp);
                    fitter.updateLinePartials(parsSqDn, segLineDn);
                    fitter.calculateStripResiduals(segLineUp, meas, measResUp);
                    fitter.calculateStripResiduals(segLineDn, meas, measResDn);

                    const Amg::Vector3D numDerivSq = (measResUp.gradient[param] - measResDn.gradient[param]) / (2.*h);
                    constexpr unsigned nPars = MdtSegmentFitter::ResidualWithPartials::nPars;
                    unsigned int hessianIdx = vecIdxFromSymMat<nPars>(param1, param);
                    const Amg::Vector3D& analiyticDerivSq{measRes.hessian[hessianIdx]};
                     if ( (numDerivSq - analiyticDerivSq).mag() / std::max(1., analiyticDerivSq.mag()) > tolerance) {
                        std::cout<<__FILE__<<":"<<__LINE__<<" -- second order <"<<toString(static_cast<ParamDefs>(param))
                                     <<">, <"<<toString(static_cast<ParamDefs>(param1))<<"> : numeric: "<<Amg::toString(numDerivSq)
                                     <<", analytic: "<<Amg::toString(analiyticDerivSq)<<std::endl;
                        std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives deviate "<< (numDerivSq - analiyticDerivSq).mag()<<std::endl;
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}