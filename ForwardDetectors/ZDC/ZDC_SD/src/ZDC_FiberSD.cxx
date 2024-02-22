/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Class header
#include "ZDC_FiberSD.h"

// CLHEP headers
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

//Geant4 headers
#include "G4ParticleTypes.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpProcessSubType.hh"

ZDC_FiberSD::ZDC_FiberSD(const G4String &name, const G4String &hitCollectionName, const float &readoutPos)
    : G4VSensitiveDetector(name), m_HitColl(hitCollectionName), m_readoutPos(readoutPos)
{
    
}


ZDC_FiberSD::~ZDC_FiberSD(){
    
}

void ZDC_FiberSD::Initialize(G4HCofThisEvent *)
{
}

/*
 * The goal of process hits for this SD is to find optical photons that are
 * totally internally reflected on the side walls the optical fiber, while not
 * also being reflected back down when it reaches the top of the fiber.
 * This is done on the track's first step by looking at the PostStepProcessVector
 * for the TotalInternalReflection optical process
*/
G4bool ZDC_FiberSD::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
    G4ThreeVector pos = aStep->GetTrack()->GetPosition();
    G4ThreeVector momentum = aStep->GetPreStepPoint()->GetMomentum();

    /*************************************************
    * Reject everything but optical photons
    **************************************************/
    if (aStep->GetTrack()->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()){
        return true;
    }


    /*************************************************
    * Reject downward going photons
    **************************************************/
    if (momentum.y() < 0.0){
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return true;
    }


    /*************************************************
    * Reject photons not at the boundary of two volumes
    **************************************************/
    G4StepPoint *endPoint = aStep->GetPostStepPoint();
    if (endPoint->GetStepStatus() != fGeomBoundary)
        return true;

    
    /*************************************************
    * Reject photons that are reflected back down 
    * from the top of the fiber
    **************************************************/

    // Get the refractive index
    G4MaterialPropertiesTable *MPT = aStep->GetPreStepPoint()->GetMaterial()->GetMaterialPropertiesTable();
    if (!MPT) //Safety check
        return true;
    G4MaterialPropertyVector *RindexMPV = MPT->GetProperty(kRINDEX);
    if (!RindexMPV)//Safety check
        return true;

    G4double Rindex;
    size_t index = 0;
    double photonEnergy = aStep->GetTrack()->GetDynamicParticle()->GetTotalMomentum();
    Rindex = RindexMPV->Value(photonEnergy, index);

    //Determine the photon's angle from the vertical axis
    G4ThreeVector momDir = aStep->GetPreStepPoint()->GetMomentumDirection();
    G4double angleFromY = atan(sqrt(1 - pow(momDir.y(), 2.0)) / momDir.y());

    // kill the photon if angle is greater than TIR critical angle
    // We're assuming the fiber's top face's normal vector is parallel to the y axis
    if (angleFromY > asin(1.0 / Rindex)){ 
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return true;
    }

    /*************************************************
    * Reject photons that are not totally internally 
    * reflected inside the fiber
    **************************************************/

    G4bool isTIR = false;
    G4ProcessVector *postStepDoItVector = G4OpticalPhoton::OpticalPhotonDefinition()->GetProcessManager()->GetPostStepProcessVector(typeDoIt);
    G4int n_proc = postStepDoItVector->entries();
    for (G4int i = 0; i < n_proc; ++i){
        G4OpBoundaryProcess *opProc = dynamic_cast<G4OpBoundaryProcess *>((*postStepDoItVector)[i]);
        if (opProc && opProc->GetStatus() == TotalInternalReflection){
            isTIR = true;
            break;
        }
    }

    if(!isTIR){ 
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return true;
    }

    /*************************************************
    * Simulate absorption by calculating the photon's
    * remaining path length and rolling a random number
    * against its absorption chance
    **************************************************/
    G4MaterialPropertyVector *AbsMPV = MPT->GetProperty(kABSLENGTH);
    G4double Absorption = AbsMPV->Value(aStep->GetTrack()->GetDynamicParticle()->GetTotalMomentum(), index);

    G4double pathLength = (m_readoutPos - pos.y()) / cos(angleFromY);
    G4double absChance = 1 - exp(-pathLength / Absorption);

    // This check amounts to if(absorbed)
    if (CLHEP::RandFlat::shoot(0.0, 1.0) < absChance){
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return true;
    }

    /*************************************************
    * Record the survivors
    **************************************************/

    //Get the hash for this volume to keep track of hits
    Identifier id;
    id = aStep->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo();
    uint32_t hash = id.get_identifier32().get_compact();

    std::map<uint32_t,ZDC_SimFiberHit*>::iterator it = m_hitMap.find(hash);

    if(it == m_hitMap.end()){
        //This is a new hit
        ZDC_SimFiberHit *hit = new ZDC_SimFiberHit(id, 1, photonEnergy);
        m_hitMap.insert(std::pair<uint32_t,ZDC_SimFiberHit*>(hash,hit));
    }else{
        it->second->Add(1, photonEnergy);
    }

    /*************************************************
    * Put the survivors out of their misery
    **************************************************/
    aStep->GetTrack()->SetTrackStatus(fStopAndKill);
    return true;
}

void ZDC_FiberSD::EndOfAthenaEvent()
{
    
    //Move the hits from the hit set to the hit container
    if (!m_HitColl.isValid())
        m_HitColl = std::make_unique<ZDC_SimFiberHit_Collection>(m_HitColl.name());

    for(auto hit : m_hitMap){
        m_HitColl->Emplace(*(hit.second));
    }


    if (verboseLevel > 5){
        G4cout << "ZDC_FiberSD::EndOfAthenaEvent(): Printing Final Energy(eV) deposited in Fibers " << G4endl;
        
        int photonCount = 0;
        float energyTotal = 0;
        for(auto hit : m_hitMap){
            photonCount += hit.second->getNPhotons();
            energyTotal += hit.second->getEdep();
        }

        G4cout << "ZDC_FiberSD::EndOfAthenaEvent(): Final Energy(eV) deposited in Fiber "
            << energyTotal << " ev and Number of Photons deposited = " << photonCount 
            << " across " << m_hitMap.size() << " volumes" << G4endl;

    }
    //Reset hit container
    m_hitMap.clear();

    return;
}
