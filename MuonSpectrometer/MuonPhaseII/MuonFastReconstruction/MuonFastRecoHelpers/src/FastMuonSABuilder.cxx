/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/FastMuonSABuilder.h"

#include <MuonSpacePoint/SpacePointHelpers.h>
#include <xAODMuonPrepData/UtilFunctions.h>

#include "ActsCalibBase/CalibrationContext.h"
#include "ActsInterop/Logger.h"

namespace {
    /** @brief Calculate the reduced chi-squared  of a segment */
    double calcRedChi2(const MuonR4::Segment& segment) {
        return segment.nDoF() > 0ul ? segment.chi2() / segment.nDoF() : segment.chi2();
    }
    /** @brief Check if the new segment is better than the old segment */
    bool betterSegment(const MuonR4::Segment& newSegment, const MuonR4::Segment& oldSegment) {
        if (newSegment.nDoF() == oldSegment.nDoF()) {
            return newSegment.chi2() < oldSegment.chi2();
        }
        return newSegment.nDoF() > oldSegment.nDoF();
    }
    using namespace Muon::MuonStationIndex;
    /** @brief Count the number of NSW hits */
    unsigned countNSWHits(const std::vector<const MuonR4::SpacePoint*>& hits, 
                          const MuonR4::SpacePointBucket* parentBucket) {
        if (toStationIndex(parentBucket->msSector()->chamberIndex()) != StIndex::EI) {
            return 0;
        }
        return std::ranges::count_if(hits, [](const MuonR4::SpacePoint* hit) { 
            return xAOD::isNSW(hit->type()); });
    }
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
}

namespace MuonR4::FastReco {
using namespace Muon::MuonStationIndex;
using namespace MuonR4::SegmentFit;
using namespace Acts::UnitLiterals;

constexpr auto phiIdx {Acts::toUnderlying(ParamDefs::phi)};
constexpr auto x0Idx {Acts::toUnderlying(ParamDefs::x0)};
constexpr auto thetaIdx {Acts::toUnderlying(ParamDefs::theta)};
constexpr auto y0Idx {Acts::toUnderlying(ParamDefs::y0)};
constexpr auto etaCovIdx {Acts::toUnderlying(SpacePoint::CovIdx::etaCov)};

FastMuonSABuilder::FastMuonSABuilder(const std::string& name, Config&& config) :
    AthMessaging{name}, m_cfg{std::move(config)} {
        /** Initialize the segment line fitter */
        SegmentLineFitter::Config fitCfg{};
        fitCfg.calibrator = config.calibrator;
        fitCfg.visionTool = config.visionTool;
        fitCfg.idHelperSvc = config.idHelperSvc;
        fitCfg.fitT0 = false;
        fitCfg.calcAlongStrip = false;
        fitCfg.recalibrate = config.recalibInFit;
        fitCfg.useFastFitter = config.useFastFitter;
        fitCfg.fastPreFitter = config.fastPreFitter;
        fitCfg.ignoreFailedPreFit = config.ignoreFailedPreFit;
        fitCfg.useHessian = config.useHessianResidual;
        fitCfg.doBeamSpot = false;
        fitCfg.outlierRemovalCut = config.outlierRemovalCut;
        fitCfg.recoveryPull = config.recoveryPull;
        fitCfg.nPrecHitCut = config.precHitCut;
        fitCfg.maxIter = config.maxIter;
        fitCfg.parsToUse = {ParamDefs::y0, ParamDefs::theta};

        /** Initialize the NSW segment fitter */
        SegmentLineFitter::Config nswFitCfg{fitCfg};
        nswFitCfg.parsToUse = {ParamDefs::x0, ParamDefs::y0, ParamDefs::theta, ParamDefs::phi};

        m_fitter = std::make_unique<LineFitter>(name, std::move(fitCfg));
        m_nswFitter = std::make_unique<LineFitter>(name, std::move(nswFitCfg));

        /** Initialize the L-R segment seeder */
        MdtSegmentSeeder::Config genCfg{};
        genCfg.hitPullCut = config.seedHitChi2;
        genCfg.busyLayerLimit = 3.;
        genCfg.startWithPattern = false;
        m_mdtSeeder = std::make_unique<MdtSegmentSeeder>(std::move(genCfg), makeActsAthenaLogger(this, name));

        /** Initialize the track seeder */
        MsTrackSeeder::Config trackCfg{};
        trackCfg.nFieldSteps = config.nFieldSteps;
        m_trackSeeder = std::make_unique<MsTrackSeeder>(name, std::move(trackCfg));

    }
xAOD::Muon*
FastMuonSABuilder::buildMuonCandidate(const EventContext& ctx, 
                                      const ActsTrk::GeometryContext& gctx,
                                      const AtlasFieldCacheCondObj& magField,
                                      const GlobalPattern& pattern,
                                      MuonCont_t& outMuons) const {
    ATH_MSG_VERBOSE(__func__<<"() Start processing " << pattern);

    std::vector<StIndex> stations {pattern.getStations()};
    std::ranges::sort(stations, [](StIndex s1, StIndex s2) {
        LayerIndex l1 {toLayerIndex(s1)}, l2 {toLayerIndex(s2)};
        if (l1 == l2) {
            return isBarrel(s1);
        }
        return layerRank(l1) < layerRank(l2);
    });

    std::vector<Segment_t> muonSegments{};
    for (const StIndex st : stations) {
        /** If we already have 3 segments, we don't try to fit more segments in other stations */
        if (muonSegments.size() > 2) {
            break;
        }
        // If we already have a segment in the layer, we don't try to fit another one
        const LayerIndex layer {toLayerIndex(st)};
        if (std::ranges::any_of(muonSegments, [&layer](const Segment_t& seg) {
                return toLayerIndex(
                    seg->measurements().back()->spacePoint()->msSector()->chamberIndex()) == layer; })) {
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
        /** We collect & sort the sectors by the number of hits, so we'll process first the 
         *  sectors with more hits. */
        std::vector<const MuonGMR4::SpectrometerSector*> sectorsInStation{};
        sectorsInStation.reserve(hitsPerSector.size());
        std::ranges::transform(hitsPerSector, std::back_inserter(sectorsInStation), 
            [](const auto& pair) { return pair.first; });
        std::ranges::sort(sectorsInStation, std::ranges::greater{}, 
            [&hitsPerSector](const MuonGMR4::SpectrometerSector* s) { 
                return hitsPerSector[s].size(); });

        std::vector<Segment_t> stSegments{};
        /** Try to fit a segment using the first sector. If it fails or the segment has
         *  poor quality, try other sectors if any */
        for (const MuonGMR4::SpectrometerSector* sector : sectorsInStation) {
            ATH_MSG_VERBOSE(__func__<<"() Start segment fitting in sector " << sector->identString() 
                <<" station " << st << " with " << hitsPerSector[sector].size() << " hits.");
            const HitVec_t& sectorHits {hitsPerSector[sector]};

            /** Find the parent bucket, needed for holes recovery in segment fitting. We expect one parent 
             *  bucket per sector; if we have multiple ones, take the one containing the most hits */
            std::vector<Bucket_t> bucketsInSector {};
            std::ranges::copy_if(buckets, std::back_inserter(bucketsInSector), 
                [&sector](Bucket_t bucket) { return bucket->msSector() == sector;
            });
            if (bucketsInSector.empty()) {
                throw std::runtime_error(std::format("No parent bucket found for sector {} in station {}", sector->identString(), stName(st)));
            }
            Bucket_t parentBucket {bucketsInSector.size() < 2u ? bucketsInSector.back() 
                : *std::ranges::max_element(bucketsInSector, std::ranges::less{}, [&sectorHits](const Bucket_t& b) {
                    return std::ranges::count_if(sectorHits, [&b](const Hit_t& hit) {
                        return std::ranges::any_of(*b, [&hit](const auto& h) {
                            return h.get() == hit; });
                    });
                })};

            Segment_t segment {fitSegment(ctx, sector->localToGlobalTransform(gctx), parentBucket, std::move(hitsPerSector[sector]))};
            if (segment) {
                ATH_MSG_VERBOSE(__func__<<"() Successfully fitted segment in station "<< st <<": Pos: "
                    << Amg::toString(segment->position())<< ", dir: "<< Amg::toString(segment->direction()) 
                    << ", chi2: "<< segment->chi2()<<", nDoF: "<<segment->nDoF()<<std::endl << print(segment->measurements()));
                stSegments.push_back(std::move(segment));
                // If we found a good segment, we don't try to fit segments in other sectors of the same station.
                if (calcRedChi2(*stSegments.back()) <= m_cfg.goodSegmentCut) {
                    break;
                }
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() No segment could be fitted. Try next sector, if any.");
        }
        if (stSegments.empty()) {
            ATH_MSG_DEBUG("No segment could be fitted in station " << st << " for pattern" << pattern);
            continue;
        }
        Segment_t& bestStSegment {stSegments.size() > 1 
            ? *std::ranges::max_element(stSegments, [](const Segment_t& s1, const Segment_t& s2) {
                    return betterSegment(*s2, *s1); }) 
            : stSegments.back()};
        ATH_MSG_VERBOSE(__func__<<"() Best segment in station " << st << ": Pos: " << Amg::toString(bestStSegment->position()) 
            << ", dir: " << Amg::toString(bestStSegment->direction()) << ", chi2: " << bestStSegment->chi2() << ", nDoF: " << bestStSegment->nDoF());

        muonSegments.push_back(std::move(bestStSegment));
    }

    ATH_MSG_VERBOSE(__func__<<"() Found "<< muonSegments.size()<<" muon segments to construct a candidate.");
    if (muonSegments.size() < 2) {
        ATH_MSG_DEBUG(__func__<<"() Not enough muon segments to construct a candidate - abort.");
        return nullptr;
    }
    /* Sort station points by radial distance. Needed when we have segments in Extended layers */
    std::ranges::sort(muonSegments, std::ranges::less{}, 
        [](const Segment_t& seg) { return seg->position().perp(); });
    
    const Amg::Vector3D planeNorm {Acts::makeDirectionFromPhiTheta(pattern.phi() + 90._degree, 90._degree)};
    auto point = [&planeNorm, this](const Segment_t& seg) {
        int sector {seg->measurements().back()->spacePoint()->msSector()->sector()};
        return std::make_pair(m_trackSeeder->segPosOntoPhiPlane(planeNorm, sector, seg->position()),
                              m_trackSeeder->segDirOntoPhiPlane(planeNorm, seg->direction()));
    };
    
    assert(muonSegments.size() <= 3);
    const double qtimesP {muonSegments.size() == 3
        ? m_trackSeeder->estimateQtimesP(magField, planeNorm, point(muonSegments[0]), point(muonSegments[1]), point(muonSegments[2]))
        : m_trackSeeder->estimateQtimesP(magField, planeNorm, point(muonSegments[0]), point(muonSegments[1]))};

    const double theta {muonSegments[0]->position().theta()};
    const double eta   {-std::log(std::tan(theta/2.))};
    const double pt    {std::abs(qtimesP) * std::sin(theta)};

    xAOD::Muon* newMuon = outMuons->push_back(std::make_unique<xAOD::Muon>());
    newMuon->setAuthor(xAOD::Muon::Author::MuidSA);
    newMuon->setP4(pt, eta, pattern.phi());
    newMuon->setCharge(qtimesP > 0. ? 1 : -1);
    newMuon->setMuonType(xAOD::Muon::MuonType::MuonStandAlone);
    return newMuon;
}
FastMuonSABuilder::Segment_t 
FastMuonSABuilder::fitSegment(const EventContext& ctx,
                              const Amg::Transform3D& localToGlobal,
                              Bucket_t parentBucket,
                              std::vector<Hit_t>&& hits) const {
    /** Check that we have at least 2 measurement layers */ 
    const unsigned firstLayer {m_spSorter.sectorLayerNum(*hits.front())};
    if (hits.size() < 2 ||
        std::ranges::none_of(hits, [&](Hit_t hit) { 
            return m_spSorter.sectorLayerNum(*hit) != firstLayer; })) {
        ATH_MSG_DEBUG(__func__<<"() Not enough layers with hits to fit a segment, skipping!");
        return nullptr;
    }
    Acts::CalibrationContext cctx {ActsTrk::getCalibrationContext(ctx)};

    /** Initialize initial parameters in the centeral phi plane of the chamber */
    Parameters initialPars{};
    initialPars[phiIdx] = 90._degree;
    initialPars[x0Idx] = 0;

    /** Handle the case of NSW. In principle it can happen to have mixed hit types, NSW and e.g. 
     *  EIL hits. In such a case, we check if NSW hits represent the majority */ 
    if (const unsigned nNSWhits {countNSWHits(hits, parentBucket)}; nNSWhits > 0) {

        if (nNSWhits < hits.size()) {
            ATH_MSG_DEBUG(__func__<<"() Mixed hit types: "<<nNSWhits<<" NSW over "
                <<hits.size()<<" total hits in the segment seed.");
            
            std::vector<Hit_t> selHits{};
            const bool useNSW {nNSWhits > (hits.size() - nNSWhits)};
            std::ranges::copy_if(hits, std::back_inserter(selHits), [useNSW](Hit_t hit) { 
                return xAOD::isNSW(hit->type()) == useNSW; });
            std::swap(hits, selHits);
        }
        /* Estimate initial parameters in bending direction with a linear regression */
        auto validHits = estimateBendingPars(std::move(hits), initialPars);
        const auto [locPos, locDir] {makeLine(initialPars)};
        ATH_MSG_VERBOSE(__func__<<"() Initial parameters: "<<toString(initialPars)
            << ", hits: "<<print(validHits));

        auto houghSeed {std::make_unique<SegmentSeed>(0., 0., 0., 0., 0., std::move(validHits), parentBucket)};

        using CalibSpacePointVec = ISpacePointCalibrator::CalibSpacePointVec;
        CalibSpacePointVec calibHits{m_cfg.calibrator->calibrate(ctx, houghSeed->getHitsInMax(), locPos, locDir, 0.)};
        return m_nswFitter->fitSegment(ctx, houghSeed.get(), initialPars, localToGlobal, std::move(calibHits));
    }

    /** If we have straws, use the MDT segment seeder to estimate initial parameters. Segments are 
     *  fitted into the chamber center phi plane. */
    auto houghSeed {std::make_unique<SegmentSeed>(0., 0., 0., 0., 0., std::move(hits), parentBucket)};
    std::vector<Segment_t> segments{};
    MdtSegmentSeeder::State_t seedState{initialPars, houghSeed.get(), m_cfg.calibrator, m_cfg.recalibSeed};

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
                    <<", nDoF: "<<seg->nDoF()<<std::endl<<print(seg->measurements()));
            }
        }
        return std::move(*std::ranges::max_element(segments, [&](const Segment_t& s1, const Segment_t& s2) {
            return betterSegment(*s2, *s1);
        }));
    }
    ATH_MSG_VERBOSE(__func__<<"() No segment seeds could be fitted.");
    return nullptr;
}
FastMuonSABuilder::HitVec_t
FastMuonSABuilder::estimateBendingPars(HitVec_t&& hits,
                                       Parameters& pars) const {
    // Estimate line parameters in the bending plane using a weighted linear regression.
    double S {0.}, Sy {0.}, Sz {0.}, Szz {0.}, Syz {0.};
    HitVec_t validHits{};
    for (const auto& hit : hits) {
        const Amg::Vector3D& locPos {hit->localPosition()};
        const double z {locPos.z()};
        const double y {locPos.y()};
        const double sigma2 {hit->covariance()[etaCovIdx]};
        if (sigma2 < 1e-4) {
            ATH_MSG_WARNING(__func__<<"() Hit"<< *hit<<" with very small eta covariance: " << sigma2 << ". Skipping the measurement.");
            continue;
        }
        validHits.push_back(hit);
        const double w {1./sigma2};
        
        S += w;
        Sy += w * y;
        Sz += w * z;
        Szz += w * z * z;
        Syz += w * z * y;
    }
    const double det {S * Szz - Sz * Sz};

    if (std::abs(det) < 1e-6) {
        ATH_MSG_VERBOSE(__func__<<"() Degenerate weigthed regression, using furthest hits...");
        const auto [minZHit, maxZHit] = std::ranges::minmax_element(hits, std::ranges::less{}, 
            [](const Hit_t& h) { return h->localPosition().z(); });
        const Amg::Vector3D& minLocPos {(*minZHit)->localPosition()};
        const Amg::Vector3D& maxLocPos {(*maxZHit)->localPosition()};
        const double deltaZ {maxLocPos.z() - minLocPos.z()};
        // This should not happen since we required at least 2 different layers.
        assert(std::abs(deltaZ) > 1e-3);

        pars[thetaIdx] = std::atan2(maxLocPos.y() - minLocPos.y(), deltaZ);
        pars[y0Idx] = minLocPos.y() - std::tan(pars[thetaIdx]) * minLocPos.z();
        return validHits;
    }
    const double slope { (S * Syz - Sz * Sy) / det };
    const double intercept { (Szz * Sy - Sz * Syz) / det };

    pars[thetaIdx] = std::atan(slope);
    pars[y0Idx] = intercept;
    return validHits;
}

}