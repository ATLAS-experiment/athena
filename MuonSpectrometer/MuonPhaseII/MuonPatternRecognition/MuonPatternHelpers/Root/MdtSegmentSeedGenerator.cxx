/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>
#include <EventPrimitives/EventPrimitivesHelpers.h>

#include <Acts/Utilities/Enumerate.hpp>

#include <Acts/Utilities/Enumerate.hpp>
#include <CxxUtils/sincos.h>
#include <format>

namespace MuonR4{
    using namespace SegmentFit;
    using namespace SegmentFitHelpers;
    using HitVec = SpacePointPerLayerSorter::HitVec;

    double driftCov(const CalibratedSpacePoint& dcHit){
        return std::visit([](const auto& cov) ->double{
            return Amg::error(cov, toInt(AxisDefs::eta));
        }, dcHit.covariance());
    }
    constexpr bool passRangeCut(const std::array<double, 2>& cutRange, const double value) {
        return cutRange[0] <= value && value <= cutRange[1];
    }

    std::ostream& MdtSegmentSeedGenerator::SeedSolution::print(std::ostream& ostr) const{
        ostr<<"two circle solution with ";
        ostr<<"theta: "<<theta / Gaudi::Units::deg<<" pm "<<dTheta / Gaudi::Units::deg<<", ";
        ostr<<"y0: "<<Y0<<" pm "<<dY0;
        return ostr;
    }
    const MdtSegmentSeedGenerator::Config& MdtSegmentSeedGenerator::config() const {
        return m_cfg;
    }
    MdtSegmentSeedGenerator::~MdtSegmentSeedGenerator() = default;
    MdtSegmentSeedGenerator::MdtSegmentSeedGenerator(const std::string& name,
                                                     const SegmentSeed* segmentSeed, 
                                                     const Config& configuration):
            AthMessaging{name},
            m_cfg{configuration},
            m_segmentSeed{segmentSeed} {

        if (m_hitLayers.mdtHits().empty()) return;
        
        if (std::ranges::find_if(m_hitLayers.mdtHits(), [this](const HitVec& vec){
            return vec.size() > m_cfg.busyLayerLimit;
        }) != m_hitLayers.mdtHits().end()) {
            m_cfg.startWithPattern = false;
        }
        // Set the start for the upper layer
        m_upperLayer = m_hitLayers.mdtHits().size()-1; 

        /** Check whether the first layer is too busy */
        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[m_lowerLayer].size() >m_cfg.busyLayerLimit){
            ++m_lowerLayer;
        }
        /** Check whether the lower layer is too busy */
        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[m_upperLayer].size() > m_cfg.busyLayerLimit) {
            --m_upperLayer;
        }

        if (msgLvl(MSG::VERBOSE)) {
            std::stringstream sstr{};
            for (const auto& [layCount, layer] : Acts::enumerate(m_hitLayers.mdtHits())) { 
                sstr<<"Mdt-hits in layer "<<layCount<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->msSector()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            for (const auto& [layCount, layer] : Acts::enumerate(m_hitLayers.stripHits())) { 
                sstr<<"Hits in layer "<<layCount<<": "<<layer.size()<<std::endl;
                for (const HoughHitType& hit : layer) {
                    sstr<<"   **** "<<hit->msSector()->idHelperSvc()->toString(hit->identify())<<" "
                        <<Amg::toString(hit->positionInChamber())<<", driftRadius: "<<hit->driftRadius()<<std::endl;
                }
            }
            ATH_MSG_VERBOSE("SeedGenerator - sorting of hits done. Mdt layers: "<<m_hitLayers.mdtHits().size()
                            <<", strip layers: "<<m_hitLayers.stripHits().size()<<std::endl<<sstr.str());
        }
    }
    
    unsigned int MdtSegmentSeedGenerator::numGenerated() const {
        return m_nGenSeeds;
    }
    inline void MdtSegmentSeedGenerator::moveToNextCandidate() {
        const HitVec& lower = m_hitLayers.mdtHits()[m_lowerLayer];
        const HitVec& upper = m_hitLayers.mdtHits()[m_upperLayer];
        /// All 8 sign combinations are tried. 
        if (++m_signComboIndex >= s_signCombos.size()){
            m_signComboIndex = 0; 
            /// Get the next lower hit & check whether boundary is exceeded
            if (++m_lowerHitIndex >= lower.size()){
                m_lowerHitIndex=0;
                /// Same for the hit in the upper layer
                if (++m_upperHitIndex >= upper.size()) {
                    m_upperHitIndex = 0;
                    /// All combinations of hits & lines in both layers are processed
                    /// Switch to the next lowerLayer. But skip the busy ones. For now  place a cut on 3 hits
                    while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[++m_lowerLayer].size() > m_cfg.busyLayerLimit){
                    } 
                    if (m_lowerLayer >= m_upperLayer){
                        m_lowerLayer = 0; 
                        while (m_lowerLayer < m_upperLayer && m_hitLayers.mdtHits()[--m_upperLayer].size() > m_cfg.busyLayerLimit){
                        }
                    }
                }
            }
        }
    }
    std::optional<MdtSegmentSeedGenerator::DriftCircleSeed> 
        MdtSegmentSeedGenerator::nextSeed(const EventContext& ctx) {
        std::optional<DriftCircleSeed> found = std::nullopt; 
        if (!m_nGenSeeds && m_cfg.startWithPattern) {
            ++m_nGenSeeds;
            found = std::make_optional<DriftCircleSeed>();
            found->parameters = m_segmentSeed->parameters();
            found->measurements = m_cfg.calibrator->calibrate(ctx,
                                                              m_segmentSeed->getHitsInMax(),
                                                              m_segmentSeed->positionInChamber(),
                                                              m_segmentSeed->directionInChamber(),0.);
            found->parentBucket = m_segmentSeed->parentBucket();
            
            SeedSolution patternSeed{};
            patternSeed.seedHits.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.solutionSigns.resize(2*m_hitLayers.mdtHits().size());
            patternSeed.Y0 = m_segmentSeed->interceptY();
            patternSeed.theta = m_segmentSeed->tanTheta();
            m_seenSolutions.push_back(std::move(patternSeed));
            return found;
        }
       
        while (m_lowerLayer < m_upperLayer) {
            const HitVec& lower = m_hitLayers.mdtHits().at(m_lowerLayer);
            const HitVec& upper = m_hitLayers.mdtHits().at(m_upperLayer);
            ATH_MSG_VERBOSE("Layers with hits: "<<m_hitLayers.mdtHits().size()
                            <<" -- next bottom hit: "<<m_lowerLayer<<", hit: "<<m_lowerHitIndex
                            <<" ("<<lower.size()<<"), topHit " <<m_upperLayer<<", "<<m_upperHitIndex
                            <<" ("<<upper.size()<<") - ambiguity "<<s_signCombos[m_signComboIndex]);

            found = buildSeed(ctx, upper.at(m_upperHitIndex), lower.at(m_lowerHitIndex), s_signCombos.at(m_signComboIndex));
            /// Increment for the next candidate
            moveToNextCandidate();
            /// If a candidate is built return it. Otherwise continue the process
            if (found) {
                return found;
            }
        }
        return std::nullopt; 
    }
    std::optional<MdtSegmentSeedGenerator::DriftCircleSeed>  
        MdtSegmentSeedGenerator::buildSeed(const EventContext& ctx,
                                           const HoughHitType & topHit, 
                                           const HoughHitType & bottomHit, 
                                           const SignComboType& signs) {
        
        const auto* bottomPrd = static_cast<const xAOD::MdtDriftCircle*>(bottomHit->primaryMeasurement()); 
        const auto* topPrd = static_cast<const xAOD::MdtDriftCircle*>(topHit->primaryMeasurement());
        if (bottomPrd->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime ||
            topPrd->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime) {
                return std::nullopt;
        }
        const int signTop = signs[0];
        const int signBot = signs[1];
        double R = signBot *bottomHit->driftRadius() - signTop * topHit->driftRadius(); 
        const Amg::Vector3D& bottomPos{bottomHit->positionInChamber()};
        const Amg::Vector3D& topPos{topHit->positionInChamber()};
        const Muon::IMuonIdHelperSvc* idHelperSvc{topHit->msSector()->idHelperSvc()};
        const Amg::Vector3D D = topPos - bottomPos;
        const double thetaTubes = std::atan2(D.y(), D.z()); 
        const double distTubes =  std::hypot(D.y(), D.z());
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<": Bottom tube "<<idHelperSvc->toString(bottomHit->identify())<<" "<<Amg::toString(bottomPos)
                    <<", driftRadius: "<<bottomHit->driftRadius()<<" - top tube "<<idHelperSvc->toString(topHit->identify())
                    <<" "<<Amg::toString(topPos)<<", driftRadius: "<<topHit->driftRadius()
                    <<", tube distance: "<<Amg::toString(D));

        DriftCircleSeed candidateSeed{};
        candidateSeed.parameters = m_segmentSeed->parameters();
        candidateSeed.parentBucket = m_segmentSeed->parentBucket();
        double theta{thetaTubes - std::asin(std::clamp(R / distTubes, -1., 1.))};
        Amg::Vector3D seedDir = Amg::dirFromAngles(90.*Gaudi::Units::deg, theta);
        double Y0 = bottomPos.y()*seedDir.z() - bottomPos.z()*seedDir.y() + signBot*bottomHit->driftRadius();
        double combDriftUncert{std::sqrt(bottomPrd->driftRadiusCov() + topPrd->driftRadiusCov())};
        if (m_cfg.recalibSeedCircles) {
            candidateSeed.parameters[toInt(ParamDefs::theta)] = theta;
            candidateSeed.parameters[toInt(ParamDefs::y0)] = Y0 / seedDir.z();
            /// Create a new line position & direction which also takes the
            /// potential phi estimates into account
            const auto [linePos, lineDir] = makeLine(candidateSeed.parameters);
            auto calibBottom = m_cfg.calibrator->calibrate(ctx, bottomHit, linePos, lineDir, 
                                                           candidateSeed.parameters[toInt(ParamDefs::time)]);
            auto calibTop = m_cfg.calibrator->calibrate(ctx, topHit, linePos, lineDir, 
                                                        candidateSeed.parameters[toInt(ParamDefs::time)]);
            R = signBot * calibBottom->driftRadius() - signTop * calibTop->driftRadius();
            /// Recalculate the seed with the calibrated parameters
            theta =  thetaTubes - std::asin(std::clamp(R / distTubes, -1., 1.));
            seedDir = Amg::dirFromAngles(90.*Gaudi::Units::deg, theta);
            Y0 = bottomPos.y()*seedDir.z() - bottomPos.z()*seedDir.y() + signBot*bottomHit->driftRadius();
            combDriftUncert = std::sqrt(driftCov(*calibBottom) + driftCov(*calibTop));
        }
        
        candidateSeed.parameters[toInt(ParamDefs::theta)] = theta;
        candidateSeed.parameters[toInt(ParamDefs::y0)] = Y0 / seedDir.z();
        /// Check that the is within the predefined window
        if (!passRangeCut(m_cfg.thetaRange, theta) || 
            !passRangeCut(m_cfg.interceptRange, candidateSeed.parameters[toInt(ParamDefs::y0)])) {
            return std::nullopt;
        }

        const Amg::Vector3D seedPos = Y0 / seedDir.z() * Amg::Vector3D::UnitY();

        assert(std::abs(topPos.y()*seedDir.z() - topPos.z() * seedDir.y() + signTop*topHit->driftRadius() - Y0) < std::numeric_limits<float>::epsilon() );
        ATH_MSG_VERBOSE("Candidate seed theta: "<<theta<<", tanTheta: "<<(seedDir.y() / seedDir.z())<<", y0: "<<Y0/seedDir.z());

        SeedSolution solCandidate{};
        solCandidate.Y0 = seedPos[toInt(ParamDefs::y0)];
        solCandidate.theta = theta;
        /// d/dx asin(x) = 1 / sqrt(1- x*x)
        double denomSquare =  1. - std::pow(R / distTubes, 2); 
        if (denomSquare < std::numeric_limits<double>::epsilon()){
            ATH_MSG_VERBOSE("Invalid seed, rejecting"); 
            return std::nullopt; 
        }
        solCandidate.dTheta =  combDriftUncert / std::sqrt(denomSquare) / distTubes;
        solCandidate.dY0 =  std::hypot(-bottomPos.y()*seedDir.y() + bottomPos.z()*seedDir.z(), 1.) * solCandidate.dTheta;
        ATH_MSG_VERBOSE("Test new "<<solCandidate<<".");

        using CalibSpacePointPtr = ISpacePointCalibrator::CalibSpacePointPtr;
        unsigned int nMdt{0};
        /** Collect all hits close to the seed line */
        for (const auto& [layerNr,  hitsInLayer] : Acts::enumerate(m_hitLayers.mdtHits())) {
            ATH_MSG_VERBOSE( hitsInLayer.size()<<" hits in layer "<<(layerNr +1));
            for (const HoughHitType testMe : hitsInLayer){
                CalibSpacePointPtr calibHit = m_cfg.calibrator->calibrate(ctx,testMe, seedPos, seedDir, 0.);
                if (!calibHit || calibHit->fitState() != CalibratedSpacePoint::State::Valid) {
                    continue;
                }
                const double pull = std::sqrt(SegmentFitHelpers::chiSqTermMdt(seedPos, seedDir, *calibHit, msg()));            
                ATH_MSG_VERBOSE("Test hit "<<idHelperSvc->toString(testMe->identify())
                            <<" "<<Amg::toString(testMe->positionInChamber())<<", pull: "<<pull);              
                if (pull < m_cfg.hitPullCut) {
                    solCandidate.seedHits.emplace_back(calibHit->spacePoint());
                    candidateSeed.measurements.push_back(std::move(calibHit));
                    ++nMdt;
                } 
            }
        }
        /** Reject seeds with too litle Mdt hit association */
        if (1.*nMdt < std::max(1.*m_cfg.nMdtHitCut, m_cfg.nMdtLayHitCut * m_hitLayers.mdtHits().size())) {
            return std::nullopt;
        }
        /* Calculate the left-right signs of the used hits */
        if (m_cfg.overlapCorridor) {
            solCandidate.solutionSigns = driftSigns(seedPos, seedDir, solCandidate.seedHits, msg());
            ATH_MSG_VERBOSE("Circle solutions for seed "<<idHelperSvc->toStringChamber(bottomHit->identify())<<" - "
                           <<solCandidate);
            /** Last check wheather another seed with the same left-right combination hasn't already been found */
            for (unsigned int a = m_cfg.startWithPattern; a< m_seenSolutions.size() ;++a) { 
                const SeedSolution& accepted = m_seenSolutions[a];
                unsigned int nOverlap{0};
                std::vector<int> corridor = driftSigns(seedPos, seedDir, accepted.seedHits,  msg());                
                ATH_MSG_VERBOSE("Test seed against accepted "<<accepted<<", updated signs: "<<corridor);
                /// All seed hits are of the same size
                for (unsigned int l = 0; l < accepted.seedHits.size(); ++l){
                    nOverlap  += corridor[l] == accepted.solutionSigns[l];
                }
                /// Including the places where no seed hit was assigned. Both solutions match in terms of 
                /// left-right solutions. It's very likely that they're converging to the same segment.
                if (nOverlap == corridor.size() && accepted.seedHits.size() >= solCandidate.seedHits.size()) {
                    ATH_MSG_VERBOSE("Same set of hits collected within the same corridor");
                    return std::nullopt;
                }
            }
        } else if (std::ranges::find_if(m_seenSolutions,
                        [&solCandidate] (const SeedSolution& seen) {
                            return std::abs(seen.Y0 - solCandidate.Y0) < std::hypot(seen.dY0, solCandidate.dY0) &&
                                   std::abs(seen.theta - solCandidate.theta) < std::hypot(seen.dTheta, solCandidate.dTheta);
                        }) != m_seenSolutions.end()){
            return std::nullopt;
        }
        
        /// Add the solution to the list. That we don't iterate twice over it
        m_seenSolutions.emplace_back(std::move(solCandidate));
        /** If we found a long Mdt seed, then ensure that all
         *  subsequent seeds have at least the same amount of Mdt hits. */
        if (m_cfg.tightenHitCut) {
            m_cfg.nMdtHitCut = std::max(m_cfg.nMdtHitCut, nMdt);
        }
        /** Let's find out whether they're topological connected by comparing the tube numbers
          *
         */
        
        ++m_nGenSeeds;
        
        ATH_MSG_VERBOSE("In event "<<ctx.eventID().event_number()<<" found new seed solution "<<toString(candidateSeed.parameters));
        if (m_cfg.fastSeedFit) {
            fitDriftCircles(candidateSeed);
        }

        // Combine the seed with the phi estimate
        {
            const Amg::Vector3D parDir = dirFromTangents(m_segmentSeed->tanPhi(), std::tan(theta));
            candidateSeed.parameters[toInt(ParamDefs::theta)] = parDir.theta();
            candidateSeed.parameters[toInt(ParamDefs::phi)] = parDir.phi();
        }

        /** Associate strip hits */
        {
            const auto [seedPos, seedDir] = makeLine(candidateSeed.parameters); 
            for (const std::vector<HoughHitType>& hitsInLayer : m_hitLayers.stripHits()) {
                HoughHitType bestHit{nullptr};
                double bestPull{m_cfg.hitPullCut};
                for (const HoughHitType testMe : hitsInLayer){
                    const double pull = std::sqrt(SegmentFitHelpers::chiSqTermStrip(seedPos, seedDir, *testMe, msg())) 
                                      / testMe->dimension();
                    ATH_MSG_VERBOSE("Test hit "<<idHelperSvc->toString(testMe->identify())
                                <<" "<<Amg::toString(testMe->positionInChamber())<<", pull: "<<pull);
                    /// Add all hits with a pull better than the threshold
                    if (pull <= bestPull) {
                        bestHit = testMe;
                        bestPull = pull;
                    }
                }
                if (!bestHit) {
                    continue;
                }
                candidateSeed.measurements.push_back(m_cfg.calibrator->calibrate(ctx,bestHit, seedPos, seedDir, 0));           
            }
        }
        return candidateSeed;
    }
    void MdtSegmentSeedGenerator::fitDriftCircles(DriftCircleSeed& inSeed) const {
  
        double theta = inSeed.parameters[toInt(ParamDefs::theta)];
        
        Amg::Vector3D seedDir = Amg::dirFromAngles(90.* Gaudi::Units::deg, theta);

        const double y0 = inSeed.parameters[toInt(ParamDefs::y0)] * seedDir.z();

        double norm{0.}, fitY0{0.};
        Amg::Vector3D centerOfGravity{Amg::Vector3D::Zero()};
        std::vector<int> driftSigns{};
        std::vector<double> invCovs{};
        driftSigns.reserve(inSeed.measurements.size());
        invCovs.reserve(inSeed.measurements.size());
        for (const std::unique_ptr<CalibratedSpacePoint>& hit : inSeed.measurements) {
            const double invCov = 1./ driftCov(*hit);
            norm += invCov;
            const Amg::Vector3D& pos{hit->positionInChamber()};
            centerOfGravity+= invCov * pos;
            invCovs.push_back(invCov);
            const int sign = y0  - pos.y() * seedDir.z() + pos.z()* seedDir.y() > 0 ? 1 : -1;
            fitY0 +=  invCov * sign * hit->driftRadius();
            driftSigns.push_back(sign);
        }
        /// Calculate the coefficients to minimize the chi2
        const double invNorm = 1./ norm;
        fitY0*= invNorm;
        centerOfGravity *= invNorm;

        double Tzzyy{0.}, Tyz{0.}, Trz{0.}, Try{0.};
        for (const auto&[covIdx, hit] : Acts::enumerate(inSeed.measurements)) {
            const double invCov = invCovs[covIdx]*invNorm;
            const int sign = driftSigns[covIdx];
            const Amg::Vector3D pos  = hit->positionInChamber() - centerOfGravity;
            Tzzyy += invCov * (std::pow(pos.z(), 2) - std::pow(pos.y(), 2));
            Tyz   += invCov * pos.y()*pos.z();
            Trz   += invCov * sign*pos.z() * hit->driftRadius();
            Try   += invCov * sign*pos.y() * hit->driftRadius();
        }
        /// Now it's time to use the guestimate
        const double thetaMin =  - (Tzzyy  - Try) / (4* Tyz + Trz);
        const double thetaDet =  std::pow(Tzzyy -Try,2) + 4*(Tyz + Trz)*(2*Tyz + 0.5*Trz);
        const double thetaGuess =  thetaMin  + (theta > thetaMin ? 1. : -1.)*std::sqrt(thetaDet) / (4*Tyz + Trz);
        // const double thetaGuess = std::atan2( 2.*(Tyz - Trz), Tzzyy) / 2.;

        ATH_MSG_VERBOSE("Start fast fit seed: "<<theta<<", guess: "<<thetaGuess
                    <<", y0: "<<y0<<", fitY0: "<<fitY0<<", centre: "<<Amg::toString(centerOfGravity));
        //// 
        theta = thetaGuess;
        bool converged{false};
        while (!converged && inSeed.nIter++ <= m_cfg.nMaxIter) {
            const CxxUtils::sincos twoTheta{2.*theta};
            const double thetaPrime = 0.5*Tzzyy *twoTheta.sn - Tyz * twoTheta.cs - Trz * seedDir.z() - Try * seedDir.y();
            if (std::abs(thetaPrime) < m_cfg.precCutOff){
                converged = true;
                break;
            }

            const double thetaTwoPrime =  Tzzyy * twoTheta.cs + 2* Tyz * twoTheta.sn + Trz * seedDir.y() - Try * seedDir.z();
            const double update = thetaPrime / thetaTwoPrime;
            ATH_MSG_VERBOSE("Fit iteration #"<<inSeed.nIter<<" -- theta: "<<theta<<", thetaPrime: "<<thetaPrime
                        <<", thetaTwoPrime: "<<thetaTwoPrime<<" -- "<<std::format("{:.8f}", update)
                        <<" --> next theta "<<(theta - thetaPrime / thetaTwoPrime));

            if (std::abs(update) < m_cfg.precCutOff) {
                converged = true;
                break;
            }
            theta -= update;
            const CxxUtils::sincos thetaUpdate{theta};
            seedDir.y() = thetaUpdate.sn;
            seedDir.z() = thetaUpdate.cs;
        }
        if (!converged) {
           return;
        }
        inSeed.parameters[toInt(ParamDefs::theta)] = theta;
        inSeed.parameters[toInt(ParamDefs::y0)] = (centerOfGravity.y() *seedDir.z() - centerOfGravity.z() * seedDir.y() + fitY0) / std::cos(theta);
        ATH_MSG_VERBOSE("Drift circle fit converged within "<<inSeed.nIter<<" iterations giving "<<toString(inSeed.parameters)<<", chi2: "<<inSeed.chi2);
    }
  
}
