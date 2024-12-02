/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <stdlib.h>
#include <format>

#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonPatternHelpers/MatrixUtils.h>
#include <TRandom3.h>
using namespace MuonR4;
using namespace SegmentFit;

/***
 *  @brief Unit test to ensure that the calculation of the closest point of approach and its following residuals
 *         from a drift circle measurement remain correct. */

Amg::Vector3D partialProject(const Amg::Vector3D& hitDir,
                             const Parameters& pars, const int param) {
    MdtSegmentFitter::LineWithPartials line{};

    MdtSegmentFitter::updateLinePartials(pars, line);

    const Amg::Vector3D projDir = (line.dir - hitDir.dot(line.dir)*hitDir).unit();
    const double projLen = std::sqrt(1. - std::pow(hitDir.dot(line.dir), 2));

    return (line.gradient[param] - line.gradient[param].dot(hitDir)*hitDir )/ projLen
            + projDir * line.dir.dot(hitDir) * line.gradient[param].dot(hitDir) / std::pow(projLen,2);
}

Parameters makeTrack(TRandom3& rndEngine){
    Parameters pars{};
    pars[toInt(ParamDefs::theta)] = rndEngine.Uniform(-85.* Gaudi::Units::deg, 85. * Gaudi::Units::deg);
    pars[toInt(ParamDefs::phi)] = rndEngine.Uniform(-120.* Gaudi::Units::deg, 120.* Gaudi::Units::deg);

    pars[toInt(ParamDefs::y0)] = rndEngine.Uniform(-Gaudi::Units::m, Gaudi::Units::m);
    pars[toInt(ParamDefs::x0)] = rndEngine.Uniform(-Gaudi::Units::m, Gaudi::Units::m);
    return pars;
}


int main(){
    int retCode{EXIT_SUCCESS};

    MdtSegmentFitter::Config cfg{};
    cfg.useSecOrderDeriv = true;
    MdtSegmentFitter fitter{"WireApproach", std::move(cfg)};
    fitter.setLevel(MSG::VERBOSE);

    std::array<CalibratedSpacePoint, 5> measCollection{CalibratedSpacePoint{nullptr, Amg::Vector3D::Zero(), Amg::Vector3D::UnitX()},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,0, 15.}, Amg::Vector3D::UnitX()},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,30., 15.}, Amg::Vector3D::UnitX()},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,0., 30.}, Amg::dirFromAngles(0.004 * Gaudi::Units::deg, 90.*Gaudi::Units::deg)},
                                              CalibratedSpacePoint{nullptr, Amg::Vector3D{0.,30., 30.}, Amg::dirFromAngles(-0.004 * Gaudi::Units::deg, 90.*Gaudi::Units::deg)}};


    /// Prepare the measurement
    constexpr double baseRad = 1.5*Gaudi::Units::mm;
    double i{0.};
    for (CalibratedSpacePoint& meas: measCollection){
        meas.setDriftRadius(baseRad + i);
        i += 0.5*Gaudi::Units::mm;
    }

    constexpr double h{1.e-7};
    constexpr double tolerance{5.e-6};

    TRandom3 rndEngine{12345};

    MdtSegmentFitter::LineWithPartials segLine{};
    MdtSegmentFitter::ResidualWithPartials measRes{};

    unsigned all{0}, good{0};
    for (unsigned int trial = 0 ; trial < 1000; ++trial){
      /** Generate random track parameters */
      const Parameters pars{makeTrack(rndEngine)};

      /** Calculate the partial derivatives */
      fitter.updateLinePartials(pars, segLine);

      if constexpr (false) {
        std::cout<<__FILE__<<":"<<__LINE__<<" Test new segment "<<toString(pars)<<" --- "
                           <<Amg::toString(segLine.pos)<<" + "<<Amg::toString(segLine.dir)<<std::endl;
      }
      constexpr unsigned nLinePars = MdtSegmentFitter::LineWithPartials::nPars;
      for (CalibratedSpacePoint& hit : measCollection) {
          ++all;
          const Amg::Vector3D& hitPos{hit.positionInChamber()};
          const Amg::Vector3D& hitDir{hit.directionInChamber()};
          const double lineDist = Amg::signedDistance(hitPos, hitDir, segLine.pos, segLine.dir);
          hit.setDriftRadius(sign(lineDist) * std::abs(hit.driftRadius()));

          fitter.calculateWireResiduals(segLine, hit, measRes);
          if constexpr(false) {
            std::cout<<__FILE__<<":"<<__LINE__<<" -- test hit : "<<Amg::toString(hitPos)<<" + "
                     <<Amg::toString(hitDir)<<", radius: "<<hit.driftRadius()
                     <<" "<<lineDist<<";"<< Amg::lineDistance(hitPos, hitDir, segLine.pos, segLine.dir) <<std::endl;
          }
          /// Check the wire residual by hand
          if ( std::abs((lineDist - hit.driftRadius()) - measRes.residual[toInt(AxisDefs::eta)]) > tolerance) {
              std::cerr<<__FILE__<<":"<<__LINE__<<" -- The line residuals are too far apart "<<Amg::toString(measRes.residual)
                       <<" vs. "<<(lineDist -hit.driftRadius())<<std::endl;
              retCode = EXIT_FAILURE;
              continue;
          }
          /// Check the distance along the wire residual
          if (hit.dimension() == 2 && std::abs(measRes.residual[toInt(AxisDefs::phi)] -
                                   Amg::intersect(segLine.pos, segLine.dir, hitPos, hitDir).value_or(0.)) > tolerance) {
              std::cerr<<__FILE__<<":"<<__LINE__<<"  -- The distances along the wire are too far apart: "
                       <<measRes.residual[toInt(AxisDefs::phi)]<<" vs. "
                       << Amg::intersect(segLine.pos, segLine.dir, hitPos, hitDir).value_or(0.)<<std::endl;
              retCode = EXIT_FAILURE;
              continue;
          }
          /// Setup the projected line direction
          const double lineProj = segLine.dir.dot(hitDir);
          const double projDirLen = std::sqrt(1. - std::pow(lineProj, 2));
          const Amg::Vector3D projDir = (segLine.dir - lineProj*hitDir).unit();
          /// Check the first derivative
          bool derivMatch{true};
          for (const int param : {toInt(ParamDefs::theta), toInt(ParamDefs::phi),
                                  toInt(ParamDefs::y0), toInt(ParamDefs::x0)}) {
              
              Parameters parsUp{pars}, parsDn{pars};
              parsUp[param] +=h;
              parsDn[param] -=h;

              MdtSegmentFitter::LineWithPartials segLineUp{}, segLineDn{};
              MdtSegmentFitter::updateLinePartials(parsUp, segLineUp);
              MdtSegmentFitter::updateLinePartials(parsDn, segLineDn);
              /// Check the derivatives of the projection lines
              if (param == toInt(ParamDefs::theta) || param ==  toInt(ParamDefs::phi)) {
                    const Amg::Vector3D partialProjDir = partialProject(hitDir, pars, param);
                    const Amg::Vector3D projDirUp = (segLineUp.dir - segLineUp.dir.dot(hitDir) * hitDir).unit();
                    const Amg::Vector3D projDirDn = (segLineDn.dir - segLineDn.dir.dot(hitDir) * hitDir).unit();

                    const Amg::Vector3D numDeriv{(projDirUp - projDirDn)/ (2.*h)};
                    if ((numDeriv - partialProjDir).mag() / std::max(partialProjDir.mag(), 1.) > tolerance){
                        std::cout<<__FILE__<<":"<<__LINE__<<" -- <"<<toString(static_cast<ParamDefs>(param))
                                 <<"> numerical: "<<Amg::toString(numDeriv)<<", analytical: "
                                 <<Amg::toString(partialProjDir)<<std::endl;
                        std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives deviate: "<<(numDeriv - partialProjDir).mag()<<std::endl;
                        retCode = EXIT_FAILURE;
                    }
                    /// Check the second derivatives of the direction projector.
                    for (const int param1 : {toInt(ParamDefs::theta), toInt(ParamDefs::phi)}){
                        Parameters parsUpSq{pars}, parsDnSq{pars};
                        parsUpSq[param1] +=h;
                        parsDnSq[param1] -=h;

                        MdtSegmentFitter::updateLinePartials(parsUpSq, segLineUp);
                        MdtSegmentFitter::updateLinePartials(parsDnSq, segLineDn);

                        const Amg::Vector3D linePartUp = partialProject(hitDir, parsUpSq, param);
                        const Amg::Vector3D linePartDn = partialProject(hitDir, parsDnSq, param);

                        const Amg::Vector3D numDerivSq = (linePartUp - linePartDn) / (2.*h);
                        
                        //// Calculate the analytic detivative
                        const unsigned partSqIdx = vecIdxFromSymMat<nLinePars>(param, param1);

                        const Amg::Vector3D partialProjDir1 = partialProject(hitDir, pars, param1);

                        const double lineProjPart = segLine.gradient[param].dot(hitDir);
                        const double lineProjPart1 = segLine.gradient[param1].dot(hitDir);

                        const Amg::Vector3D analiyticDerivSq = (segLine.hessian[partSqIdx] - segLine.hessian[partSqIdx].dot(hitDir) * hitDir ) / projDirLen
                                                             + (lineProj *lineProjPart1) / std::pow(projDirLen, 2) * partialProjDir
                                                             + (lineProj *lineProjPart) / std::pow(projDirLen, 2) * partialProjDir1
                                                             + (segLine.hessian[partSqIdx].dot(hitDir) * lineProj) / std::pow(projDirLen, 2) * projDir
                                                             + (lineProjPart * lineProjPart1) / std::pow(projDirLen, 4) * projDir;

                        if ( (numDerivSq - analiyticDerivSq).mag() / std::max(analiyticDerivSq.mag(), 1.) > tolerance) {
                            std::cout<<__FILE__<<":"<<__LINE__<<" -- second order <"<<toString(static_cast<ParamDefs>(param))
                                     <<">, <"<<toString(static_cast<ParamDefs>(param1))<<"> : numeric: "<<Amg::toString(numDerivSq)
                                     <<", analytic: "<<Amg::toString(analiyticDerivSq)<<std::endl;
                            std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives deviate "<< (numDerivSq - analiyticDerivSq).mag()<<std::endl;
                            retCode = EXIT_FAILURE;
                            return retCode;
                        }
                    }
              }
              MdtSegmentFitter::ResidualWithPartials measResUp{}, measResDn{};
              MdtSegmentFitter::updateLinePartials(parsUp, segLineUp);
              MdtSegmentFitter::updateLinePartials(parsDn, segLineDn);

              fitter.calculateWireResiduals(segLineUp, hit, measResUp);
              fitter.calculateWireResiduals(segLineDn, hit, measResDn);

              const Amg::Vector3D numericalDeriv{(measResUp.residual - measResDn.residual) / (2.*h)};
              if ( (numericalDeriv - measRes.gradient[param]).mag() / std::max(measRes.gradient[param].mag(), 1.) > tolerance) {
                    std::cout<<__FILE__<<":"<<__LINE__<<" -- Partial <"<<toString(static_cast<ParamDefs>(param))
                             <<"> analytic: "<<Amg::toString(measRes.gradient[param])<<", numeric: "<<Amg::toString(numericalDeriv)<<std::endl;
                    std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives deviate "<< (numericalDeriv - measRes.gradient[param]).mag()<<std::endl;
                    derivMatch = false;
                    continue;
              }
              for (const int param1 : {toInt(ParamDefs::theta), toInt(ParamDefs::phi),
                                       toInt(ParamDefs::y0), toInt(ParamDefs::x0)}) {
                    Parameters parsUpSq{pars}, parsDnSq{pars};
                    parsUpSq[param1] +=h;
                    parsDnSq[param1] -=h;
                    fitter.updateLinePartials(parsUpSq, segLineUp);
                    fitter.updateLinePartials(parsDnSq, segLineDn);

                    fitter.calculateWireResiduals(segLineUp, hit, measResUp);
                    fitter.calculateWireResiduals(segLineDn, hit, measResDn);
                    const Amg::Vector3D numDeriv{(measResUp.gradient[param] - measResDn.gradient[param]) / (2.*h)};
                    const Amg::Vector3D& anaDeriv{measRes.hessian[vecIdxFromSymMat<toInt(ParamDefs::nPars)>(param, param1)]};
                    if ( (numDeriv - anaDeriv).mag() / std::max(anaDeriv.mag(), 1.) > 1.e-3) {
                        std::cout<<__FILE__<<":"<<__LINE__<<" -- derivative <"<<toString(static_cast<ParamDefs>(param))
                             <<"><"<<toString(static_cast<ParamDefs>(param1))<<">  -- numeric: "<<Amg::toString(numDeriv)
                             <<", analytic: "<<Amg::toString(anaDeriv)<<std::endl;
                        std::cerr<<__FILE__<<":"<<__LINE__<<" -- Derivatives deviate "<< (numDeriv - anaDeriv).mag()<<std::endl;
                        derivMatch = false;
                    }
              }
          }
          good +=derivMatch;
       }
    }
    if (1.*good / all < 0.99) {
        retCode = EXIT_FAILURE;
    }
    if (retCode == EXIT_SUCCESS) {
        std::cout<<std::format("Test was successful with {:d}/{:d} ({:3.2f}%) good derivatives. ", good, all, 100.*good / all)<<std::endl;
    }  else {
        std::cerr<<std::format("Test failed because the amount of good derivatives {:d}/{:d} ({:3.2f}%) is too low. ", good, all, 100.*good / all)<<std::endl;
    }
    return retCode;
}