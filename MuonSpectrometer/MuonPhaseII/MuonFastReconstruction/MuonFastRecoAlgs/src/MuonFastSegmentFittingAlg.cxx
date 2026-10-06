/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastSegmentFittingAlg.h"

#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonSpacePoint/SpacePointHelpers.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonViews/ContainerDecorator.h"

#include "ActsCalibBase/CalibrationContext.h"
#include "ActsInterop/Logger.h"

namespace {
    /** @brief Calculate the number of degrees of freedom of a segment. We need a 
     *         custom function because the fitter parameters are set externally from
     *         the fitter intead of deducing them from the available measurements. */
    unsigned int NDoF(const MuonR4::Segment& segment) {
        /** In the NSW we fit all 4 parameters, so the nDOF are those returned by the fitter. */
        const bool isNSW {std::ranges::any_of(segment.measurements(), 
            [](const auto& meas) { return xAOD::isNSW(meas->type()); })};
        if (isNSW) {
            return segment.nDoF();
        }
        /** In the other stations we performed only 2D fits in the bending plane, no
         *  matter if we have DoF in phi. */
        unsigned int nMeas {0};
        for (const auto& meas : segment.measurements()) {
            if (!MuonR4::isGoodHit(*meas)) {
                continue;
            }
            nMeas += meas->measuresEta();
        }
        return nMeas - 2u;
    }
    /** @brief Calculate the reduced chi-squared  of a segment */
    double calcRedChi2(const MuonR4::Segment& segment) {
        const unsigned int nDoF {NDoF(segment)};
        return nDoF > 0ul ? segment.chi2() / nDoF : segment.chi2();
    }
    /** @brief Check if the new segment is better than the old segment */
    bool betterSegment(const MuonR4::Segment& newSegment, 
                       const MuonR4::Segment& oldSegment) {
        const unsigned int nDoFNew {NDoF(newSegment)};
        const unsigned int nDoFOld {NDoF(oldSegment)};
        if (nDoFNew == nDoFOld) {
            return newSegment.chi2() < oldSegment.chi2();
        }
        return nDoFNew > nDoFOld;
    }
    using namespace Muon::MuonStationIndex;
    /** @brief Return the rank of a layer */
    auto layerRank = [](LayerIndex l) {
        switch (l) {
            case LayerIndex::Inner:           return 0;
            case LayerIndex::Middle:          return 1;
            case LayerIndex::Outer:           return 2;
            case LayerIndex::BarrelExtended:  return 3;
            case LayerIndex::Extended:        return 4;
            default:                          return 5;
        }
    };
    /** @brief Minimum variance in z for a valid line computation: 1e-2 mm^2 */
    static constexpr double minZVariance {Acts::square(0.1 * Gaudi::Units::mm)};
}

namespace MuonR4 {
using namespace Muon::MuonStationIndex;
using namespace MuonR4::SegmentFit;
using namespace Acts::UnitLiterals;

StatusCode MuonFastSegmentFittingAlg::initialize() {
    ATH_CHECK(m_outSegments.initialize());
    ATH_CHECK(m_patternKey.initialize());
    ATH_CHECK(m_localSegParKey.initialize());
    ATH_CHECK(m_localSegCovKey.initialize());
    ATH_CHECK(m_prdLinkKey.initialize());
    ATH_CHECK(m_prdStateKey.initialize());
    ATH_CHECK(m_combMeasKey.initialize());
    ATH_CHECK(m_inPatterns.initialize());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_segmentCnvTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());

    ATH_CHECK(m_segVisionTool.retrieve(EnableTool(!m_segVisionTool.empty())));

    /** Initialize the segment line fitter */
    SegmentLineFitter::Config fitCfg{};
    fitCfg.calibrator = m_calibTool.get();
    fitCfg.visionTool = m_segVisionTool.get();
    fitCfg.idHelperSvc = m_idHelperSvc.get();
    fitCfg.fitT0 = false;
    fitCfg.calcAlongStrip = false;
    fitCfg.recalibrate = m_recalibInFit;
    fitCfg.useFastFitter = m_useFastFitter;
    fitCfg.fastPreFitter = m_fastPreFitter;
    fitCfg.ignoreFailedPreFit = m_ignoreFailedPreFit;
    fitCfg.useHessian = m_hessianResidual;
    fitCfg.doBeamSpot = false;
    fitCfg.outlierRemovalCut = m_outlierRemovalCut;
    fitCfg.recoveryPull = m_recoveryPull;
    fitCfg.nPrecHitCut = m_precHitCut;
    fitCfg.maxIter = m_maxIter;
    fitCfg.parsToUse = {ParamDefs::y0, ParamDefs::theta};

    /** Initialize the NSW segment fitter */
    SegmentLineFitter::Config nswFitCfg{fitCfg};
    nswFitCfg.parsToUse = {ParamDefs::x0, ParamDefs::y0, ParamDefs::theta, ParamDefs::phi};

    m_fitter = std::make_unique<LineFitter>(name(), std::move(fitCfg));
    m_nswFitter = std::make_unique<LineFitter>(name(), std::move(nswFitCfg));

    /** Initialize the L-R segment seeder */
    MdtSegmentSeeder::Config genCfg{};
    genCfg.hitPullCut = m_seedHitChi2;
    genCfg.busyLayerLimit = 3;
    genCfg.startWithPattern = false;
    m_mdtSeeder = std::make_unique<MdtSegmentSeeder>(std::move(genCfg), 
        makeActsAthenaLogger(this, name()));

    ATH_MSG_DEBUG("FastMuonSABuilder Configuration:\n"
            << " Recalibrate in fit: " << m_recalibInFit << "\n"
            << " Use Hessian in residual: " << m_hessianResidual << "\n"
            << " Seed hit chi2: " << m_seedHitChi2 << "\n"
            << " Recalibrate seed: " << m_recalibSeed << "\n"
            << " Good segment cut (reduced chi2): " << m_goodSegmentCut << "\n"
            << " Outlier removal cut (chi2/nDoF): " << m_outlierRemovalCut << "\n"
            << " Recovery pull: " << m_recoveryPull << "\n"
            << " Precision hit cut: " << m_precHitCut << "\n"
            << " Use fast fitter: " << m_useFastFitter << "\n"
            << " Fast fitter as pre-fitter: " << m_fastPreFitter << "\n"
            << " Ignore failed pre-fits: " << m_ignoreFailedPreFit << "\n"
            << " Max iterations: " << m_maxIter);
    
    m_beamspotCov(Amg::x, Amg::x) = m_beamspotCov(Amg::y, Amg::y) = 
        Acts::square(m_beamSpotRadius);
    m_beamspotCov(Amg::z, Amg::z) = Acts::square(m_beamSpotLength);

    return StatusCode::SUCCESS;
}

StatusCode MuonFastSegmentFittingAlg::execute(const EventContext& ctx) const {

    const ActsTrk::GeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    const GlobalPatternContainer* inPatterns{nullptr};
    ATH_CHECK(SG::get(inPatterns, m_inPatterns, ctx));

    IxAODSegmentCnvTool::DataShip ship{};
    ATH_CHECK(ship.segmentContainer.record(m_outSegments, ctx));
    ATH_CHECK(ship.setupLocalParameters(m_localSegParKey, m_localSegCovKey, ctx));
    ATH_CHECK(ship.setupMeasurementLink(m_combMeasKey, m_prdLinkKey, m_prdStateKey, ctx));

    using PatLink_t = ElementLink<GlobalPatternContainer>;
    xAOD::ContainerDecorator<xAOD::MuonSegmentContainer, PatLink_t> dec_patternLink{m_patternKey, ctx};

    std::size_t patIdx{0};
    ATH_MSG_DEBUG(__func__<<"() Start fitting segments in " << inPatterns->size() << " global patterns");
    for (const GlobalPattern* pat : *inPatterns) {

        std::vector<SegmentSeedPair_t> seedSegPairs{processPattern(ctx, *gctx, *pat)};

        if (seedSegPairs.size() <= 1u) {
            ATH_MSG_DEBUG(__func__<<"() Not enough muon segments to construct a candidate - abort pattern"
                << std::endl << *pat);
            patIdx++;
            continue;
        }
        /** If no phi measurements are found, we add a fake one, in order to prevent the segment selector
         *  during the candidate construction to discard the candidate. */
        const bool hasPhi {std::ranges::any_of(seedSegPairs, [](const SegmentSeedPair_t& seg) {
            return std::ranges::any_of(seg.second->measurements(), [](const auto& meas) {
                return isGoodHit(*meas) && meas->measuresPhi(); });
            })};
        const Segment* toAddPhi{nullptr};
        if(!hasPhi) {
            toAddPhi = findSegmentToAddPhi(seedSegPairs);
        }

        /** We fill the segment container by preserving the ordering by parent pattern */
        ship.segmentContainer->reserve(ship.segmentContainer->size() + seedSegPairs.size());
        for (const SegmentSeedPair_t& seedSegPair : seedSegPairs) {
            const Segment_t& seg = seedSegPair.second;
            xAOD::MuonSegment* segXAOD = m_segmentCnvTool->convertSegment(ctx, *seg, ship);
            
            /** Decorate with pattern link */
            dec_patternLink(*segXAOD) = PatLink_t{*inPatterns, patIdx};

            /** Add the phi hit if needed */
            if (toAddPhi && toAddPhi == seg.get()) {
                segXAOD->setNHits(segXAOD->nPrecisionHits(), 
                                  segXAOD->nPhiLayers() + 1u, 
                                  segXAOD->nTrigEtaLayers());
            }

            ATH_MSG_VERBOSE(__func__<<"() Converted segment " << printSegment(*segXAOD));
        }
        patIdx++;
    }
    ATH_MSG_DEBUG("Written "<<ship.segmentContainer->size()<<" xAOD::Segments into StoreGate.");
    return StatusCode::SUCCESS;
}

std::vector<MuonFastSegmentFittingAlg::SegmentSeedPair_t> 
MuonFastSegmentFittingAlg::processPattern(const EventContext& ctx, 
                                          const ActsTrk::GeometryContext& gctx,
                                          const GlobalPattern& pattern) const {
    ATH_MSG_VERBOSE(__func__<<"() Start processing " << pattern);

    std::vector<StIndex> stations {pattern.getStations()};
    std::ranges::sort(stations, [](StIndex s1, StIndex s2) {
        LayerIndex l1 {toLayerIndex(s1)}, l2 {toLayerIndex(s2)};
        if (l1 == l2) {
            return isBarrel(s1);
        }
        return layerRank(l1) < layerRank(l2);
    });

    std::vector<SegmentSeedPair_t> muonSegments{};
    muonSegments.reserve(3u);
    for (const StIndex st : stations) {
        /** If we already have 3 segments, we don't try to fit more segments in other stations.
         *  Same if we are at the last station and haven't found any segments yet */
        if (muonSegments.size() > 2 || (st == stations.back() && muonSegments.size() == 0.)) {
            break;
        }
        // If we already have a segment in the layer, we don't try to fit another one
        const LayerIndex layer {toLayerIndex(st)};
        if (std::ranges::any_of(muonSegments, [&layer](const SegmentSeedPair_t& seg) {
                return toLayerIndex(seg.second->msSector()->chamberIndex()) == layer; })) {
            ATH_MSG_VERBOSE(__func__<<"() Already found a segment in layer " << layer 
                << " - skip station " << st);
            continue;
        }
        /** Retrieve the pattern hits & parent buckets in the station */
        const HitVec_t& hits {pattern.hitsInStation(st)};
        const std::vector<Bucket_t>& buckets {pattern.bucketsInStation(st)};

        /** Since we can have multiple sectors in a station, we need to group hits by sector */
        std::unordered_map<const MuonGMR4::SpectrometerSector*, HitVec_t> hitsPerSector{};
        std::ranges::for_each(hits, [&hitsPerSector](Hit_t hit) {
            hitsPerSector[hit->msSector()].push_back(hit);
        });
        /** We collect the sectors with hits in the station. We can either have 1 or 2 sectors
         *  (in the overlap regions). If we have 2 sectors, we ensure one is small and the other is large */
        std::vector<const MuonGMR4::SpectrometerSector*> sectorsInStation{};
        sectorsInStation.reserve(hitsPerSector.size());
        std::ranges::transform(hitsPerSector, std::back_inserter(sectorsInStation), 
            [](const auto& pair) { return pair.first; });
        assert(sectorsInStation.size() > 0 && sectorsInStation.size() <= 2u &&
               (sectorsInStation.size() == 1u || isSmall(sectorsInStation[0]->chamberIndex()) != isSmall(sectorsInStation[1]->chamberIndex())));
        /** Ensure the small sector is processed first to maximize the lever arm */
        if (sectorsInStation.size() == 2u && !isSmall(sectorsInStation[0]->chamberIndex())) {
            std::swap(sectorsInStation[0], sectorsInStation[1]);
        }

        std::vector<SegmentSeedPair_t> stSegments{};
        stSegments.reserve(sectorsInStation.size());
        /** Try to fit a segment using the first sector. If it fails or the segment has
         *  poor quality, try other sectors if any */
        for (const MuonGMR4::SpectrometerSector* sector : sectorsInStation) {
            ATH_MSG_VERBOSE(__func__<<"() Start segment fitting in sector " << sector->identString() 
                <<" station " << st << " with " << hitsPerSector[sector].size() << " hits.");
            HitVec_t& sectorHits {hitsPerSector[sector]};

            /** Find the parent bucket, needed for holes recovery in segment fitting. We expect one parent 
             *  bucket per sector; if we have multiple ones, take the one containing the most hits */
            std::vector<Bucket_t> bucketsInSector {};
            std::ranges::copy_if(buckets, std::back_inserter(bucketsInSector), 
                [&sector](Bucket_t bucket) { return bucket->msSector() == sector;
            });
            if (bucketsInSector.empty()) {
                throw std::runtime_error(std::format("No parent bucket found for sector {} in station {}", 
                                        sector->identString(), stName(st)));
            }
            if (bucketsInSector.size() > 1u) {
                Bucket_t primaryBucket {*std::ranges::max_element(bucketsInSector, 
                    std::ranges::less{}, [&sectorHits](const Bucket_t& b) {
                        return std::ranges::count_if(sectorHits, [&b](const Hit_t& hit) {
                            return std::ranges::any_of(*b, [&hit](const auto& h) {
                                return h.get() == hit; });
                        });
                    })};
                bucketsInSector.clear();
                bucketsInSector.push_back(primaryBucket);
            }
            Bucket_t parentBucket {bucketsInSector.back()};

            SegmentSeedOpt_t segment{fitSegment(ctx, sector->localToGlobalTransform(gctx), 
                                                parentBucket, std::move(sectorHits))};
            if (segment) {
                ATH_MSG_VERBOSE(__func__<<"() Successfully fitted segment in station "<< st 
                    <<": Pos: "<< Amg::toString(segment->second->position())
                    << ", dir: "<< Amg::toString(segment->second->direction()) 
                    << ", chi2: "<< segment->second->chi2()<<", nDoF: "<<NDoF(*segment->second)<<std::endl 
                    << print(segment->second->measurements()));
                stSegments.push_back(std::move(*segment));
                // If we found a good segment, we don't try to fit segments in other sectors of the same station.
                const Segment_t& newSegment {stSegments.back().second};
                if (calcRedChi2(*newSegment) <= m_goodSegmentCut && 
                    NDoF(*newSegment) >= m_goodSegmentDoF) {
                    break;
                }
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() No segment could be fitted. Try next sector, if any.");
        }
        if (stSegments.empty()) {
            ATH_MSG_DEBUG("No segment could be fitted in station " << st << " for " << pattern);
            continue;
        }
        SegmentSeedPair_t& bestSeg {stSegments.size() > 1 
            ? *std::ranges::max_element(stSegments, [](const SegmentSeedPair_t& s1, const SegmentSeedPair_t& s2) {
                    return betterSegment(*s2.second, *s1.second); }) 
            : stSegments.back()};
        ATH_MSG_VERBOSE(__func__<<"() Best segment in station " << st << ": Pos: " << Amg::toString(bestSeg.second->position()) 
            << ", dir: " << Amg::toString(bestSeg.second->direction()) << ", chi2: " << bestSeg.second->chi2() << ", nDoF: " << NDoF(*bestSeg.second));

        muonSegments.push_back(std::move(bestSeg));
    }
    ATH_MSG_DEBUG(__func__<<"() Found "<< muonSegments.size()<<" muon segments to construct a candidate.");
    return muonSegments;
}
MuonFastSegmentFittingAlg::SegmentSeedOpt_t
MuonFastSegmentFittingAlg::fitSegment(const EventContext& ctx,
                                      const Acts::Transform3& localToGlobal,
                                      Bucket_t parentBucket,
                                      std::vector<Hit_t>&& hits) const {
    /** Reject segment candidates with insufficient precision hits */
    std::set<unsigned> layers{};
    std::ranges::for_each(hits, [this, &layers](Hit_t hit) {
        if (isPrecisionHit(*hit)) {
            layers.insert(m_spSorter.sectorLayerNum(*hit));
        }
    });
    if (layers.size() < m_precHitCut) {
        ATH_MSG_VERBOSE(__func__<<"() Not enough layers with hits to fit a segment, skipping!");
        return std::nullopt;
    }
    Acts::CalibrationContext cctx {ActsTrk::getCalibrationContext(ctx)};

    /** Initialize initial parameters */
    auto [ValidHits, initialPars] {initializePars(localToGlobal, hits)};
    if (ValidHits.empty()) {
        ATH_MSG_VERBOSE(__func__<<"() Failed to initialize initial parameters for segment fitting.");
        return std::nullopt;
    }
    ATH_MSG_VERBOSE(__func__<<"() Start segment fitting with initial parameters: "
        <<toString(initialPars)<<", hits: "<<print(ValidHits));
    
    const auto [locPos, locDir] {makeLine(initialPars)};
    auto houghSeed {std::make_unique<SegmentSeed>(houghTanBeta(locDir), locPos.y(), 
                                                  houghTanAlpha(locDir), locPos.x(), 
                                                  ValidHits.size(), std::move(ValidHits), parentBucket)};

    /** Handle the case of NSW. */
    if (toStationIndex(parentBucket->msSector()->chamberIndex()) == StIndex::EI &&
        std::ranges::all_of(houghSeed->getHitsInMax(), [](const Hit_t& hit) { 
            return xAOD::isNSW(hit->type()); })) {
        
        ATH_MSG_VERBOSE(__func__<<"() Found NSW hits. Use the NSW fitter to fit the segment.");
        using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;
        CalibSpacePointVec calibHits{m_calibTool->calibrate(ctx, 
            houghSeed->getHitsInMax(), locPos, locDir, 0.)};
        Segment_t res {m_nswFitter->fitSegment(ctx, 
            houghSeed.get(), initialPars, localToGlobal, std::move(calibHits))};
        if (res) {
            return std::make_pair(std::move(houghSeed), std::move(res));
        }
        return std::nullopt;
    }

    /** If we have straws, use the MDT segment seeder to estimate initial parameters. Segments are 
     *  fitted into the chamber center phi plane. */
    std::vector<Segment_t> segments{};
    MdtSegmentSeeder::State_t seedState{initialPars, houghSeed.get(), m_calibTool.get(), m_recalibSeed.value()};

    ATH_MSG_VERBOSE(__func__<<"() Start segment seed search");
    while (auto seed = m_mdtSeeder->nextSeed(cctx, seedState)) {
        ATH_MSG_VERBOSE(__func__<<"() Found a seed. Try to fit the segment...");

        Segment_t segment {m_fitter->fitSegment(ctx, houghSeed.get(), seed->parameters,
            localToGlobal, std::move(seed->hits))};
        if (segment) {
            segments.push_back(std::move(segment));
        }
    }

    if (!segments.empty()) {
        ATH_MSG_VERBOSE(__func__<<"() In total "<<segments.size()<<" segment were constructed. Keep the best one.");
        if (msgLvl(MSG::VERBOSE) && segments.size() > 1) {
            for (const Segment_t& seg : segments) {
                ATH_MSG_VERBOSE(__func__<<"() Segment: Pos: "<<Amg::toString(seg->position())
                    <<", dir: "<<Amg::toString(seg->direction())<<", chi2: "<<seg->chi2()
                    <<", nDoF: "<<NDoF(*seg)<<std::endl<<print(seg->measurements()));
            }
        }
        Segment_t& bestSegment {*std::ranges::max_element(segments, [&](const Segment_t& s1, const Segment_t& s2) {
            return betterSegment(*s2, *s1);
        })};
        return std::make_pair(std::move(houghSeed), std::move(bestSegment));
    }
    ATH_MSG_VERBOSE(__func__<<"() No segment seeds could be fitted.");
    return std::nullopt;
}
std::pair<MuonFastSegmentFittingAlg::HitVec_t, Parameters>
MuonFastSegmentFittingAlg::initializePars(const Acts::Transform3& localToGlobal,
                                          const HitVec_t& hits) const { 
                                            
    auto [etaHits, etaPars] = linearRegression(CoordPlane::etaPlane, hits);
    if (!etaPars || etaHits.size() < m_precHitCut) {
        /** No enough hits or bad topology, return empty result */
        ATH_MSG_VERBOSE(__func__<<"() Failed to initialize eta parameters.");
        return std::pair<HitVec_t, Parameters>{};
    }
    const Amg::Vector3D beamspotPos {localToGlobal.inverse().translation()};
    const Beamspot beamspot{beamspotPos.x(), beamspotPos.z(), 
        beamspotCov(CoordPlane::phiPlane, localToGlobal)};
    ATH_MSG_VERBOSE(__func__<<"() Use: "<<beamspot<<" for regression in "<<CoordPlane::phiPlane);
    auto [phiHits, phiPars] = linearRegression(CoordPlane::phiPlane, hits, beamspot);
    
    if (!phiPars) {
        /** Failed to initialize phi parameters, fit the segment in the chamber phi plane */
        phiHits.clear();
        phiPars = std::make_optional(Line2D_t{0., 0.});
    }

    /** Add phi-only hits to eta hits */
    for (Hit_t hit : phiHits) {
        if (!hit->measuresEta()) {
            etaHits.push_back(hit);
        }
    }
    Parameters pars{};
    const auto [tanAlpha, x0] = *phiPars;
    const auto [tanBeta , y0] = *etaPars;
    pars[Acts::toUnderlying(ParamDefs::y0)] = y0;
    pars[Acts::toUnderlying(ParamDefs::x0)] = x0;
    const Amg::Vector3D dir {Acts::makeDirectionFromAxisTangents(tanAlpha, tanBeta)};
    pars[Acts::toUnderlying(ParamDefs::theta)] = dir.theta();
    pars[Acts::toUnderlying(ParamDefs::phi)] = dir.phi();

    return std::make_pair(std::move(etaHits), pars);
}
MuonFastSegmentFittingAlg::RegressionRes_t
MuonFastSegmentFittingAlg::linearRegression(const CoordPlane Plane,
                                            const HitVec_t& hits,
                                            const std::optional<Beamspot>& beamspot) const {
    /** TO DO: The linear regression should be improved to handle the covariance in z, not only in
     *         the x and y directions. This is needed for both straws measurements and beamspot. */
    ATH_MSG_VERBOSE(__func__<<"() Start linear regression in the "<<Plane
        <<" with "<<hits.size()<<" hits"<<(beamspot ? " & beamspot." : "."));

    const auto CovIdx {Plane == CoordPlane::etaPlane 
        ? Acts::toUnderlying(SpacePoint::CovIdx::etaCov) 
        : Acts::toUnderlying(SpacePoint::CovIdx::phiCov)};

    double S {0.}, Sy {0.}, Sz {0.}, Szz {0.}, Syz {0.};
    /** Helper function to accumulate the regression sums */
    auto accumulate = [&](const double y, const double z, const double w) {
        S += w;
        Sy += w * y;
        Sz += w * z;
        Szz += w * z * z;
        Syz += w * z * y;
    };

    HitVec_t validHits {};
    for (Hit_t hit : hits) {
        if ((Plane == CoordPlane::etaPlane && !hit->measuresEta()) ||
            (Plane == CoordPlane::phiPlane && !hit->measuresPhi())) {
            continue;
        }
        const double sigma2 {(Plane == CoordPlane::etaPlane && hit->isStraw())
            ? hit->covariance()[CovIdx] + Acts::square(hit->driftRadius())
            : hit->covariance()[CovIdx]};

        validHits.push_back(hit);
        const Amg::Vector3D& locPos {hit->localPosition()};

        accumulate(Plane == CoordPlane::etaPlane ? locPos.y() : locPos.x(), 
                   locPos.z(), 
                   1./sigma2);
    }
    if (beamspot) {
        accumulate(beamspot->coord, beamspot->z, 1./beamspot->cov_coordCoord);
    }
    /** Address the case of not enough valid hits */
    if (validHits.size() + beamspot.has_value() < 2u) {
        ATH_MSG_VERBOSE(__func__<<"() Not enough hits to do a linear regression in the "
            <<Plane<<". Valid hits: "<<print(validHits));
        return std::make_pair(std::move(validHits), std::nullopt);
    }
    /** Handle invalid determinant: require a non-zero total weight
     *  and sufficient weighted variance in z of the hits (at least 1 mm) */
    const double det {S * Szz - Acts::square(Sz)};
    if (S <= Acts::s_epsilon || 
        (det / Acts::square(S)) < minZVariance) {
        ATH_MSG_WARNING(__func__<<"() Degenerate regression in the "
            <<Plane<<": total weight: "<<S<<", variance in z: "
            <<det/Acts::square(S)<<" valid hits: "<<print(validHits));
        return std::make_pair(std::move(validHits), std::nullopt);
    }
    Line2D_t pars{};
    pars[Acts::toUnderlying(ParamDefs2D::slope)] = (S * Syz - Sz * Sy) / det;
    pars[Acts::toUnderlying(ParamDefs2D::intercept)] = (Szz * Sy - Sz * Syz) / det;
    ATH_MSG_VERBOSE(__func__<<"() Linear regression in the "
        <<Plane<<" -> "<<pars<<", with "<<validHits.size()
        <<" valid hits"<<(beamspot ? " and beamspot." : "."));
    return std::make_pair(std::move(validHits), std::move(pars));
}
const Segment* MuonFastSegmentFittingAlg::findSegmentToAddPhi(std::vector<SegmentSeedPair_t>& segs) const {
    using enum LayerIndex;

    /** Find the first segment which is on the most favoured layer to be a seed in the 
     *  muon bulding process. If there is none, continue to the next. */
    for (const LayerIndex layer : {Middle, Inner, Outer, Extended, BarrelExtended}) {

        auto it = std::ranges::find_if(segs, [&layer](const SegmentSeedPair_t& seg) {
            return toLayerIndex(seg.second->msSector()->chamberIndex()) == layer; });
        if (it != segs.end()) {
            return it->second.get();
        }
    }
    return nullptr;
}
double MuonFastSegmentFittingAlg::beamspotCov(const CoordPlane Plane, 
                                              const Acts::Transform3& localToGlobal) const {

    const Amg::Vector3D localAxisDir {Amg::Vector3D::Unit(Plane == CoordPlane::etaPlane)};
    const Amg::Vector3D globalAxisDir {localToGlobal.rotation() * localAxisDir};
    
    return globalAxisDir.dot(m_beamspotCov * globalAxisDir);
}

std::ostream& operator<<(std::ostream& os, MuonFastSegmentFittingAlg::CoordPlane plane) {
    switch (plane) {
        case MuonFastSegmentFittingAlg::CoordPlane::etaPlane:
            return os << "etaPlane";
        case MuonFastSegmentFittingAlg::CoordPlane::phiPlane:
            return os << "phiPlane";
    }
    return os;
}
std::ostream& operator<<(std::ostream& os, MuonFastSegmentFittingAlg::ParamDefs2D pars) {
    switch (pars) {
        case MuonFastSegmentFittingAlg::ParamDefs2D::intercept:
            return os << "Intercept";
        case MuonFastSegmentFittingAlg::ParamDefs2D::slope:
            return os << "Slope";
        default:
            return os;
    }
}
std::ostream& operator<<(std::ostream& os, const MuonFastSegmentFittingAlg::Line2D_t& line) {
    return os << "Line [slope, intercept]: [" 
        << line[Acts::toUnderlying(MuonFastSegmentFittingAlg::ParamDefs2D::slope)] << ", " 
        << line[Acts::toUnderlying(MuonFastSegmentFittingAlg::ParamDefs2D::intercept)] << "]";
}
std::ostream& operator<<(std::ostream& os, const MuonFastSegmentFittingAlg::Beamspot& beamspot) {
    return os << "Beamspot [z, coord, sigmaCoord]: ["
        << beamspot.z<< ", "<<beamspot.coord<< ", "<< std::sqrt(beamspot.cov_coordCoord) << "]";
}
}
