/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

MmSensitiveDetector::MmSensitiveDetector(const std::string& name, 
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

void MmSensitiveDetector::Initialize(G4HCofThisEvent*) {
  if (m_writeHandle.isValid()) {
      ATH_MSG_VERBOSE("Simulation hit container "<<m_writeHandle.fullKey()<<" is already written");
      return;
  }
  if (!m_writeHandle.recordNonConst(std::make_unique<xAOD::MuonSimHitContainer>(),
                                    std::make_unique<xAOD::MuonSimHitAuxContainer>()).isSuccess()) {
      THROW_EXCEPTION("Failed to record "<<m_writeHandle.fullKey());
  }
  ATH_MSG_DEBUG("Output container "<<m_writeHandle.fullKey()<<" has been successfully created");
}

G4bool MmSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*) {

  G4Track* currentTrack = aStep->GetTrack();
  // MDTs sensitive to charged particle only
  if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
    if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
    else if (currentTrack->GetDefinition()==G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
  }

  /// Reject secondary particles
  constexpr double velCutOff = 10.*Gaudi::Units::micrometer / Gaudi::Units::second;
  if (currentTrack->GetVelocity() < velCutOff) return true;

  ActsGeometryContext gctx{};

  SG::ReadHandle<DetectorAlignStore> trfStoreHandle{m_trfCacheKey};
  if (!trfStoreHandle.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve "<<m_trfCacheKey.fullKey()<<".");
    return false;
  }
  gctx.setStore(std::make_unique<DetectorAlignStore>(*trfStoreHandle));

  const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(currentTrack->GetTouchable());
  const MuonGMR4::MmReadoutElement* readOutEle = getReadoutElement(gctx, touchHist);
  

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
  std::optional<double> travelDist = Amg::intersect<3>(localPos, localDir, Amg::Vector3D::UnitX(), 0.);
  if (!travelDist) return true;
  const Amg::Vector3D locGapCross = localPos + (*travelDist) * localDir;
  ATH_MSG_VERBOSE("Propagation to the gas gap center: "<<Amg::toString(locGapCross, 2));
  const Amg::Vector3D gapCenterCross = globalToLocal.inverse() * locGapCross;

  const Identifier hitID = getIdentifier(gctx, readOutEle, gapCenterCross);
  if (!hitID.is_valid()) {
      ATH_MSG_VERBOSE("No valid hit found");
      return true;
  }
  const double globalTime = currentTrack->GetGlobalTime() + (*travelDist) / currentTrack->GetVelocity();
  const Amg::Transform3D gapTrans{readOutEle->globalToLocalTrans(gctx, hitID)};
  const Amg::Vector3D locHitDir = gapTrans.linear() * Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection());
  const Amg::Vector3D locHitPos = gapTrans * gapCenterCross;

  xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
  m_writeHandle->push_back(hit);  
  
  TrackHelper trHelp(aStep->GetTrack());
  hit->setIdentifier(hitID); 
  hit->setLocalPosition(xAOD::toStorage(locHitPos));  
  hit->setLocalDirection(xAOD::toStorage(locHitDir));
  hit->setMass(currentTrack->GetDefinition()->GetPDGMass());
  hit->setGlobalTime(globalTime);
  hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
  hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit());
  hit->setKineticEnergy(currentTrack->GetKineticEnergy());
  hit->setGenParticleLink(trHelp.GenerateParticleLink());
  return true;
}

Identifier MmSensitiveDetector::getIdentifier(const ActsGeometryContext& gctx,
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
const MuonGMR4::MmReadoutElement* MmSensitiveDetector::getReadoutElement(const ActsGeometryContext& gctx,
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