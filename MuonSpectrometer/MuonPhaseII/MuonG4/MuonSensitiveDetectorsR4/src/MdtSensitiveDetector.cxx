/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSensitiveDetectorsR4/Utils.h"
#include "MdtSensitiveDetector.h"


#include <MCTruth/TrackHelper.h>
#include <G4Geantino.hh>
#include <G4ChargedGeantino.hh>

#include <limits>
#include <iostream>
#include <GeoPrimitives/CLHEPtoEigenConverter.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <GaudiKernel/SystemOfUnits.h>

using namespace MuonGMR4;
using namespace CxxUtils;
using namespace ActsTrk;
namespace MuonG4R4{

MdtSensitiveDetector::MdtSensitiveDetector(const std::string& name, 
                                           const std::string& output_key,
                                           const MuonGMR4::MuonDetectorManager* detMgr):
    G4VSensitiveDetector{name},
    AthMessaging{name},
    m_writeHandle{output_key},
    m_detMgr{detMgr} {

}
// Implemenation of memebr functions
void MdtSensitiveDetector::Initialize(G4HCofThisEvent*) {
  if (m_writeHandle.isValid()) {
      ATH_MSG_VERBOSE("Simulation hit container "<<m_writeHandle.fullKey()<<" is already written");
      return;
  }
  if (!m_writeHandle.recordNonConst(std::make_unique<xAOD::MuonSimHitContainer>(),
                                    std::make_unique<xAOD::MuonSimHitAuxContainer>()).isSuccess()) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to record "<<m_writeHandle.fullKey());
      throw std::runtime_error("Container saving is impossible");
  }
  ATH_MSG_DEBUG("Output container "<<m_writeHandle.fullKey()<<" has been successfully created");
}

const MuonGMR4::MdtReadoutElement* MdtSensitiveDetector::getReadoutElement(const G4TouchableHistory* touchHist) const {
   /// The third volume in the history is the volume corresponding to the Muon multilayer
   const std::string stationVolume = touchHist->GetVolume(2)->GetName();
   using namespace MuonGMR4;
   const std::vector<std::string> volumeTokens = tokenize(stationVolume, "_");
   ATH_MSG_VERBOSE("Name of the station volume is "<<stationVolume);
   if (volumeTokens.size() != 5) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Cannot deduce the station name from "<<stationVolume);
      throw std::runtime_error("Invalid station Identifier");
   }
   /// Find the Detector element from the Identifier
   const std::string stName = volumeTokens[0].substr(0,3);
   const int stationEta = atoi(volumeTokens[2]);
   const int stationPhi = atoi(volumeTokens[3]) + 1;
   const int multiLayer = atoi(volumeTokens[4]);
   const MdtIdHelper& idHelper{m_detMgr->idHelperSvc()->mdtIdHelper()};
   /// Build first the Identifier to find the detector element
   const Identifier detElId = idHelper.multilayerID(idHelper.elementID(stName,stationEta, stationPhi), multiLayer);
   /// Then retrieve the Detector element
   const MdtReadoutElement* readOutEle = m_detMgr->getMdtReadoutElement(detElId);
   if (!readOutEle) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to retrieve a valid detector element from "
                    <<m_detMgr->idHelperSvc()->toStringDetEl(detElId)<<" "<<stationVolume);
      throw std::runtime_error("Invalid detector Element");
   }
   return readOutEle;
}
 
G4bool MdtSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory* /*ROHist*/) {
    G4Track* currentTrack = aStep->GetTrack();

    // MDTs sensitive to charged particle only
    if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
      if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
      else if (currentTrack->GetDefinition() != G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
    }
 
    /// Reject secondary particles
    constexpr double velCutOff = 10.*Gaudi::Units::micrometer / Gaudi::Units::second;
    if (currentTrack->GetVelocity() < velCutOff) return true;
 
    const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(currentTrack->GetTouchable());
    const MdtReadoutElement* reEle{getReadoutElement(touchHist)};
    const Identifier HitID = getIdentifier(reEle, touchHist);
    if (!HitID.is_valid()) {
        ATH_MSG_VERBOSE("No valid hit found");
        return true;
    }

    const Amg::Transform3D& globalToLocal{reEle->globalToLocalTrans(m_gctx, reEle->measurementHash(HitID))};

    // transform pre and post step positions to local positions
    const Amg::Vector3D trackPosition{Amg::Hep3VectorToEigen(currentTrack->GetPosition())};
    const Amg::Vector3D trackDirection{Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection())};

    const Amg::Vector3D trackLocPos{globalToLocal * trackPosition};  
    const Amg::Vector3D trackLocDir{globalToLocal.linear()* trackDirection};
  
    /// Calculate the closest approach of the track w.r.t the z-axis
    const double lambda = Amg::intersect<3>(Amg::Vector3D::Zero(), Amg::Vector3D::UnitZ(),
                                       trackLocPos, trackLocDir).value_or(0);
  
    const Amg::Vector3D driftHit{trackLocPos + lambda * trackLocDir};

    const double globalTime{currentTrack->GetGlobalTime() +  lambda / currentTrack->GetVelocity()};
  
    TrackHelper trHelp{currentTrack};

    ATH_MSG_VERBOSE(" Dumping of hit "<<m_detMgr->idHelperSvc()->toString(HitID)
                  <<", barcode: "<<trHelp.GetParticleLink().barcode()
                  <<", "<<(*currentTrack)
                  <<", driftCircle: "<<Amg::toString(driftHit, 2)
                  <<", direction "<<Amg::toString(trackLocDir, 2)
                  <<" to SimHit container ahead. ");

  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);  
  hit->setIdentifier(HitID); 
  hit->setLocalPosition(xAOD::toStorage(driftHit));  
  hit->setLocalDirection(xAOD::toStorage(trackLocDir));
  hit->setStepLength(aStep->GetStepLength());
  hit->setGlobalTime(globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(currentTrack->GetKineticEnergy());
  hit->setGenParticleLink(trHelp.GetParticleLink());

  return true;
}
Identifier MdtSensitiveDetector::getIdentifier(const MuonGMR4::MdtReadoutElement* readOutEle,
                                               const G4TouchableHistory* touchHist) const {
   const Amg::Transform3D localToGlobal{getTransform(touchHist, 0)};
   /// The Geant transform takes a hit global -> local --> inverse goes back to the global system
   /// Compose this one with the global to local transformation of the first tube in the layer -->
   Amg::Vector3D refTubePos = (readOutEle->globalToLocalTrans(m_gctx, readOutEle->measurementHash(1,1)) * localToGlobal).translation();
   ATH_MSG_VERBOSE("Position of the tube wire w.r.t. the first tube in the multi layer "<<Amg::toString(refTubePos, 2));
   /// equilateral triangle
   static const double layerPitch = 1./ std::sin(60*Gaudi::Units::deg);
   const int layer = std::round(refTubePos.x() * layerPitch / readOutEle->tubePitch()) +1;
   if (layer <= 0) {
      ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" It seems that the tube position "
                            <<Amg::toString(refTubePos, 2)<<", perp: "<<refTubePos.perp()
                            <<" is outside of the volume envelope "
                            <<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())<<". ");
      throw std::runtime_error("Tube hit in Nirvana");
   }
   /// Update the reference tube position to be in the proper layer
   refTubePos  = (readOutEle->globalToLocalTrans(m_gctx, readOutEle->measurementHash(layer,1)) * localToGlobal).translation();
   const double tubePitches = refTubePos.y() / readOutEle->tubePitch();
   unsigned int tube = std::round(tubePitches) + 1;
   tube = std::max(1u, std::min(readOutEle->numTubesInLay(), tube));
   /// It can happen that the tube is assigned to zero by numerical precision
   /// Catch these cases if the layer is fine
  const Amg::Transform3D closureCheck{readOutEle->globalToLocalTrans(m_gctx, 
                                      readOutEle->measurementHash(layer, tube))*localToGlobal};
    if (!Amg::isIdentity(closureCheck)) {
        ATH_MSG_WARNING("Correction needed "<<layer<<","<<tube<<" "<<Amg::toString(closureCheck));
        if (closureCheck.translation().y() > 0) ++tube;
        else --tube;
    }
   
   const IdentifierHash tubeHash = readOutEle->measurementHash(layer, tube);
   const Identifier tubeId = readOutEle->measurementId(tubeHash);
   {
        const Amg::Transform3D closureCheck{readOutEle->globalToLocalTrans(m_gctx, tubeHash)*localToGlobal};
        if (!Amg::isIdentity(closureCheck)) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" It seems that the tube position "
                             <<Amg::toString(refTubePos, 2)<<", perp: "<<refTubePos.perp()
                             <<" is outside of the volume envelope "
                             <<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())<<". "
                             <<"Layer: "<<layer<<", tube: "<<tube<<" "
                             <<Amg::toString(closureCheck)
                             <<"tube volume : "<<touchHist->GetVolume(0)->GetName()
                             <<" mdt chamber: "<<touchHist->GetVolume(2)->GetName());
            throw std::runtime_error("Tube hit in Nirvana");
        }
   }
   ATH_MSG_VERBOSE("Tube & layer number candidate "<<tube<<", "<<layer<<" back and forth transformation "
                     <<Amg::toString(closureCheck));
   return tubeId;  
}

}