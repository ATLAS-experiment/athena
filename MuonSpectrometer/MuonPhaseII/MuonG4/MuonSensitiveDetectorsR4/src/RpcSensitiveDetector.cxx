/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcSensitiveDetector.h"
#include "MuonSensitiveDetectorsR4/Utils.h"
#include "G4ThreeVector.hh"
#include "G4Trd.hh"
#include "G4Geantino.hh"
#include "G4ChargedGeantino.hh"

#include "MCTruth/TrackHelper.h"
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/throwExcept.h"

using namespace MuonGMR4;
using namespace CxxUtils;
using namespace ActsTrk;

// construction/destruction
namespace MuonG4R4 {


G4bool RpcSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*) {

  if (!processStep(aStep)) {
      return true;
  }
  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
  const MuonGMR4::RpcReadoutElement* readOutEle = getReadoutElement(touchHist);
  if (!readOutEle) {
      return false;
  }

  const ActsGeometryContext gctx{getGeoContext()};

  const Amg::Transform3D localToGlobal = getTransform(touchHist, 0);
  ATH_MSG_VERBOSE(" Track is inside volume "
                 <<touchHist->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<Amg::toString(localToGlobal));
  /** The Rpc readout may have two strip panels per a single gasGap --> 
   *  extrapolate the step towards the gap centre */
  const Amg::Vector3D locPreStep{localToGlobal.inverse()*Amg::Hep3VectorToEigen(aStep->GetPreStepPoint()->GetPosition())};
  const Amg::Vector3D locPostStep{localToGlobal.inverse()*Amg::Hep3VectorToEigen(aStep->GetPostStepPoint()->GetPosition())};
  
  const Amg::Vector3D locStepDir = (locPostStep - locPreStep).unit();
  const std::optional<double> lambda = Amg::intersect<3>(locPreStep, locStepDir, Amg::Vector3D::UnitX(), 0.);
  Amg::Vector3D gapCentreCross =  localToGlobal * ( (lambda ? 1. : 0.) *locPreStep + lambda.value_or(0.) * locStepDir);
  const Identifier etaHitID = getIdentifier(gctx, readOutEle, gapCentreCross, false);
  if (!etaHitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  /// Fetch the local -> global transformation  
  const Amg::Transform3D toGasGap{readOutEle->globalToLocalTrans(gctx, etaHitID)};
  propagateAndSaveStrip(etaHitID, toGasGap, aStep);
  return true;
}

Identifier RpcSensitiveDetector::getIdentifier(const ActsGeometryContext& gctx,
                                               const MuonGMR4::RpcReadoutElement* readOutEle, 
                                               const Amg::Vector3D& hitAtGapPlane, bool phiGap) const {
  const RpcIdHelper& idHelper{m_detMgr->idHelperSvc()->rpcIdHelper()};

  const Identifier firstChan = idHelper.channelID(readOutEle->identify(),
                                                  readOutEle->doubletZ(),
                                                  readOutEle->doubletPhi(), 1, phiGap, 1);
  
  const Amg::Vector3D locHitPos{readOutEle->globalToLocalTrans(gctx, firstChan) * 
                                hitAtGapPlane};
  const double gapHalfWidth = readOutEle->stripEtaLength() / 2;
  const double gapHalfLength = readOutEle->stripPhiLength()/ 2;
  ATH_MSG_VERBOSE("Detector element: "<<m_detMgr->idHelperSvc()->toStringDetEl(firstChan)
                <<" locPos: "<<Amg::toString(locHitPos, 2)
                <<" gap thickness "<<readOutEle->gasGapPitch()
                <<" gap width: "<<gapHalfWidth
                <<" gap length: "<<gapHalfLength);
  const int doubletPhi = std::abs(locHitPos.y()) > gapHalfWidth ? readOutEle->doubletPhiMax() :
                                                                  readOutEle->doubletPhi();
  const int gasGap = std::round(std::abs(locHitPos.z()) /  readOutEle->gasGapPitch()) + 1;

  return idHelper.channelID(readOutEle->identify(),
                            readOutEle->doubletZ(),
                            doubletPhi, gasGap, phiGap, 1);
}
const MuonGMR4::RpcReadoutElement* RpcSensitiveDetector::getReadoutElement(const G4TouchableHistory* touchHist) const {
  /// The fourth volume is the envelope volume of the rpc gas gap
   const std::string stationVolume = touchHist->GetVolume(3)->GetName();
   
   const std::vector<std::string> volumeTokens = tokenize(stationVolume, "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<stationVolume);
   if (volumeTokens.size() != 7) {
      THROW_EXCEPTION(" Cannot deduce the station name from "<<stationVolume);
   }
   /// Find the Detector element from the Identifier
    ///       <STATIONETA>_<STATIONPHI>_<DOUBLETR>_<DOUBLETPHI>_<DOUBLETZ>
   const std::string stName = volumeTokens[0].substr(0,3);
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]);
   const int doubletR = atoi(volumeTokens[4]);
   const int doubletPhi = atoi(volumeTokens[5]);
   const int doubletZ = atoi(volumeTokens[6]);
   const RpcIdHelper& idHelper{m_detMgr->idHelperSvc()->rpcIdHelper()};

   const Identifier detElId = idHelper.padID(idHelper.stationNameIndex(stName), 
                                             stationEta, stationPhi, doubletR, doubletZ, doubletPhi);
   const RpcReadoutElement* readOutElem = m_detMgr->getRpcReadoutElement(detElId);
   if (!readOutElem) {
      THROW_EXCEPTION(" Failed to retrieve a valid detector element from "
                    <<m_detMgr->idHelperSvc()->toStringDetEl(detElId)<<" "<<stationVolume);
   }
   return readOutElem;
}
}
