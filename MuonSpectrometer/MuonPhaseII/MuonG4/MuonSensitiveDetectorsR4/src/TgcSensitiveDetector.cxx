/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcSensitiveDetector.h"
#include "MuonSensitiveDetectorsR4/Utils.h"

#include "G4ThreeVector.hh"

#include "MCTruth/TrackHelper.h"
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/throwExcept.h"

using namespace ActsTrk;

// construction/destruction
namespace MuonG4R4 {

G4bool TgcSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*) {

    if (!processStep(aStep)) {
      return true;
    }
    
    const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(aStep->GetPreStepPoint()->GetTouchable());
    
    const MuonGMR4::TgcReadoutElement* readOutEle = getReadoutElement(touchHist);
    if (!readOutEle) {
       return false;
    }
    const ActsTrk::GeometryContext gctx{getGeoContext()};
    
    const Amg::Transform3D localToGlobal = getTransform(touchHist, 0);
    ATH_MSG_VERBOSE(" Track is inside volume "
                   <<touchHist->GetHistory()->GetTopVolume()->GetName()
                   <<" transformation: "<<Amg::toString(localToGlobal));
  
    const Identifier etaHitID = getIdentifier(gctx, readOutEle, localToGlobal.translation(), false);
    if (!etaHitID.is_valid()) {
        ATH_MSG_VERBOSE("No valid hit found");
        return true;
    }
    /// Fetch the local -> global transformation  
    const Amg::Transform3D toGasGap{readOutEle->globalToLocalTrans(gctx, etaHitID)};
    propagateAndSaveStrip(etaHitID, toGasGap, aStep);   
    return true;
}
const MuonGMR4::TgcReadoutElement* TgcSensitiveDetector::getReadoutElement(const G4TouchableHistory* touchHist) const {
    /// The third volume encodes the information of the Tgc detector
    const std::string stationVolume = touchHist->GetVolume(2)->GetName();
    const std::vector<std::string> volumeTokens = CxxUtils::tokenize(stationVolume, "_");
    /// We should have a string which kind of looks like
    ///     av_319_impr_1_MuonR4::LogVolMuonStation_pv_190_T3E_Station2_-2_46
    /// Of interest are only the T1E part and the last 2 numbers
    const size_t nTokens{volumeTokens.size()};
    const TgcIdHelper& idHelper{m_detMgr->idHelperSvc()->tgcIdHelper()};
    const std::string stationName{volumeTokens[nTokens-4].substr(0,3)};
    const int stationEta = CxxUtils::atoi(volumeTokens[nTokens-2]);
    const int stationPhi = CxxUtils::atoi(volumeTokens[nTokens-1]);
    bool isValid{false};
    const Identifier stationId = idHelper.elementID(stationName, stationEta, stationPhi,  isValid);
    if (!isValid) {
       THROW_EXCEPTION("Failed to deduce station name from "+stationVolume);
    }
    return m_detMgr->getTgcReadoutElement(stationId);
}

Identifier TgcSensitiveDetector::getIdentifier(const ActsTrk::GeometryContext& gctx,
                                               const MuonGMR4::TgcReadoutElement* readOutEle, 
                                               const Amg::Vector3D& hitAtGapPlane, bool phiGap) const {
    const TgcIdHelper& idHelper{m_detMgr->idHelperSvc()->tgcIdHelper()};
    const Identifier firstChan = idHelper.channelID(readOutEle->identify(), 1, phiGap, 1);
 
    const Amg::Vector3D locHitPos{readOutEle->globalToLocalTrans(gctx, firstChan) * hitAtGapPlane};   
  
    const int gasGap = std::round(std::abs(locHitPos.z()) /  readOutEle->gasGapPitch()) + 1;
    ATH_MSG_VERBOSE("Detector element: "<<m_detMgr->idHelperSvc()->toStringDetEl(firstChan)
                  <<" locPos: "<<Amg::toString(locHitPos, 2)
                  <<" gap thickness: "<<readOutEle->gasGapPitch()
                  <<" gasGap: "<<gasGap);

    return idHelper.channelID(readOutEle->identify(), gasGap, phiGap, 1);
}

}
