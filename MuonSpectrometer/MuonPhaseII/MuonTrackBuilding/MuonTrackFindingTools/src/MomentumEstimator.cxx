/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MomentumEstimator.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonStationIndex/MuonStationIndex.h"

#include <bitset>
#include <limits>
#include <stdexcept>

namespace {
    /** @brief Check if a segment is an NSW segment. */
    bool isNswSegment(const xAOD::MuonSegment& seg) {
        using namespace Muon::MuonStationIndex;
        return seg.technology() == TechnologyIndex::STGC ||
               seg.technology() == TechnologyIndex::MM;
    }
    /** @brief Project a point along a direction onto a plane. */
    Acts::Vector3 projectAlongWire(const Acts::Vector3& x, const Acts::Vector3& wireDir,
                                   const Acts::Vector3& planeNormal, const double offset = 0.) {
        return Acts::PlanarHelper::intersectPlane(x, wireDir, planeNormal, offset).position();
    }
    /** @brief Convert rad to deg. */
    constexpr double inDeg(double angle) {
        using namespace Acts::UnitLiterals;
        return angle / 1._degree;
    }
    /** Return the reduced chi2 of a fit. It returns +max if the fit is not successful. */
    double redChi2(const MuonR4::MomentumEstimator::Result_t& res) {
        return res ? res->redChi2() : std::numeric_limits<double>::max();
    }
    /** @brief Get the index of an enum value. */
    template <auto E> 
    constexpr std::size_t idx = Acts::toUnderlying(E);
}

namespace MuonR4 {
using namespace Acts::UnitLiterals;

MomentumEstimator::MomentumEstimator(Config&& config,
                                     std::unique_ptr<const Acts::Logger> actsLogger)
    : m_cfg{std::move(config)}, m_logger{std::move(actsLogger)} {
    if (!m_cfg.wireDir) {
        throw std::invalid_argument("The wire direction function is not set");
    }
    if (!m_cfg.sectFrame) {
        throw std::invalid_argument("The sector frame function is not set");
    }
    if (!m_cfg.bField){
        throw std::invalid_argument("The magnetic field provider is not set");
    }
    if (m_cfg.nFieldSteps == 0) {
        throw std::invalid_argument("The number of field steps must not be zero");
    }
    /** Initialize the field extraction steps */
    m_fieldExtpSteps.reserve(m_cfg.nFieldSteps);
    const double stepSize {1. / m_cfg.nFieldSteps};
    for (std::size_t i = 0; i < m_cfg.nFieldSteps; ++i) {
        m_fieldExtpSteps.push_back((static_cast<double>(i) + 0.5) * stepSize);
    }
}

MomentumEstimator::Result_t
        MomentumEstimator::fitQoverP(const Acts::GeometryContext& tgContext,
                                     const Acts::MagneticFieldContext& mfContext,
                                     const MsTrackSeed& seed,
                                     const bool withRecovery,
                                     const std::optional<BeamSpot>& bs) const {
        using namespace Muon::MuonStationIndex;

        Acts::MagneticFieldProvider::Cache mfCache {m_cfg.bField->makeCache(mfContext)};

        const BendingPlane plane {fitBendingPlane(seed)};
        std::vector<SegmentData> segmentData {projectSegments(tgContext, plane, seed.segments())};
        const std::size_t nSegments {segmentData.size()};
        
        /** Fit of a set of segments; if fails (the path crosses no bending field), second chance with the beam spot */
        auto fitSegments = [&](std::span<const SegmentData> segs, const bool forceBeamspot = false) -> Result_t {
            Result_t fit {std::nullopt};
            BendingPlane subPlane {plane};
            std::vector<SegmentData> reprojected{};
            if (m_cfg.refitBendingPlane) {
                const bool droppedPhi {std::ranges::any_of(seed.segments(), 
                    [&](const xAOD::MuonSegment* s) { 
                        return s->nPhiLayers() && std::ranges::none_of(segs, 
                            [&](const SegmentData& d) { return d.segment == s; }); })};
                if (droppedPhi) {
                    std::vector<const xAOD::MuonSegment*> kept{};
                    std::ranges::transform(segs, std::back_inserter(kept), 
                        [](const SegmentData& d) { return d.segment; });
                    const std::optional<BendingPlane> refit {fitBendingPlane(kept)};
                    if (refit) {
                        subPlane = *refit;
                        reprojected = projectSegments(tgContext, subPlane, kept);
                        segs = reprojected;
                    }
                }
            }
            /** No beamspot: we need at least 2 segments */
            if (!forceBeamspot && segs.size() >= 2) {
                ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Attempting fit without beamspot...");
                fit = fitQoverP(mfCache, subPlane, segs);
            }
            /** With beamspot: we need at least one segment */
            if (!fit && bs && !segs.empty()) {
                ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Attempting fit with beamspot...");
                fit = fitQoverP(mfCache, subPlane, segs, bs);
            }
            return fit;
        };

        Result_t fit {fitSegments(segmentData)};
        if (!withRecovery || isGoodFit(fit)) {
            /** If no recovery is requested or the fit is good, return it */
            return fit;
        }

        /** Recovery of wrong segments: the largest subset of segments (at least two) with a clearly better fit. At each
         *  size all the subsets are fitted; the best is taken if its chi2/ndf is smaller by the factor qpOutlierChi2 than
         *  the second best and than the current fit. A good but ambiguous best stops the search, a bad one goes on with
         *  smaller subsets. Recovery runs for minimum 3 initial segments, with only two the fitter cannot tell which one 
         *  is wrong. A cap is placed on the number of segments to avoid combinatorial explosion. */
        using Selection = std::bitset<s_maxSegments>;
        if (nSegments >= 3 && nSegments <= s_maxSegments) {
            std::vector<SegmentData> subset{};
            Selection bestSet{};
            const std::size_t minSize {nSegments > m_cfg.maxDroppedSegments + 2u 
                ? nSegments - m_cfg.maxDroppedSegments : 2u};
            for (std::size_t size = nSegments - 1; size >= minSize; --size) {
                std::vector<std::pair<Result_t, Selection>> trials{};
                for (std::uint32_t m = 0; m < (1u << nSegments); ++m) {
                    const Selection trial {m};
                    if (trial.count() != size) {
                        continue;
                    }
                    subset.clear();
                    // Copy segments keeping the order along the track
                    for (const auto& [s, seg] : Acts::enumerate(segmentData)) {
                        if (trial.test(s)) {
                            subset.push_back(seg);   
                        }
                    }
                    const Result_t trialRes {fitSegments(subset)};
                    if (trialRes) {
                        trials.emplace_back(trialRes, trial);
                    }
                }
                if (trials.empty()) {
                    continue;
                }
                std::ranges::sort(trials, {}, [&](const auto& t) { return redChi2(t.first); });
                const Result_t& bestTrial {trials[0].first};
                ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<size<<" of "<<nSegments
                    <<" segments: best chi2/ndf "<<redChi2(bestTrial)<<", current "<<redChi2(fit));
                    
                const double secondChi2 {trials.size() > 1 
                    ? std::min(redChi2(trials[1].first), redChi2(fit)) 
                    : redChi2(fit)};
                const bool clearBest {redChi2(bestTrial) < secondChi2 / m_cfg.outlierChi2};
                if (!clearBest) {
                    if (isGoodFit(bestTrial)) {
                        // good but ambiguous: if the first best and second best are similar, we
                        // cannot tell which one is wrong, so we don't choose any and stop the search.
                        break;      
                    }
                    // nothing clearly better at this size: try smaller subsets
                    continue;       
                }
                if (isGoodFit(bestTrial)) {
                    // good and clearly better: take it and stop the search
                    return bestTrial;  
                }
                fit = bestTrial;
                bestSet = trials[0].second;
            }
            /** Keep only the segments of the subset taken (in the order along the track) */
            if (bestSet.any()) {
                std::vector<SegmentData> kept{};
                for (const auto& [s, seg] : Acts::enumerate(segmentData)) {
                    if (bestSet.test(s)) {
                        kept.push_back(seg);
                    }
                }
                segmentData = std::move(kept);
            }
        }
        /** If after the recovery the fit is still bad and no inner segment is left, try the beam spot in 
         *  place of the inner station: the beam-spot fit is taken if its chi2/ndf is lower. */
        const bool hasInner {std::ranges::any_of(segmentData, [](const SegmentData& seg) {
            return toLayerIndex(seg.segment->chamberIndex()) == LayerIndex::Inner;
        })};
        if (bs && !hasInner && !segmentData.empty()) {
            const Result_t withBeamSpot {fitSegments(segmentData, /*forceBeamSpot=*/true)};
            if (withBeamSpot && redChi2(withBeamSpot) < redChi2(fit)) {
                ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - beam spot in place of the inner station, chi2/ndf "
                                            <<redChi2(withBeamSpot)<<" vs "<<redChi2(fit));
                fit = withBeamSpot;
            }
        }
        return fit;
    }

    MomentumEstimator::Result_t 
        MomentumEstimator::fitQoverP(Acts::MagneticFieldProvider::Cache& mfCache,
                                     const BendingPlane& bendingPlane,
                                     const std::span<const SegmentData> segmentData,
                                     const std::optional<BeamSpot>& bs) const {
        using namespace Acts::UnitLiterals;
        /** Bending plane */
        const auto [circPhi, planeOffset] = bendingPlane;
        const Acts::Vector3 eR {Acts::makeDirectionFromPhiTheta(circPhi, 90._degree)};
        const Acts::Vector3 planeNorm {Acts::makeDirectionFromPhiTheta(circPhi + 90._degree, 90._degree)};

        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Bending plane: phi "<<inDeg(circPhi)
                             <<" [deg], offset "<<planeOffset);

        /** Beamspot: retrieve its plane projection point and uncertainy along z */
        Acts::Vector3 bsOnPlane {Acts::Vector3::Zero()};
        if (bs) {
            bsOnPlane = bs->position - (planeNorm.dot(bs->position) - planeOffset) * planeNorm;
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Beamspot: global "<<Amg::toString(bs->position)
                                 <<", on plane "<<Amg::toString(bsOnPlane)<<", sigma_z "<<bs->sigmaZ);
        }
        /** Retrieve the positions on the bending plane compute the force integrals*/
        std::vector<Acts::Vector3> positions{};
        positions.reserve(segmentData.size() + bs.has_value());
        if (bs) {
            positions.push_back(bsOnPlane);
        }
        for (const SegmentData& seg : segmentData) {
            using enum iSegProj;
            positions.push_back(seg.proj[idx<eRho>] * eR + planeOffset * planeNorm +
                                seg.proj[idx<eZ>] * Acts::Vector3::UnitZ());
        }
        const std::vector<FieldIntegrals> integrals {
            forceIntegration(positions, planeNorm, mfCache)};
        /** The momentum is measured only if the path crosses bending field: sum of |Delta B1| over the gaps between
         *  consecutive points (a field changing sign between the stations still counts) */
        double bendingPower {0.};
        for (std::size_t j = 1; j < integrals.size(); ++j) {
            bendingPower += std::abs(integrals[j][idx<iFieldInt::eB1>] - integrals[j - 1][idx<iFieldInt::eB1>]);
        }
        if (bendingPower < m_cfg.minBendingPower) {
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - bending not measured: field integral "
                <<bendingPower / (Acts::UnitConstants::T * Acts::UnitConstants::m)<<" T m along the path");
            return std::nullopt;
        }
        
        /** Reference point and frame: the fit is linearised around a straight reference line: */
        const std::size_t first {bs.has_value()};
        SegProj refLine {segmentData[0].proj};
        if (bs) {
            using enum iSegProj;
            refLine[idx<eRho>] = bsOnPlane.dot(eR);
            refLine[idx<eZ>] = bsOnPlane.z();
            refLine[idx<eAlpha>] = std::atan2(segmentData[0].proj[idx<eRho>] - refLine[idx<eRho>],
                                              segmentData[0].proj[idx<eZ>] - refLine[idx<eZ>]);
        }
        const double alphaRef {refLine[idx<iSegProj::eAlpha>]};
        const Acts::Vector2 eURef {std::cos(alphaRef), -std::sin(alphaRef)};
        
        /** We change the bending plane rotation center: the plane rotation around the z axis for a 
         *  segment far from it looks like a shift of the plane -> express the rotation about the 
         *  average rho of the segments (d/dphi - rhoMean d/dd), nearly uncorrelated with the shift. */
        double rhoMean {mean(segmentData, [](const SegmentData& seg) {
            return seg.proj[idx<iSegProj::eRho>];
        })};

        /** Accumulating the normal equations [(A^T W A) * P = A^T W m] with P being the 5 fit parameters. 
         *  The lhs and rhs can be expressed as sums over segment blocks, since they are uncorrelated. 
         *  Here we accumulate lhs, rhs, and store segment quantities to compute the chi2 later. */
        using enum iFitPars;
        Acts::SquareMatrix<s_nFitPars> lhs {Acts::SquareMatrix<s_nFitPars>::Zero()};
        Acts::Vector<s_nFitPars> rhs {Acts::Vector<s_nFitPars>::Zero()};
        std::vector<std::tuple<DesignMatrix, SegMeas, SegMeasCov>> segTerms{};
        segTerms.reserve(segmentData.size());
        for (const auto& [s, seg] : Acts::enumerate(segmentData)) {
            const FieldIntegrals& integral {integrals[s + first]};

            ProjPlaneJac relativeDeriv {seg.planeJac};
            relativeDeriv.col(idx<iNuisance::ePlaneRot>) -= rhoMean * relativeDeriv.col(idx<iNuisance::ePlaneShift>);

            DesignMatrix design {DesignMatrix::Zero()};
            
            using enum iSegMeas;
            design(idx<eU>, idx<eU0>) = design(idx<eAlpha>, idx<eAlpha0>) = 1.;
            design(idx<eU>, idx<eAlpha0>) = integral[idx<iFieldInt::eS>];
            design(idx<eU>, idx<eK>) = integral[idx<iFieldInt::eB2>];
            design(idx<eU>, idx<ePlaneShift>) = 
                relativeDeriv.block<2, 1>(idx<iSegProj::eRho>, idx<iNuisance::ePlaneShift>).dot(eURef);
            design(idx<eU>, idx<ePlaneRot>) = 
                relativeDeriv.block<2, 1>(idx<iSegProj::eRho>, idx<iNuisance::ePlaneRot>).dot(eURef);
            design(idx<eAlpha>, idx<eK>) = integral[idx<iFieldInt::eB1>];
            design(idx<eAlpha>, idx<ePlaneShift>) = 
                relativeDeriv(idx<iSegProj::eAlpha>, idx<iNuisance::ePlaneShift>);
            design(idx<eAlpha>, idx<ePlaneRot>) = 
                relativeDeriv(idx<iSegProj::eAlpha>, idx<iNuisance::ePlaneRot>);

            const Acts::Vector2 relPos {seg.proj.segment<2>(idx<iSegProj::eRho>) - 
                                        refLine.segment<2>(idx<iSegProj::eRho>)};
            const SegMeas meas {relPos.dot(eURef), // offset u
                                std::remainder(seg.proj[idx<iSegProj::eAlpha>] - alphaRef, 360._degree)}; // angle alpha
            const SegMeasCov invCov {seg.cov.inverse()};
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - segment "<<printSegment(*seg.segment)
                <<": meas u "<<meas[idx<iSegMeas::eU>]<<", alpha "<<inDeg(meas[idx<iSegMeas::eAlpha>])
                <<" [deg], invCov "<<Amg::toString(invCov.col(0))<<", "<<Amg::toString(invCov.col(1)));
            lhs += design.transpose() * invCov * design;
            rhs += design.transpose() * invCov * meas;
            segTerms.emplace_back(design, meas, invCov);
        }

        /** Beamspot: it's just one measurement with relative position (z += sig)*(-sin(alphaRef)),
         *  so A^T W A = 1 / (sigma * sin(alphaRef))^2 */
        const double bsWeight {bs ? 1. / Acts::square(bs->sigmaZ * std::sin(alphaRef)) : 0.};
        lhs(idx<eU0>, idx<eU0>) += bsWeight;

        /** Nuisance parameters: they enter as measurements */
        const bool barrelSeed {isBarrel(toStationIndex(segmentData[0].segment->chamberIndex()))};
        const double shiftSigma {barrelSeed ? m_cfg.planeShiftSigmaB : m_cfg.planeShiftSigmaE};
        const double shiftWeight {1. / Acts::square(shiftSigma)};
        lhs(idx<ePlaneShift>, idx<ePlaneShift>) += shiftWeight;
        const double rotSigma {barrelSeed ? m_cfg.planeRotSigmaB : m_cfg.planeRotSigmaE};
        const double rotWeight {1. / Acts::square(rotSigma)};
        lhs(idx<ePlaneRot>, idx<ePlaneRot>) += rotWeight;

        const Acts::Vector<s_nFitPars> fitPars {lhs.inverse() * rhs};
        Result res{};
        for (const auto& [design, meas, weight] : segTerms) {
            const Acts::Vector2 residual {meas - design * fitPars};
            res.chi2 += residual.dot(weight * residual);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - segment residuals: u "<<residual[idx<iSegMeas::eU>]
                                 <<", alpha "<<inDeg(residual[idx<iSegMeas::eAlpha>])<<" [deg], "
                                 <<" chi2 "<<residual.dot(weight * residual));
        }
        res.chi2 += bsWeight * Acts::square(fitPars[idx<eU0>]) + shiftWeight * Acts::square(fitPars[idx<ePlaneShift>])
                                                               + rotWeight * Acts::square(fitPars[idx<ePlaneRot>]);
        res.ndf = 2* segmentData.size() + bs.has_value() - 3;
        res.qOverP = fitPars[idx<eK>];
        return res;
    }

    MomentumEstimator::BendingPlane 
        MomentumEstimator::fitBendingPlane(const MsTrackSeed& seed) const {

        std::optional<BendingPlane> plane {fitBendingPlane(seed.segments())};
        if (!plane) { 
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - No segment with phi layers: plane of the expanded sector");
            return BendingPlane{seed.sector().phi(), 0.};
        }
        return *plane;
    }

    std::optional<MomentumEstimator::BendingPlane> 
        MomentumEstimator::fitBendingPlane(std::span<const xAOD::MuonSegment* const> segs) const {
        /** Orients the bending plane outwards */
        auto orientOutwards = [](double& phi, double& offset, const Acts::Vector2& reference) {
            const Acts::Vector2 lineDir {std::cos(phi), std::sin(phi)};
            if (lineDir.dot(reference) >= 0.) {
                return;
            }
            phi = (-lineDir).phi();
            offset = -offset;
        };
        
        std::vector<const xAOD::MuonSegment*> withPhi{};
        for (const xAOD::MuonSegment* seg : segs) {
            if (!seg->nPhiLayers() || 
                    (m_cfg.skipNswInBendingPlane && isNswSegment(*seg))) {
                continue;
            }
            withPhi.push_back(seg);
        }
        if (withPhi.empty() && m_cfg.skipNswInBendingPlane) {
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - No segment with phi layers, using NSW segment.");
            const auto nswSegWithPhi = std::ranges::find_if(segs, 
                [](const xAOD::MuonSegment* segment) {
                    return segment->nPhiLayers() && isNswSegment(*segment); });
            if (nswSegWithPhi != segs.end()) {
                withPhi.push_back(*nswSegWithPhi);
            }
        }
        if (withPhi.empty()) { 
            return std::nullopt;
        }
        if (withPhi.size() == 1) {
            /** Only one segment: it defines the bending plane */
            const xAOD::MuonSegment& seg {*withPhi.front()};
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Bending plane from the line of "<<printSegment(seg));
            const Acts::Vector2 pos {seg.position().head<2>()};
            const Acts::Vector2 dir {seg.direction().head<2>().normalized()};
            double phi = dir.phi();
            double offset = Acts::Vector2{-dir.y(), dir.x()}.dot(pos);
            orientOutwards(phi, offset, pos);
            return BendingPlane{phi, offset};
        }
        /** Two or more: the line through their positions (orthogonal regression in the transverse plane) */
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Bending plane from the positions of "<<withPhi.size()<<" segments");
        const Acts::Vector2 centroid {mean(withPhi, 
            [](const xAOD::MuonSegment* segment) -> Acts::Vector2 {
                return segment->position().head<2>();
        })};
        double sxx {0.}, syy {0.}, sxy {0.};
        for (const xAOD::MuonSegment* segment : withPhi) {
            const Acts::Vector2 d {segment->position().head<2>() - centroid};
            sxx += Acts::square(d.x());
            syy += Acts::square(d.y());
            sxy += d.x() * d.y();
        }
        double phi = 0.5 * std::atan2(2. * sxy, sxx - syy);
        double offset = Acts::Vector2{-std::sin(phi), std::cos(phi)}.dot(centroid);
        orientOutwards(phi, offset, centroid);
        return BendingPlane{phi, offset};
    }

    MomentumEstimator::SegmentData
        MomentumEstimator::projectSegment(const Acts::GeometryContext& tgContext,
                                          const BendingPlane& bendingPlane,
                                          const xAOD::MuonSegment& seg) const {
        using namespace Acts::UnitLiterals;
        using enum iSegProj;
        using enum iNuisance;
        SegmentData data{};
        data.segment = std::addressof(seg);
        
        const auto [circPhi, planeOffset] = bendingPlane;
        const Acts::Vector3 eR {Acts::makeDirectionFromPhiTheta(circPhi, 90._degree)};
        const Acts::Vector3 planeNorm {Acts::makeDirectionFromPhiTheta(circPhi + 90._degree, 90._degree)};

        /** Position and direction moved along the wire onto the plane */
        const Acts::Vector3 wireDir {m_cfg.wireDir(tgContext, seg)};
        const Acts::Vector3 projSegPos {projectAlongWire(seg.position(), wireDir, planeNorm, planeOffset)};
        const Acts::Vector3 projDir {projectAlongWire(seg.direction(), wireDir, planeNorm).normalized()};
        data.proj[idx<eRho>] = projSegPos.dot(eR);
        data.proj[idx<eZ>] = projSegPos.z();
        data.proj[idx<eAlpha>] = std::atan2(projDir.dot(eR), projDir.z());

        /** Derivatives of (rho, z, alpha) with respect to the plane with offset (d) and angle (phi). */
        const Acts::Vector3 eU {planeNorm.cross(projDir)};
        const double nDotW {planeNorm.dot(wireDir)};
        ProjPlaneJac& planeJac {data.planeJac};
        planeJac(idx<eRho>, idx<ePlaneShift>) = eR.dot(wireDir) / nDotW;
        planeJac(idx<eRho>, idx<ePlaneRot>) = 
            data.proj[idx<eRho>] * planeJac(idx<eRho>, idx<ePlaneShift>) + planeOffset;
        planeJac(idx<eZ>, idx<ePlaneShift>) = wireDir.z() / nDotW;                            
        planeJac(idx<eZ>, idx<ePlaneRot>) = 
            data.proj[idx<eRho>] * planeJac(idx<eZ>, idx<ePlaneShift>);
        planeJac(idx<eAlpha>, idx<ePlaneRot>) = projDir.dot(eR) * eU.dot(wireDir) / nDotW;

        /** Covariance of (u, alpha), u along eU = n x projDir: sigma(u) = |cos theta| sigma(y0), 
         *  sigma(alpha) = sigma(theta), correlated as (y0, theta) with the signs of the changes of u and alpha. */
        const std::optional<SegmentFit::Covariance> locCov {SegmentFit::localSegmentCov(seg)};
        if (!locCov) {
            if (nMeasurements(seg) > 0ul) {
                ACTS_WARNING(__func__<<"() "<<__LINE__<<" - No covariance decoration on the segment "
                                     <<printSegment(seg)<<", fixed covariance used.");
            }
            /** Covariance of (u, alpha) of the segments without a measured one: 1 mm and 1 mrad, no correlation */
            data.cov(idx<iSegMeas::eU>, idx<iSegMeas::eU>) = Acts::square(1._mm);
            data.cov(idx<iSegMeas::eAlpha>, idx<iSegMeas::eAlpha>) = Acts::square(1._mrad);
            return data;
        }
        const Acts::RotationMatrix3 locToGlob {m_cfg.sectFrame(tgContext, seg).linear()};
        const double signAlpha {std::copysign(1., -eU.dot(locToGlob.col(Amg::z)))};
        const double signU {std::copysign(1., eU.dot(locToGlob.col(Amg::y)))};
        const double cosTheta {std::abs(seg.direction().dot(locToGlob.col(Amg::z)))};

        constexpr auto iY0 {Acts::toUnderlying(SegmentFit::ParamDefs::y0)};
        constexpr auto iTheta {Acts::toUnderlying(SegmentFit::ParamDefs::theta)};
        SegMeasCov& cov {data.cov};
        cov(idx<iSegMeas::eU>, idx<iSegMeas::eU>) = Acts::square(cosTheta) * (*locCov)(iY0, iY0);
        cov(idx<iSegMeas::eAlpha>, idx<iSegMeas::eAlpha>) = (*locCov)(iTheta, iTheta);
        cov(idx<iSegMeas::eU>, idx<iSegMeas::eAlpha>) = cov(idx<iSegMeas::eAlpha>, idx<iSegMeas::eU>) = 
            signU * signAlpha * cosTheta * (*locCov)(iY0, iTheta);
        return data;
    }

    std::vector<MomentumEstimator::SegmentData> 
        MomentumEstimator::projectSegments(const Acts::GeometryContext& tgContext,
                                           const BendingPlane& bendingPlane,
                                           const std::span<const xAOD::MuonSegment* const> segments) const {
        std::vector<SegmentData> data{};
        data.reserve(segments.size());
        for (const xAOD::MuonSegment* segment : segments) {
            data.push_back(projectSegment(tgContext, bendingPlane, *segment));
        }
        return data;
    }

    std::vector<MomentumEstimator::FieldIntegrals> 
        MomentumEstimator::forceIntegration(const std::vector<Acts::Vector3>& points,
                                            const Acts::Vector3& planeNorm,
                                            Acts::MagneticFieldProvider::Cache& mfCache) const {
        std::vector<FieldIntegrals> integrals{};
        integrals.reserve(points.size());
        /** Moments of the field along the path: M0 = int B.n ds, M1 = int s B.n ds; B2_j = s_j M0 - M1 */
        double s {0.}, M0 {0.}, M1 {0.};
        integrals.push_back(FieldIntegrals::Zero());
        for (std::size_t j = 1; j < points.size(); ++j) {
            const Acts::Vector3& pos1 {points[j - 1]};
            const Acts::Vector3& pos2 {points[j]};
            const double length {(pos2 - pos1).mag()};
            const double dS {length / static_cast<double>(m_fieldExtpSteps.size())};
            for (const double fieldStep : m_fieldExtpSteps) {
                const Acts::Vector3 extPos {(1. - fieldStep) * pos1 + fieldStep * pos2};
                auto fieldResult = m_cfg.bField->getField(extPos, mfCache);
                if (!fieldResult.ok()) {
                    ACTS_WARNING(__func__<<"() "<<__LINE__<<" - failed retrieving the field at "
                                         <<Amg::toString(extPos));
                    continue;
                }
                const double bNorm {-fieldResult->dot(planeNorm)};
                M0 += bNorm * dS;
                M1 += (s + fieldStep * length) * bNorm * dS;
            }
            s += length;
            integrals.emplace_back(s, M0, s * M0 - M1);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - point "<<j<<": s "<<s
                <<" B1 "<<M0 / (Acts::UnitConstants::T * Acts::UnitConstants::m)<<" [T m]"
                <<" B2 "<<(s * M0 - M1)  / (Acts::UnitConstants::T * Acts::UnitConstants::m)<<" [T m mm]");
        }
        return integrals;
    }
}