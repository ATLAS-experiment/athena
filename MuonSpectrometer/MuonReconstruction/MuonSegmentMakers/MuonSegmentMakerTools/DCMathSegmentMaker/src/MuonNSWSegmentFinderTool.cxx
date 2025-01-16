/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSWSegmentFinderTool.h"

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "MuonReadoutGeometry/MuonPadDesign.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkTrack/Track.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"

#include "CxxUtils/inline_hints.h"

namespace {
    using MuonSegmentVec = std::vector<std::unique_ptr<Muon::MuonSegment>>;
    const MuonGM::MuonChannelDesign* getDesign(const Muon::MuonClusterOnTrack* cl) {
        const Trk::TrkDetElementBase* ele = cl->detectorElement();
        if (ele->detectorType() == Trk::DetectorElemType::MM)
            return static_cast<const MuonGM::MMReadoutElement*>(ele)->getDesign(cl->identify());
        if (ele->detectorType() == Trk::DetectorElemType::sTgc)
            return static_cast<const MuonGM::sTgcReadoutElement*>(ele)->getDesign(cl->identify());
        return nullptr;
    }

    int clusterSize(const Muon::MuonClusterOnTrack* cl) {
        const Trk::PrepRawData* prd = cl->prepRawData();
        if (prd->type(Trk::PrepRawDataType::MMPrepData)) {
            return static_cast<const Muon::MMPrepData*>(prd)->stripNumbers().size();
        }
        if (prd->type(Trk::PrepRawDataType::sTgcPrepData)) {
          return static_cast<const Muon::sTgcPrepData*>(prd)->stripNumbers().size();
        }
        return -1;
    }

    std::string to_string(const Amg::Vector3D& v) {
        std::stringstream sstr{};
        sstr<<"[x,y,z]="<<Amg::toString(v, 2)<<" [theta/eta/phi]=("<<(v.theta() / Gaudi::Units::degree)<<","<<v.eta()<<","<<(v.phi()/ Gaudi::Units::degree)<<")";
        return sstr.str();
    }
    /// Coarse eta cut on the segment direction if the beam spot constraint is activated
    constexpr double minEtaNSW = 1.2;
    constexpr double maxEtaNSW = 2.8;

}  // namespace
namespace Muon {
    ///  Stereo seeds can be formed using hits from 4 independent layers by solving the following system of equations
    ///     I:  \vec{C}_{0} + \lambda \vec{e}_{0}  = \vec{X}_{\mu}
    ///    II:  \vec{C}_{1} + \alpha  \vec{e}_{1}  = \vec{X}_{\mu} + A \vec{D}_{\mu}
    ///   III:  \vec{C}_{2} + \gamma  \vec{e}_{2}  = \vec{X}_{\mu} + G \vec{D}_{\mu}
    ///    IV:  \vec{C}_{3} + \kappa  \vec{e}_{3}  = \vec{X}_{\mu} + K \vec{D}_{\mu}
    ///  where \vec{C}_{i} are the geometrical strip centres, \vec_{e}_{i} describe the orientations of each strip,
    ///         X_{0} is the seed position and \vec{D}_{\mu} points along a straight line muon. The prefactors A,G,K are the
    ///         distances in Z of each layer from the first one.
    ///  K * (III) - G * (IV)
    ///     K * \vec{C}_{2} + \gamma * K * \vec{e}_{2} - G*\vec{C}_{3} - G* \kappa  \vec{e}_{3} = (K-G) * \vec{X}_{\mu}
    ///
    ///  ---> (K-G) * (I):
    ///   (K-G) * \vec{C}_{0} + (K-G) * \lambda \vec{e}_{0} = K * \vec{C}_{2} + \gamma * K * \vec{e}_{2} - G*\vec{C}_{3} - G* \kappa  \vec{e}_{3}
    ///   (K-G) * \lambda = <K * \vec{C}_{2} - G*\vec{C}_{3} - (K-G) * \vec{C}_{0}, \vec{e}_{0}> + \gamma * K * <\vec{e}_{2},\vec{e}_{0}> - G* \kappa  <\vec{e}_{3} ,\vec{e}_{0}>
    ///  ---> Define: \vec{Y}_{0} = K * \vec{C}_{2} - G*\vec{C}_{3} - (K-G) * \vec{C}_{0}
    ///
    ///     (K-G) * \lambda = <\vec{Y}_{0}, \vec{e}_{0}> + \gamma * K * <\vec{e}_{2},\vec{e}_{0}> - G* \kappa  <\vec{e}_{3} ,\vec{e}_{0}>
    ///  --> (K-G) \vec{X}_{\mu} = (K-G) * \vec{C}_{0} + <\vec{Y}_{0}, \vec{e}_{0}> \vec{e}_{0}
    ///                             + \gamma * K * <\vec{e}_{2},\vec{e}_{0}>\vec{e}_{0}
    ///                            - G* \kappa  <\vec{e}_{3} ,\vec{e}_{0}>\vec{e}_{0}
    ///
    ///  (III) - (II) = V:
    ///     \vec{C}_{1} + \alpha \vec{e}_{1} - \vec{C}_{2} - \gamma  \vec{e}_{2} = (A-G) * \vec{D}_{\mu}
    ///  (IV) - (III) = VI:
    ///     \vec{C}_{3} + \kappa \vec{e}_{3} - \vec{C}_{2} - \gamma  \vec{e}_{2} = (K-G) * \vec{D}_{\mu}

    /// ---> (K-G) * V = (A-G) *VI
    /// ---> (K-G) *\vec{C}_{1} + (K-G) *\alpha \vec{e}_{1} - (K-G) *\vec{C}_{2} - (K-G) *\gamma  \vec{e}_{2} =
    ///         (A-G) *\vec{C}_{3} + (A-G) *\kappa \vec{e}_{3} - (A-G) *\vec{C}_{2} - (A-G) *\gamma  \vec{e}_{2}
    ///
    /// ==>  (K-G) *\alpha \vec{e}_{1} = (A-G) *\vec{C}_{3} - (K-G) *\vec{C}_{1} + (A-G) *\kappa \vec{e}_{3}
    ///                                   + (K-A) *\vec{C}_{2} + (K-A) *\gamma  \vec{e}_{2}
    ///
    /// Define: \vec{Y}_{1} = (A-G) *\vec{C}_{3} - (K-G) *\vec{C}_{1} + (K-A) *\vec{C}_{2}
    ///   (K-G) * \alpha = <\vec{Y}_{1}, \vec{e}_{1}> +
    ///                       (A-G) *\kappa <\vec{e}_{3}, \vec{e}_{1}> +
    ///                       (K-A) *\gamma * <\vec{e}_{2}, \vec{e}_{1}>

    /// (K-G) * II:
    ///    (K-G)* \vec{C}_{1} + (K-G) * \alpha \vec{e}_{1} = (K-G)*\vec{X}_{\mu} + A*(K-G)*\vec{D}_{\mu}
    ///    (K-G)* \vec{C}_{1} + (K-G) * \alpha \vec{e}_{1} = (K-G) * \vec{C}_{0} + <\vec{Y}_{0}, \vec{e}_{0}> \vec{e}_{0}
    ///                                                       + \gamma * K * <\vec{e}_{2},\vec{e}_{0}>\vec{e}_{0}
    ///                                                       - G* \kappa  <\vec{e}_{3} ,\vec{e}_{0}>\vec{e}_{0}
    ///                                                       + A*[\vec{C}_{3} + \kappa \vec{e}_{3} - \vec{C}_{2} - \gamma  \vec{e}_{2}]
    ///
    using MeasVec= NSWSeed::MeasVec;
    NSWSeed::SeedMeasurement::SeedMeasurement(const Muon::MuonClusterOnTrack* cl):
        m_cl{cl},
        m_dir{cl->detectorElement()->transform(cl->identify()).linear() * Amg::Vector3D::UnitY()} {}

    NSWSeed::NSWSeed(const MuonNSWSegmentFinderTool* parent, const std::array<SeedMeasurement, 4>& seed,
                     const std::array<double,2>& lengths) :
      m_parent{parent},
      m_pos{seed[0].pos() + lengths[0] * seed[0].dir()} {

        const Amg::Vector3D un_dir = (seed[1].pos() + lengths[1] *seed[1].dir() - m_pos);
        m_dir = un_dir.unit()*(un_dir.z() * seed[0].pos().z() > 0 ? 1 : -1);

        if (m_parent->msgLvl(MSG::VERBOSE))
            m_parent->msgStream() << MSG::VERBOSE << m_parent->printSeed(seed)<<" is a valid seed "<<to_string(m_pos) <<" pointing to "<<to_string(m_dir)<<"."<<endmsg;

        /// Insert the measurements
        for (const SeedMeasurement& cl : seed) {
            insert(cl);
            m_width = std::hypot(m_width, Amg::error(cl->localCovariance(), Trk::locX));
        }
        m_width /= std::sqrt(3);
    }
    NSWSeed::NSWSeed(const MuonNSWSegmentFinderTool* parent, const SeedMeasurement& _leftM,
                     const SeedMeasurement& _rightM) :
        m_parent{parent}, m_pos{_leftM.pos()} {
        m_dir = (_rightM.pos() - m_pos).unit();
        m_width = std::hypot(Amg::error(_leftM->localCovariance(), Trk::locX),
                             Amg::error(_rightM->localCovariance(), Trk::locX)) / std::sqrt(2);
        insert(_leftM);
        insert(_rightM);
    }
    NSWSeed::NSWSeed(const MuonNSWSegmentFinderTool* parent, const Muon::MuonSegment& seg) :
        m_parent{parent}, m_pos{seg.globalPosition()}, m_dir{seg.globalDirection()} {
        for (const Trk::MeasurementBase* meas : seg.containedMeasurements()) {
            const Muon::MuonClusterOnTrack* clus = dynamic_cast<const Muon::MuonClusterOnTrack*>(meas);
            if (!clus) continue;
            insert(clus);
            m_width = std::hypot(m_width, Amg::error(clus->localCovariance(), Trk::locX));
        }
        m_width /= std::sqrt(size());
    }
    NSWSeed::NSWSeed(const MuonNSWSegmentFinderTool* parent, const Amg::Vector3D& pos, const Amg::Vector3D& dir) :
        m_parent{parent}, m_pos{pos}, m_dir{dir}, m_size{1} {}
    int NSWSeed::channel(const SeedMeasurement& meas) const {return m_parent->channel(meas);}
    double NSWSeed::distance(const SeedMeasurement& meas) const {
        const Amg::Vector3D& A = pos();
        const Amg::Vector3D& B = dir();
        const Amg::Vector3D& C = meas.pos();
        const Amg::Vector3D& D = meas.dir();
        const double BdotD = B.dot(D);
        const double divisor = (1. - std::pow(BdotD, 2));
        const Amg::Vector3D diff = C - A;
        if (std::abs(divisor) < std::numeric_limits<double>::epsilon()) return diff.mag();
        const double beta = (diff.dot(B) - diff.dot(D) * BdotD) / divisor;
        const double delta = (diff.dot(B) * BdotD - diff.dot(D)) / divisor;
        return (beta * B - delta * D - diff).mag();
    }
    bool NSWSeed::add(SeedMeasurement meas, double max_uncert) {
        if (!size() || !meas) return false;
        if (find(meas)) return true;
        Trk::Intersection intersect = meas->associatedSurface().straightLineIntersection(pos(), dir(), false, false);
        Amg::Vector2D lpos_seed{Amg::Vector2D::Zero()};
        if (!meas->associatedSurface().globalToLocal(intersect.position, dir(), lpos_seed)) return false;
        // Dont allow seeds outside of active areas to create seeds
        // Happens rarely but is good protection to reduce wrong channel to track association
        if (!meas->associatedSurface().insideBounds(lpos_seed)) return false;

        // Dont save sTGC wires in inner Q1
        if (m_parent->isWire(meas)) {
            const sTgcPrepData* prd = static_cast<const sTgcPrepData*>(meas->prepRawData());
            if (prd->detectorElement()->isEtaZero(prd->identify(), lpos_seed)) return false;
        }

        if (m_parent->isPad(meas)) {
            const sTgcPrepData* prd = dynamic_cast<const sTgcPrepData*>(meas->prepRawData());
            if (!prd) return false;
            const MuonGM::MuonPadDesign* design = prd->detectorElement()->getPadDesign(prd->identify());
            if (!design) return false;
            Amg::Vector2D padDist = design->distanceToPad(lpos_seed, channel(meas));
            const double dist = std::hypot(padDist.x(), padDist.y());
            meas.setDistance(dist);
            const double uncertD = std::max(1., m_width);
            return (meas.distance() / uncertD < max_uncert) && insert(meas);
        }
        meas.setDistance(distance(meas));
        const double uncertD = std::max(1.,std::hypot(m_width, Amg::error(meas->localCovariance(), Trk::locX)));
        if (m_parent->msgLvl(MSG::VERBOSE)) {
            m_parent->msgStream() << MSG::VERBOSE << m_parent->print(meas) << " is separated  from "
                                  << to_string(pos())<<" + lambda " <<to_string(dir())<<" " << meas.distance()
                                  << ". covariance: " << (meas.distance() / uncertD) << endmsg;
        }
        return (meas.distance() / uncertD < max_uncert) && insert(meas);
    }
    NSWSeed::SeedOR NSWSeed::overlap(const NSWSeed& other) const {
        SeedOR res{SeedOR::Same};
        unsigned int only_th{0}, only_oth{0};
        for (size_t m = 0; m < m_measurements.size(); ++m) {
            if (m_measurements[m]== other.m_measurements[m] &&
                m_phiMeasurements[m] == other.m_phiMeasurements[m])
                continue;
            res = SeedOR::NoOverlap;
            if (!m_measurements[m]) ++only_oth;
            if (!m_phiMeasurements[m]) ++only_oth;
            if (!other.m_measurements[m]) ++only_th;
            if (!other.m_phiMeasurements[m]) ++only_th;
        }
        if (only_oth && !only_th) return SeedOR::SubSet;
        if (only_th && !only_oth) return SeedOR::SuperSet;
        return res;
    }
    MeasVec NSWSeed::measurements() const {
        MeasVec meas;
        meas.reserve(size());
        for (size_t m = 0; m < m_measurements.size(); ++m) {
            if (m_measurements[m]) meas.push_back(m_measurements[m]);
            if (m_phiMeasurements[m]) meas.push_back(m_phiMeasurements[m]);
            if (m_padMeasurements[m]) meas.push_back(m_padMeasurements[m]);
        }
        std::sort(meas.begin(), meas.end(), [](const SeedMeasurement& a, const SeedMeasurement& b) {
            return std::abs(a->globalPosition().z()) < std::abs(b->globalPosition().z());
        });
        return meas;
    }
    bool NSWSeed::insert(const Muon::MuonClusterOnTrack* cl) {
        SeedMeasurement meas{cl};
        meas.setDistance(distance(meas));
        return insert(std::move(meas));
    }
    bool NSWSeed::insert(SeedMeasurement meas) {
        SeedMeasCache& seed_vec = m_parent->idHelper()->measuresPhi(meas->identify()) ?
                                 (m_parent->isPad(meas) ? m_padMeasurements : m_phiMeasurements ): m_measurements;
        SeedMeasurement& seed = seed_vec[m_parent->layerNumber(meas)];
        if (!seed || meas.distance() < seed.distance()) {
            m_size += !seed;
            m_chi2 += meas.distance() / Amg::error(meas->localCovariance(), Trk::locX);
            // From this point the measurement is now the old one
            std::swap(seed, meas);
            if (meas)  m_chi2 -= meas.distance() / Amg::error(meas->localCovariance(), Trk::locX);
            return true;
        }
        return false;
    }
    const Muon::MuonClusterOnTrack* NSWSeed::newCalibClust(std::unique_ptr<const Muon::MuonClusterOnTrack> new_clust) {
        return m_calibClust.emplace(std::move(new_clust)).first->get();
    }
    bool NSWSeed::find(const SeedMeasurement& meas) const {
        const int lay = m_parent->layerNumber(meas);
        if (m_parent->isPad(meas)) return m_padMeasurements[lay] == meas;
        if (m_parent->idHelper()->measuresPhi(meas->identify())) return m_phiMeasurements[lay] == meas;
        return m_measurements[lay] == meas;
    }

    //============================================================================
    MuonNSWSegmentFinderTool::MuonNSWSegmentFinderTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonNSWSegmentFinderTool>(this);
    }

    //============================================================================
    StatusCode MuonNSWSegmentFinderTool::initialize() {
        ATH_CHECK(m_slTrackFitter.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_ambiTool.retrieve());
        ATH_CHECK(m_trackToSegmentTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackCleaner.retrieve());
        ATH_CHECK(m_trackSummary.retrieve());
        ATH_CHECK(m_muonClusterCreator.retrieve());
        if (m_ipConstraint && m_caloConstraint) {
            ATH_MSG_FATAL("The muon should come from the IP && horizontally from the Calo..."<<
            "We need to place a very good basball player at the calo exit to deflect the muon horizontally.");
            return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG(" Max cut " << m_maxClustDist);
        return StatusCode::SUCCESS;
    }

    const IMuonIdHelperSvc* MuonNSWSegmentFinderTool::idHelper() const { return m_idHelperSvc.get(); }
    //============================================================================
    void MuonNSWSegmentFinderTool::find(const EventContext& ctx, SegmentMakingCache& cache) const {

        std::vector<const Muon::MuonClusterOnTrack*> muonClusters{};
        muonClusters.reserve(cache.inputClust.size());
        std::transform(cache.inputClust.begin(), cache.inputClust.end(), std::back_inserter(muonClusters),
                        [](const std::unique_ptr<const MuonClusterOnTrack>& cl){return cl.get();});
        ATH_MSG_DEBUG("Entering MuonNSWSegmentFinderTool with " << muonClusters.size() << " clusters to be fit");

        MuonSegmentVec out_segments{};
        {
            MuonSegmentVec stereoSegs = findStereoSegments(ctx, muonClusters, 0);
            ATH_MSG_VERBOSE("Found " << stereoSegs.size() << " MMG stereo seeded segments");
            out_segments.insert(out_segments.end(), std::make_move_iterator(stereoSegs.begin()),
                                                    std::make_move_iterator(stereoSegs.end()));
        }

        auto dump_output = [&]() {
            cache.constructedSegs.reserve(cache.constructedSegs.size() + out_segments.size());
            for (std::unique_ptr<Muon::MuonSegment>& seg : out_segments) {
                for (const Trk::MeasurementBase* meas : seg->containedMeasurements()) {
                     const Muon::MuonClusterOnTrack* clus = dynamic_cast<const Muon::MuonClusterOnTrack*>(meas);
                     if (clus) cache.usedHits.insert(clus->identify());
                }
                cache.constructedSegs.push_back(std::move(seg));
            }
        };

        std::vector<const Muon::MuonClusterOnTrack*> clustPostStereo{};
        /// Remove all stereo segment hits from further processing
        std::array<std::set<Identifier>, 16> masked_segs{};
        for (std::unique_ptr<Muon::MuonSegment>& seg : out_segments) {
            for (const Trk::MeasurementBase* meas : seg->containedMeasurements()) {
                const Muon::MuonClusterOnTrack* clus = dynamic_cast<const Muon::MuonClusterOnTrack*>(meas);
                if (clus) masked_segs[layerNumber(clus)].insert(clus->identify());
            }
        }

        if (!out_segments.empty()) {
            clustPostStereo.reserve(muonClusters.size());
            for (const Muon::MuonClusterOnTrack* clus : muonClusters) {
                if (!masked_segs[layerNumber(clus)].count(clus->identify())) clustPostStereo.push_back(clus);
            }
        }
        const std::vector<const Muon::MuonClusterOnTrack*>& segmentInput = !out_segments.empty() ? clustPostStereo : muonClusters;

        /// All segments
        {
            MuonSegmentVec etaSegs = findStgcPrecisionSegments(ctx, segmentInput);
            ATH_MSG_VERBOSE("Found " << etaSegs.size() << " stgc seeded eta segments");
            MuonSegmentVec precSegs = find3DSegments(ctx, segmentInput, etaSegs);
            ATH_MSG_VERBOSE("Found " << precSegs.size() << " 3D segments");
            out_segments.insert(out_segments.end(),
                                std::make_move_iterator(precSegs.begin()),
                                std::make_move_iterator(precSegs.end()));
        }



        if (!cache.buildQuads) {
            dump_output();
            return;
        }
        /// Single wedge segments. Important for the alignment runs
        for (std::unique_ptr<Muon::MuonSegment>& seg : out_segments) {
            std::vector<const MuonClusterOnTrack*> seg_hits{};
            seg_hits.reserve(seg->containedMeasurements().size());
            for (const Trk::MeasurementBase* meas : seg->containedMeasurements()) {
                const Muon::MuonClusterOnTrack* clus = dynamic_cast<const Muon::MuonClusterOnTrack*>(meas);
                if (clus) seg_hits.push_back(clus);
            }
            /// Loop over the 4 quads to create the segment container
            for (int iWedge{1}; iWedge<=4 ; ++iWedge) {
                MeasVec quad_hits = cleanClusters(seg_hits, HitType::Eta | HitType::Phi, iWedge);
                /// The micromegas need 4 hits on track
                if ( quad_hits.size () < 2 || ((iWedge == 2 || iWedge == 3) && quad_hits.size() < 4)) continue;
                std::vector<const Trk::MeasurementBase*> fit_meas{};
                std::unique_ptr<Trk::PseudoMeasurementOnTrack> pseudoVtx{ipConstraint(ctx)};
                if (pseudoVtx) fit_meas.push_back(pseudoVtx.get());
                std::copy(quad_hits.begin(), quad_hits.end(), std::back_inserter(fit_meas));
                /// Create the perigee parameter
                const Trk::Surface& surf = quad_hits.front()->associatedSurface();
                Trk::Intersection intersect = surf.straightLineIntersection(seg->globalPosition(), seg->globalDirection(), false, false);
                const Amg::Vector3D& gpos_seg = intersect.position;
                Amg::Vector3D gdir_seg{seg->globalDirection()};
                Amg::Vector3D perpos = gpos_seg - 10 * gdir_seg.unit();
                if (perpos.dot(gdir_seg) < 0) gdir_seg *= -1;
                Trk::Perigee startpar{perpos, gdir_seg, 0, perpos};
                /// Create the segment
                std::unique_ptr<Trk::Track> segtrack = fit(ctx, fit_meas, startpar);
                if (!segtrack) continue;

                std::unique_ptr<MuonSegment> seg{m_trackToSegmentTool->convert(ctx, *segtrack)};
                if (seg) {
                    ATH_MSG_VERBOSE(" adding new quad segment " << m_printer->print(*seg) << std::endl
                                            <<"position: "<<to_string(seg->globalPosition())<< std::endl
                                            <<"direction: "<<to_string(seg->globalDirection())<< std::endl
                                            << m_printer->print(seg->containedMeasurements()));
                    seg->setAuthor(Trk::Segment::NswQuadAlign);
                    cache.quadSegs.emplace_back(std::move(seg));
                }
            }
        }
        dump_output();
    }
    MuonSegmentVec MuonNSWSegmentFinderTool::findStereoSegments(const EventContext& ctx,
                                                                const std::vector<const Muon::MuonClusterOnTrack*>& allClusts,
                                                                int singleWedge) const {

       if (!m_useStereoSeeding){
          ATH_MSG_VERBOSE("MMStereoSeeding disabled!"); 
          return {};
       }

       ATH_MSG_VERBOSE("Running MMStereoSeeding"); 
        /// Order any parsed hit into the layer structure
        LayerMeasVec orderedClust =
            classifyByLayer(cleanClusters(allClusts, HitType::Eta | HitType::Phi, singleWedge), HitType::Wire | HitType::Pad);

        for (MeasVec& hitsInLayer : orderedClust) hitsInLayer = vetoBursts(std::move(hitsInLayer));
        orderedClust.erase(std::remove_if(orderedClust.begin(), orderedClust.end(),
                                             [](const MeasVec& vec) { return vec.empty(); }),
                              orderedClust.end());
        if (orderedClust.empty()) return {};


        std::vector<NSWSeed> seeds = segmentSeedFromMM(orderedClust);
        ATH_MSG_VERBOSE("Retrieved " << seeds.size() << " seeds in the MMStereoAlg after the ambiguity resolution" );

        if (seeds.empty()) return {};
        TrackCollection trackSegs{SG::OWN_ELEMENTS};
        /// Loop over the seeds
        for (NSWSeed& seed : seeds) {
            /// Require that the seed has at least one extra hit, if we're not
            /// restricting ourselves to a single wedge
            std::vector<const Trk::MeasurementBase*> fit_meas{};

            std::unique_ptr<Trk::PseudoMeasurementOnTrack> pseudoVtx{ipConstraint(ctx)};
            if (pseudoVtx) fit_meas.push_back(pseudoVtx.get());
            MeasVec calib_clust = getCalibratedClusters(seed);
            std::copy(calib_clust.begin(), calib_clust.end(), std::back_inserter(fit_meas));
            /// Craete the perigee parameter
            const Trk::Surface& surf = calib_clust.front()->associatedSurface();

            Trk::Intersection intersect = surf.straightLineIntersection(seed.pos(), seed.dir(), false, false);
            const Amg::Vector3D& gpos_seg = intersect.position;
            Amg::Vector3D gdir_seg{seed.dir()};
            Amg::Vector3D perpos = gpos_seg - 10 * gdir_seg.unit();
            if (perpos.dot(gdir_seg) < 0) gdir_seg *= -1;

            Trk::Perigee startpar{perpos, gdir_seg, 0, perpos};

            /// Create the segment
            std::unique_ptr<Trk::Track> segtrack = fit(ctx, fit_meas, startpar);
            if (segtrack) trackSegs.push_back(std::move(segtrack));
        }
        return resolveAmbiguities(ctx, trackSegs, Trk::Segment::Author::NswStereoSeeded);
    }

    std::unique_ptr<Trk::Track> MuonNSWSegmentFinderTool::fit(const EventContext& ctx,
                                                              const std::vector<const Trk::MeasurementBase*>& fit_meas,
                                                              const Trk::TrackParameters& perigee) const {
        ATH_MSG_VERBOSE("Fit segment from (" << to_string(perigee.position())<< "  pointing to " <<
                                            to_string(perigee.momentum())<<". Contained measurements in candidate: " << std::endl
                                            << m_printer->print(fit_meas));
        std::unique_ptr<Trk::Track> segtrack = m_slTrackFitter->fit(ctx, fit_meas, perigee, false, Trk::nonInteracting);
        if (!segtrack) {
            ATH_MSG_VERBOSE("Fit failed");
            return nullptr;
        }
        ATH_MSG_VERBOSE("--> Fit succeeded");
        std::unique_ptr<Trk::Track> cleanedTrack = m_trackCleaner->clean(*segtrack, ctx);
        if (cleanedTrack && cleanedTrack->perigeeParameters() != segtrack->perigeeParameters()) { segtrack.swap(cleanedTrack); }
        // quality criteria
        if (!m_edmHelperSvc->goodTrack(*segtrack, 10)) {
            if (segtrack->fitQuality()) {
                ATH_MSG_DEBUG("Segment fit with chi^2/nDoF = " << segtrack->fitQuality()->chiSquared() << "/"
                                                               << segtrack->fitQuality()->numberDoF());
            }
            return nullptr;
        }
        // update the track summary and add the track to the collection
        m_trackSummary->updateTrack(ctx, *segtrack);
        ATH_MSG_VERBOSE("Segment accepted with chi^2/nDoF = " << segtrack->fitQuality()->chiSquared() << "/"
                                                               << segtrack->fitQuality()->numberDoF());
        return segtrack;
    }

    //============================================================================
    // find the precision (eta) segments
    MuonSegmentVec MuonNSWSegmentFinderTool::findStgcPrecisionSegments(const EventContext& ctx,
                                                                       const std::vector<const Muon::MuonClusterOnTrack*>& muonClusters,
                                                                       int singleWedge) const {

       if (!m_usesTGCSeeding){
          ATH_MSG_VERBOSE("2D sTGC seeding disabled!"); 
          return {};
       }

       ATH_MSG_VERBOSE("Running 2D sTGC seeding"); 


        // clean the muon clusters; select only the eta hits.
        // in single-wedge mode the eta seeds are retrieved from the specific wedge
        
        ATH_MSG_DEBUG("Cleaning eta clusters in stgc seeded 2D segments ");
        MeasVec clusters = cleanClusters(muonClusters, HitType::Eta, singleWedge);  // eta hits only
        ATH_MSG_VERBOSE("  After hit cleaning, there are " << clusters.size() << " precision 2D clusters");

        // classify eta clusters by layer
        LayerMeasVec orderedClusters = classifyByLayer(clusters, 0);

        if (orderedClusters.size() < 4) return {};  // at least four layers with eta hits (MM and sTGC)

        // create segment seeds
        std::vector<NSWSeed> seeds = segmentSeedFromStgc(orderedClusters, false);
        ATH_MSG_DEBUG("  Found " << seeds.size() << " 2D seeds");
        // Loop on seeds: find all clusters near the seed and try to fit
        MeasVec etaHitVec, phiHitVec;
        TrackCollection segTrkColl{SG::OWN_ELEMENTS};

        for (NSWSeed& seed : seeds) {
            if (seed.size() < 4){
               ATH_MSG_VERBOSE(__func__<<" :"<<__LINE__<< " - Seed size: " << seed.size() << " is below the cut (4)");
               continue;
            }

            etaHitVec = seed.measurements();
            const Trk::PlaneSurface& surf = static_cast<const Trk::PlaneSurface&>(etaHitVec.front()->associatedSurface());
            // calculate start parameters for the fit
            // local position and direction of the eta-seed on the surface of the first cluster
            Trk::Intersection intersect = surf.straightLineIntersection(seed.pos(), seed.dir(), false, false);
            Amg::Vector2D lpos_seed{Amg::Vector2D::Zero()};
            Trk::LocalDirection ldir_seed{};
            surf.globalToLocal(intersect.position, intersect.position, lpos_seed);
            surf.globalToLocalDirection(seed.dir(), ldir_seed);

            // use the seed info to generate start parameters (dummy values for phi)
            Amg::Vector2D lpos(lpos_seed[Trk::locX], 0.);
            Trk::LocalDirection ldir(ldir_seed.angleXZ(), -M_PI_2);
            Amg::Vector3D gpos_seg{Amg::Vector3D::Zero()}, gdir_seg{Amg::Vector3D::Zero()};
            surf.localToGlobal(lpos, gpos_seg, gpos_seg);
            surf.localToGlobalDirection(ldir, gdir_seg);

            Amg::Vector3D perpos = gpos_seg - 10 * gdir_seg.unit();
            if (perpos.dot(gdir_seg) < 0) gdir_seg *= -1;
            const auto startpar = Trk::Perigee(perpos, gdir_seg, 0, perpos);
            ATH_MSG_VERBOSE(" start parameter " << perpos << " pp " << startpar.position() << " gd " << gdir_seg.unit() << " pp "
                                                << startpar.momentum().unit());

            // fit the hits
            hitsToTrack(ctx, etaHitVec, phiHitVec, startpar, segTrkColl);
        }
        /// Resolve the ambiguities amongsty the tracks and convert the result
        return resolveAmbiguities(ctx, segTrkColl, Trk::Segment::Author::NswStgcSeeded);
    }
    MuonSegmentVec MuonNSWSegmentFinderTool::resolveAmbiguities(const EventContext& ctx,
                                                                const TrackCollection& segTrkColl,
                                                                const Trk::Segment::Author a) const {
        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG("Tracks before ambi solving: ");
            for (const Trk::Track* trk : segTrkColl) {
                ATH_MSG_DEBUG(m_printer->print(*trk));
                const DataVector<const Trk::MeasurementBase>* meas = trk->measurementsOnTrack();
                if (meas) ATH_MSG_DEBUG(m_printer->print(meas->stdcont()));
            }
        }

        MuonSegmentVec segments{};
        std::unique_ptr<const TrackCollection> resolvedTracks(m_ambiTool->process(&segTrkColl));
        ATH_MSG_DEBUG("Resolved track candidates: old size " << segTrkColl.size() << " new size " << resolvedTracks->size());

        // store the resolved segments
        for (const Trk::Track* trk : *resolvedTracks) {
            const auto* measurements = trk->measurementsOnTrack();
            const bool has_eta = std::find_if(measurements->begin(), measurements->end(),
                                              [this](const Trk::MeasurementBase* meas) {
                                                  Identifier id = m_edmHelperSvc->getIdentifier(*meas);
                                                  return id.is_valid() && !m_idHelperSvc->measuresPhi(id);
                                              }) != trk->measurementsOnTrack()->end();
            if (!has_eta) continue;
            std::unique_ptr<MuonSegment> seg{m_trackToSegmentTool->convert(ctx, *trk)};
            if (seg) {
                ATH_MSG_DEBUG(" adding " << m_printer->print(*seg) << std::endl << m_printer->print(seg->containedMeasurements()));
                seg->setAuthor(a);
                segments.emplace_back(std::move(seg));
            } else {
                ATH_MSG_VERBOSE("Segment conversion failed, no segment created. ");
            }
        }
        return segments;
    }
    //============================================================================
    MuonSegmentVec MuonNSWSegmentFinderTool::find3DSegments(const EventContext& ctx,
                                                            const std::vector<const Muon::MuonClusterOnTrack*>& muonClusters,
                                                            MuonSegmentVec& etaSegs,
                                                            int singleWedge) const {
        MuonSegmentVec segments{};
        // cluster cleaning #1; select only phi hits (must be from all wedges, in order to phi-seed)
        
        ATH_MSG_DEBUG("Cleaning phi clusters in stgc seeded 3D segments ");
        MeasVec phiClusters = cleanClusters(muonClusters, HitType::Phi | HitType::Wire, singleWedge);
        ATH_MSG_DEBUG("After hit cleaning, there are " << phiClusters.size() << " phi clusters to be fit");


        // classify the phi clusters by layer
        LayerMeasVec orderedWireClusters = classifyByLayer(phiClusters, HitType::Wire);
        LayerMeasVec orderedPadClusters = classifyByLayer(phiClusters, HitType::Pad);  // pads only
        if (orderedWireClusters.size() + orderedPadClusters.size() < 2) {
            ATH_MSG_DEBUG("Not enough phi hits present, cannot perform the 3D fit!");
            segments.insert(segments.end(), std::make_move_iterator(etaSegs.begin()), std::make_move_iterator(etaSegs.end()));
            return segments;
        }

        // cluster cleaning #2; select only eta hits
        ATH_MSG_DEBUG("Cleaning eta clusters in stgc seeded 3D segments ");
        MeasVec etaClusters = cleanClusters(muonClusters, HitType::Eta, singleWedge);
        LayerMeasVec orderedEtaClusters = classifyByLayer(etaClusters, HitType::Eta);

        // loop on eta segments
        bool triedWireSeed{false};  // wire seeds need to be retrieved only once (the first time they are needed)
        std::vector<NSWSeed> seeds_WiresSTGC;
        TrackCollection segTrkColl{SG::OWN_ELEMENTS};
        // Loop on eta segments
        for (std::unique_ptr<Muon::MuonSegment>& etaSeg : etaSegs) {
            bool is3Dseg{false};
            NSWSeed seed2D{this, *etaSeg};
            getClustersOnSegment(orderedEtaClusters, seed2D, {});  // eta clusters

            std::vector<NSWSeed> seeds;
            /// 1 - All micromega layers have ideally been consumed. Try the seeding from
            ///     the phi wires.
            if (std::abs(etaSeg->globalPosition().eta()) < 2.4) {
                if (!triedWireSeed) {
                    // wire seeds need to be retrieved only once (they don't depend on the eta segment)
                    triedWireSeed = true;
                    seeds_WiresSTGC = segmentSeedFromStgc(orderedWireClusters, true);
                }

                if (!seeds_WiresSTGC.empty()) {
                    seeds = seeds_WiresSTGC;
                    ATH_MSG_DEBUG(" Seeding from sTGC wires");
                }
            }

            // 3 - last resort, try sTGC pads
            if (seeds.empty()) {
                seeds = segmentSeedFromPads(orderedPadClusters, *etaSeg);
                ATH_MSG_DEBUG(" Seeding from sTGC pads");
            }

            // Loop on phi seeds
            MeasVec phiHitVec;
            const Trk::PlaneSurface& etaSegSurf = etaSeg->associatedSurface();
            double etaSegLocX = etaSeg->localParameters()[Trk::locX];
            double etaSegLocXZ = etaSeg->localDirection().angleXZ();

            for (NSWSeed& seed : seeds) {
                // calculate start parameters for the fit
                // combine the local position and direction of the eta-seed (segment)
                // and local position and direction of the phi-seed to generate 3D starting parameters
                Trk::Intersection intersect = etaSegSurf.straightLineIntersection(seed.pos(), seed.dir(), false, false);
                Amg::Vector2D lpos_seed{Amg::Vector2D::Zero()};
                Trk::LocalDirection ldir_seed{};
                etaSegSurf.globalToLocal(intersect.position, intersect.position, lpos_seed);
                etaSegSurf.globalToLocalDirection(seed.dir(), ldir_seed);

                Amg::Vector2D lpos_seg(etaSegLocX, lpos_seed[Trk::locY]);
                Trk::LocalDirection ldir_seg(etaSegLocXZ, ldir_seed.angleYZ());

                Amg::Vector3D gpos_seg{Amg::Vector3D::Zero()}, gdir_seg{Amg::Vector3D::Zero()};
                etaSegSurf.localToGlobal(lpos_seg, gpos_seg, gpos_seg);
                etaSegSurf.localToGlobalDirection(ldir_seg, gdir_seg);

                Amg::Vector3D perpos = gpos_seg - 10 * gdir_seg.unit();
                if (perpos.dot(gdir_seg) < 0) gdir_seg *= -1;
                const Trk::Perigee startpar{perpos, gdir_seg, 0, perpos};

                NSWSeed seed3D{this, perpos, gdir_seg};

                // gather phi hits aligned with the segment
                int nPhiHits = getClustersOnSegment(orderedPadClusters, seed3D,{});  // add pad hits  (from the requested wedge if any)
                nPhiHits += getClustersOnSegment(orderedWireClusters, seed3D, {});    // add wire hits (from the requested wedge if any)
                if (nPhiHits < 2) continue;                                       // at least two phi hits

                MeasVec phiHitVec = seed3D.measurements();
                // calibrate the eta hits
                MeasVec etaHitsCalibrated = getCalibratedClusters(seed2D);

                // fit
                if (hitsToTrack(ctx, etaHitsCalibrated, phiHitVec, startpar, segTrkColl)) {
                    is3Dseg = true;
                    ATH_MSG_VERBOSE("Segment successfully fitted for wedge "<<singleWedge<<std::endl<<
                                   m_printer->print(*segTrkColl.back()));
                }
            }  // end loop on phi seeds

            // if we failed to combine the eta segment with phi measurements,
            // just add the eta segment to the collection.
            if (!is3Dseg) { segments.push_back(std::move(etaSeg)); }
        }  // end loop on precision plane segments
        MuonSegmentVec new_segs = resolveAmbiguities(ctx, segTrkColl, Trk::Segment::NswStgcSeeded);
        segments.insert(segments.end(), std::make_move_iterator(new_segs.begin()), std::make_move_iterator(new_segs.end()));
        return segments;
    }

    std::unique_ptr<Trk::PseudoMeasurementOnTrack> MuonNSWSegmentFinderTool::caloConstraint(const Trk::TrackParameters& startpar) const {
         if (!m_caloConstraint) return nullptr;
         constexpr double errVtx{1.*Gaudi::Units::m};
         Amg::MatrixX covVtx(2,2);
         covVtx.setIdentity();
         covVtx = errVtx * errVtx * covVtx;
         const Amg::Vector3D parPos{startpar.position()};
         Amg::Vector3D projection{parPos.x(), parPos.y(), 0};
        //  project
        Trk::PerigeeSurface perVtx(projection);
        return std::make_unique<Trk::PseudoMeasurementOnTrack>(Trk::LocalParameters(Trk::DefinedParameter(0, Trk::locX),
                                                                                    Trk::DefinedParameter(1, Trk::locY)), 
                                                               std::move(covVtx),
                                                               std::move(perVtx));

    }

    std::unique_ptr<Trk::PseudoMeasurementOnTrack> MuonNSWSegmentFinderTool::ipConstraint(const EventContext& /*ctx*/) const {
        if (!m_ipConstraint) return nullptr;
        constexpr double errVtx{10.*Gaudi::Units::cm};
        Amg::MatrixX covVtx(1, 1);
        covVtx(0, 0) = errVtx * errVtx;
        /// Beamspot constraint?
        Trk::PerigeeSurface perVtx(Amg::Vector3D::Zero());
        return std::make_unique<Trk::PseudoMeasurementOnTrack>(Trk::LocalParameters(Trk::DefinedParameter(0, Trk::locX)), std::move(covVtx),
                                                               std::move(perVtx));
    }
    bool MuonNSWSegmentFinderTool::isPad(const Muon::MuonClusterOnTrack* clust) const {
        const Identifier id = clust->identify();
        return m_idHelperSvc->issTgc(id) && m_idHelperSvc->stgcIdHelper().channelType(id) == sTgcIdHelper::Pad;
    }
    bool MuonNSWSegmentFinderTool::isStrip(const Muon::MuonClusterOnTrack* clust) const {
        const Identifier id = clust->identify();
        return m_idHelperSvc->issTgc(id) && m_idHelperSvc->stgcIdHelper().channelType(id) == sTgcIdHelper::Strip;
    }
    bool MuonNSWSegmentFinderTool::isWire(const Muon::MuonClusterOnTrack* clust) const {
        const Identifier id = clust->identify();
        return m_idHelperSvc->issTgc(id) && m_idHelperSvc->stgcIdHelper().channelType(id) == sTgcIdHelper::Wire;
    }
    //============================================================================
    bool MuonNSWSegmentFinderTool::hitsToTrack(const EventContext& ctx,
                                               const MeasVec& etaHitVec,
                                               const MeasVec& phiHitVec,
                                               const Trk::TrackParameters& startpar,
                                               TrackCollection& segTrkColl) const {
        // vector of hits for the fit
        std::vector<const Trk::MeasurementBase*> vecFitPts;
        unsigned int nHitsEta = etaHitVec.size();
        unsigned int nHitsPhi = phiHitVec.size();
        vecFitPts.reserve(nHitsEta + nHitsPhi + 2 + m_ipConstraint + m_caloConstraint);

        std::unique_ptr<Trk::PseudoMeasurementOnTrack> pseudoVtx{ipConstraint(ctx)}, 
                                                       pseudoVtxCalo{caloConstraint(startpar)},
                                                       pseudoPhi1{nullptr}, pseudoPhi2{nullptr};
        // is chosen, add a pseudo measurement as vtx at the center of ATLAS
        if (pseudoVtx) { vecFitPts.push_back(pseudoVtx.get()); }
        if (pseudoVtxCalo) { vecFitPts.push_back(pseudoVtxCalo.get()); }

        if (!nHitsPhi) {
            // generate two pseudo phi measurements for the fit,
            // one on the first hit surface and one on the last hit surface.

            Amg::MatrixX cov(1, 1);
            constexpr double pseudoPrecision = 100 * Gaudi::Units::micrometer;
            cov(0, 0) = pseudoPrecision; 
            static const Trk::LocalParameters loc_pseudopars{Trk::DefinedParameter(0, Trk::locY)};
            pseudoPhi1 = std::make_unique<Trk::PseudoMeasurementOnTrack>(Trk::LocalParameters(loc_pseudopars),
                                                                         Amg::MatrixX(cov),
                                                                         etaHitVec.front()->associatedSurface());
            pseudoPhi2 = std::make_unique<Trk::PseudoMeasurementOnTrack>(Trk::LocalParameters(loc_pseudopars),
                                                                         Amg::MatrixX(cov),
                                                                         etaHitVec.back()->associatedSurface());

            // add the first pseudo phi hit, the hit vector, and the second pseudo phi hit
            vecFitPts.push_back(pseudoPhi1.get());
            std::copy(etaHitVec.begin(), etaHitVec.end(), std::back_inserter(vecFitPts));
            vecFitPts.push_back(pseudoPhi2.get());
            ATH_MSG_VERBOSE("Fitting a 2D-segment track with " << nHitsEta << " Eta hits");

        } else {
            // sorted eta and sorted phi hits combined (sorted by their the z-coordinate)
            std::merge(phiHitVec.begin(), phiHitVec.end(), etaHitVec.begin(), etaHitVec.end(), std::back_inserter(vecFitPts),
                       [](const Muon::MuonClusterOnTrack* c1, const Muon::MuonClusterOnTrack* c2) {
                           double z1 = std::abs(c1->detectorElement()->center(c1->identify()).z());
                           double z2 = std::abs(c2->detectorElement()->center(c2->identify()).z());
                           return z1 < z2;
                       });
            ATH_MSG_VERBOSE("Fitting a 3D-segment track with " << nHitsEta << " Eta hits and " << nHitsPhi << " Phi hits");
        }

        // fit the hits and generate the Trk::Track
        std::unique_ptr<Trk::Track> segtrack = fit(ctx, vecFitPts, startpar);
        if (!segtrack) return false;
        segTrkColl.push_back(std::move(segtrack));
        return true;
    }

    //============================================================================
    MeasVec MuonNSWSegmentFinderTool::cleanClusters(
        const std::vector<const Muon::MuonClusterOnTrack*>& muonClusters, int hit_sel, int singleWedge /*= 0*/) const {
        // Keep only eta (MM && sTGC) or phi (sTGC) clusters
        // In single-wedge mode keep only clusters from the requested wedge
        MeasVec clusters;
        clusters.reserve(muonClusters.size());
        for (const Muon::MuonClusterOnTrack* cluster : muonClusters) {
            if (!cluster) continue;
            if (singleWedge && singleWedge != wedgeNumber(cluster)) continue;
            const Identifier id = cluster->identify();
            if (((hit_sel & HitType::Eta) && !m_idHelperSvc->measuresPhi(id)) ||
                ((hit_sel & HitType::Phi) && m_idHelperSvc->measuresPhi(id)))
                clusters.emplace_back(cluster);
        }

        ATH_MSG_VERBOSE(" After hit cleaning, there are " << clusters.size() );
        return clusters;
    }

    //============================================================================
    MuonNSWSegmentFinderTool::LayerMeasVec MuonNSWSegmentFinderTool::classifyByLayer(
        const MeasVec& clusters, int hit_sel) const {
        // Classifies clusters by layer, starting from the layer closest to the IP and moving outwards.
        // "clusters" is expected to contain only eta (MM+sTGC strip) or only phi hits (sTGC pads XOR wires).
        // The returned vector contains only layers that have hits.

        LayerMeasVec orderedClusters(16);
        std::array<std::set<Identifier>,16> used_hits{};
        int nBad{0};
        for (const Muon::MuonClusterOnTrack* hit : clusters) {
            const int iorder = layerNumber(hit);
            if (iorder < 0) {
                ++nBad;
                continue;
            }
            const Identifier id = hit->identify();
            if (m_idHelperSvc->issTgc(id)) {
                const int channelType = m_idHelperSvc->stgcIdHelper().channelType(id);
                // skip sTGC pads if using wires, or skip wires if using pads
                if (!(hit_sel & HitType::Pad) && channelType == sTgcIdHelper::Pad) continue;
                if (!(hit_sel & HitType::Wire) && channelType == sTgcIdHelper::Wire) continue;
            }
            std::set<Identifier>& lay_hits = used_hits[iorder];
            if (lay_hits.count(id)) continue;
            lay_hits.insert(id);
            orderedClusters[iorder].emplace_back(hit);
        }
        if (nBad) ATH_MSG_WARNING("Unable to classify " << nBad << " clusters by their layer since they are neither MM nor sTGC");

        // Erase layers without hits
        orderedClusters.erase(std::remove_if(orderedClusters.begin(), orderedClusters.end(),
                                             [](const MeasVec& vec) { return vec.empty(); }),
                              orderedClusters.end());


        for( MeasVec& lays: orderedClusters){
            std::sort(lays.begin(),lays.end(), [this](const SeedMeasurement& a, const SeedMeasurement& b){
                return channel(a) < channel(b);
            });
            ATH_MSG_DEBUG("Found in layer "<<m_idHelperSvc->toStringDetEl(lays[0]->identify())<<" "<<lays.size()<<" clusters");


        }
        ATH_MSG_VERBOSE("Collected clusters "<<print(orderedClusters));

        return orderedClusters;
    }

    //============================================================================
    std::vector<NSWSeed> MuonNSWSegmentFinderTool::segmentSeedFromStgc(const LayerMeasVec& orderedClusters,
                                                                       bool usePhi) const {
        std::vector<NSWSeed> seeds;

        // oderedClusters should contain either eta clusters (MM and sTGC)
        // or sTGC phi hits. For MM phi, use the dedicated function.
        if (orderedClusters.size() < 4) return seeds;

        // Create seeds using each pair of hits on the two most distant layers (that containing hits).
        // m_nOfSeedLayers (default = 1) dictates whether we want to also use hits from inner layers.

        // Loop on layers to get the first seed point
        int seedingLayersL{0};
        for (unsigned int ilayerL{0}; (ilayerL < orderedClusters.size() && seedingLayersL < m_nOfSeedLayers); ++ilayerL) {
            bool usedLayerL{false};
            for (const SeedMeasurement& hitL : orderedClusters[ilayerL]) {
                /// Reject
                if (usePhi != m_idHelperSvc->measuresPhi(hitL->identify())) continue;
                else if(m_idHelperSvc->isMM(hitL->identify())) break;


                // For the second point, loop on layers in reverse to be as far as possible from the first.
                int seedingLayersR{0};
                for (unsigned int ilayerR = orderedClusters.size() - 1; (ilayerR > ilayerL && seedingLayersR < m_nOfSeedLayers);
                     --ilayerR) {
                    bool usedLayerR{false};
                    for (const SeedMeasurement& hitR : orderedClusters[ilayerR]) {
                        if (usePhi != m_idHelperSvc->measuresPhi(hitR->identify())) continue;
                        else if (m_idHelperSvc->isMM(hitR->identify())) break;
                        NSWSeed seed{this,hitL, hitR};
                        if (!usePhi && m_ipConstraint) {
                            const double eta = seed.dir().perp() > std::numeric_limits<float>::epsilon() ? std::abs(seed.dir().eta()): FLT_MAX;
                            if (eta < minEtaNSW || eta > maxEtaNSW) {
                                continue;
                            }
                        }
                        usedLayerR = true;
                        usedLayerL = true;
                        getClustersOnSegment(orderedClusters, seed, {ilayerL, ilayerR}, false);
                        seeds.emplace_back(std::move(seed));

                    }
                    if (usedLayerR) ++seedingLayersR;
                }
            }
            if (usedLayerL) ++seedingLayersL;
        }

        return resolveAmbiguities(std::move(seeds));
    }

    //============================================================================
    int MuonNSWSegmentFinderTool::wedgeNumber(const Muon::MuonClusterOnTrack* cluster) const {
        if (m_idHelperSvc->isMM(cluster->identify()))
            return m_idHelperSvc->mmIdHelper().multilayer(cluster->identify()) + 1;  // [IP:2, HO:3]
        if (m_idHelperSvc->issTgc(cluster->identify()))
            return 3 * (m_idHelperSvc->stgcIdHelper().multilayer(cluster->identify()) - 1) + 1;  // [IP:1, HO:4];
        return -1;
    }
    int MuonNSWSegmentFinderTool::layerNumber(const Muon::MuonClusterOnTrack* cluster) const {
        // Internal logic. Initialize with 16 layers:
        // [0-3]   for the four sTGC IP layers
        // [4-11]  for the eight MM IP+HO layers (empty when phi hits are requested)
        // [12-15] for the four sTGC HO layers
        int layer{0};
        if (m_idHelperSvc->isMM(cluster->identify())) layer = m_idHelperSvc->mmIdHelper().gasGap(cluster->identify());
        if (m_idHelperSvc->issTgc(cluster->identify())) layer = m_idHelperSvc->stgcIdHelper().gasGap(cluster->identify());
        return 4 * (wedgeNumber(cluster) - 1) + layer - 1;
    }
    int MuonNSWSegmentFinderTool::channel(const Muon::MuonClusterOnTrack* cluster) const{
        if (m_idHelperSvc->isMM(cluster->identify())) return m_idHelperSvc->mmIdHelper().channel(cluster->identify());
        if (m_idHelperSvc->issTgc(cluster->identify())) return m_idHelperSvc->stgcIdHelper().channel(cluster->identify());
        return -1;
    }

    //============================================================================
    int MuonNSWSegmentFinderTool::getClustersOnSegment(const LayerMeasVec& orderedclusters,
                                                       NSWSeed& seed, const std::set<unsigned int>& exclude, bool useStereo) const {
        ATH_MSG_VERBOSE(" getClustersOnSegment: layers " << orderedclusters.size());
        int nHitsAdded{0};

        for (const MeasVec& surfHits : orderedclusters) {
            if (exclude.count(layerNumber(surfHits[0]))) continue;
            // get the best hit candidate on this layer
            
            for (const SeedMeasurement& hit : surfHits) { 

                  const Identifier id = hit->identify();
                  const MuonGM::MuonChannelDesign* design = getDesign(hit);

                  //In case of the 2D eta stgc seeding we don't want the MMG stereo hits on the segment
                  if ( !useStereo && m_idHelperSvc->isMM(id) && design->hasStereoAngle() ) continue;

                  nHitsAdded += seed.add(hit, m_maxClustDist); 
            }
        }
        ATH_MSG_VERBOSE(" getClustersOnSegment: returning " << nHitsAdded << " hits ");
        return nHitsAdded;
    }

    //============================================================================
    std::vector<NSWSeed> MuonNSWSegmentFinderTool::segmentSeedFromPads(
        const LayerMeasVec& orderedClusters, const Muon::MuonSegment& etaSeg) const {
        std::vector<NSWSeed> seeds;
        /// Do not run an empty container
        if (orderedClusters.empty()) return seeds;

        std::vector<std::vector<const Muon::sTgcPrepData*>> sTgcIP(4);  // IP: layers nearest to the IP will be added first
        std::vector<std::vector<const Muon::sTgcPrepData*>> sTgcHO(4);  // HO: layers furthest from the IP will be added first

        // Process clusters separately for each multilayer
        for (int iml : {1, 2}) {
            int il = (iml == 1) ? 0 : orderedClusters.size() - 1;
            int iend = (iml == 1) ? orderedClusters.size() : -1;
            int idir = (iml == 1) ? 1 : -1;
            unsigned int nLayersWithHitMatch{0};

            // Loop on layers (reverse loop for HO)
            for (; il != iend; il += idir) {
                double lastDistance{1000.};
                if (nLayersWithHitMatch >= sTgcIP.size()) {
                    sTgcIP.resize(nLayersWithHitMatch + 1);
                    sTgcHO.resize(nLayersWithHitMatch + 1);
                }
                std::vector<const Muon::sTgcPrepData*>& matchedHits =
                    (iml == 1) ? sTgcIP.at(nLayersWithHitMatch) : sTgcHO.at(nLayersWithHitMatch);

                // Loop on the hits on this layer. Find the one closest (in eta) to the segment intersection.
                for (const Muon::MuonClusterOnTrack* rio : orderedClusters[il]) {
                    const sTgcPrepData* padHit = dynamic_cast<const sTgcPrepData*>(rio->prepRawData());
                    if (!padHit) continue;

                    // check the multilayer the hit is on
                    if (m_idHelperSvc->stgcIdHelper().multilayer(padHit->identify()) != iml) continue;

                    const MuonGM::MuonPadDesign* design = padHit->detectorElement()->getPadDesign(padHit->identify());
                    if (!design) continue;

                    // local position of the segment intersection with the plane
                    const Trk::Surface& surf = padHit->detectorElement()->surface(padHit->identify());
                    Trk::Intersection intersect =
                        surf.straightLineIntersection(etaSeg.globalPosition(), etaSeg.globalDirection(), false, false);
                    Amg::Vector2D segLocPosOnSurf{Amg::Vector2D::Zero()};
                    surf.globalToLocal(intersect.position, intersect.position, segLocPosOnSurf);

                    // eta distance between the hit and the segment intersection with the plane
                    // check that it's no more than half of the pad eta-pitch.
                    double chWidth = design->channelWidth(padHit->localPosition(), false);
                    double etaDistance = std::abs(padHit->localPosition().y() - segLocPosOnSurf[1]);
                    if (etaDistance > 0.5 * chWidth) continue;
                    ATH_MSG_DEBUG(" etaDistance " << etaDistance << " between pad center and position on the pad.");

                    if (matchedHits.empty()) {
                        // first hit
                        matchedHits.push_back(padHit);
                        ATH_MSG_DEBUG(" best etaDistance: " << etaDistance);
                    } else if (std::abs(etaDistance - lastDistance) < 0.001) {
                        // competing hit pad, keep both (all hit pads of the same eta row will be candidates)
                        matchedHits.push_back(padHit);
                        ATH_MSG_DEBUG(" added etaDistance: " << etaDistance << " size " << matchedHits.size());
                    } else if (etaDistance < lastDistance) {
                        // found a better hit; clear the old ones (possible only for clustered pad hits)
                        matchedHits.clear();
                        matchedHits.push_back(padHit);
                        ATH_MSG_DEBUG(" replacing best etaDistance with: " << etaDistance);
                    } else {
                        continue;
                    }
                    lastDistance = etaDistance;
                }  // end of loop on hits

                if (!matchedHits.empty()) ++nLayersWithHitMatch;

            }  // end of loop on layers

            // need at least one hit in each multilayer to create a seed
            if (!nLayersWithHitMatch) return seeds;

        }  // end of loop on multilayers

        // get refined phi ranges on each ml, by taking into account pad staggering
        std::vector<std::pair<double, double>> sTgcIP_phiRanges = getPadPhiOverlap(sTgcIP);
        std::vector<std::pair<double, double>> sTgcHO_phiRanges = getPadPhiOverlap(sTgcHO);

        // reference prds on the outermost hit surfaces
        const sTgcPrepData* prdL1 = sTgcIP.front().front();
        const sTgcPrepData* prdL2 = sTgcHO.front().front();
        const auto& surfPrdL1 = prdL1->detectorElement()->surface();
        const auto& surfPrdL2 = prdL2->detectorElement()->surface();

        // create a seed for each combination of IP and HO points
        for (const std::pair<double, double>& range1 : sTgcIP_phiRanges) {
            double midPhi1 = 0.5 * (range1.first + range1.second);
            Amg::Vector2D lp1(midPhi1, prdL1->localPosition().y());
            Amg::Vector3D gpL1{Amg::Vector3D::Zero()};
            surfPrdL1.localToGlobal(lp1, gpL1, gpL1);

            for (const std::pair<double, double>& range2 : sTgcHO_phiRanges) {
                double midPhi2 = 0.5 * (range2.first + range2.second);
                Amg::Vector2D lp2(midPhi2, prdL2->localPosition().y());
                Amg::Vector3D gpL2{Amg::Vector3D::Zero()};
                surfPrdL2.localToGlobal(lp2, gpL2, gpL2);
                // create the seed taking the average position (w.r.t. IP)
                // as global direction (as for an infinite momentum track).
                Amg::Vector3D gDir = (gpL2 + gpL1).unit();
                seeds.emplace_back(this, gpL1, gDir);
            }
        }

        ATH_MSG_DEBUG(" segmentSeedFromPads: seeds.size() " << seeds.size());
        return seeds;
    }

    //============================================================================
    std::vector<NSWSeed> MuonNSWSegmentFinderTool::segmentSeedFromMM(
        const LayerMeasVec& orderedClusters) const {
        std::vector<NSWSeed> seeds;
        std::array<unsigned int, 4> layers{};
        unsigned int trials{0}, used_layers{0};
        /// layers 12-15 contain stgcs and are not of interest...
        constexpr size_t lastMMLay = 11;
        std::vector<NSWSeed> laySeeds;

        /// Combinatorics  debugging stream
        std::stringstream sstr{};
        for (int e4  = std::min(lastMMLay, orderedClusters.size() -1); e4 >= 3 ; --e4) {
            layers[3] = e4;
            for (int e3 = e4 -1 ; e3 >= 2; --e3) {
                layers[2] = e3;
                for (int e2 = 1 ; e2 < e3; ++e2) {
                    layers[1] = e2;
                    for (int e1= 0; e1< e2; ++e1) {
                        layers[0] = e1;
                        const unsigned int old_trials = trials;
                        laySeeds = segmentSeedFromMM(orderedClusters,layers, trials);
                        if (old_trials == trials) continue;

                        used_layers += !laySeeds.empty();
                        seeds.insert(seeds.end(), std::make_move_iterator(laySeeds.begin()),
                                                  std::make_move_iterator(laySeeds.end()));

                        if (msgLvl(MSG::VERBOSE)) {
                            sstr<<" Attempts thus far "<<old_trials<<" attempts now "<<trials<<" --- "<<e1<<","<<e2<<","<<e3<<","<<e4<<std::endl;
                            for (int lay : layers) {
                                sstr<<"Layer: "<<lay<<" number of measurements "<<orderedClusters[lay].size()<<std::endl;
                                for (const SeedMeasurement& meas : orderedClusters[lay] ){
                                    sstr<<" **** "<< print(meas)<<std::endl;
                                }
                                sstr<<std::endl<<std::endl<<std::endl;
                            }
                        }
                    }
                }
            }
        }
        if (trials > 100000) {
            ATH_MSG_VERBOSE(sstr.str());
        }
        ATH_MSG_VERBOSE("Out of "<<trials<<" possible seeds, "<<seeds.size()<<" were finally built. Used in total "<<used_layers<<" layers");
        return resolveAmbiguities(std::move(seeds));
    }

    #if defined(FLATTEN)
    ATH_FLATTEN
    #endif
    inline std::vector<NSWSeed> MuonNSWSegmentFinderTool::segmentSeedFromMM(const LayerMeasVec& orderedClusters,
                                                                            std::array<unsigned int,4> selLayers,
                                                                            unsigned int& trial_counter) const {
        std::vector<NSWSeed> seeds{};

        std::array<unsigned int, 4> lay_ord{};
        for (size_t s = 0; s < selLayers.size(); ++s) {
            unsigned int lay = selLayers[s];
            const SeedMeasurement& seed = orderedClusters[lay].front();
            const Identifier id = seed->identify();
            if (!m_idHelperSvc->isMM(id)) return seeds;
            const MuonGM::MuonChannelDesign* design = getDesign(seed);
            /// Determine whether the layer is X(1) / U(2) or V(3)
            if (!design->hasStereoAngle())  lay_ord[s] = 1;
            else if (design->stereoAngle() >0.) lay_ord[s] = 2;
            else lay_ord[s] = 3;
        }
       auto swap_strips = [&selLayers, &lay_ord] (unsigned int i, unsigned j){
            std::swap(lay_ord[i], lay_ord[j]);
            std::swap(selLayers[i],selLayers[j]);
       };
        /// Order the strips such that the first and second pair consist each of crossing strips
        if (lay_ord[0] == lay_ord[1]){
            if (lay_ord[1] != lay_ord[2]) swap_strips(1,2);
             else if (lay_ord[1] != lay_ord[3]) swap_strips(1,3);
             else {
                ATH_MSG_VERBOSE("Strips are all parallel.");
                return seeds;
            }
        }
        /// The second pair is parallel
        if (lay_ord[2] == lay_ord[3]) {
            // Check if the last hit can be exchanged by the first one.
            // But also ensure that the second and fourth are not the same
            if (lay_ord[3] != lay_ord[0] && lay_ord[3] != lay_ord[1]) swap_strips(3,0);
            else {
               ATH_MSG_VERBOSE("No way to rearrange the strips such that the latter two strips cross.");
               return seeds;
            }
        }
        /// Assign the first measurement of each layer to calculate the linear transformation
        std::array<SeedMeasurement, 4> base_seed{};
        for (size_t s = 0; s < selLayers.size(); ++s) {
            unsigned int lay = selLayers[s];
            base_seed[s] = orderedClusters[lay].front();
        }

        const double A = (base_seed[1].pos().z() - base_seed[0].pos().z());
        const double G = (base_seed[2].pos().z() - base_seed[0].pos().z());
        const double K = (base_seed[3].pos().z() - base_seed[0].pos().z());

        AmgSymMatrix(2) diamond{AmgSymMatrix(2)::Zero()};
        diamond.block<2, 1>(0, 1) = ((A - G) * base_seed[3].dirDot(base_seed[1]) * base_seed[1].dir() + G * base_seed[3].dirDot(base_seed[0]) * base_seed[0].dir() - A * base_seed[3].dir()).block<2, 1>(0, 0);
        diamond.block<2, 1>(0, 0) = ((K - A) * base_seed[2].dirDot(base_seed[1]) * base_seed[1].dir() - K * base_seed[2].dirDot(base_seed[0]) * base_seed[0].dir() + A * base_seed[2].dir()).block<2, 1>(0, 0);

        if (std::abs(diamond.determinant()) < std::numeric_limits<float>::epsilon()) {
            ATH_MSG_VERBOSE(" The seed built from " << printSeed(base_seed) << " cannot constrain phi as " << std::endl
                                                    << diamond << std::endl
                                                    << " is singular " << diamond.determinant() << " with rank "
                                                    << (Eigen::FullPivLU<AmgSymMatrix(2)>{diamond}.rank()));

            return seeds;
        }
        ATH_MSG_VERBOSE("The combination of " << printSeed(base_seed) << " to " << std::endl
                                              << diamond << std::endl
                                              << "May give a couple of stereo seeds " << diamond.determinant());

        /// Habemus valid strip quartett
        const AmgSymMatrix(2) seed_builder = diamond.inverse();
        /// Function to calculate the muon crossing
        const double KmG = K-G;
        const double KmA = K-A;
        const double AmG = A-G;

        const double TwoDotZero = base_seed[2].dirDot(base_seed[0]);
        const double ThreeDotZero = base_seed[3].dirDot(base_seed[0]);
        const double ThreeDotOne = base_seed[3].dirDot(base_seed[1]);
        const double TwoDotOne = base_seed[2].dirDot(base_seed[1]);

        auto estimate_muon = [&] () -> std::optional<std::array<double,2>> {
            const Amg::Vector3D Y0 = K * base_seed[2].pos() - G * base_seed[3].pos() - KmG * base_seed[0].pos();
            const Amg::Vector3D Y1 = AmG * base_seed[3].pos() - KmG * base_seed[1].pos() + KmA * base_seed[2].pos();
            const double Y0dotE0 = base_seed[0].dirDot(Y0);
            const double Y1dotE1 = base_seed[1].dirDot(Y1);

            const AmgVector(2) centers = (KmG * (base_seed[0].pos() - base_seed[1].pos()) +
                                           Y0dotE0 * base_seed[0].dir() +
                                           A * (base_seed[3].pos() - base_seed[2].pos()) -
                                          Y1dotE1 * base_seed[1].dir())
                                         .block<2, 1>(0, 0);

            const AmgVector(2) sol_pars = seed_builder * centers;
            const std::array<double, 4> lengths{(Y0dotE0 + K * sol_pars[0] * TwoDotZero - G * sol_pars[1] * ThreeDotZero) / KmG,
                                            (Y1dotE1 + AmG * sol_pars[1] *ThreeDotOne + KmA * sol_pars[0] * TwoDotOne) / KmG, sol_pars[0], sol_pars[1]};
            bool accept{true};
            ATH_MSG_VERBOSE("Check intersections of "<<printSeed(base_seed));
            constexpr double tolerance = 10.* Gaudi::Units::mm;
            std::optional<Amg::Vector3D> seg_pos{std::nullopt}, seg_dir{std::nullopt};
            for (unsigned int i = 0; i < base_seed.size(); ++i) {
                const MuonGM::MuonChannelDesign* design = getDesign(base_seed[i]);
                const double halfLength = design->channelHalfLength(channel(base_seed[i]), true);
                accept &= (halfLength  + tolerance > std::abs(lengths[i]));
                if (msgLvl(MSG::VERBOSE)) {
                    if (!seg_pos) {
                        seg_pos = std::make_optional<Amg::Vector3D>(base_seed[0].pos() +
                                                                    lengths[0] * base_seed[0].dir());
                        ATH_MSG_VERBOSE("Position "<<to_string(*seg_pos));
                    }
                    if (!seg_dir){
                            seg_dir = std::make_optional<Amg::Vector3D>((base_seed[1].pos() +
                                                                    lengths[1] *base_seed[1].dir() - (*seg_pos)).unit());
                        ATH_MSG_VERBOSE("Direction "<<to_string(*seg_dir));
                    }
                    std::optional<double> mu_crossing = Amg::intersect<3>(*seg_pos, *seg_dir, base_seed[i].pos(),base_seed[i].dir());
                    ATH_MSG_VERBOSE(" ----- "<<(i+1)<<" at "<<to_string(base_seed[i].pos() + lengths[i]*base_seed[i].dir())
                                << " ("<< std::string( halfLength > std::abs(lengths[i]) ? "inside" : "outside")<<" wedge) "
                                << halfLength <<" vs. "<<std::abs(lengths[i])<<" crossing point: "<<std::abs(*mu_crossing));
                } else if (!accept) return std::nullopt;
            }
            if (!accept) return std::nullopt;
            return std::make_optional<std::array<double,2>>({lengths[0], lengths[1]});
        };

        /// Reserve space for 200 seeds
        seeds.reserve(200);
        MeasVec::const_iterator begin2{orderedClusters[selLayers[1]].begin()};
        MeasVec::const_iterator begin3{orderedClusters[selLayers[2]].begin()};
        MeasVec::const_iterator begin4{orderedClusters[selLayers[3]].begin()};

        const MeasVec::const_iterator end2{orderedClusters[selLayers[1]].end()};
        const MeasVec::const_iterator end3{orderedClusters[selLayers[2]].end()};
        const MeasVec::const_iterator end4{orderedClusters[selLayers[3]].end()};

        for (const SeedMeasurement& lay1 : orderedClusters[selLayers[0]]) {
            base_seed[0] = lay1;
            for (MeasVec::const_iterator lay2 = begin2; lay2 != end2; ++lay2) {

                base_seed[1] = *lay2;
                ChannelConstraint chCheck = compatiblyFromIP(lay1, *lay2);
                /// The two channels are too narrow.
                /// Same conclusion holds for all previous hits from this layer combined with the next ones of layer 1
                if (chCheck == ChannelConstraint::TooNarrow) {
                    begin2 = lay2 + 1;
                    continue;
                }
                /// The opening angle of these two channels is just too wide
                else if (chCheck == ChannelConstraint::TooWide) {
                    break;
                }
                /// The first or the second one can cross with the third one
                for (MeasVec::const_iterator lay3 = begin3; lay3 != end3; ++lay3) {
                    chCheck = compatiblyFromIP(lay1, *lay3);
                    const ChannelConstraint chCheck1 = compatiblyFromIP(*lay2, *lay3);
                    if (chCheck == ChannelConstraint::TooNarrow && chCheck1 == ChannelConstraint::TooNarrow) {
                        begin3 = lay3 + 1;
                        continue;
                    } else if (chCheck == ChannelConstraint::TooWide && chCheck1 == ChannelConstraint::TooWide) {
                        break;
                    }
                    base_seed[2] = *lay3;
                    /// Reject combinations that consist only of 1 strip clusters
                    for (MeasVec::const_iterator lay4 = begin4 ; lay4 != end4; ++lay4) {
                        chCheck = compatiblyFromIP(*lay3, *lay4);
                        if (chCheck == ChannelConstraint::TooNarrow) {
                            begin4 = lay4 + 1;
                            continue;
                        }  else if (chCheck == ChannelConstraint::TooWide) {
                            break;
                        }

                        base_seed[3] = (*lay4);
                        std::optional<std::array<double, 2>> isects = estimate_muon();
                        ++trial_counter;
                        if (!isects) continue;
                        NSWSeed seed{this, base_seed, *isects};

                        if (seed.size() < 4) continue;
                        if (m_ipConstraint) {
                            const double eta = std::abs(seed.dir().eta());
                            if (eta < minEtaNSW || eta > maxEtaNSW) {
                                continue;
                            }
                            if (seed.dir().block<2,1>(0,0).dot(seed.pos().block<2,1>(0,0)) < 0.) continue;
                            /// We will revise this requirement in the near future. Keep the block for the moment
                            static const Muon::MuonSectorMapping  sector_mapping{};
                            const double deltaPhi = std::abs(seed.dir().deltaPhi(seed.pos()));
                            if (deltaPhi > sector_mapping.sectorWidth(m_idHelperSvc->sector(base_seed[0]->identify()))) continue;
                        }
                        getClustersOnSegment(orderedClusters, seed, {selLayers[0], selLayers[1],selLayers[2], selLayers[3]});
                        seeds.emplace_back(std::move(seed));
                    }
                }
            }
        }
        return seeds;
    }
    inline MuonNSWSegmentFinderTool::ChannelConstraint
        MuonNSWSegmentFinderTool::compatiblyFromIP(const SeedMeasurement& meas1, const SeedMeasurement& meas2) const {
        if (!m_ipConstraint) return ChannelConstraint::InWindow;

        // For a given dZ the measurements can only be separated by a certain dR such that the
        /// direction of final segment is well compatible with the IP cut
        const double dZ = std::abs(meas2->globalPosition().z()) -
                          std::abs(meas1->globalPosition().z());

        /// Use the Identity that 1./sinh(eta) = tan(theta)
        /// https://www.wolframalpha.com/input?i=1.%2Fsinh%28-ln%28tan%28x%2F2%29%29%29+-+tan%28x%29
        /// Add another 0.25 as safety margin
        static const double minTanTheta = 0.75 / std::sinh(maxEtaNSW);
        static const double maxTanTheta = 1.25 / std::sinh(minEtaNSW);

        const double minDR = minTanTheta * std::abs(dZ);
        const double maxDR = maxTanTheta * std::abs(dZ);

        const std::pair<double, double> rad1 = coveredRadii(meas1);
        const std::pair<double, double> rad2 = coveredRadii(meas2);
        /// Calculate the transverse radii at the edges of the
        const double dlR = rad2.first - rad1.first;
        const double drR = rad2.second - rad1.second;
        ATH_MSG_VERBOSE("compatiblyFromIP() -- Measurements "<<std::endl
                        <<print(meas1)<<std::endl<<print(meas2)
                        <<std::endl<<". Separation in dR (left/right) "<<dlR<<"/"<<drR<<", dZ "<<dZ
                        <<" --> dR has to be in "<<minDR<<" "<<maxDR);
        if ((std::abs(dlR) < minDR && std::abs(drR) < minDR) ||
            (dZ > 0 &&  dlR <0 && drR <0) || (dZ < 0 &&  dlR >0 && drR > 0)) {
            return ChannelConstraint::TooNarrow;
        } if (std::abs(dlR) > maxDR && std::abs(drR) > maxDR && dlR * drR > 0.){
            return ChannelConstraint::TooWide;
        }
        return ChannelConstraint::InWindow;
    }
    inline std::pair<double, double> MuonNSWSegmentFinderTool::coveredRadii(const SeedMeasurement& meas) const {
        const MuonGM::MuonChannelDesign* design = getDesign(meas);
        const int chNum = channel(meas);
        Amg::Vector2D left{Amg::Vector2D::Zero()}, right{Amg::Vector2D::Zero()};
        design->leftEdge(chNum, left);
        design->rightEdge(chNum, right);
        const double radLeft{meas->associatedSurface().localToGlobal(left).perp()};
        const double radRight{meas->associatedSurface().localToGlobal(right).perp()};
        return std::make_pair(radLeft, radRight);
    }
    std::vector<NSWSeed> MuonNSWSegmentFinderTool::resolveAmbiguities(std::vector<NSWSeed>&& unresolved) const {
        std::vector<NSWSeed> seeds;
        seeds.reserve(unresolved.size());
        std::sort(unresolved.begin(), unresolved.end(),[](const NSWSeed& a, const NSWSeed& b){
                                                            return a.chi2() < b.chi2();
                                                     });
        for (NSWSeed& seed : unresolved) {
            bool add_seed{true};
            for (NSWSeed& good : seeds) {
                NSWSeed::SeedOR ov = good.overlap(seed);
                if (ov == NSWSeed::SeedOR::SubSet) {
                    std::swap(seed, good);
                    add_seed = false;
                    break;
                } else if (ov == NSWSeed::SeedOR::Same || ov == NSWSeed::SeedOR::SuperSet) {
                    add_seed = false;
                    break;
                }
            }
            if (add_seed) seeds.push_back(std::move(seed));
        }
        ATH_MSG_VERBOSE(seeds.size()<<" out of "<<unresolved.size()<<" passed the overlap removal");
        return seeds;
    }

    //============================================================================
    std::vector<std::pair<double, double>> MuonNSWSegmentFinderTool::getPadPhiOverlap(
        const std::vector<std::vector<const Muon::sTgcPrepData*>>& pads) const {
        // 'pads' contains segment hit candidates, classified in four layers (IP or HO).
        // Layers are ordered; for IP, the layer with hits that is nearest to
        // the IP is first, while for HO, the one furthest from the IP is first.

        std::vector<std::vector<double>> padsPhiL, padsPhiR;
        std::vector<double> padsPhiC;

        // Loop on layers
        for (const std::vector<const Muon::sTgcPrepData*>& surfHits : pads) {
            // Loop on layer hits
            std::vector<double> surfPadsPhiL, surfPadsPhiR;
            for (const Muon::sTgcPrepData* prd : surfHits) {
                const Identifier id = prd->identify();
                const MuonGM::MuonPadDesign* design = prd->detectorElement()->getPadDesign(id);
                if (!design) {
                    ATH_MSG_WARNING("No design available for " << m_idHelperSvc->toString(id));
                    continue;
                }

                // Phi boundaries of this pad in local coordinates
                const double halfWidthX = 0.5 * design->channelWidth(prd->localPosition(), true);
                const double hitPadX = prd->localPosition().x();  // x is in the phi direction

                // Reject hit candidates on pads too close (in phi) to any pad kept so far
                // (pad fuzziness) to constrain the number of combinations.
                bool samePhi = std::find_if(padsPhiC.begin(), padsPhiC.end(), [&hitPadX, &halfWidthX](const double prevPadPhi) {
                                   return std::abs(hitPadX - prevPadPhi) < 0.9 * halfWidthX;
                               }) != padsPhiC.end();

                if (samePhi) continue;

                // Store the new pad candidate
                surfPadsPhiL.push_back(hitPadX - halfWidthX);
                surfPadsPhiR.push_back(hitPadX + halfWidthX);
                padsPhiC.push_back(hitPadX);
                ATH_MSG_DEBUG(" keep pad id " << m_idHelperSvc->toString(id) << " local x: " << hitPadX << " width: " << halfWidthX);
            }

            padsPhiL.push_back(std::move(surfPadsPhiL));
            padsPhiR.push_back(std::move(surfPadsPhiR));
        }

        unsigned int nSurf = padsPhiR.size();

        // number of combinations we can make out of pads in different layers
        // we want to keep combinations of overlapping pads.
        unsigned int nCombos{1};
        for (const std::vector<double>& surfPadsPhiR : padsPhiR) {
            if (!surfPadsPhiR.empty()) nCombos *= surfPadsPhiR.size();
        }

        std::vector<std::pair<double, double>> phiOverlap;
        phiOverlap.reserve(nCombos);

        if (nCombos <= 100) {
            unsigned int N{nCombos};
            for (unsigned int isurf{0}; isurf < nSurf; ++isurf) {
                if (padsPhiR[isurf].empty()) continue;
                unsigned int nSurfHits = padsPhiR[isurf].size();
                N /= nSurfHits;

                for (unsigned int icombo{0}; icombo < nCombos; ++icombo) {
                    // index of the pad that corresponds to this combination
                    unsigned int padIdx = (icombo / N) % nSurfHits;
                    if (isurf == 0) {
                        // first surface: just add the range of each hit pad
                        phiOverlap.emplace_back(padsPhiL[isurf][padIdx], padsPhiR[isurf][padIdx]);
                    } else {
                        // subsequent surfaces: use staggering to narrow the phi ranges
                        phiOverlap[icombo].first = std::max(padsPhiL[isurf][padIdx], phiOverlap[icombo].first);
                        phiOverlap[icombo].second = std::min(padsPhiR[isurf][padIdx], phiOverlap[icombo].second);
                    }
                }
            }

            // delete bad combinations with xmin > xmax (indicates non overlapping pads)
            phiOverlap.erase(std::remove_if(phiOverlap.begin(), phiOverlap.end(),
                                            [](std::pair<double, double>& range) { return range.first >= range.second; }),
                             phiOverlap.end());
            ATH_MSG_DEBUG("Pad seeding - #combinations initial: " << nCombos
                                                                  << ", after cleaning for non overlapping pads: " << phiOverlap.size());

        } else {
            // in case combinations are too many, store the phi ranges of individual pads
            for (unsigned int isurf{0}; isurf < nSurf; ++isurf) {
                unsigned int nSurfHits = padsPhiR[isurf].size();
                for (unsigned int ihit{0}; ihit < nSurfHits; ++ihit) {
                    phiOverlap.emplace_back(padsPhiL[isurf][ihit], padsPhiR[isurf][ihit]);
                }
            }
            ATH_MSG_DEBUG("Pad seeding - #combinations: " << nCombos << " is too large. Seeding from" << phiOverlap.size()
                                                          << " individual pads.");
        }

        return phiOverlap;
    }

    //============================================================================
    MeasVec MuonNSWSegmentFinderTool::getCalibratedClusters(NSWSeed& seed) const {

        MeasVec calibratedClusters;
        MeasVec clusters = seed.measurements();

        // loop on the segment clusters and use the phi of the seed to correct them
        for (const SeedMeasurement& clus : clusters) {
            std::unique_ptr<const Muon::MuonClusterOnTrack> newClus;

            // get the intercept of the seed direction with the cluster surface
            const Identifier hitID = clus->identify();
            const Trk::Surface& surf = clus->associatedSurface();
            Trk::Intersection intersect = surf.straightLineIntersection(seed.pos(), seed.dir(), false, false);

            if (m_idHelperSvc->isMM(hitID)) {
                // build a  new MM cluster on track with correct position
                std::unique_ptr<const Muon::MuonClusterOnTrack> newClus {m_muonClusterCreator->correct(*clus->prepRawData(), intersect.position, seed.dir())};
                calibratedClusters.emplace_back(seed.newCalibClust(std::move(newClus)));
            } else if (m_idHelperSvc->issTgc(hitID)) {
                // build a  new sTGC cluster on track with correct position
                std::unique_ptr<const Muon::MuonClusterOnTrack> newClus {m_muonClusterCreator->correct(*clus->prepRawData(), intersect.position, seed.dir())};
                calibratedClusters.emplace_back(seed.newCalibClust(std::move(newClus)));
            }
        }

        return calibratedClusters;
    }

    //============================================================================
    template <size_t N>
    std::string MuonNSWSegmentFinderTool::printSeed(const std::array<SeedMeasurement, N>& seed) const {
        std::stringstream sstr{};
        sstr << std::endl;
        for (const SeedMeasurement& cl : seed) sstr << " *** " << print(cl) << std::endl;
        return sstr.str();
    }

    //============================================================================
    std::string MuonNSWSegmentFinderTool::print(const SeedMeasurement& cl) const {
        std::stringstream sstr{};
        sstr << m_idHelperSvc->toString(cl->identify()) << " at " <<to_string(cl.pos())
            <<" pointing to (" <<to_string(cl.dir())<<" cluster size: "<<clusterSize(cl);

        return sstr.str();
    }
    std::string MuonNSWSegmentFinderTool::print(const MeasVec& measurements) const {
        std::stringstream sstr{};
        for (const SeedMeasurement& cl : measurements){
            sstr<<" *** "<<print(cl)<<std::endl;
        }
        return sstr.str();
    }
    std::string MuonNSWSegmentFinderTool::print(const LayerMeasVec& sortedVec) const {
        std::stringstream sstr{};
        unsigned int lay{0};
        for (const MeasVec& clusts: sortedVec){
            sstr<<"Clusters in Layer: "<<(lay+1)<<std::endl;
            sstr<<"#################################################"<<std::endl;
            sstr<<print(clusts);
            ++lay;
        }
        return sstr.str();
    }

    MeasVec MuonNSWSegmentFinderTool::vetoBursts( MeasVec && clustInLay ) const {
        if (clustInLay.size() < m_ocupMmNumPerBin || m_idHelperSvc->issTgc(clustInLay[0]->identify())) return clustInLay;
        /// The micromegas are slobbering quite a lot in the 2023 data taking leading to uncopable comptuing times.
        /// The burst vetoing aims to mitigate this situation. A histogram with a bin width of m_ocupMmBinWidth is defined
        /// Hits are filled into the accoring bin using their channel number
        /// Bins with large activity are then cleared. In the second step, pairs of bins where their sum is too exhaustive
        /// are removed.
        MeasVec prunedMeas{};
        prunedMeas.reserve(clustInLay.size());
        const unsigned int firstCh = channel(clustInLay[0]);
        const unsigned int lastCh = channel(clustInLay[clustInLay.size() -1]);
        /// Define the number of bins
        const unsigned int deltaCh = lastCh - firstCh;
        const unsigned int nBins = deltaCh / m_ocupMmBinWidth + (deltaCh % m_ocupMmBinWidth > 0);
        ATH_MSG_VERBOSE("Clusters in layer "<<print(clustInLay)<<" lowest channel: "<<
                        firstCh<<", highest channel: "<<lastCh<<" bin width: "<<m_ocupMmBinWidth<<" number of bins"<<nBins);

        LayerMeasVec occupancyHisto{};
        occupancyHisto.resize(nBins);
        for (MeasVec& bin : occupancyHisto) {
            bin.reserve(clustInLay.size());
        }
        ATH_MSG_VERBOSE("Clusters sorted into bins "<<print(occupancyHisto));
        /// Fill the measurements into the histograms
        for (SeedMeasurement& meas : clustInLay){
            unsigned int bin = (channel(meas) - firstCh) % nBins;
            occupancyHisto[bin].push_back(std::move(meas));
        }
        /// Apply the single bin cleaning
        for (MeasVec& bin : occupancyHisto) {
            if(bin.size() >= m_ocupMmNumPerBin){
                ATH_MSG_VERBOSE("The micromegas are slobbering. Detected too many clusters "<<bin.size()<<std::endl<<print(bin));
                bin.clear();
            }
        }
        /// Apply the pair wise bin cleaning
        for (unsigned int i = 0; i < occupancyHisto.size() -1; ++i) {
            if (occupancyHisto[i].size() + occupancyHisto[i+1].size() >= m_ocupMmNumPerPair){
                ATH_MSG_VERBOSE("The two neighbouring bins "<<i<<"&"<<(i+1)<<" have too many clusters "<<std::endl<<
                print(occupancyHisto[i])<<std::endl<<print(occupancyHisto[i+1]));
                occupancyHisto[i].clear();
                occupancyHisto[i+1].clear();
            }
        }
        /// Copy the rest over
        for (MeasVec& bin : occupancyHisto){
            std::copy(std::make_move_iterator(bin.begin()),
                      std::make_move_iterator(bin.end()),
                      std::back_inserter(prunedMeas));
        }

        ATH_MSG_VERBOSE("Number of measurements before pruning "<<clustInLay.size()<<" number of measurments survived the pruning "<<prunedMeas.size());
        return prunedMeas;

    }
}  // namespace Muon
