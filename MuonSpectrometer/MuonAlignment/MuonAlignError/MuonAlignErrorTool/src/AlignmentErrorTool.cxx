/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////
////  Authors: Pierre-Francois Giraud, Camilla Maiani
////  Contacts: pierre-francois.giraud@cern.ch, cmaiani@cern.ch
////////////////////////////////////////////////////////////////////

#include "AlignmentErrorTool.h"

#include <boost/functional/hash.hpp>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonAlignErrorBase/AlignmentRotationDeviation.h"
#include "MuonAlignErrorBase/AlignmentTranslationDeviation.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/Track.h"

namespace MuonAlign {

AlignmentErrorTool::AlignmentErrorTool(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t, n, p) {
    declareInterface<Trk::ITrkAlignmentDeviationTool>(this);
}

StatusCode AlignmentErrorTool::initialize() {
    ATH_MSG_INFO("*****************************************");
    ATH_MSG_INFO("AlignmentErrorTool::initialize()");

    ATH_CHECK(m_idTool.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_readKey.initialize());

    return StatusCode::SUCCESS;
}

void AlignmentErrorTool::makeAlignmentDeviations(const Trk::Track& track, std::vector<Trk::AlignmentDeviation*>& deviations) const {
    ATH_MSG_DEBUG("AlignmentErrorTool::makeAlignmentDeviations()");

    SG::ReadCondHandle<MuonAlignmentErrorData> readHandle{m_readKey};
    const MuonAlignmentErrorData* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("nullptr to the read conditions object");
        return;
    }
    const auto& deviationVec = readCdo->getAlignmentErrorRules();
    std::vector<deviationSummary_t> devSumVec;
    devSumVec.reserve(deviationVec.size());
    deviationSummary_t aDevSumm;
    for (const auto & i : deviationVec) {
        aDevSumm.translation = i.translation;
        aDevSumm.rotation = i.rotation;
        aDevSumm.stationName = i.stationName;
        aDevSumm.multilayer = i.multilayer;
        devSumVec.emplace_back(std::move(aDevSumm));
    }

    const auto& MuonAlignmentErrorRuleCacheVec = readCdo->getMuonAlignmentErrorRuleCache();
    const auto& map_struct = MuonAlignmentErrorRuleCacheVec[0];

    typedef Trk::TrackStates tsosc_t;
    const tsosc_t* tsosc = track.trackStateOnSurfaces();

    // LOOP ON HITS ON TRACK //
    unsigned int nPrecisionHits = 0;
    for (const auto *tsos : *tsosc) {
        if (!tsos->type(Trk::TrackStateOnSurface::Measurement)) {
            continue;
        }
        const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
        const auto* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);

        if (!rot) {
            const auto* crot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(meas);
            if (crot) {
                unsigned int index = crot->indexOfMaxAssignProb();
                rot = &(crot->rioOnTrack(index));
            }
        }
        if (!rot) continue;

        Identifier channelId = rot->identify();
        if (!m_idHelperSvc->isMuon(channelId)) {
            // the RIO_OnTrack Identifiers could also come from ID or Calo, but this tool is only interested in MS hits
            ATH_MSG_VERBOSE("Given Identifier " << channelId.get_compact() << " is not a muon Identifier, continuing");
            continue;
        }

        // Keep only the precision coordinate hits
        if (m_idHelperSvc->isRpc(channelId)
            || m_idHelperSvc->isTgc(channelId)
            || (m_idHelperSvc->isCsc(channelId) && m_idHelperSvc->cscIdHelper().measuresPhi(channelId) == 1)
            || (m_idHelperSvc->issTgc(channelId) && m_idHelperSvc->stgcIdHelper().channelType(channelId) != sTgcIdHelper::sTgcChannelTypes::Strip)) {
            continue;
        }

        // To maintain backward compatibility with the old error CLOBs, activate
        // the NSW hits only if specified in the CLOB, else disccard them
        if (!readCdo->hasNswHits()
            && (m_idHelperSvc->issTgc(channelId) || m_idHelperSvc->isMM(channelId))) {
            continue;
        }

        MuonCalib::MuonFixedLongId calibId = m_idTool->idToFixedLongId(channelId);
        if (!calibId.isValid()) continue;

        ATH_MSG_DEBUG("Hit is in station " << calibId << " (MuonFixedLongId)");
        ++nPrecisionHits;

        // Compute deviationSummary_t building blocks
        const Trk::PrepRawData* prd = rot->prepRawData();
        const Trk::Surface& sur = prd->detectorElement()->surface(prd->identify());

        double w2 = 1.0 / (rot->localCovariance()(Trk::loc1, Trk::loc1));
        Amg::Vector3D hitP = tsos->trackParameters()->position();
        Amg::Vector3D hitU = tsos->trackParameters()->momentum().unit();

        // Wire direction for MDT, strip direction for MM or sTGC
        int icol = (calibId.is_mdt()||calibId.is_csc()) ? 2 : 1;
        Amg::Vector3D hitV = sur.transform().rotation().col(icol);

        // Enforce orientation of the V vectors
        static const Amg::Vector3D zATLAS(0., 0., 1.);
        if (hitP.cross(zATLAS).dot(hitV)<0.0) {
            hitV *= -1.0;
        }

        // FOR CROSS-CHECK
        bool is_matched = false;

        // Construct detector element identifier from channelId, remaining only chamber name and multilayer information
        int multilayer = 1;
        if (calibId.is_mdt()) {
            multilayer = calibId.mdtMultilayer();
        } else if (calibId.is_mmg()) {
            multilayer = calibId.mmgMultilayer();
        } else if (calibId.is_stg()) {
            multilayer = calibId.stgMultilayer();
        }

        Identifier key_id{};
        if (m_idHelperSvc->isMdt(channelId)){
            key_id = m_idHelperSvc->mdtIdHelper().channelID(channelId, multilayer, 1, 1);
        }
        if (m_idHelperSvc->isRpc(channelId)){
            key_id = m_idHelperSvc->rpcIdHelper().elementID(channelId);
        }
        if (m_idHelperSvc->isMM(channelId)){
            key_id = m_idHelperSvc->mmIdHelper().channelID(channelId, multilayer, 1, 1);
        }
        if (m_idHelperSvc->issTgc(channelId)){
            key_id = m_idHelperSvc->stgcIdHelper().channelID(channelId, multilayer, 1, 0, 1);
        }
        if (m_idHelperSvc->isCsc(channelId)){
            key_id = m_idHelperSvc->cscIdHelper().elementID(channelId);
        }
        if (m_idHelperSvc->isTgc(channelId)){
            key_id = m_idHelperSvc->tgcIdHelper().elementID(channelId);
        }

        // Find deviation (pointer) for current detector element
        auto range = map_struct.id_rule_map.equal_range(key_id);
        for (auto i = range.first; i != range.second; ++i){
            MuonAlignmentErrorData::MuonAlignmentErrorRuleIndex rule_idx = i->second;

            // ASSOCIATE EACH NUISANCE TO A LIST OF HITS
            devSumVec[rule_idx].hits.push_back(rot);

            devSumVec[rule_idx].sumW2 += w2;
            devSumVec[rule_idx].sumP += w2 * hitP;
            devSumVec[rule_idx].sumU += w2 * hitU;
            devSumVec[rule_idx].sumV += w2 * hitV;

            // FOR CROSS-CHECK
            is_matched = true;


        }  // LOOP ON DEVIATIONS

        if (!is_matched) {
            ATH_MSG_WARNING("The hits in the station " << calibId << " (MuonFixedLongId)" 
                            << " couldn't be matched to any deviation regexp in the list.");
        }

    }  // LOOP ON TSOS

    // Nuisance parameters covering the complete track are not wanted. (MS/ID
    // error treated differently for now). Removing the deviations covering the
    // full track in further processing.
    for (auto& dev : devSumVec) {
        if (dev.hits.size() == nPrecisionHits) {
            dev.hits.clear();
        }
    }

    // GET RID OF PERFECT OVERLAPS BY COMBINING ERRORS
    std::vector<const Trk::RIO_OnTrack*> v1, v2;
    for (auto iti = devSumVec.begin(); iti != devSumVec.end(); ++iti) {

        v1 = iti->hits;
        if (v1.empty()) {
            continue;
        }

        std::stable_sort(v1.begin(), v1.end());

        for (auto itj = iti+1; itj != devSumVec.end(); ++itj) {

            if (iti->hits.size() != itj->hits.size()) {
                continue;
            }

            v2 = itj->hits;
            std::stable_sort(v2.begin(), v2.end());

            if (v1 == v2) {
                auto iDev = std::distance(devSumVec.begin(), iti);
                auto jDev = std::distance(devSumVec.begin(), itj);
                ATH_MSG_DEBUG("Found deviations " << iDev << " and " << jDev << " related to the same list of hits. Merging...");
                ATH_MSG_DEBUG("old (translation, rotation) systematic uncertainties for "
                              << iDev << ": " << iti->translation << ", " << iti->rotation);
                ATH_MSG_DEBUG("old (translation, rotation) systematic uncertainties for "
                              << jDev << ": " << itj->translation << ", " << itj->rotation);

                // MERGE THE TWO DEVIATIONS ASSOCIATED TO THE SAME LIST OF HITS //
                double new_translation = std::hypot(iti->translation, itj->translation);
                double new_rotation = std::hypot(iti->rotation, itj->rotation);

                // NOW PREPARE TO ERASE ONE OF THE TWO COPIES //
                itj->hits.clear();

                // ASSIGN NEW TRASLATION/ROTATION TO THE REMAINING COPY //
                iti->translation = new_translation;
                iti->rotation = new_rotation;
                ATH_MSG_DEBUG("New combined (translation, rotation) systematic uncertainties: " << new_translation << ", " << new_rotation);

            }  // FIND AN OVERLAP IN THE HITS LISTS
        }      // SECOND LOOP ON DEVIATIONS
    }          // FIRST LOOP ON DEVIATIONS


    // NOW BUILD THE DEVIATIONS
    deviations.clear();
    ATH_MSG_DEBUG("************************************");
    ATH_MSG_DEBUG("FINAL LIST OF DEVIATIONS");
    for (const auto & iDev : devSumVec) {
        if (iDev.hits.empty()) {
            continue;
        }

        double rotation = iDev.rotation;
        double translation = iDev.translation;

        Amg::Vector3D sumP = iDev.sumP;
        Amg::Vector3D sumU = iDev.sumU;
        Amg::Vector3D sumV = iDev.sumV;
        double sumW2 = iDev.sumW2;

        sumP *= (1. / sumW2);
        sumU *= (1. / sumW2);
        sumV *= (1. / sumW2);

        if (translation >= 0.001 * Gaudi::Units::mm) {
            std::size_t hitshash = 0;
            for (const auto *it : iDev.hits) {
                boost::hash_combine(hitshash, (it->identify()).get_compact());
            }
            deviations.push_back(
                new AlignmentTranslationDeviation(sumU.cross(sumV), translation * Gaudi::Units::mm, iDev.hits));
            deviations.back()->setHashOfHits(hitshash);

            ATH_MSG_DEBUG("A translation along ("
                          << sumU.x() << ", " << sumU.y() << ", " << sumU.z() << ") with sigma=" << translation * Gaudi::Units::mm
                          << " mm was applied to " << iDev.hits.size()
                          << " hits matching the station: " << iDev.stationName.str() << " and the multilayer "
                          << iDev.multilayer.str());
        }
        if (rotation >= 0.000001 * Gaudi::Units::rad) {
            std::size_t hitshash = 0;
            for (const auto *it : iDev.hits) {
                boost::hash_combine(hitshash, (it->identify()).get_compact());
            }
            deviations.push_back(new AlignmentRotationDeviation(sumP, sumV, rotation * Gaudi::Units::rad, iDev.hits));
            deviations.back()->setHashOfHits(hitshash);

            ATH_MSG_DEBUG("A rotation around the center = (" << sumP.x() << ", " << sumP.y() << ", " << sumP.z() << ") and axis = ("
                                                             << sumV.x() << ", " << sumV.y() << ", " << sumV.z()
                                                             << ") with sigma=" << rotation / Gaudi::Units::mrad << " mrad was applied to "
                                                             << iDev.hits.size() << " hits matching the station "
                                                             << iDev.stationName.str() << " and the multilayer "
                                                             << iDev.multilayer.str());
        }

    }  // LOOP ON NUISANCES

    ATH_MSG_DEBUG("******************************");
    ATH_MSG_DEBUG("FINAL CHECKUP");
    ATH_MSG_DEBUG("Found " << deviations.size() << " nuisances after duplicates merging");
    ATH_MSG_DEBUG("******************************");
}

}  // namespace MuonAlign
