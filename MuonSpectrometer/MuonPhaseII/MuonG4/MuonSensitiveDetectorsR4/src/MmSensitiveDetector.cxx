/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MmSensitiveDetector.h"
#include "MuonSensitiveDetectorsR4/Utils.h"
#include "G4ThreeVector.hh"
#include "G4Trd.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

#include "MCTruth/TrackHelper.h"
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"
#include "GeoModelKernel/throwExcept.h"
#include "GaudiKernel/SystemOfUnits.h"


using namespace MuonGMR4;
using namespace CxxUtils;
using namespace ActsTrk;

namespace {
   constexpr double tolerance = 10. * Gaudi::Units::micrometer;
}

// construction/destruction
namespace MuonG4R4 {

G4bool MmSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*) {

  if (!processStep(aStep)) {
    return true;
  }
  const ActsTrk::GeometryContext gctx{getGeoContext()};

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  const MuonGMR4::MmReadoutElement* readOutEle = getReadoutElement(gctx, touchHist);

  const Amg::Transform3D localToGlobal = getTransform(touchHist, 0);
  ATH_MSG_VERBOSE(" Track is inside volume "<< touchHist->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<Amg::toString(localToGlobal));

  const Identifier hitID = getIdentifier(gctx, readOutEle, localToGlobal.translation());
  if (!hitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  /// Fetch the local -> global transformation  
  const Amg::Transform3D toGasGap{readOutEle->globalToLocalTrans(gctx, hitID)};
  propagateAndSaveStrip(hitID, toGasGap, aStep);
  return true;
}

Identifier MmSensitiveDetector::getIdentifier(const ActsTrk::GeometryContext& gctx,
                                              const MuonGMR4::MmReadoutElement* readOutEle, 
                                              const Amg::Vector3D& hitAtGapPlane) const {
  /// that's the poor man's solution to find out in which gas gap we're
  for (unsigned int gap = 1; gap <= readOutEle->nGasGaps(); ++gap){
     const Amg::Vector3D gapCentre = readOutEle->center(gctx, MmReadoutElement::createHash(gap, 0));
     ATH_MSG_VERBOSE("Try to match "<<Amg::toString(hitAtGapPlane)<<" to "<<Amg::toString(gapCentre)
                  <<" in "<<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())<<" dZ: "
                  <<std::abs(gapCentre.z() - hitAtGapPlane.z()));
     if (std::abs(gapCentre.z() - hitAtGapPlane.z()) < tolerance) {
         ATH_MSG_VERBOSE("Assign hit "<<Amg::toString(hitAtGapPlane)<<" to "
                        <<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())<<" gasGap: "<<gap);
         return readOutEle->measurementId(MmReadoutElement::createHash(gap, 1));
     }
  }
  THROW_EXCEPTION("Invalid gasgap matching for hit "<<Amg::toString(hitAtGapPlane)<<" and detector element "
                  <<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify()));
  return Identifier{};
}
const MuonGMR4::MmReadoutElement* MmSensitiveDetector::getReadoutElement(const ActsTrk::GeometryContext& gctx,
                                                                         const G4TouchableHistory* touchHist) const {
   /// The fourth volume is the envelope volume of the NSW station. It will tell us the sector and station eta
   const std::string& stationVolume = touchHist->GetVolume(4)->GetName();
   ///      av_4375_impr_1_MuonR4::NSW_SM2_StationMuonStation_pv_9_NSW_SM2_Station_-2_1
   const std::vector<std::string> volumeTokens = tokenize(stationVolume.substr(stationVolume.rfind("NSW") + 4), "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<volumeTokens);
   if (volumeTokens.size() != 4) {
      THROW_EXCEPTION(" Cannot deduce the station name from "<<stationVolume);
   }
   /// Find the Detector element from the Identifier  
   const std::string stName = volumeTokens[0][0] == 'S' ? "MMS" : "MML";
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]);

   const MmIdHelper& idHelper{m_detMgr->idHelperSvc()->mmIdHelper()};
   const Identifier detElIdMl1 = idHelper.channelID(idHelper.stationNameIndex(stName), stationEta, stationPhi, 1, 1, 1);
   const Identifier detElIdMl2 = idHelper.multilayerID(detElIdMl1, 2);
   const MmReadoutElement* readOutElemMl1 = m_detMgr->getMmReadoutElement(detElIdMl1);
   const MmReadoutElement* readOutElemMl2 = m_detMgr->getMmReadoutElement(detElIdMl2);
   if (!readOutElemMl1 || !readOutElemMl2) {
      THROW_EXCEPTION(" Failed to retrieve a valid detector element from "
                    <<m_detMgr->idHelperSvc()->toStringDetEl(detElIdMl1)<<" "<<stationVolume);    
   }
   /// retrieve the translation of the transformation going into the current current gasVolume
   const Amg::Vector3D transformCenter = getTransform(touchHist, 0).translation();
   /// Let's use the position of the first gasGap in the second quad as a reference. If the
   /// absolute z value is smaller than its z value the hit must be located in quad number one
   const Amg::Vector3D centerMl2 = readOutElemMl2->center(gctx, detElIdMl2);
   ATH_MSG_VERBOSE("Local gap position "<<Amg::toString(centerMl2)<<" transform center "<<Amg::toString(transformCenter));
   return std::abs(centerMl2.z())  - tolerance <= std::abs(transformCenter.z()) ? readOutElemMl2 : readOutElemMl1;
}
}