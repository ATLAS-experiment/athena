
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSensitiveDetector.h"

#include <MuonSensitiveDetectorsR4/Utils.h>
#include <GeoPrimitives/CLHEPtoEigenConverter.h>
#include <GeoModelKernel/throwExcept.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>

#include <MCTruth/TrackHelper.h>
#include <G4Geantino.hh>
#include <G4ChargedGeantino.hh>

#include "MCTruth/TrackInformation.h"

using namespace ActsTrk;

namespace {
    static const SG::Decorator<int> dec_G4TrkId{"MuonSim_G4TrkId"};
}

namespace MuonG4R4 {
    MuonSensitiveDetector::MuonSensitiveDetector(const std::string& name, 
                                                 const std::string& output_key,
                                                 const std::string& trfStore_key,
                                                 const MuonGMR4::MuonDetectorManager* detMgr):
        G4VSensitiveDetector{name},
        AthMessaging{name},
        m_writeHandle{output_key},
        m_trfCacheKey{trfStore_key},
        m_detMgr{detMgr} {
        m_trfCacheKey.initialize().ignore();
    }
    void MuonSensitiveDetector::Initialize(G4HCofThisEvent*) {
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
    ActsTrk::GeometryContext MuonSensitiveDetector::getGeoContext() const {
        ActsTrk::GeometryContext gctx{};
        SG::ReadHandle trfStoreHandle{m_trfCacheKey};
        if (!trfStoreHandle.isValid()) {
            THROW_EXCEPTION("Failed to retrieve "<<m_trfCacheKey.fullKey()<<".");
        }
        gctx.setStore(std::make_unique<DetectorAlignStore>(*trfStoreHandle));
        return gctx;
    }
    bool MuonSensitiveDetector::processStep(const G4Step* aStep) const {
        const G4Track* currentTrack = aStep->GetTrack();
        ATH_MSG_VERBOSE("Check whether step pdgId: "<<(*currentTrack)<<" will be processed.");
        /// Reject secondary particles
        constexpr double velCutOff = 10.*Gaudi::Units::micrometer / Gaudi::Units::second;
        if (aStep->GetStepLength() < std::numeric_limits<float>::epsilon() || currentTrack->GetVelocity() < velCutOff) {
            ATH_MSG_VERBOSE("Step length is too short ");
            return false;
        }
        /// Sensitive detector is only sensitive to charged particles or Geantinos
        if (currentTrack->GetDefinition()->GetPDGCharge() == 0.0) {
            ATH_MSG_VERBOSE("Particle is neutral");
            return currentTrack->GetDefinition() == G4Geantino::GeantinoDefinition() ||
                   currentTrack->GetDefinition() == G4ChargedGeantino::ChargedGeantinoDefinition();
        }
        return true;
    }
    xAOD::MuonSimHit* MuonSensitiveDetector::propagateAndSaveStrip(const Identifier& hitID,
                                                                   const Amg::Transform3D& toGasGap,
                                                                   const G4Step* aStep) {
        
        const G4Track* currentTrack = aStep->GetTrack();
        /// Fetch the step end-points
        const Amg::Vector3D locPostStep{toGasGap*Amg::Hep3VectorToEigen(aStep->GetPostStepPoint()->GetPosition())};
        Amg::Vector3D locPreStep{toGasGap*Amg::Hep3VectorToEigen(aStep->GetPreStepPoint()->GetPosition())};

        /// Hit direction as the momentum direction
        Amg::Vector3D locHitDir = toGasGap.linear() * Amg::Hep3VectorToEigen(currentTrack->GetMomentumDirection());

        /// Electrons randomly work through the gas instead of drifting through the gas -> take
        /// the prestep of the first snap shop inside the gas as pre step point
        xAOD::MuonSimHit* prevHit = lastSnapShot(hitID, aStep);
        if (std::abs(currentTrack->GetParticleDefinition()->GetPDGEncoding()) == 11 && prevHit) {
            locPreStep = xAOD::toEigen(prevHit->localPosition()) - 0.5* prevHit->stepLength() * 
                        xAOD::toEigen(prevHit->localDirection());

            locHitDir = (locPostStep - locPreStep).unit();
        }
        const Amg::Vector3D locHitPos = 0.5* (locPreStep + locPostStep);
        ATH_MSG_VERBOSE( m_detMgr->idHelperSvc()->toStringGasGap(hitID)<<" - track: "<<(*currentTrack)
                        <<", deposit: "<<aStep->GetTotalEnergyDeposit()<<", -- local coords: "
                        <<"prestep: "<<Amg::toString(locPreStep)<<",  post step: "<<Amg::toString(locPostStep) 
                        <<" mid point: "<< Amg::toString(locHitPos)<<", direction: "<<Amg::toString(locHitDir)
                        <<", deposit: "<<aStep->GetTotalEnergyDeposit());

        const double globalTime = currentTrack->GetGlobalTime() + locHitDir.dot(locPostStep - locHitPos) / currentTrack->GetVelocity();
  
        xAOD::MuonSimHit* newHit = saveHit(hitID, locHitPos, locHitDir, globalTime, aStep);
        if (prevHit) {
            newHit->setStepLength((locPostStep - locPreStep).mag());
        }
        return newHit;
    }
    xAOD::MuonSimHit* MuonSensitiveDetector::saveHit(const Identifier& hitId,
                                                     const Amg::Vector3D& hitPos,
                                                     const Amg::Vector3D& hitDir,
                                                     const double globTime,
                                                     const G4Step* aStep) {
        const G4Track* currentTrack = aStep->GetTrack();
        TrackHelper trHelper{currentTrack};
        // If Geant4 propagates the same track through the same volume just update the last hit and don't write a new one
        xAOD::MuonSimHit* hit = lastSnapShot(hitId, aStep);
        bool newHit{false};
        if (!hit) {
            hit = m_writeHandle->push_back(std::make_unique<xAOD::MuonSimHit>());
            newHit = true;
        }
        dec_G4TrkId(*hit) = currentTrack->GetTrackID();
        hit->setIdentifier(hitId); 
        hit->setLocalPosition(xAOD::toStorage(hitPos));  
        hit->setLocalDirection(xAOD::toStorage(hitDir));
        hit->setMass(currentTrack->GetDefinition()->GetPDGMass());
        hit->setGlobalTime(globTime);
        hit->setPdgId(currentTrack->GetDefinition()->GetPDGEncoding());
        hit->setEnergyDeposit(aStep->GetTotalEnergyDeposit() + (newHit ? 0. : hit->energyDeposit()));
        hit->setKineticEnergy(currentTrack->GetKineticEnergy());
        hit->setGenParticleLink(trHelper.GenerateParticleLink());
        hit->setStepLength(aStep->GetStepLength());
        
        ATH_MSG_VERBOSE("Save new hit "<<m_detMgr->idHelperSvc()->toString(hitId)
                        <<", pdgId: "<<hit->pdgId()
                        <<", "<<hit->genParticleLink()
                        <<", trackId: "<<currentTrack->GetTrackID()<<", "
                        <<", "<<hit->genParticleLink().cptr()<<std::endl
                        <<"pos: "<<Amg::toString(hitPos)<<", dir: "<<Amg::toString(hitDir)<<", time: "<<globTime
                        <<", energy: "<<hit->kineticEnergy()<<", stepLength: "<<hit->stepLength()<<", "
                        <<", deposit energy: "<<hit->energyDeposit());
        return hit;
    }
    xAOD::MuonSimHit* MuonSensitiveDetector::lastSnapShot(const Identifier& hitId,
                                                          const G4Step* hitStep) {
        TrackHelper trkHelper{hitStep->GetTrack()};
        /// There's only a snapshot if the last saved hit has the same Identifier & the same
        /// particle Link + G4Track Id
        if (m_writeHandle->empty() || 
            m_writeHandle->back()->identify() != hitId ||
            trkHelper.GenerateParticleLink() != m_writeHandle->back()->genParticleLink() ||
            dec_G4TrkId(*m_writeHandle->back()) != hitStep->GetTrack()->GetTrackID()) {
            return nullptr;
        }
        return m_writeHandle->back();
    }
}
