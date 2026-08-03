/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TriggerChamberClusterOnTrackCreator.h"

#include <functional>
#include <ranges>

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

namespace Muon {

StatusCode TriggerChamberClusterOnTrackCreator::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_clusterCreator.retrieve());
    return StatusCode::SUCCESS;
}

std::unique_ptr<CompetingMuonClustersOnTrack>
TriggerChamberClusterOnTrackCreator::createBroadCluster(
    const std::list<const Trk::PrepRawData*>& prdList, const double) const {
    ATH_MSG_VERBOSE("enter createBroadCluster: number of prds " << prdList.size());

    // make some PRD consistency checks
    if (prdList.empty()) {
        ATH_MSG_WARNING("fails: empty PRD list ");
        return nullptr;
    }
    std::vector<const Trk::PrepRawData*> prds{prdList.begin(), prdList.end()};
    const Trk::TrkDetElementBase* detectorElement = prds.front()->detectorElement();
    Identifier channelId = (*prds.front()).identify();
    const bool isRpc = m_idHelperSvc->isRpc(channelId);
    const bool isTgc = m_idHelperSvc->isTgc(channelId);
    if (!isRpc && !isTgc) {
        ATH_MSG_WARNING("fails: PRD must be from rpc or tgc ");
        return nullptr;
    }

    const bool measuresPhi = m_idHelperSvc->measuresPhi(channelId);
    for (const Trk::PrepRawData* prd : prds) {
        channelId = prd->identify();
        if (m_idHelperSvc->measuresPhi(channelId) != measuresPhi) {
            ATH_MSG_WARNING("fails: PRDs must measure same coordinate ");
            return nullptr;
        }
        if (prd->detectorElement() != detectorElement) {
            ATH_MSG_WARNING("fails: PRDs must be from same detectorElement ");
            return nullptr;
        }
        ATH_MSG_VERBOSE("Create trigger prd from "
                        << m_idHelperSvc->toString(channelId)
                        << ", nDim: " << prd->localCovariance().rows());
    }

    // create a rot for each prd (which gets weight zero)
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> rots = createPrdRots(prds);
    auto assocProbs = std::vector<double>(rots.size(), 0.);

    // for each surface, find the first and last rot forming the cluster
    std::list<int> limitingChannels;
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> limitingRots;
    makeClustersBySurface(limitingChannels, limitingRots, prds, rots);

    // cluster consistency - discard any surfaces not contributing to the final cluster
    applyClusterConsistency(limitingChannels, limitingRots);

    // overall localPosition, error matrix and surface
    Trk::LocalParameters parameters{};
    Amg::MatrixX errorMatrix{};
    std::unique_ptr<Trk::Surface> surface{};
    makeOverallParameters(parameters, errorMatrix, surface, limitingChannels,
                          limitingRots);

    // clear lists
    limitingChannels.clear();

    // return the competingMuonClusterOnTrack object containing the final parameters,
    // error matrix, surface, list of rots and weights
    return std::make_unique<CompetingMuonClustersOnTrack>(
        std::move(parameters), std::move(errorMatrix), surface.release(),
        std::move(rots), std::move(assocProbs));
}

void TriggerChamberClusterOnTrackCreator::applyClusterConsistency(
    std::list<int>& limitingChannels,
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& limitingRots) const {
    // remove any clusters that will NOT contribute to the final cluster
    int numClusters = limitingChannels.size() / 2;
    int sizeMax = 0;
    int sizeMin = 999;
    for (std::list<int>::iterator l = limitingChannels.begin();
         l != limitingChannels.end() &&
         l != std::prev(limitingChannels.end());) {
        int end = *l++;
        int beg = *l++;
        int size = std::abs(end - beg);
        sizeMax = std::max(sizeMax, size);
        sizeMin = std::min(sizeMin, size);
    }

    std::list<int>::iterator discard = limitingChannels.end();
    for (std::list<int>::iterator l = limitingChannels.begin();
         l != limitingChannels.end() &&
         l != std::prev(limitingChannels.end());) {
        std::list<int>::iterator first = l;
        int end = *l++;
        int beg = *l++;
        int size = std::abs(end - beg);
        if (m_chooseBroadestCluster && size < sizeMax) {
            discard = first;
        }
        if (!m_chooseBroadestCluster && size > sizeMin) {
            discard = first;
        }
    }
    if (discard == limitingChannels.begin()) {
        ATH_MSG_VERBOSE(" discard cluster #" << 1);
        limitingRots.erase(limitingRots.begin());
        limitingRots.erase(limitingRots.begin());
        limitingChannels.pop_front();
        limitingChannels.pop_front();
    } else if (discard != limitingChannels.end()) {
        ATH_MSG_VERBOSE(" discard cluster #" << numClusters);
        limitingRots.pop_back();
        limitingRots.pop_back();
        limitingChannels.pop_back();
        limitingChannels.pop_back();
    }
}

std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>
TriggerChamberClusterOnTrackCreator::createPrdRots(
    std::vector<const Trk::PrepRawData*>& prds) const {
    // create clusterRot for each PRD
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> rots{};
    if (prds.empty()) {
        ATH_MSG_WARNING("empty PRD list ");
        return rots;
    }
    std::optional<int> dim{};
    for (const Trk::PrepRawData* prd : prds) {
        Identifier id = prd->identify();
        const Trk::TrkDetElementBase* detectorElement = prd->detectorElement();
        const Amg::Vector3D globalPosition = detectorElement->center(id);
        std::unique_ptr<const Muon::MuonClusterOnTrack> cluster{
            m_clusterCreator->createRIO_OnTrack(*prd, globalPosition)};
        if (!cluster) {
            ATH_MSG_WARNING("Cannot create a ROT from "
                            << m_idHelperSvc->toString(id) << ".");
            continue;
        }
        if (!dim) {
            dim = cluster->localCovariance().cols();
        } else if ((*dim) != cluster->localCovariance().cols()) {
            ATH_MSG_WARNING("The covariance dimension of "<<m_idHelperSvc->toString(id)
                            <<" does not match "<<(*dim));
            continue;
        }
        rots.push_back(std::move(cluster));
    }
    if (rots.size() != prds.size()) {
        auto [begin, end] = std::ranges::remove_if(prds, [&](const Trk::PrepRawData* prd){
            return std::ranges::none_of(rots, [prd](const std::unique_ptr<const Muon::MuonClusterOnTrack>& rot){
                return rot->identify() == prd->identify();
            });
        });
        prds.erase(begin, end);
    }
    return rots;
}

void TriggerChamberClusterOnTrackCreator::makeClustersBySurface(
    std::list<int>& limitingChannels,
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& limitingRots,
    const std::vector<const Trk::PrepRawData*>& prds,
    const std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& rots) const {
    if (prds.empty()) {
        ATH_MSG_WARNING("makeClustersBySurface- empty PRD list ");
        return;
    }
    std::unordered_set<const Trk::PrepRawData*> usedPrd;
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>::const_iterator r = rots.begin();
    for (std::vector<const Trk::PrepRawData*>::const_iterator p = prds.begin();
         p != prds.end(); ++p, ++r) {

        const Trk::PrepRawData* prd{*p};
        if (!usedPrd.insert(prd).second) {
            continue;
        }
        int channel = 0;
        int gasGap = 0;
        const Identifier channelId = prd->identify();
        const bool isRpc = m_idHelperSvc->isRpc(channelId);
        if (isRpc) {
            gasGap = m_idHelperSvc->rpcIdHelper().gasGap(channelId);
            channel = m_idHelperSvc->rpcIdHelper().strip(channelId);
        } else {
            gasGap = m_idHelperSvc->tgcIdHelper().gasGap(channelId);
            channel = m_idHelperSvc->tgcIdHelper().channel(channelId);
        }
        int channelMax = channel;
        int channelMin = channel;
        const Muon::MuonClusterOnTrack *rotMax{r->get()}, *rotMin{r->get()};
        std::vector<const Trk::PrepRawData*>::const_iterator q = p;
        std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>::const_iterator s = r;
        for (++q, ++s; q != prds.end(); ++q, ++s) {
            const Identifier channelId1 = (**q).identify();
            if ((isRpc && m_idHelperSvc->rpcIdHelper().gasGap(channelId1) != gasGap) ||
                (!isRpc && m_idHelperSvc->tgcIdHelper().gasGap(channelId1) != gasGap)) {
                continue;
            }
            usedPrd.insert(*q);
            if (isRpc) {
                channel = m_idHelperSvc->rpcIdHelper().strip(channelId1);
            } else {
                channel = m_idHelperSvc->tgcIdHelper().channel(channelId1);
            }
            if (channel > channelMax) {
                channelMax = channel;
                rotMax = s->get();
            }
            if (channel < channelMin) {
                channelMin = channel;
                rotMin = s->get();
            }
        }
        limitingChannels.push_back(channelMin);
        limitingChannels.push_back(channelMax);
        limitingRots.emplace_back(rotMin->clone());
        limitingRots.emplace_back(rotMax->clone());
    }
    ATH_MSG_VERBOSE("makeClustersBySurface - " << limitingChannels.size()
                                               << ", " << limitingRots.size());
}

void TriggerChamberClusterOnTrackCreator::makeOverallParameters(
    Trk::LocalParameters& parameters, Amg::MatrixX& errorMatrix,
    std::unique_ptr<Trk::Surface>& surface, std::list<int>& limitingChannels,
    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>& limitingRots) const {

    std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>>::const_iterator r = limitingRots.begin();
    Amg::Vector3D centre		= (**r).associatedSurface().center();
    Amg::MatrixX covariance	= (**r).localCovariance();
    parameters				= Trk::LocalParameters{(**r).localParameters()};
    const bool isRpc = m_idHelperSvc->isRpc((**r).identify());
    
   
    for (++r;
	 r != limitingRots.end();
	 ++r)
    {
      centre		+= (**r).associatedSurface().center();
      covariance	+= (**r).localCovariance();
      parameters	+= (**r).localParameters();
    }
    const double norm			= 1. /static_cast<double>(limitingRots.size());
    std::list<int>::iterator l	= limitingChannels.begin();
    int firstChannel   		= *l;
    double width		= static_cast<double>(1 + std::abs(*(++l) - firstChannel));
    if (limitingRots.size() > 2)
    {
      int offset = std::abs(*(++l) - firstChannel);
      if (!isRpc && offset < 2) {
        width *= 0.5;
      } else {
        width += static_cast<double>(offset);
      }
    }

    // get parameter means
    centre	*= norm;
    covariance	*= width*width*norm;
    parameters	*= norm;

    ATH_MSG_VERBOSE("Final parameters "<<m_idHelperSvc->toString((*limitingRots.begin())->identify())<<", centre: "<<Amg::toString(centre)<<", "
                    <<", covariance: "<<covariance<<", parameters: "<<parameters<<" , limiting ROTs: "<<limitingRots.size());
    // finally create the mean ErrorMatrix and the average Surface
    // note the cluster surfaces are assumed to have identical orientation and bounds
    errorMatrix = Amg::MatrixX(covariance);
    const Trk::Surface& surf = (**limitingRots.begin()).associatedSurface();

    if (const auto* rectbds =
            dynamic_cast<const Trk::RectangleBounds*>(&surf.bounds())) {
        surface = std::make_unique<Trk::PlaneSurface>(
            surf.transform(), std::make_unique<Trk::RectangleBounds>(*rectbds));
    } else if (const auto* trapbds =
                   dynamic_cast<const Trk::TrapezoidBounds*>(&surf.bounds())) {
        surface = std::make_unique<Trk::PlaneSurface>(
            surf.transform(), std::make_unique<Trk::TrapezoidBounds>(*trapbds));
    } else if (const auto* rottrapbds =
                   dynamic_cast<const Trk::RotatedTrapezoidBounds*>(
                       &surf.bounds())) {
        surface = std::make_unique<Trk::PlaneSurface>(
            surf.transform(),
            std::make_unique<Trk::RotatedTrapezoidBounds>(*rottrapbds));
    }
}

}  // namespace Muon
