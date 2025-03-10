/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcSensitiveDetector.h"

#include "MuonSensitiveDetectorsR4/Utils.h"
#include "G4ThreeVector.hh"

#include "MCTruth/TrackHelper.h"
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/throwExcept.h"


using namespace MuonGMR4;
using namespace CxxUtils;

namespace {
   constexpr double tolerance = 10. * Gaudi::Units::micrometer;
}

// construction/destruction
namespace MuonG4R4 {


G4bool sTgcSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*) {
  if (!processStep(aStep)) {
    return false;
  }

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  
  const ActsGeometryContext gctx{getGeoContext()};

  const MuonGMR4::sTgcReadoutElement* readOutEle = getReadoutElement(gctx, touchHist);

  const Amg::Transform3D localToGlobal = getTransform(touchHist, 0);
  ATH_MSG_VERBOSE(" Track is inside volume "
                 <<touchHist->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<Amg::toString(localToGlobal));

  const Identifier etaHitID = getIdentifier(gctx, readOutEle, localToGlobal.translation(), 
                                            sTgcIdHelper::sTgcChannelTypes::Strip);
  if (!etaHitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  /// Fetch the local -> global transformation  
  const Amg::Transform3D toGasGap{readOutEle->globalToLocalTrans(gctx, etaHitID)};
  propagateAndSaveStrip(etaHitID, toGasGap, aStep);
  return true;
}

Identifier sTgcSensitiveDetector::getIdentifier(const ActsGeometryContext& gctx,
                                                const MuonGMR4::sTgcReadoutElement* readOutEle, 
                                                const Amg::Vector3D& hitAtGapPlane, 
                                                sTgcIdHelper::sTgcChannelTypes chType) const {

  const sTgcIdHelper& idHelper{m_detMgr->idHelperSvc()->stgcIdHelper()};
  const Identifier firstChan = idHelper.channelID(readOutEle->identify(),
                                                  readOutEle->multilayer(), 1, chType, 1);
  
  const Amg::Vector3D locHitPos{readOutEle->globalToLocalTrans(gctx, firstChan) * hitAtGapPlane};
  ATH_MSG_VERBOSE("Detector element: "<<m_detMgr->idHelperSvc()->toStringDetEl(firstChan)
                <<" locPos: "<<Amg::toString(locHitPos, 2)
                <<" gap thickness "<<readOutEle->gasGapPitch()
                <<" gasGap: "<< (std::abs(locHitPos.z()) /  readOutEle->gasGapPitch()) + 1);
  
  const int gasGap = std::round(std::abs(locHitPos.z()) /  readOutEle->gasGapPitch()) + 1;
  return idHelper.channelID(readOutEle->identify(),readOutEle->multilayer(), gasGap, chType, 1);
}
const MuonGMR4::sTgcReadoutElement* sTgcSensitiveDetector::getReadoutElement(const ActsGeometryContext& gctx,
                                                                             const G4TouchableHistory* touchHist) const {
   
   
   /// The fourth volume is the envelope volume of the NSW station. It will tell us the sector and station eta
   const std::string& stationVolume = touchHist->GetVolume(3)->GetName();
   ///      av_4368_impr_1_MuonR4::NSW_QS3_StationMuonStation_pv_9_NSW_QS3_Station_-3_1
   const std::vector<std::string> volumeTokens = tokenize(stationVolume.substr(stationVolume.rfind("Q")), "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<volumeTokens);
   if (volumeTokens.size() != 4) {
      THROW_EXCEPTION("Cannot deduce the station name from "<<stationVolume);
   }
   /// Find the Detector element from the Identifier  
   const std::string stName = volumeTokens[0][1] == 'S' ? "STS" : "STL";
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]);

   const sTgcIdHelper& idHelper{m_detMgr->idHelperSvc()->stgcIdHelper()};
   const Identifier detElIdMl1 = idHelper.channelID(idHelper.stationNameIndex(stName), stationEta, stationPhi, 1, 1,
                                                    sTgcIdHelper::sTgcChannelTypes::Strip, 1);
   const Identifier detElIdMl2 = idHelper.multilayerID(detElIdMl1, 2);
   const sTgcReadoutElement* readOutElemMl1 = m_detMgr->getsTgcReadoutElement(detElIdMl1);
   const sTgcReadoutElement* readOutElemMl2 = m_detMgr->getsTgcReadoutElement(detElIdMl2);
   if (!readOutElemMl1 || !readOutElemMl2) {
      THROW_EXCEPTION("Failed to retrieve a valid detector element from "
                    <<m_detMgr->idHelperSvc()->toStringDetEl(detElIdMl1)<<" "<<stationVolume);
   }
   /// retrieve the translation of the transformation going into the current current gasVolume
   const Amg::Vector3D transformCenter = getTransform(touchHist, 0).translation();
   /// Let's use the position of the first gasGap in the second quad as a reference. If the
   /// absolute z value is smaller than its z value the hit must be located in quad number one
   const Amg::Vector3D centerMl2 = readOutElemMl2->center(gctx, detElIdMl2);
   return std::abs(centerMl2.z())  - tolerance <= std::abs(transformCenter.z()) ? readOutElemMl2 : readOutElemMl1;
}
}