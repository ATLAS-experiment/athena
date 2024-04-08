/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcSensitiveDetector.h"
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
#include "GeoModelHelpers/throwExcept.h"


using namespace MuonGMR4;
using namespace CxxUtils;
using namespace ActsTrk;

namespace {
   constexpr double tolerance = 10. * Gaudi::Units::micrometer;
}

// construction/destruction
namespace MuonG4R4 {

sTgcSensitiveDetector::sTgcSensitiveDetector(const std::string& name, 
                                             const std::string& output_key,
                                             const std::string& trf_storeKey,
                                             const MuonGMR4::MuonDetectorManager* detMgr):
    G4VSensitiveDetector{name},
    AthMessaging{name},
    m_writeHandle{output_key},
    m_trfCacheKey{trf_storeKey},
    m_detMgr{detMgr} {
      m_trfCacheKey.initialize().ignore();
}

void sTgcSensitiveDetector::Initialize(G4HCofThisEvent*) {
  if (m_writeHandle.isValid()) {
      ATH_MSG_VERBOSE("Simulation hit container "<<m_writeHandle.fullKey()<<" is already written");
      return;
  }
  if (!m_writeHandle.recordNonConst(std::make_unique<xAOD::MuonSimHitContainer>(),
                                    std::make_unique<xAOD::MuonSimHitAuxContainer>()).isSuccess()) {
      THROW_EXCEPTION(" Failed to record "<<m_writeHandle.fullKey());
  }
  ATH_MSG_DEBUG("Output container "<<m_writeHandle.fullKey()<<" has been successfully created");
}

G4bool sTgcSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*) {


  G4Track* currentTrack = aStep->GetTrack();

  // MDTs sensitive to charged particle only
  if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
    if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
    else if (currentTrack->GetDefinition()==G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
  }

  /// Reject secondary particles
  constexpr double velCutOff = 10.*Gaudi::Units::micrometer / Gaudi::Units::second;
  if (currentTrack->GetVelocity() < velCutOff) return true;

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(currentTrack->GetTouchable());
  
  ActsGeometryContext gctx{};

  SG::ReadHandle<DetectorAlignStore> trfStoreHandle{m_trfCacheKey};
  if (!trfStoreHandle.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve "<<m_trfCacheKey.fullKey()<<".");
    return false;
  }
  gctx.setStore(std::make_unique<DetectorAlignStore>(*trfStoreHandle));


  const MuonGMR4::sTgcReadoutElement* readOutEle = getReadoutElement(gctx, touchHist);
  


  const Amg::Transform3D globalToLocal = getTransform(touchHist, 0).inverse();
  ATH_MSG_VERBOSE(" Track is inside volume "
                 << touchHist->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<Amg::toString(globalToLocal));
  // transform pre and post step positions to local positions
  
  const Amg::Vector3D localPos{globalToLocal*Amg::Hep3VectorToEigen(currentTrack->GetPosition())};
  const Amg::Vector3D localDir{globalToLocal.linear()*Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection())};
  ATH_MSG_VERBOSE("Entry / exit point in "<<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())
               <<" "<<Amg::toString(localPos, 2)<<" / "<<Amg::toString(localDir, 2));
  
  
  // The middle of the gas gap is at X= 0
  std::optional<double> travelDist = Amg::intersect<3>(localPos, localDir, Amg::Vector3D::UnitZ(), 0.);
  if (!travelDist) return true;
  const Amg::Vector3D locGapCross = localPos + (*travelDist) * localDir;
  ATH_MSG_VERBOSE("Propagation to the gas gap center: "<<Amg::toString(locGapCross, 2));
  const Amg::Vector3D gapCenterCross = globalToLocal.inverse() * locGapCross;

  const Identifier etaHitID = getIdentifier(gctx, readOutEle, 
                                            gapCenterCross, sTgcIdHelper::Strip);
  if (!etaHitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  const double globalTime = currentTrack->GetGlobalTime() + (*travelDist) / currentTrack->GetVelocity();
  const Amg::Transform3D gapTrans{readOutEle->globalToLocalTrans(gctx, etaHitID)};
  const Amg::Vector3D locHitDir = gapTrans.linear() * Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection());
  const Amg::Vector3D locHitPos = gapTrans * gapCenterCross;
  /// Final check that the hit is located at zero
  if (std::abs(locHitPos.z()) > tolerance) {
      THROW_EXCEPTION("The hit "<<Amg::toString(locHitPos)<<" doest not match "<<m_detMgr->idHelperSvc()->toString(etaHitID)
                      <<"Wrong gas gap picked");
  }

  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);  
  
  TrackHelper trHelp(aStep->GetTrack());
  hit->setIdentifier(etaHitID); 
  hit->setLocalPosition(xAOD::toStorage(locHitPos));  
  hit->setLocalDirection(xAOD::toStorage(locHitDir));
  hit->setStepLength(aStep->GetStepLength());
  hit->setGlobalTime(globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(currentTrack->GetKineticEnergy());
  hit->setGenParticleLink(trHelp.GetParticleLink());
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
   const int stationPhi = atoi(volumeTokens[3]) + 1;

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