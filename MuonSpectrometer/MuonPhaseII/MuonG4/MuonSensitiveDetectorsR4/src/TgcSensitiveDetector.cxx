
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcSensitiveDetector.h"
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

using namespace ActsTrk;

namespace {
   constexpr double tolerance = 10. * Gaudi::Units::micrometer;
}
// construction/destruction
namespace MuonG4R4 {

TgcSensitiveDetector::TgcSensitiveDetector(const std::string& name, 
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

void TgcSensitiveDetector::Initialize(G4HCofThisEvent*) {
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

G4bool TgcSensitiveDetector::ProcessHits(G4Step* aStep, G4TouchableHistory*) {

    G4Track* currentTrack = aStep->GetTrack();
    
    // TGC sensitive to charged particle only
    if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
      if (currentTrack->GetDefinition()!= G4Geantino::GeantinoDefinition()) return true;
      else if (currentTrack->GetDefinition()==G4ChargedGeantino::ChargedGeantinoDefinition()) return true;
    }

    /// Reject secondary particles
    constexpr double velCutOff = 10.*Gaudi::Units::micrometer / Gaudi::Units::second;
    if (currentTrack->GetVelocity() < velCutOff) return true;

    const G4TouchableHistory* touchHist = static_cast<const G4TouchableHistory*>(currentTrack->GetTouchable());
    
    const MuonGMR4::TgcReadoutElement* readOutEle = getReadoutElement(touchHist);
    if (!readOutEle) {
       return false;
    }
    ActsGeometryContext gctx{};

    SG::ReadHandle<DetectorAlignStore> trfStoreHandle{m_trfCacheKey};
    if (!trfStoreHandle.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve "<<m_trfCacheKey.fullKey()<<".");
      return false;
    }
    gctx.setStore(std::make_unique<DetectorAlignStore>(*trfStoreHandle));
    
    const Amg::Transform3D globalToLocal = getTransform(currentTrack->GetTouchable(), 0).inverse();
    ATH_MSG_VERBOSE(" Track is inside volume "
                 <<currentTrack->GetTouchable()->GetHistory()->GetTopVolume()->GetName()
                 <<" transformation: "<<Amg::toString(globalToLocal));
    const Amg::Vector3D locPos{globalToLocal*Amg::Hep3VectorToEigen(currentTrack->GetPosition())};
    const Amg::Vector3D locDir{globalToLocal.linear()*Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection())};
    ATH_MSG_VERBOSE("Track is located in "<<m_detMgr->idHelperSvc()->toStringDetEl(readOutEle->identify())
                  <<" at "<<Amg::toString(locPos)<<" pointing towards "<<Amg::toString(locDir, 3));

    /// The x-axis of the gas gap coordinate system points to the outer big wheel -> extrapolate into the plane X = 0.
    std::optional<double> travelDist = Amg::intersect<3>(locPos, locDir, Amg::Vector3D::UnitX(), 0);
    if (!travelDist) {
      ATH_MSG_VERBOSE("Hit cannot be extrapolated into the gas gap origin");
      return true;
    }
    const Amg::Vector3D lPosAtGap = locPos + (*travelDist) * locDir;
    ATH_MSG_VERBOSE("Extrpolated by "<<(*travelDist)<<" mm to the gasGap center "<<Amg::toString(lPosAtGap, 2));
    
    const Amg::Vector3D gapCenterCross = globalToLocal.inverse() * lPosAtGap;
    const Identifier etaHitID = getIdentifier(gctx, readOutEle, gapCenterCross, false);
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
        ATH_MSG_FATAL("The hit "<<Amg::toString(locHitPos)<<" doest not match "<<m_detMgr->idHelperSvc()->toString(etaHitID));
        throw std::runtime_error("Picked wrong gas gap");
    }

    /// At the very last clear the cache of the readout element
    readOutEle->releaseUnAlignedTrfs();


    xAOD::MuonSimHit* hit = new xAOD::MuonSimHit();
    m_writeHandle->push_back(hit);  
  
    TrackHelper trHelp(aStep->GetTrack());
    hit->setIdentifier(etaHitID); 
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

Identifier TgcSensitiveDetector::getIdentifier(const ActsGeometryContext& gctx,
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