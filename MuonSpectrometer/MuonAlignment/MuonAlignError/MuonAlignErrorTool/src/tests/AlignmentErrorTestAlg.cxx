#include "AlignmentErrorTestAlg.h"

#include <format>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MuonAlignErrorBase/AlignmentRotationDeviation.h"
#include "MuonAlignErrorBase/AlignmentTranslationDeviation.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkToolInterfaces/ITrkAlignmentDeviationTool.h"

namespace MuonAlign {

AlignmentErrorTestAlg::AlignmentErrorTestAlg(const std::string& name,
                                             ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) {}

StatusCode AlignmentErrorTestAlg::initialize() {
  ATH_CHECK(m_alignmentErrorTool.retrieve());
  ATH_CHECK(m_idTool.retrieve());
  ATH_CHECK(m_trackCollection.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode AlignmentErrorTestAlg::execute() {
  const EventContext& ctx = getContext();
  SG::ReadHandle<::TrackCollection> trackCollection{m_trackCollection, ctx};
  if (!trackCollection.isValid()) {
    throw std::runtime_error("Cannot retrieve track collection");
  }

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
  if (eventInfo.isValid()) {
    ATH_MSG_INFO("Reading event " << eventInfo->eventNumber()
                 << " from run " << eventInfo->runNumber()
                 << " with timestamp " << eventInfo->timeStamp());
  }

  ATH_MSG_INFO("Retrieved track collection with " << trackCollection->size()
                                                  << " entries");

  auto getListOfHits = [](const Trk::Track* track) -> std::vector<const Trk::RIO_OnTrack*> {
    std::vector<const Trk::RIO_OnTrack*> ret;
    for (const auto *tsos : *(track->trackStateOnSurfaces())) {
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
      ret.emplace_back(rot);
    }
    return ret;
  };

  auto makeChamberString = [&](const std::vector<const Trk::RIO_OnTrack*>& hits) -> std::string {
    std::set<std::string> chambers;
    for (const auto* rot : hits) {
      MuonCalib::MuonFixedLongId calibId =
        m_idTool->idToFixedLongId(rot->identify());
      if (calibId.isValid()) {
        chambers.emplace(calibId.stationNameString());
      }
    }
    std::string chambers_str;
    bool first = true;
    for (const auto& str : chambers) {
      if (!first) {
        chambers_str.append(",");
      }
      chambers_str.append(str);
      first = false;
    }
    return chambers_str;
  };

  int itrack = 0;
  for (const Trk::Track* track : *trackCollection) {
    ATH_MSG_INFO("Track " << itrack++);
    ATH_MSG_INFO("Collected chambers: " << makeChamberString(getListOfHits(track)));
    std::vector<Trk::AlignmentDeviation*> deviations;
    m_alignmentErrorTool->makeAlignmentDeviations(*track, deviations);
    for (const Trk::AlignmentDeviation* np : deviations) {
      const auto& hits = np->getListOfHits();
      if (const auto* tdev =
              dynamic_cast<const MuonAlign::AlignmentTranslationDeviation*>(np)) {
        Amg::Vector3D u = tdev->getU();
        double sigma = tdev->getSigma();
        ATH_MSG_INFO(
            std::format("TranslationNP U = {:14.6f} {:14.6f} {:14.6f}"
                        " sigma = {:8.6f} chambers = {}",
                        u.x(), u.y(), u.z(), sigma, makeChamberString(hits)));
      } else if (const auto* rdev =
                     dynamic_cast<const MuonAlign::AlignmentRotationDeviation*>(np)) {
        Amg::Vector3D c = rdev->getCenter();
        Amg::Vector3D a = rdev->getAxis();
        double sigma = rdev->getSigma();
        ATH_MSG_INFO(std::format(
            "RotationNP    C = {:14.6f} {:14.6f} {:14.6f} "
            "A = {:14.6f} {:14.6f} {:14.6f} sigma = {:11.9f} chambers = {}",
            c.x(), c.y(), c.z(), a.x(), a.y(), a.z(), sigma, makeChamberString(hits)));
      } else {
        throw std::runtime_error("Deviation type is not implemented");
      }
    }
  }

  return StatusCode::SUCCESS;
}

}  // namespace MuonAlign
