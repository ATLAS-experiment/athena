/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentFitter.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <TrkEventPrimitives/ParamDefs.h>
#include <EventPrimitives/EventPrimitivesCovarianceHelpers.h>
#include <GaudiKernel/PhysicalConstants.h>
#include <MuonSpacePoint/UtilFunctions.h>
#include <CxxUtils/sincos.h>

#include <format>
/*   * Residual strip hit:  R= (P + <P-H|D>*D -H) 
     *        dR                   
     *    --> -- dP = dP + <dP|D>*D
     *        dP
     *      
     *        dR
     *        -- dD = <P-H|D>dD + <P-H|dD>*D
     *        dD
     *
     *    chi2 = <R|1./cov^{2} | R>
     * 
     *   dchi2 = <dR | 1./cov^{2} | R> + <R | 1./cov^{2} | dR>
     */
namespace {
    constexpr double c_inv = 1. / Gaudi::Units::c_light;
    /* Cut off value for the determinant. Hessian matrices with a determinant smaller than this 
       are considered to be invalid */
    constexpr double detCutOff = 1.e-8;
}

namespace MuonR4{
    using namespace SegmentFit;
    using HitType = SegmentFitResult::HitType;
    using HitVec = SegmentFitResult::HitVec;
 
     MdtSegmentFitter::Config::RangeArray 
        MdtSegmentFitter::Config::defaultRanges() {
            RangeArray rng{};
            constexpr double spatRang = 10.*Gaudi::Units::m;
            constexpr double timeTange = 25 * Gaudi::Units::ns;
            rng[toInt(ParamDefs::y0)] = std::array{-spatRang, spatRang};
            rng[toInt(ParamDefs::x0)] = std::array{-spatRang, spatRang};
            rng[toInt(ParamDefs::phi)] = std::array{-175.* Gaudi::Units::deg, 175. * Gaudi::Units::deg};
            rng[toInt(ParamDefs::theta)] = std::array{-85. * Gaudi::Units::deg,  85. * Gaudi::Units::deg};
            rng[toInt(ParamDefs::time)] = std::array{-timeTange, timeTange};            
            return rng;
    }
    MdtSegmentFitter::MdtSegmentFitter(const std::string& name, Config&& config):
        AthMessaging{name},
        m_cfg{std::move(config)}{}

    inline void MdtSegmentFitter::updateDriftSigns(const Amg::Vector3D& segPos, const Amg::Vector3D& segDir, 
                                                   SegmentFitResult& fitResult) const {
        for (std::unique_ptr<CalibratedSpacePoint>& meas : fitResult.calibMeasurements) {
            meas->setDriftRadius(SegmentFitHelpers::driftSign(segPos,segDir, *meas, msg())*meas->driftRadius());
        }
    }

    inline bool MdtSegmentFitter::recalibrate(const EventContext& ctx,
                                              SegmentFitResult& fitResult) const{
        
        using State = CalibratedSpacePoint::State;
        const auto [segPos, segDir] = makeLine(fitResult.segmentPars);

        fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), segPos, segDir,
                                                                  fitResult.segmentPars[toInt(ParamDefs::time)]);
        if (!updateHitSummary(fitResult)){
            return false;
        }
        updateDriftSigns(segPos, segDir, fitResult);
        /// Switch off the time fit if too little degrees of freedom are left
        if (fitResult.timeFit && fitResult.nDoF <= 1) {
            fitResult.timeFit = false;
            ATH_MSG_DEBUG("Switch of the time fit because nDoF: "<<fitResult.nDoF);
            fitResult.segmentPars[toInt(ParamDefs::time)] = 0.;
            /// Recalibrate the measurements
            fitResult.calibMeasurements = m_cfg.calibrator->calibrate(ctx, std::move(fitResult.calibMeasurements), segPos, segDir,
                                                                      fitResult.segmentPars[toInt(ParamDefs::time)]);
        ///
        } else if (!fitResult.timeFit && m_cfg.doTimeFit) {
            ATH_MSG_DEBUG("Somehow a measurement is on the narrow ridge of validity. Let's try if the time can be fitted now ");
            fitResult.timeFit = true;
        }

        return true;
    }
    inline bool MdtSegmentFitter::updateHitSummary(SegmentFitResult& fitResult) const {
        /// Count the phi & time measurements measurements
         using State = CalibratedSpacePoint::State;
        fitResult.nPhiMeas = fitResult.nDoF = fitResult.nTimeMeas = 0;
        for (const HitType& hit : fitResult.calibMeasurements) {
            if (hit->fitState() != State::Valid){
                continue;
            }

            fitResult.nPhiMeas+= hit->measuresPhi();
            fitResult.nDoF+= hit->measuresPhi();
            fitResult.nDoF+= hit->measuresEta();
            /// Mdts are already counted in the measures eta category. Don't count them twice
            fitResult.nDoF += (m_cfg.doTimeFit && hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType && hit->measuresTime());
            fitResult.nTimeMeas+=hit->measuresTime();              
        }
        if (!fitResult.nDoF) {
            ATH_MSG_WARNING("TSCHUUUUUUUUSS Measurements...");
            return false;
        }
        if (!fitResult.nPhiMeas) {
            ATH_MSG_VERBOSE("No phi measurements are left.");
            fitResult.segmentPars[toInt(ParamDefs::phi)] = 90. * Gaudi::Units::deg; 
            fitResult.segmentPars[toInt(ParamDefs::x0)] = 0;
        }

        fitResult.nDoF = fitResult.nDoF - 2 - (fitResult.nPhiMeas > 0 ? 2 : 0);

        return true;
    }
    inline Amg::Vector3D MdtSegmentFitter::partialPlaneIntersect(const Amg::Vector3D& normal, const double offset, 
                                                                 const LineWithPartials& line,
                                                                 const ParamDefs fitPar) {
        const double normDot = normal.dot(line.dir);
        switch (fitPar) {
           case ParamDefs::phi:
           case ParamDefs::theta:{                
                const double travelledDist = (offset - line.pos.dot(normal)) / normDot;
                const double partialDist = - travelledDist / normDot * normal.dot(line.gradient[toInt(fitPar)]);
                return travelledDist * line.gradient[toInt(fitPar)] + partialDist * line.dir;
                break;
           } case ParamDefs::y0:
             case ParamDefs::x0:
                return line.gradient[toInt(fitPar)] - line.gradient[toInt(fitPar)].dot(normal) / normDot * line.dir;
                break;
            default:
                break;
        }        
       return Amg::Vector3D::Zero();
    }
    

    void MdtSegmentFitter::updateLinePartials(const Parameters& params, 
                                              LineWithPartials& line) {
        
        line.pos[Amg::x] = params[toInt(ParamDefs::x0)];
        line.pos[Amg::y] = params[toInt(ParamDefs::y0)];
        line.dir = Amg::dirFromAngles(params[toInt(ParamDefs::phi)], 
                                      params[toInt(ParamDefs::theta)]);
        line.gradient[toInt(ParamDefs::x0)] = Amg::Vector3D::UnitX();
        line.gradient[toInt(ParamDefs::y0)] = Amg::Vector3D::UnitY();
        /**          x_{0}             cos (phi) sin (theta)
         *  segPos = y_{0}  , segDir = sin (phi) sin (theta)
         *             0                     cos theta
         *                          
         * 
         *   d segDir     cos (phi) cos(theta)     dSegDir     -sin(phi) sin (theta)
         *  ----------=   sin (phi) cos(theta)     -------- =   cos(phi) sin (theta)
         *   dTheta            - sin (theta)        dPhi              0
         * 
        *******************************************************************************/
        const CxxUtils::sincos theta{params[toInt(ParamDefs::theta)]},
                               phi{params[toInt(ParamDefs::phi)]};
        line.gradient[toInt(ParamDefs::theta)] = Amg::Vector3D{phi.cs*theta.cs, phi.sn*theta.cs, -theta.sn};
        line.gradient[toInt(ParamDefs::phi)]   =  Amg::Vector3D{-theta.sn *phi.sn, theta.sn*phi.cs, 0};
       /********************************************************************************* 
        *   Non-vanishing second order derivatives
        *       
        *    d^{2} segDir                 d^{2} segDir                    cos phi        
        *    ------------- = - segDir ,   ------------  = - sin(theta)    sin phi
        *    d^{2} theta                  d^{2} phi                          0
        * 
        *   d^{2} segDir                 -sin phi
        *   -------------  = cos(theta)   cos phi
        *    d theta dPhi                   0
        ************************************************************************************/
        constexpr unsigned nPars = LineWithPartials::nPars;
        constexpr int idxThetaSq = vecIdxFromSymMat<nPars>(toInt(ParamDefs::theta), toInt(ParamDefs::theta));
        constexpr int idxPhiSq = vecIdxFromSymMat<nPars>(toInt(ParamDefs::phi), toInt(ParamDefs::phi));
        constexpr int idxPhiTheta = vecIdxFromSymMat<nPars>(toInt(ParamDefs::theta), toInt(ParamDefs::phi));
        line.hessian[idxThetaSq] = - Amg::Vector3D{phi.cs*theta.sn,phi.sn*theta.sn, theta.cs};
        line.hessian[idxPhiSq] = -theta.sn * Amg::Vector3D{phi.cs, phi.sn, 0.};
        line.hessian[idxPhiTheta] = theta.cs * Amg::Vector3D{-phi.sn, phi.cs, 0.};
    }
    void MdtSegmentFitter::calculateWireResiduals(const LineWithPartials& line,
                                                  const CalibratedSpacePoint& hit,
                                                  ResidualWithPartials& measResidual) const {
        /** Fetch the hit position & direction */
        const Amg::Vector3D& hitPos{hit.positionInChamber()};
        const Amg::Vector3D& hitDir{hit.directionInChamber()};
        /** Cache the scalar product & the lengths as they're appearing in the formulas below more often */
        const double planeProject = line.dir.dot(hitDir);
        const double projDirLenSq = 1. - std::pow(planeProject, 2);
        if (projDirLenSq < std::numeric_limits<float>::epsilon()) {
            measResidual.residual.setZero();
            for (Amg::Vector3D& grad : measResidual.gradient){
                grad.setZero();
            }
            if (!m_cfg.useSecOrderDeriv) {
                return;
            }
            for (Amg::Vector3D& hess : measResidual.hessian) {
                hess.setZero();
            }
            ATH_MSG_WARNING("Segment is parallel along the wire");
            return;
        }
        const double invProjLenSq = 1. / projDirLenSq;
        const double invProjLen = std::sqrt(invProjLenSq);
        /** Project the segment onto the plane */
        const Amg::Vector3D projDir = (line.dir - planeProject*hitDir) * invProjLen;
        /** Calculate the distance from the two reference points  */
        const Amg::Vector3D hitMinSeg = hitPos - line.pos ;
        /** Distance from the segment line to the tube wire */
        const double lineDist = projDir.cross(hitDir).dot(hitMinSeg);
        const double resVal = (lineDist - hit.driftRadius());
        measResidual.residual = resVal * Amg::Vector3D::Unit(toInt(AxisDefs::eta));
        ATH_MSG_VERBOSE("Mdt drift radius: "<<hit.driftRadius()<<" distance: "<<lineDist<<";"
                        <<Amg::signedDistance(hitPos, hitDir, line.pos, line.dir)<<", residual: "<<resVal);
        /// If the tube is a twin-tube, the hit position is no longer arbitrary along the wire. Calculate the
        /// distance along the wire towards the point of closest approach.
        if (hit.dimension() == 2) {
            measResidual.residual[toInt(AxisDefs::phi)] = (hitMinSeg.dot(line.dir)*planeProject - hitMinSeg.dot(hitDir)) * invProjLenSq;
        }
        constexpr unsigned nLinePars = LineWithPartials::nPars;
        /// Cache of the partial derivatives of the projected line parameters w.r.t the fit parameters
        auto partProjDir{make_array<Amg::Vector3D, nLinePars>(Amg::Vector3D::Zero())};
        /// Dot product between the angular derivative & the hit-plane normal
        auto partPlaneProject{make_array<double, nLinePars>(0.)};

        /** Calculate the first derivative of the residual */
        for (const int param : {toInt(ParamDefs::y0), toInt(ParamDefs::x0), 
                                toInt(ParamDefs::theta), toInt(ParamDefs::phi)}) {
            if (!measResidual.evalPhiPars && (param == toInt(ParamDefs::x0) || param == toInt(ParamDefs::phi))){
                continue;
            }
            switch (param) {
                case toInt(ParamDefs::theta):
                case toInt(ParamDefs::phi): {
                    partPlaneProject[param] = line.gradient[param].dot(hitDir);
                    partProjDir[param] = (line.gradient[param] - partPlaneProject[param]*hitDir) * invProjLen
                                       +  partPlaneProject[param]*planeProject* projDir * invProjLenSq;
                    const double partialDist = partProjDir[param].cross(hitDir).dot(hitMinSeg);

                    measResidual.gradient[param] =  partialDist * Amg::Vector3D::Unit(toInt(AxisDefs::eta));
                    if (hit.dimension() == 2) {
                        measResidual.gradient[param][toInt(AxisDefs::phi)] = 
                            ( hitMinSeg.dot(line.gradient[param]) * planeProject +
                              hitMinSeg.dot(line.dir)*partPlaneProject[param]) * invProjLenSq
                            +2.* measResidual.residual[toInt(AxisDefs::phi)]*( planeProject * partPlaneProject[param]) * invProjLenSq;
                    }
                    break;
                } case toInt(ParamDefs::y0):
                  case toInt(ParamDefs::x0): {
                        const double partialDist = - projDir.cross(hitDir).dot(line.gradient[param]);
                        measResidual.gradient[param] =  partialDist * Amg::Vector3D::Unit(toInt(AxisDefs::eta));
                        if (hit.dimension() == 2) {
                            measResidual.gradient[param][toInt(AxisDefs::phi)] = -(line.gradient[param].dot(line.dir) * planeProject - 
                                                                                   line.gradient[param].dot(hitDir)) * invProjLenSq;
                        }
                        break;
                }
                /** No variation of the track parameters w.r.t. the time */
                default:
                    break;
            }
        }
        if (!m_cfg.useSecOrderDeriv) {
            return;
        }
        /** Loop to include the second order derivatvies */
        for (int param = toInt(ParamDefs::phi); param >=0; --param){
            if (!measResidual.evalPhiPars && (param == toInt(ParamDefs::x0) || param == toInt(ParamDefs::phi))){
                continue;
            }
            for (int param1 = param; param1>=0; --param1) {
                if (!measResidual.evalPhiPars && (param1 == toInt(ParamDefs::x0) || param1 == toInt(ParamDefs::phi))){
                    continue;
                }
                const int lineIdx = vecIdxFromSymMat<nLinePars>(param, param1);
                const int resIdx = vecIdxFromSymMat<toInt(ParamDefs::nPars)>(param, param1);
                /// Pure angular derivatives of the residual
                if ( (param == toInt(ParamDefs::theta) || param == toInt(ParamDefs::phi)) &&
                     (param1 == toInt(ParamDefs::theta) || param1 == toInt(ParamDefs::phi))) {
                    
                    const double partSqLineProject = line.hessian[lineIdx].dot(hitDir);
                    const Amg::Vector3D projDirPartSq = (line.hessian[lineIdx] - partSqLineProject * hitDir) * invProjLen
                                                      + (partPlaneProject[param1] * planeProject) * invProjLenSq * partProjDir[param]
                                                      + (partPlaneProject[param] * planeProject) * invProjLenSq * partProjDir[param1]
                                                      + (partSqLineProject*planeProject) * invProjLenSq * projDir
                                                      + (partPlaneProject[param1] * partPlaneProject[param]) * std::pow(invProjLenSq, 2) * projDir;

                    const double partialSqDist =  projDirPartSq.cross(hitDir).dot(hitMinSeg);
                    measResidual.hessian[resIdx] = partialSqDist * Amg::Vector3D::Unit(toInt(AxisDefs::eta));
                     if (hit.dimension() == 2) {
                         const double partialSqAlongWire =2.*measResidual.residual[toInt(AxisDefs::phi)]*planeProject*partSqLineProject * invProjLenSq
                                                         +2.*measResidual.residual[toInt(AxisDefs::phi)]*partPlaneProject[param]*partPlaneProject[param1]*invProjLenSq
                                                         +2.*measResidual.gradient[param1][toInt(AxisDefs::phi)]*planeProject*partPlaneProject[param]*invProjLenSq
                                                         +2.*measResidual.gradient[param][toInt(AxisDefs::phi)]*planeProject*partPlaneProject[param1]*invProjLenSq
                                                         + hitMinSeg.dot(line.hessian[lineIdx]) *planeProject * invProjLenSq
                                                         + hitMinSeg.dot(line.dir)*partSqLineProject * invProjLenSq
                                                         + hitMinSeg.dot(line.gradient[param])*partPlaneProject[param1]*invProjLenSq
                                                         + hitMinSeg.dot(line.gradient[param1])*partPlaneProject[param]*invProjLenSq;
                         measResidual.hessian[resIdx][toInt(AxisDefs::phi)] = partialSqAlongWire;
                     }
                }
                /// Angular & Spatial mixed terms
                else if (param == toInt(ParamDefs::theta) || param == toInt(ParamDefs::phi)){
                    const double partialSqDist = - partProjDir[param].cross(hitDir).dot(line.gradient[param1]);    
                    measResidual.hessian[resIdx] = partialSqDist * Amg::Vector3D::Unit(toInt(AxisDefs::eta));
                    if (hit.dimension() == 2) {
                        const double partialSqAlongWire = -(line.gradient[param1].dot(line.gradient[param])*planeProject +
                                                            line.gradient[param1].dot(line.dir)*partPlaneProject[param]) * invProjLenSq
                                                        + 2.* measResidual.gradient[param1][toInt(AxisDefs::phi)]*( planeProject * partPlaneProject[param]) * invProjLenSq;
                        measResidual.hessian[resIdx][toInt(AxisDefs::phi)] = partialSqAlongWire;
                    }
                }
            }
        }
    }
    void MdtSegmentFitter::calculateStripResiduals(const LineWithPartials& line,
                                                   const CalibratedSpacePoint& hit,
                                                   ResidualWithPartials& residual) const {
        
        
        const Amg::Vector3D& hitPos{hit.positionInChamber()};
        const Amg::Vector3D normal = hit.type() != xAOD::UncalibMeasType::Other 
                                   ? hit.spacePoint()->planeNormal() : Amg::Vector3D::UnitZ();
        const double planeOffSet = normal.dot(hitPos);

        const double normDot = normal.dot(line.dir); 
        if (std::abs(normDot) < std::numeric_limits<double>::epsilon()){
            residual.residual.setZero();
            for (Amg::Vector3D& deriv: residual.gradient) {
                deriv.setZero();
            }
            ATH_MSG_WARNING("The hit parallel with the segment line "<<Amg::toString(line.dir));
            return;
        }
        const double travelledDist = (planeOffSet - line.pos.dot(normal)) / normDot;
        
        const Amg::Vector3D planeIsect = line.pos + travelledDist * line.dir;
        /// Update the residual accordingly
        residual.residual.block<2,1>(0,0) = (planeIsect - hitPos).block<2,1>(0,0);

        for (unsigned fitPar = 0; fitPar < line.gradient.size(); ++fitPar) {
            switch (fitPar) {
                case toInt(ParamDefs::phi):
                case toInt(ParamDefs::theta):{                
                    const double partialDist = - travelledDist / normDot * normal.dot(line.gradient[fitPar]);
                    residual.gradient[fitPar].block<2,1>(0,0) = (travelledDist * line.gradient[fitPar] + partialDist * line.dir).block<2,1>(0,0);
                    break;
                } case toInt(ParamDefs::y0):
                  case toInt(ParamDefs::x0): {
                    residual.gradient[fitPar].block<2,1>(0,0) = (line.gradient[fitPar] - line.gradient[fitPar].dot(normal) / normDot * line.dir).block<2,1>(0,0);
                    break;
                }
                default: {
                    break;
                }
            }
        }
        if (!m_cfg.useSecOrderDeriv) {
            return;
        }
        constexpr unsigned nLinePars = LineWithPartials::nPars;
        for (int param = toInt(ParamDefs::phi); param >=0; --param){
            for (int param1 = param; param1>=0; --param1) {
                const int lineIdx = vecIdxFromSymMat<nLinePars>(param, param1);
                const int resIdx = vecIdxFromSymMat<toInt(ParamDefs::nPars)>(param, param1);
                if ( (param == toInt(ParamDefs::theta) || param == toInt(ParamDefs::phi)) &&
                     (param1 == toInt(ParamDefs::theta) || param1 == toInt(ParamDefs::phi))) {
                    residual.hessian[resIdx].block<2,1>(0,0) =(
                        travelledDist * line.hessian[lineIdx] - travelledDist *(normal.dot(line.hessian[lineIdx])) / normDot * line.dir
                        - normal.dot(line.gradient[param1]) / normDot * residual.gradient[param]
                        - normal.dot(line.gradient[param]) / normDot * residual.gradient[param1]).block<2,1>(0,0);
                } else if (param == toInt(ParamDefs::theta) || param == toInt(ParamDefs::phi)) {
                    const double gradientDisplace = normal.dot(line.gradient[param1]);
                    if (gradientDisplace > std::numeric_limits<float>::epsilon()){
                        residual.hessian[resIdx].block<2,1>(0,0) = gradientDisplace*( normal.dot(line.gradient[param]) / std::pow(normDot,2)*line.dir
                                                                  - line.gradient[param] / normDot ).block<2,1>(0,0);
                    } else {
                        residual.hessian[resIdx].setZero();
                    }
                }    
            }
        }
    }

    SegmentFitResult MdtSegmentFitter::fitSegment(const EventContext& ctx,
                                                  HitVec&& calibHits,
                                                  const Parameters& startPars,
                                                  const Amg::Transform3D& localToGlobal) const {

        const Muon::IMuonIdHelperSvc* idHelperSvc{calibHits[0]->spacePoint()->msSector()->idHelperSvc()};
        using State = CalibratedSpacePoint::State;

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream hitStream{};
            const auto [startPos, startDir] = makeLine(startPars);
            for (const HitType& hit : calibHits) {
                hitStream<<"       **** "<<(hit->type() != xAOD::UncalibMeasType::Other ? idHelperSvc->toString(hit->spacePoint()->identify()): "beamspot" )
                         <<" position: "<<Amg::toString(hit->positionInChamber());
                if (hit->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                    hitStream<<", driftRadius: "<<SegmentFitHelpers::driftSign(startPos,startDir, *hit, msg())*hit->driftRadius() ;
                }
                hitStream<<", channel dir: "<<Amg::toString(hit->directionInChamber())<<std::endl;
            }
            ATH_MSG_VERBOSE("Start segment fit with parameters "<<toString(startPars)
                          <<", plane location: "<<Amg::toString(localToGlobal)<<std::endl<<hitStream.str());

        }

        SegmentFitResult fitResult{};
        fitResult.segmentPars = startPars;
        fitResult.timeFit = m_cfg.doTimeFit;
        fitResult.calibMeasurements = std::move(calibHits);
        if (!updateHitSummary(fitResult)) {
            ATH_MSG_WARNING("No valid segment seed parsed from the beginning");
            return fitResult;
        }
        if (!m_cfg.reCalibrate) {
            const auto [segPos, segDir] = makeLine(startPars);
            updateDriftSigns(segPos, segDir, fitResult);
        }
        Parameters gradient{AmgVector(5)::Zero()}, prevGrad{AmgVector(5)::Zero()}, prevPars{AmgVector(5)::Zero()};
        AmgSymMatrix(5) hessian{AmgSymMatrix(5)::Zero()};

        /// Cache of the partial derivatives of the line parameters w.r.t the fit parameters
        LineWithPartials segmentLine{};
        /// Partials of the residual w.r.t. the fit parameters
        ResidualWithPartials residual{};
 
        unsigned int noChangeIter{0};
        while (fitResult.nIter++ < m_cfg.nMaxCalls) {
            ATH_MSG_VERBOSE("Iteration: "<<fitResult.nIter<<" parameters: "<<toString(fitResult.segmentPars)<<"chi2: "<<fitResult.chi2);
            /// Update the partial derivatives of the direction vector
            updateLinePartials(fitResult.segmentPars, segmentLine);

            /// First step calibrate the hits
            if (m_cfg.reCalibrate && !recalibrate(ctx, fitResult)) {
                break;
            }
            /// Reset chi2
            fitResult.chi2 = 0;
            hessian.setZero();
            gradient.setZero();

            /** Loop over the hits to calculate the partial derivatives */
            for (const HitType& hit : fitResult.calibMeasurements) {
                if (hit->fitState() != State::Valid) {
                    continue;
                }
                const Amg::Vector3D& hitPos{hit->positionInChamber()};
                const Amg::Vector3D& hitDir{hit->directionInChamber()};
                ATH_MSG_VERBOSE("Update chi2 from measurement "<<(hit->spacePoint() ? idHelperSvc->toString(hit->spacePoint()->identify())  
                                : "pseudo meas")<<" position: "<<Amg::toString(hitPos)<<" + "<<Amg::toString(hitDir));

                /// Which parameters are affected by the residual
                const int start = toInt(fitResult.nPhiMeas ? ParamDefs::phi : ParamDefs::theta);
                switch (hit->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType: {
                        calculateWireResiduals(segmentLine, *hit, residual);
                        break;
                    }
                    case xAOD::UncalibMeasType::RpcStripType: 
                    case xAOD::UncalibMeasType::TgcStripType:{
                        const Amg::Vector3D normal = hit->spacePoint()->planeNormal();
                        const double planeOffSet = normal.dot(hit->positionInChamber());
                        const Amg::Vector3D planeIsect = segmentLine.pos 
                                                        + Amg::intersect<3>(segmentLine.pos, segmentLine.dir, normal, planeOffSet).value_or(0)* segmentLine.dir; 

                        /// The complementary coordinate does not contribute if the measurement is 1D
                        residual.residual.block<2,1>(0,0) = (hitPos - planeIsect).block<2,1>(0,0);

                        if (fitResult.timeFit && hit->measuresTime()) {
                           /// need to calculate the global time of flight
                           const double totFlightDist = (localToGlobal * planeIsect).mag();
                           residual.residual[toInt(AxisDefs::t0)] = hit->time() - totFlightDist * c_inv - fitResult.segmentPars[toInt(ParamDefs::time)];
                        }

                        for (int p = start;  p >= 0; --p) {
                            const ParamDefs par{static_cast<ParamDefs>(p)};
                            residual.gradient[toInt(par)].block<2,1>(0, 0) = - partialPlaneIntersect(normal, planeOffSet, segmentLine, par).block<2,1>(0,0);

                            if (fitResult.timeFit && hit->measuresTime()) {
                                residual.gradient[toInt(par)][toInt(AxisDefs::t0)] = -residual.gradient[toInt(par)].perp() * c_inv;
                            }
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual.residual)<<" "<<
                                          Amg::toString(multiply(inverse(hit->covariance()), residual.residual))
                                          <<" "<<std::endl<<toString(hit->covariance())<<std::endl<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(residual.gradient[toInt(par)]));

                        }
                        if (fitResult.timeFit && hit->measuresTime()) {
                           constexpr ParamDefs par = ParamDefs::time;
                           residual.gradient[toInt(par)] = -Amg::Vector3D::Unit(toInt(AxisDefs::t0));
                           ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual.residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(residual.gradient[toInt(par)]));
                       }
                       break;
                    } case xAOD::UncalibMeasType::Other:{
                        static const Amg::Vector3D normal = Amg::Vector3D::UnitZ();
                        const double planeOffSet = normal.dot(hit->positionInChamber());
                        const Amg::Vector3D planeIsect = segmentLine.pos + 
                                            Amg::intersect<3>(segmentLine.pos, segmentLine.dir, normal, planeOffSet).value_or(0)* segmentLine.dir;

                        residual.residual.block<2,1>(0,0) = (hitPos - planeIsect).block<2,1>(0,0);
                        residual.residual[toInt(AxisDefs::t0)] =0.;
                        for (int p = start;  p >= 0; --p) {
                            const ParamDefs par{static_cast<ParamDefs>(p)};
                            residual.gradient[toInt(par)].block<2,1>(0, 0) = - partialPlaneIntersect(normal, planeOffSet, segmentLine, par).block<2,1>(0,0);

                            residual.gradient[toInt(par)][toInt(AxisDefs::t0)] = 0;
                            ATH_MSG_VERBOSE("Partial derivative of "<<idHelperSvc->toString(hit->spacePoint()->identify())
                                          <<" residual "<<Amg::toString(residual.residual)<<" w.r.t "<<toString(par)<<"="
                                          <<Amg::toString(residual.gradient[toInt(par)]));
                        
                        }
                        break;
                    } default:
                        ATH_MSG_WARNING("MdtSegmentFitter() - Unsupported measurment type" <<typeid(*hit->spacePoint()->primaryMeasurement()).name());
                }
    
                ATH_MSG_VERBOSE("Update derivatives for hit "<< (hit->spacePoint() ? idHelperSvc->toString(hit->spacePoint()->identify()) : "beamspot"));
                updateDerivatives(residual, hit->covariance(), gradient, hessian, fitResult.chi2, 
                                  fitResult.timeFit && hit->measuresTime() ? toInt(ParamDefs::time) : start);
            }
 
            /// Loop over hits is done. Symmetrise the Hessian
            for (int k =1; k < 5 - (!fitResult.timeFit); ++k){
                for (int l = 0; l< k; ++l){
                    hessian(l,k) = hessian(k,l);
                }
            }
            /// Check whether the gradient is already sufficiently small
            if (gradient.mag() < m_cfg.tolerance) {
                fitResult.converged = true;
                ATH_MSG_VERBOSE("Fit converged after "<<fitResult.nIter<<" iterations with "<<fitResult.chi2);
                break;
            }
            ATH_MSG_VERBOSE("Chi2: "<<fitResult.chi2<<", gradient: "<<Amg::toString(gradient)<<"hessian: "<<std::endl<<hessian);
            /// Pure eta segment fit
            UpdateStatus paramUpdate{UpdateStatus::outOfBounds};
            if (!fitResult.nPhiMeas && !fitResult.timeFit) {
                paramUpdate = updateParameters<2>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 
            } else if (!fitResult.nPhiMeas && fitResult.timeFit) {
                /// In the case that the time is fit & that there are no phi measurements -> compress matrix by swaping
                /// the time column with whaever the second column is... it's zero
                hessian.col(2).swap(hessian.col(toInt(ParamDefs::time)));
                hessian.row(2).swap(hessian.row(toInt(ParamDefs::time)));
                std::swap(gradient[2], gradient[toInt(ParamDefs::time)]);
                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[toInt(ParamDefs::time)]);

                paramUpdate = updateParameters<3>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 

                std::swap(fitResult.segmentPars[2], fitResult.segmentPars[toInt(ParamDefs::time)]);
                hessian.col(2).swap(hessian.col(toInt(ParamDefs::time)));
                hessian.row(2).swap(hessian.row(toInt(ParamDefs::time)));
                std::swap(gradient[2], gradient[toInt(ParamDefs::time)]);
            } else if (fitResult.nPhiMeas && !fitResult.timeFit) {
                paramUpdate = updateParameters<4>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 
            } else if (fitResult.nPhiMeas && fitResult.timeFit) {
                paramUpdate = updateParameters<5>(fitResult.segmentPars, prevPars, gradient, prevGrad, hessian); 
            } 
            if  (paramUpdate == UpdateStatus::noChange) {
                if ((++noChangeIter) >= m_cfg.noMoveIter){
                    fitResult.converged = true;
                    break;
                }
            } else if (paramUpdate == UpdateStatus::allOkay) {
                noChangeIter = 0;
            } else if (paramUpdate == UpdateStatus::outOfBounds){
               return fitResult;
            }
        }
        
        /// Subtract 1 degree of freedom to take the time into account
        fitResult.nDoF-=fitResult.timeFit;
        
        /// Calculate the chi2 per measurement
        const auto [segPos, segDir] = fitResult.makeLine();
        fitResult.chi2 =0.;
        
        std::optional<double> toF = fitResult.timeFit ? std::make_optional<double>((localToGlobal * segPos).mag() * c_inv) : std::nullopt;
        /** Sort the measurements by ascending z */
        std::ranges::stable_sort(fitResult.calibMeasurements, [](const HitType&a, const HitType& b){
                return a->positionInChamber().z() < b->positionInChamber().z();
        });

        /*** Remove the drift sign again */
        fitResult.chi2 =0.;
        for (const HitType& hit : fitResult.calibMeasurements) {
            hit->setDriftRadius(std::abs(hit->driftRadius()));
            fitResult.chi2 +=SegmentFitHelpers::chiSqTerm(segPos, segDir, fitResult.segmentPars[toInt(ParamDefs::time)], toF, *hit, msg());
        }
        /// Update the covariance
        if (!fitResult.nPhiMeas&& !fitResult.timeFit) {
            blockCovariance<2>(std::move(hessian), /* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
        }else if (!fitResult.nPhiMeas && fitResult.timeFit) {
            hessian.col(2).swap(hessian.col(toInt(ParamDefs::time)));
            hessian.row(2).swap(hessian.row(toInt(ParamDefs::time)));
            blockCovariance<3>(std::move(hessian),/* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
            fitResult.segmentParErrs.col(2).swap(fitResult.segmentParErrs.col(toInt(ParamDefs::time)));
            fitResult.segmentParErrs.row(2).swap(fitResult.segmentParErrs.row(toInt(ParamDefs::time)));
        } else if (fitResult.nPhiMeas) {
            blockCovariance<4>(std::move(hessian), /* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
        } else if (fitResult.nPhiMeas && fitResult.timeFit) {
            blockCovariance<5>(std::move(hessian),/* std::move(gradient), std::move(prevGrad), */ fitResult.segmentParErrs);
        }
        return fitResult;    
    }

    void MdtSegmentFitter::updateDerivatives(const ResidualWithPartials& fitMeas,
                                             const MeasCov_t& covariance,
                                             AmgVector(5)& gradient, AmgSymMatrix(5)& hessian,
                                             double& chi2, int startPar) const {
        const MeasCov_t invCov{inverse(covariance)};
        const Amg::Vector3D covRes = multiply(invCov, fitMeas.residual);
        chi2 += covRes.dot(fitMeas.residual);
        for (int p = startPar; p >=0 ; --p) {
            gradient[p] +=2.*covRes.dot(fitMeas.gradient[p]);
            for (int k=p; k>=0; --k) {
                hessian(p,k)+= 2.*contract(invCov, fitMeas.gradient[p], fitMeas.gradient[k]);
                if (m_cfg.useSecOrderDeriv) {
                    const int symMatIdx = vecIdxFromSymMat<ResidualWithPartials::nPars>(p,k);
                    hessian(p,k)+=contract(invCov, covRes, fitMeas.hessian[symMatIdx]);
                }
            }
        }
        ATH_MSG_VERBOSE("After derivative update --- chi2: "<<chi2<<"("<<covRes.dot(fitMeas.residual)<<"), gradient: "
                      <<toString(gradient)<<", Hessian:\n"<<hessian<<", measurement covariance\n"<<toString(invCov));
    }

    template <unsigned int nDim>
        void MdtSegmentFitter::blockCovariance(const AmgSymMatrix(5)& hessian,
                                               SegmentFit::Covariance& covariance) const {

            covariance.setIdentity();
            AmgSymMatrix(nDim) miniHessian = hessian.block<nDim, nDim>(0,0);
            if (std::abs(miniHessian.determinant()) <= detCutOff) {
                ATH_MSG_VERBOSE("Boeser mini hessian ("<<miniHessian.determinant()<<")\n"<<miniHessian<<"\n\n"<<hessian);
                return;
            }
            ATH_MSG_VERBOSE("Hessian matrix: \n"<<hessian<<",\nblock Hessian:\n"<<miniHessian<<",\n determinant: "<<miniHessian.determinant());
            covariance.block<nDim,nDim>(0,0) = miniHessian.inverse();
            ATH_MSG_VERBOSE("covariance: \n"<<covariance);
    }

    template <unsigned int nDim> 
        MdtSegmentFitter::UpdateStatus
            MdtSegmentFitter::updateParameters(Parameters& currPars, Parameters& prevPars,
                                               Parameters& currGrad, Parameters& prevGrad,
                                               const AmgSymMatrix(5)& currentHessian) const {
            
            AmgSymMatrix(nDim) miniHessian = currentHessian.block<nDim, nDim>(0,0);
            ATH_MSG_VERBOSE("Parameter update -- \ncurrenPars: "<<toString(currPars)<<", \ngradient: "<<toString(currGrad)
                          <<", hessian ("<<miniHessian.determinant()<<")"<<std::endl<<miniHessian);

            double changeMag{0.};
            if (std::abs(miniHessian.determinant()) > detCutOff) {
                prevPars.block<nDim,1>(0,0) = currPars.block<nDim,1>(0,0);
                // Update the parameters accrodingly to the hessian
                const AmgVector(nDim) updateMe =  miniHessian.inverse()* currGrad.block<nDim, 1>(0,0);
                changeMag = std::sqrt(updateMe.dot(updateMe));
                currPars.block<nDim,1>(0,0) -= updateMe;
                prevGrad.block<nDim,1>(0,0)  = currGrad.block<nDim,1>(0,0);
                ATH_MSG_VERBOSE("Hessian inverse:\n"<<miniHessian.inverse()<<"\n\nUpdate the parameters by -"
                             <<Amg::toString(updateMe));
            } else {
                const AmgVector(nDim) gradDiff = (currGrad - prevGrad).block<nDim,1>(0,0);
                const double gradDiffMag = gradDiff.mag2();
                double denom = (gradDiffMag > std::numeric_limits<float>::epsilon() ? gradDiffMag : 1.); 
                const double gamma = std::abs((currPars - prevPars).block<nDim,1>(0,0).dot(gradDiff))
                                   / denom;
                ATH_MSG_VERBOSE("Hessian determinant invalid. Try deepest descent - \nprev parameters: "
                             <<toString(prevPars)<<",\nprevious gradient: "<<toString(prevGrad)<<", gamma: "<<gamma);
                prevPars.block<nDim, 1>(0,0) = currPars.block<nDim, 1>(0,0);
                currPars.block<nDim, 1>(0,0) -= gamma* currGrad.block<nDim, 1>(0,0);
                prevGrad.block<nDim, 1>(0,0) = currGrad.block<nDim,1>(0,0);
                changeMag = std::abs(gamma) *  currGrad.block<nDim, 1>(0,0).mag(); 
            }
            /// Check that all parameters remain within the parameter boundary window
            unsigned int nOutOfBound{0};
            for (unsigned int p =0; p< nDim; ++p) {
                if (m_cfg.ranges[p][0] > currPars[p] || m_cfg.ranges[p][1]< currPars[p]) {
                    ATH_MSG_WARNING("The "<<p<<"-th parameter "<<toString(static_cast<ParamDefs>(p))<<" is out of range "<<currPars[p]
                                    <<"["<<m_cfg.ranges[p][0]<<"-"<<m_cfg.ranges[p][1]<<"]");
                    ++nOutOfBound;
                }
                currPars[p] = std::clamp(currPars[p], m_cfg.ranges[p][0], m_cfg.ranges[p][1]);
            }
            if (nOutOfBound > m_cfg.nParsOutOfBounds){
                return UpdateStatus::outOfBounds;
            }
            if (changeMag <= m_cfg.tolerance) {
                return UpdateStatus::noChange;
            }
            return UpdateStatus::allOkay;
        }
}
