/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSensitiveDetectorsR4/Utils.h"

#include "MdtSensitiveDetector.h"


#include <MCTruth/TrackHelper.h>

#include <limits>
#include <iostream>
#include <GeoPrimitives/CLHEPtoEigenConverter.h>
#include <GeoModelKernel/throwExcept.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <StoreGate/ReadHandle.h>

using namespace MuonGMR4;
using namespace CxxUtils;
using namespace ActsTrk;
namespace MuonG4R4{

const MuonGMR4::MdtReadoutElement* MdtSensitiveDetector::getReadoutElement(const G4TouchableHistory* touchHist) const {
   /// The third volume in the history is the volume corresponding to the Muon multilayer
   const std::string stationVolume = touchHist->GetVolume(2)->GetName();
   using namespace MuonGMR4;
   const std::vector<std::string> volumeTokens = tokenize(stationVolume, "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<stationVolume);
   if (volumeTokens.size() != 5) {
      THROW_EXCEPTION(" Cannot deduce the station name from "<<stationVolume);
   }
   /// Find the Detector element from the Identifier
   const std::string stName = volumeTokens[0].substr(0,3);
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]);
   const int multiLayer = atoi(volumeTokens[4]);
   const MdtIdHelper& idHelper{m_detMgr->idHelperSvc()->mdtIdHelper()};
   /// Build first the Identifier to find the detector element
   const Identifier detElId = idHelper.multilayerID(idHelper.elementID(stName,stationEta, stationPhi), multiLayer);
   /// Then retrieve the Detector element
   const MdtReadoutElement* readOutEle = m_detMgr->getMdtReadoutElement(detElId);
   if (!readOutEle) {
      THROW_EXCEPTION(" Failed to retrieve a valid detector element from "
                     <<m_detMgr->idHelperSvc()->toStringDetEl(detElId)<<" "<<stationVolume);
   }
   return readOutEle;
}
 
G4bool MdtSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* /*ROHist*/) {

    /// Check whether the detector is sensitive to the traversing particle
    if (!processStep(aStep)) {
      return true;
    }
    G4Track* currentTrack = aStep->GetTrack();
    const G4StepPoint* preStep = aStep->GetPreStepPoint();
    const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(preStep->GetTouchable());
    const MdtReadoutElement* reEle{getReadoutElement(touchHist)};

    const ActsGeometryContext gctx{getGeoContext()};

  
    const Identifier HitID = getIdentifier(gctx, reEle, touchHist);
    if (!HitID.is_valid()) {
        ATH_MSG_VERBOSE("No valid hit found");
        return true;
    }

    const Amg::Transform3D globalToLocal{reEle->globalToLocalTrans(gctx, reEle->measurementHash(HitID))};

    // transform pre and post step positions to local positions
    const Amg::Vector3D prePosition{Amg::Hep3VectorToEigen(preStep->GetPosition())};
    const Amg::Vector3D postPosition{Amg::Hep3VectorToEigen(aStep->GetPostStepPoint()->GetPosition())};
    
    const Amg::Vector3D trackDirection{(postPosition - prePosition).unit()};

    const Amg::Vector3D trackLocPos{globalToLocal * prePosition};  
    const Amg::Vector3D trackLocDir{globalToLocal.linear()* trackDirection};
  
    /// Calculate the closest approach of the track w.r.t. the wire. We're starting with the post step
    /// position and if, there's a closer approach to the wire from that, the propagation distance 
    /// will be always less than zero. Otherwise one would need to go forward along the trajectory
    double lambda = Amg::intersect<3>(Amg::Vector3D::Zero(), Amg::Vector3D::UnitZ(),
                                      trackLocPos, trackLocDir).value_or(0);
    if (std::abs(currentTrack->GetDefinition()->GetPDGEncoding()) == 11) {
        lambda = std::clamp(lambda, 0., aStep->GetStepLength());
    }
    /// Closest approach of the step
    const Amg::Vector3D driftHit{trackLocPos + lambda * trackLocDir};


    const double globalTime{preStep->GetGlobalTime() +  lambda / currentTrack->GetVelocity()};

    if (std::abs(currentTrack->GetDefinition()->GetPDGEncoding()) == 11) {
        const xAOD::MuonSimHit* lastRecord = lastSnapShot(HitID, aStep);
        if (lastRecord && lastRecord->localPosition().perp() < driftHit.perp()) {
            return true;
        }
    }
  
    TrackHelper trkHelp{currentTrack};

    ATH_MSG_VERBOSE(" Dumping of hit "<<m_detMgr->idHelperSvc()->toString(HitID)
                  <<", barcode: "<<trkHelp.GenerateParticleLink().barcode()
                  <<", "<<(*currentTrack) <<", driftCircle: "<<Amg::toString(driftHit, 4)
                  <<", direction "<<Amg::toString(trackLocDir, 4) <<" to SimHit container ahead. ");
    saveHit(HitID, driftHit, trackLocDir, globalTime, aStep);
    return true;
}
Identifier MdtSensitiveDetector::getIdentifier(const ActsGeometryContext& gctx,
                                               const MuonGMR4::MdtReadoutElement* readOutEle,
                                               const G4TouchableHistory* touchHist) const {
   const Amg::Transform3D localToGlobal{getTransform(touchHist, 0)};
   /// The Geant transform takes a hit global -> local --> inverse goes back to the global system
   /// Compose this one with the global to local transformation of the first tube in the layer -->
   Amg::Vector3D refTubePos = (readOutEle->globalToLocalTrans(gctx, readOutEle->measurementHash(1,1)) * localToGlobal).translation();
   ATH_MSG_VERBOSE("Position of the tube wire w.r.t. the first tube in the multi layer "<<Amg::toString(refTubePos, 2));
   /// equilateral triangle
   static const double layerPitch = 1./ std::sin(60*Gaudi::Units::deg);
   const int layer = std::round(refTubePos.x() * layerPitch / readOutEle->tubePitch()) +1;
   if (layer <= 0) {
      THROW_EXCEPTION("Tube hit in nirvana -- It seems that the tube position "
                      <<Amg::toString(refTubePos, 2)<<", perp: "<<refTubePos.perp()
                      <<" is outside of the volume envelope "
                      <<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())<<". ");      
   }
   /// Update the reference tube position to be in the proper layer
   refTubePos  = (readOutEle->globalToLocalTrans(gctx, readOutEle->measurementHash(layer,1)) * localToGlobal).translation();
   const double tubePitches = refTubePos.y() / readOutEle->tubePitch();
   unsigned int tube = std::round(tubePitches) + 1;
   tube = std::max(1u, std::min(readOutEle->numTubesInLay(), tube));
   /// It can happen that the tube is assigned to zero by numerical precision
   /// Catch these cases if the layer is fine
  const Amg::Transform3D closureCheck{readOutEle->globalToLocalTrans(gctx, 
                                      readOutEle->measurementHash(layer, tube))*localToGlobal};
    if (!Amg::isIdentity(closureCheck)) {
        ATH_MSG_WARNING("Correction needed "<<layer<<","<<tube<<" "<<Amg::toString(closureCheck));
        if (closureCheck.translation().y() > 0) ++tube;
        else --tube;
    }
   
   const IdentifierHash tubeHash = readOutEle->measurementHash(layer, tube);
   const Identifier tubeId = readOutEle->measurementId(tubeHash);
   {
        const Amg::Transform3D closureCheck{readOutEle->globalToLocalTrans(gctx, tubeHash)*localToGlobal};
        if (!Amg::isIdentity(closureCheck)) {
            THROW_EXCEPTION("Tube hit in Nirvana --  It seems that the tube position "
                             <<Amg::toString(refTubePos, 2)<<", perp: "<<refTubePos.perp()
                             <<" is outside of the volume envelope "
                             <<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())<<". "
                             <<"Layer: "<<layer<<", tube: "<<tube<<" "
                             <<Amg::toString(closureCheck)
                             <<"tube volume : "<<touchHist->GetVolume(0)->GetName()
                             <<" mdt chamber: "<<touchHist->GetVolume(2)->GetName());    
        }
   }
   ATH_MSG_VERBOSE("Tube & layer number candidate "<<tube<<", "<<layer<<" back and forth transformation "
                     <<Amg::toString(closureCheck));
   return tubeId;  
}

}