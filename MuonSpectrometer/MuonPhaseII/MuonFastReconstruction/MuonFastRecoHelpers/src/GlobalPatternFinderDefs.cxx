/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/GlobalPatternFinderDefs.h"

#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "FourMomUtils/P4Helpers.h"

/// Macro printing verbose messages
#define PRINT_VERBOSE( xmsg )                                      \
    do {                                                           \
        if( logger->msgLvl( MSG::VERBOSE ) ) {                     \
            logger->msg( MSG::VERBOSE ) << xmsg << endmsg;         \
        }                                                          \
   } while( 0 ) 

namespace {
    const Muon::MuonSectorMapping sectorMap{};

    /** @brief Helper function to construct the gradient of the azimuthal coordinate
     *  @param pos The position vector where the gradient is to be computed
     *  @return The gradient of the azimuthal coordinate */
    Amg::Vector3D phiGradient(const Amg::Vector3D& pos) {
        return Amg::Vector3D{-pos.y(), pos.x(), 0.} / pos.perp2();
    }
    /** @brief Helper functon to compute the size of an expanded sector */
    double expandedSectorSize (const MuonR4::ExpandedSector& sect) {
        unsigned sector1 {sect.msSector()};
        unsigned sector2 {sect.adjacentMsSector()};

        if (sector1 == sector2) {
            return sectorMap.sectorSize(sector1);
        }
        /** The overlap size is the same for small and large sectors */
        return sectorMap.sectorWidth(sector1) - sectorMap.sectorSize(sector1);
    };
    /** @brief Convert an angle from radians to degrees */
    constexpr double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
}

namespace MuonR4::FastReco {
    using namespace Acts::UnitLiterals;
    
    GlobalPatternFinder::HitPayload::HitPayload(const Acts::GeometryContext& gctx,
                                                const SpacePoint* spacepoint,
                                                const SpacePointBucket* bucket,
                                                const Amg::Transform3D& localToGlobal)
        : position{localToGlobal * spacepoint->localPosition()}, 
          sp{spacepoint}, bucket{bucket} {

        using CovIdx = SpacePoint::CovIdx;
        
        if (!sp->measuresEta()) {
            // Phi-only measurements
            const Amg::Vector3D phiMeasDir {localToGlobal.rotation() * sp->toNextSensor()};

            phiCov = sp->covariance()[Acts::toUnderlying(CovIdx::phiCov)] *
                Acts::square(phiMeasDir.dot(phiGradient(position)));
            return;
        }
        const auto& surfLinearTrf = surface.localToGlobalTransform(gctx).linear();
        
        if (sp->isStraw()) {
            // Remember that for straw hits, the x component of secondaryMeasDir is repurposed 
            // to store the transverse covariance of the drift radius
            double& discCov = stripAngle; 
            discCov = Acts::square(sp->driftRadius()) +
                sp->covariance()[Acts::toUnderlying(CovIdx::etaCov)];
            
            if (sp->measuresPhi()) {
                phiCov = discCov / Acts::square(position.perp()) +
                    Acts::square(sensorDir(gctx).dot(phiGradient(position))) * 
                        (sp->covariance()[Acts::toUnderlying(CovIdx::phiCov)] - discCov);
            }
        } else if (sp->measuresPhi()) {
            const Amg::Vector3D phiMeasDir = surfLinearTrf.col(Amg::y);
            const Amg::Vector3D gradPhi {phiGradient(position)};
            /** @brief Helper method to compute the contribution of a 1D measurement to the residual variance */
            auto oneDimContribution = [&](CovIdx idx, const Amg::Vector3D& measDir) -> double {
                return sp->covariance()[Acts::toUnderlying(idx)] * 
                    Acts::square(measDir.dot(gradPhi));
            };
            
            // Handle the case of TGC separately
            if (sp->type() == xAOD::UncalibMeasType::TgcStripType) {
                const Amg::Vector3D etaMeasDir = localToGlobal.rotation() * sp->toNextSensor();
                const Amg::Vector3D phiSensorDir = surfLinearTrf.col(Amg::x);
                
                const double c {etaMeasDir.dot(phiMeasDir)};
                if (std::abs(c) > Acts::s_epsilon) {
                    stripAngle = std::atan2(etaMeasDir.dot(phiSensorDir), c);
                    nonOrthogonalStrips = true;
                }
                phiCov = oneDimContribution(CovIdx::etaCov, etaMeasDir) +
                         oneDimContribution(CovIdx::phiCov, phiMeasDir);
            } else {
                const Amg::Vector3D etaMeasDir = surfLinearTrf.col(Amg::x);
                phiCov = oneDimContribution(CovIdx::etaCov, etaMeasDir) +
                         oneDimContribution(CovIdx::phiCov, phiMeasDir);
            }
        }
    }
    Amg::Vector3D GlobalPatternFinder::HitPayload::sensorDir(const Acts::GeometryContext& gctx) const {
        const auto& surfLinearTrf = surface.localToGlobalTransform(gctx).linear();
        
        if (sp->isStraw()) {
            return surfLinearTrf.col(Amg::z);
        } else {
            if (nonOrthogonalStrips) {
                return - std::sin(stripAngle) * surfLinearTrf.col(Amg::y)
                       + std::cos(stripAngle) * surfLinearTrf.col(Amg::x);
            }
            return surfLinearTrf.col(Amg::y);
        }
    }
    double 
    GlobalPatternFinder::HitPayload::residualVariance(const Acts::GeometryContext& gctx,
                                                      const Amg::Vector3D& contractionVector, 
                                                      const bool isProjected) const {
        using CovIdx = SpacePoint::CovIdx;

        /** If the hit is not projected, the contraction vector is the residual direction */
        assert(isProjected || std::abs(contractionVector.mag() - 1.0) < Acts::s_epsilon);
        
        if (sp->isStraw()) {
            const double discCov {stripAngle};
            if (!isProjected) {
                assert(sp->measuresPhi());
                const double vDotRsq {Acts::square(sensorDir(gctx).dot(contractionVector))};
                return discCov * (1 - vDotRsq) + 
                       vDotRsq * sp->covariance()[Acts::toUnderlying(CovIdx::phiCov)];
            }
            /** If the hit is projected, the contraction vector is J^T * residualDirection,
            *  where J is the Jacobian of the projection. The phi measurement if available is
            *  cancelled by the projection. */
            return discCov * contractionVector.mag2();

        } else if (sp->measuresEta()) {
            const auto& surfLinearTrf = surface.localToGlobalTransform(gctx).linear();
            /** @brief Helper method to compute the contribution of a 1D measurement to the residual variance */
            auto oneDimContribution = [&](CovIdx idx, const Amg::Vector3D& measDir) -> double {
                return sp->covariance()[Acts::toUnderlying(idx)] * 
                    Acts::square(measDir.dot(contractionVector));
            };
            if (sp->measuresPhi()) {
                const Amg::Vector3D etaMeasDir {nonOrthogonalStrips 
                    ? sensorDir(gctx).cross(surfLinearTrf.col(Amg::z))
                    : surfLinearTrf.col(Amg::x)};
                const Amg::Vector3D phiMeasDir {surfLinearTrf.col(Amg::y)};
                return oneDimContribution(CovIdx::etaCov, etaMeasDir) + 
                       oneDimContribution(CovIdx::phiCov, phiMeasDir);
            }
            const Amg::Vector3D etaMeasDir {surfLinearTrf.col(Amg::x)};
            return oneDimContribution(CovIdx::etaCov, etaMeasDir);
        } else {
            throw std::runtime_error("Phi only hits are not meant to be used for residual computation.");
        }
    }
    bool GlobalPatternFinder::HitPayload::operator==(const HitPayload& other) const {
        return sp == other.sp;
    }

    GlobalPatternFinder::PatternState::PatternState(const CandidateHit& seed,
                                                    const std::int8_t expSector,          
                                                    const Config* cfg,
                                                    const AthMessaging* logger)
            : cfg{cfg},
              logger{logger},
              lastInsertedHit{seed},
              prevLayerHit{seed},
              lineAnchorHit{seed},
              patTheta{seed->position.theta()},
              expSect{ExpandedSector{expSector}} {
                
        /** Add the new hit */
        hitsPerStation[Acts::toUnderlying(seed.station)].push_back(seed);

        /** Update the hit counts */
        if (seed->isPrecision) nPrecisionLayers++;
        else nTriggerLayers++;

        if (seed.sp()->measuresPhi()) nPhiLayers++;

        updatePatternPhi();
        needLineUpdate = true;
    }
    LineTestRes GlobalPatternFinder::PatternState::checkLineComp(const Acts::GeometryContext& gctx,
                                                                 const CandidateHit& testHit,
                                                                 const Amg::Vector3D& beamSpot) {

        if (testHit.sp()->measuresPhi() && !isPhiCompatible(*testHit)) {
            PRINT_VERBOSE(__func__<<"() Test hit phi "<<testHit->position.phi()
                <<" not compatible with "<<brief(*this));
            return LineTestRes{};
        }
        
        /** @brief Helper function to make the result
        *  @param decision The decision for the test result if the residual is within the acceptance window 
        *  @return The test result */
        auto makeResult = [&](const LineTestDecision decision) -> LineTestRes {
            LineTestRes res{computeLineResidual(gctx, testHit)};
            double accWindow {cfg->nResidualSigma * res.sigma};
            /** Loosen the window when we use the beamspot or when we are looking for hits in a new station, as
             *  the straight line approximation becomes less accurate on large distances. TO DO: investigate this further */
            if (useBeamspot || testHit->station != lastInsertedHit.station ||
                (testHit->station != prevLayerHit.station && testHit.globLayer == lastInsertedHit.globLayer)) {
                accWindow *= 2.;
            }
            if (res.residual < accWindow) {
                res.result = decision;
            }
            if (visualInfo) {
                visualInfo->hitLineInfo[testHit.sp()] =
                    std::make_pair(std::tan(lineDir.theta()), accWindow);
            }
            return res;
        };

        if(testHit.globLayer != lastInsertedHit.globLayer) {
            updateLineParameters(gctx, beamSpot);
            return makeResult(LineTestDecision::eAddHit);
        }
        if (testHit == lastInsertedHit) {
            PRINT_VERBOSE(__func__<<"() Test hit is the same as last inserted hit - reject.");
            return LineTestRes{};
        }
        if (lineAnchorHit.globLayer == lastInsertedHit.globLayer) {
            PRINT_VERBOSE(__func__<<"() Test hit on same layer as seed with no prior hits - reject.");
            return LineTestRes{};
        }
        return makeResult(LineTestDecision::eBranchPattern);
    }
    void GlobalPatternFinder::PatternState::moveLineAnchorHit(const CandidateHit& refHit) {
        // Treat first the special case where we have only one station
        if (nStations(/*onlyGoodStations=*/ false) < 2) {
            // If we call this method with only one station, it means that we inverted the hit search direction without
            // finding any hit in other stations beside the initial one. So the anchor is the last added hit.
            lineAnchorHit = lastInsertedHit;
            return;
        }
        // Find first the closest station to the reference station among the pattern stations
        const auto& closestStIt = std::ranges::min_element(hitsPerStation, std::ranges::less{},
            [&refHit](const auto& hits){
                if (hits.empty() || hits.front().station == refHit.station) {
                    return std::numeric_limits<int>::max();
                }
                return std::abs(hits.front().globLayer - refHit.globLayer);
            });

        // Then find the closest hit in that station to the reference hit
        const auto& hits {*closestStIt};
        lineAnchorHit = *std::ranges::min_element(hits, std::ranges::less{},
            [&refHit](const CandidateHit& hit){
                return std::abs(hit.globLayer - refHit.globLayer); });
    }
    void GlobalPatternFinder::PatternState::updateLineParameters(const Acts::GeometryContext& gctx,
                                                                 const Amg::Vector3D& beamSpot) {
        if (!needLineUpdate) {
            return;
        }
        Amg::Vector3D pos1 {projToPhiPlane(gctx, *lineAnchorHit)};
        Amg::Vector3D pos2 {projToPhiPlane(gctx, *lastInsertedHit)};
        Amg::Vector3D d {pos2 - pos1};
        leverArm = d.mag();
        
        /** Check whether we have to use the beamspot instead of the anchor hit to draw the line. */
        useBeamspot = (lastInsertedHit.station == lineAnchorHit.station) && 
                    leverArm < cfg->minHitDistance4Line;
        if (useBeamspot) {
            linePos = beamSpot;
            d = pos2 - beamSpot;
            leverArm = d.mag();
        } else {
            linePos = pos1;
        }
        lineDir = d / leverArm;
        needLineUpdate = false;

        PRINT_VERBOSE(__func__<<"() Updated --> linePos R/z/theta: "<<linePos.perp()<<" / "<<linePos.z()
            <<" / "<<inDeg(linePos.theta())<<", lineDir theta: "<<inDeg(lineDir.theta())
            <<", LeverArm: "<<leverArm<<", Use beamspot: "<<useBeamspot);
    }
    LineTestRes GlobalPatternFinder::PatternState::computeLineResidual(const Acts::GeometryContext& gctx, 
                                                                       const CandidateHit& testHit) const {
        LineTestRes res{};

        /** We project the test hit onto the phi plane only when the test hit does 
         *  not measure phi or when we have no phi layers, otherwise we do not project
         *  so the residual include the error in the phi direction. */
        const bool projectTestHit {!testHit.sp()->measuresPhi() || nPhiLayers == 0u};
        const Amg::Vector3D testPos {projectTestHit ? projToPhiPlane(gctx,*testHit) 
                                                    : testHit->position};
        const Amg::Vector3D K {testPos - linePos};
        const double KdotD {K.dot(lineDir)};
        
        const Amg::Vector3D R {K - KdotD * lineDir}; // Residual vector
        res.residual = R.mag();
        if (res.residual < Acts::s_epsilon) {
            /** If the residual is very small, it's likely due to bad topology, reject it. */
            res.residual = std::numeric_limits<double>::max();
            res.sigma = 0.;
            return res;
        }
        const Amg::Vector3D resDir {R / res.residual}; // Residual direction
        /** Alpha represents the extrapolation distance along the pattern line */
        const double alpha {KdotD / leverArm};
        
        /** Accumulate the derivatives of the scalar residual with respect to the common phi-plane angle. */
        double phiPlaneDerivativeAcc {0.};
        /** Accumulate the covariance contributions of the hits to the residual */
        double residualCovAcc {0.};

        /** @brief Accumulate the covariance contributions of one hit to the residual.
         *         Each hit contributes with its intrinsic covariance and, if projected,  
         *         with the uncertainty in the common phi-plane angle of the pattern.
         *   
         *  Intrinsic covariance: for a projected hit, the residual direction is  
         *  transformed with the projection jacobian J^T * dir.
         *  dir^T * J * cov * J^T * dir = (J^T * dir)^T * cov * (J^T * dir)
         *
         *  Phi plane uncertainty: for a projected hit, the derivative of the scalar  
         *  residual with respect to the common phi-plane angle for the projected hits.
         *
         *  @param hit: the hit for which to compute the covariance
         *  @param isProjected: flag indicating if the hit is projected
         *  @param pos: projected position (needed for the phi-plane derivative of projected hits)
         *  @param preFactor: factor, function of alpha, to scale the covariance contributions */
        auto covarianceTerm = [&](const HitPayload& hit,
                                  const Amg::Vector3D& pos,
                                  const double preFactor,
                                  bool isProjected) -> void {
                    
            if (!isProjected) {
                residualCovAcc += Acts::square(preFactor) * 
                    hit.residualVariance(gctx, resDir, /*isProjected=*/false);
                return;
            }
            const Amg::Vector3D sensorDir {hit.sensorDir(gctx)};
            const double projFactor {sensorDir.dot(resDir) / 
                                     sensorDir.dot(bendPlaneNorm)};
            const Amg::Vector3D trfDir {resDir - projFactor * bendPlaneNorm};
            
            residualCovAcc += Acts::square(preFactor) 
                * hit.residualVariance(gctx, trfDir, /*isProjected=*/true);  
            phiPlaneDerivativeAcc += preFactor * pos.perp() * projFactor;
        };

        /** Compute the covariance contributions of the first line point */
        if (useBeamspot) {
            const double covS1 = cfg->beamSpotLength * Acts::square(resDir.z()) + 
                                 cfg->beamSpotRadius * (1 - Acts::square(resDir.z()));
            residualCovAcc += Acts::square(alpha - 1) * covS1;
        } else {
            covarianceTerm(*lineAnchorHit, linePos, alpha - 1, /*isProjected=*/true);
        }

        /** Compute the covariance contributions of the second line point */
        const Amg::Vector3D pos2 {linePos + leverArm * lineDir};
        covarianceTerm(*lastInsertedHit, pos2, -alpha, /*isProjected=*/true);

        /** Compute the covariance contributions of the test hit */
        covarianceTerm(*testHit, testPos, 1., projectTestHit);
      
        res.sigma = std::sqrt(residualCovAcc + Acts::square(phiPlaneDerivativeAcc) * patPhiCov);

        PRINT_VERBOSE(__func__<<"() "<< brief(*this)<<"\nUse beamspot: "<<useBeamspot
            <<", alpha: "<<alpha<<", Residual: "<<res.residual<<" +- "<<res.sigma
            <<", linePos R/theta: "<<linePos.perp()<<" / "<<inDeg(linePos.theta())
            <<", lineDir theta: "<<inDeg(lineDir.theta())
            <<", testPos R/theta/phi: "<<testPos.perp()<<" / "<<inDeg(testPos.theta())<<" / "<<inDeg(testPos.phi())
            <<", resDir theta/phi: "<<inDeg(resDir.theta())<<" / "<<inDeg(resDir.phi())
            <<", hit pos sigma: "<<std::sqrt(residualCovAcc)
            <<", phi plane sigma: "<<std::abs(phiPlaneDerivativeAcc)*std::sqrt(patPhiCov));
        return res;
    }
    Amg::Vector3D GlobalPatternFinder::PatternState::projToPhiPlane(const Acts::GeometryContext& gctx, 
                                                                    const HitPayload& hit) const {
        return Acts::PlanarHelper::intersectPlane(hit.position, hit.sensorDir(gctx),
            bendPlaneNorm, Amg::Vector3D::Zero()).position();        
    }
    void GlobalPatternFinder::PatternState::updatePatternPhi() {
        if (!nPhiLayers) {
            /** If there are no phi hits, we just use the central phi of the sector/overlap region, 
             *  with a standard deviation based on the expanded sector size. */
            patPhi = sectorMap.sectorOverlapPhi(expSect.msSector(), 
                                                expSect.adjacentMsSector());
            patPhiCov = Acts::square(expandedSectorSize(expSect)) / 3.;
            bendPlaneNorm = Acts::makeDirectionFromPhiTheta(patPhi + 90._degree, 90._degree);
            PRINT_VERBOSE(__func__<<"() No phi hits in the pattern, set pattern phi to "
                <<inDeg(patPhi)<<" +- "<<inDeg(std::sqrt(patPhiCov)));
            return;
        }
        double sumSin{0.}, sumCos{0.}, sumWeight{0.};

        auto processPhiHit = [&sumSin, &sumCos, &sumWeight](const HitPayload& hit){
            if (!hit->measuresPhi()) {
                return;
            }
            if (hit.phiCov < Acts::s_epsilon) {
                std::stringstream ss {};
                ss << "Unexpected to have a phi hit with zero variance in phi direction: " << *hit.sp << "\n";
                throw std::runtime_error(ss.str());
            }
            const double w = 1./hit.phiCov;

            const double phi {hit.position.phi()};
            sumSin += w * std::sin(phi);
            sumCos += w * std::cos(phi);
            sumWeight += w;
        };
        for (const std::vector<CandidateHit>&  hits : hitsPerStation) {
            for (const auto& hit : hits) {
                processPhiHit(*hit);
            }
        }
        for (const HitPayload& hit : phiOnlyHits) {
            processPhiHit(hit);
        }
        
        patPhi = std::atan2(sumSin, sumCos);
        patPhiCov = 1./sumWeight;
        bendPlaneNorm = Acts::makeDirectionFromPhiTheta(patPhi + 90._degree, 90._degree);
        PRINT_VERBOSE(__func__<<"() Updated pattern phi to "
            <<inDeg(patPhi)<<" +- "<<inDeg(std::sqrt(patPhiCov)));
    }
    bool GlobalPatternFinder::PatternState::isPhiCompatible(const HitPayload& hit) const {
        /** We check that the test hit is compatible with the pattern phi, if available, which is given by the first
         *  phi measurement in the pattern. If the pattern doesn't have a phi yet, we check that the test hit is in 
         *  the same pattern sector(s) */
        const double testPhi {hit.position.phi()};
        if (nPhiLayers) {
            const double deltaPhiSigma {std::sqrt(patPhiCov + hit.phiCov)};
            const double deltaPhi {P4Helpers::deltaPhi(patPhi, testPhi)};
            if (std::abs(deltaPhi) > cfg->nPhiSigma * deltaPhiSigma) {
                PRINT_VERBOSE(__func__<<"() The pattern with phi = "<<inDeg(patPhi)<<" +- "<<inDeg(std::sqrt(patPhiCov))
                    <<" is not compatible with the test hit with phi "<<inDeg(testPhi) <<" +- "<<inDeg(std::sqrt(hit.phiCov)));
                return false;
            }
        } else {
            const unsigned sector1 {expSect.msSector()};
            const unsigned sector2 {expSect.adjacentMsSector()};
            const bool isCompatible {sector1 == sector2 
                ? sectorMap.insideSector(sector1, testPhi)
                : sectorMap.insideSector(sector1, testPhi) && sectorMap.insideSector(sector2, testPhi)};
            if (!isCompatible) {
                PRINT_VERBOSE(__func__<<"() The test hit with phi = "<<inDeg(testPhi)
                    <<" is not inside the pattern sectors: "<<sector1<<" and "<<sector2);
                return false;
            }            
        }
        return true;
    }
    void GlobalPatternFinder::PatternState::addHit(const CandidateHit& hit,
                                                   const double residual,
                                                   const double resSigma) {
        /** Add the new hit */
        hitsPerStation[Acts::toUnderlying(hit.station)].push_back(hit);

        /** Update the hit counts */
        if (hit->isPrecision) nPrecisionLayers++;
        else nTriggerLayers++;

        if (hit.sp()->measuresPhi()) {
            nPhiLayers++;
            updatePatternPhi();
        }

        /** Update the pointers to previous layer hit */
        const bool isNewStation {hit.station != lastInsertedHit.station};
        prevLayerHit = lastInsertedHit;
        lastInsertedHit = hit;

        meanNormResidual2 += Acts::square(residual / resSigma);
        lastResSigma = resSigma;
        lastResidual = residual;

        /** If the new compatible hit is in a different station, update the line anchor */
        if (isNewStation) {
            moveLineAnchorHit(hit);
        }
        needLineUpdate = true;
    }
    void GlobalPatternFinder::PatternState::overWriteHit(const CandidateHit& newHit,
                                                         const double newResidual,
                                                         const double newResSigma) {
        const StIndex st {newHit.station};
        if (st != lastInsertedHit.station || lastInsertedHit.globLayer != newHit.globLayer) {
            throw std::runtime_error(std::format(
                "Trying to overwrite a hit in station/layer {}/{} with another one from station/layer {}/{}", 
                    stName(lastInsertedHit.station), lastInsertedHit.globLayer, stName(st), newHit.globLayer));
        }
        /* We expect to overwrite hits of the same type (precision/trigger), since we only branch when we have 
         * compatible hits in the same layer, except for sTGC hits, where we have pad and strips in the same layer */
        if (lastInsertedHit->isPrecision != newHit->isPrecision) {
            if (newHit.sp()->type() != xAOD::UncalibMeasType::sTgcStripType) {
                std::stringstream ss {};
                ss << "Trying to overwrite a hit with incompatible type\n";
                ss << "Old hit: " << **lastInsertedHit << ", isPrecision: " << lastInsertedHit->isPrecision << ", measuresEta: " << lastInsertedHit.sp()->measuresEta() << "\n";
                ss << "New hit: " << **newHit << ", isPrecision: " << newHit->isPrecision << ", measuresEta: " << newHit.sp()->measuresEta();
                throw std::runtime_error(ss.str());
            }
            if (newHit->isPrecision) {
                nPrecisionLayers++;
                nTriggerLayers--;
            } else {
                nPrecisionLayers--;
                nTriggerLayers++;
            }
        }
        /** Update the phi counts */
        bool updatePhi {false};
        if (lastInsertedHit.sp()->measuresPhi()) {
            nPhiLayers--;
            updatePhi = true;
        }
        if (newHit.sp()->measuresPhi()) {
            nPhiLayers++;
            updatePhi = true;
        }
        /** Update the residual */
        meanNormResidual2 += Acts::square(newResidual / newResSigma) - 
                            Acts::square(lastResidual / lastResSigma);
        lastResSigma = newResSigma;
        lastResidual = newResidual;

        /** Remove the last inserted hit */
        if (visualInfo) {
            visualInfo->replacedHits.push_back(lastInsertedHit.sp());
        }
        auto& stHits {hitsPerStation[Acts::toUnderlying(st)]};
        if (stHits.back() != lastInsertedHit) {
            std::stringstream ss {};
            ss << "Trying to overwrite a hit that is not the last inserted hit in station/layer " 
            << stName(st) << "/" << lastInsertedHit.globLayer << "\n";
            ss << "Last inserted hit: " << **lastInsertedHit << "\n";
            ss << "Last hit in station: " << **stHits.back();
            throw std::runtime_error(ss.str());
        }
        stHits.pop_back();

        /** Add the new hit */
        stHits.push_back(newHit);
        lastInsertedHit = newHit;

        if (updatePhi) {
            updatePatternPhi();
        }
        needLineUpdate = true;
    }
    bool GlobalPatternFinder::PatternState::isInPattern(const HitPayload& hit) const {
        const auto& hits {hitsPerStation[Acts::toUnderlying(hit.station)]};
        return std::ranges::find_if(hits, 
            [&hit](const CandidateHit& c){ return *c == hit; }) != hits.end();                                           
    }
    uint8_t GlobalPatternFinder::PatternState::nStations(const bool onlyGoodStations) const {
        uint8_t nStations {0u};
        for (uint8_t st{0u}; st < s_nStations; ++st) {
            const auto& hits {hitsPerStation[st]};
            if (!hits.empty() && 
                    (!onlyGoodStations || hits.size() >= cfg->minStationLayers)) {
                nStations++;
            }
        }
        return nStations;
    }
    uint8_t GlobalPatternFinder::PatternState::nBendingLayers() const {
        return nPrecisionLayers + nTriggerLayers;
    }
    double GlobalPatternFinder::PatternState::getMeanResidual2() const {
        if (isFinalized) {
            return meanNormResidual2;
        }
        return meanNormResidual2 / nBendingLayers();
    }
    std::vector<const SpacePointBucket*> 
    GlobalPatternFinder::PatternState::getParentBuckets() const {
        std::vector<const SpacePointBucket*> buckets{};
        for (const std::vector<CandidateHit>&  hits : hitsPerStation) {
            for (const auto& hit : hits) {
                if (std::ranges::find(buckets, hit->bucket) == buckets.end()) {
                    buckets.push_back(hit->bucket);
                }
            }
        }
        return buckets;
    }
    void GlobalPatternFinder::PatternState::print(std::ostream& ostr, bool detailed) const {
        ostr<<"PatternState Exp Sector: "<<static_cast<int>(expSect.sector())
        <<", Theta: "<<inDeg(patTheta) << ", Phi: "<<inDeg(patPhi)<<" +- "<<inDeg(std::sqrt(patPhiCov));
        ostr<<", nPrec: "<<static_cast<int>(nPrecisionLayers)<<", nEtaNonPrec: "
            <<static_cast<int>(nTriggerLayers)<<", nPhi: "<<static_cast<int>(nPhiLayers);
        ostr<<", mean norma res sq: "<<getMeanResidual2()<<", dirTheta: "<<inDeg(lineDir.theta());
        ostr<<", Hit per station: \n";
        for (uint8_t st{0u}; st < s_nStations; ++st) {
            const auto& hits {hitsPerStation[st]};
            if (hits.empty()) continue;

            ostr<<"  Station "<<static_cast<StIndex>(st)<<" has "<<hits.size()<<" hits ";
            if (detailed) {
                ostr<<"\n";
                for (const auto& hit : hits) {
                    ostr<<"    "<<hit<<"\n";
                }
            }
        }
        if (!detailed) {
            ostr <<"\n    Last hit: "<<lastInsertedHit<<"\n    prevLayerHit: "
                <<prevLayerHit << "\n    lineAnchorHit: "<<lineAnchorHit;
        }
    }

    void GlobalPatternFinder::CandidateHit::print(std::ostream& ostr) const {
        ostr<<*sp()<<", glob Z/R/phi: "<<hit->position.z()<<" / "<<hit->position.perp()<<" / "
            <<inDeg(hit->position.phi())<< ", st: " << station <<", loc/glob lay: "
            <<static_cast<int>(hit->locLayer)<<"/"<<static_cast<int>(globLayer);
    }

    GlobalPatternFinder::PatternPrintView 
    GlobalPatternFinder::brief(const PatternState& p) {
        return {p, /*detailed=*/false};
    }
    GlobalPatternFinder::PatternPrintView 
    GlobalPatternFinder::detailed(const PatternState& p) {
        return {p, /*detailed=*/true};
    }
}