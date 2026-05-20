/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include <utility>

#include "LArG4Code/LArG4SimpleSD.h"

#include "LArG4Code/ILArCalculatorSvc.h"
#include "LArG4Code/LArHitContainerBuilder.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/LArID_Exception.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "StoreGate/StoreGateSvc.h"

#include "G4EventManager.hh"
#include "HitManagement/HitCollectionMap.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

#include "G4Step.hh"
#ifdef DEBUG_IDENTIFIERS
#include "G4ThreeVector.hh"
#endif

LArG4SimpleSD::LArG4SimpleSD(G4String a_name, ILArCalculatorSvc* calc,
                             std::string hitCollectionName,
                             const std::string& type, const float width)
  : G4VSensitiveDetector(std::move(a_name))
  , m_calculator(calc)
  , m_numberInvalidHits(0)
  , m_timeBinType(LArG4SimpleSD::HitTimeBinDefault)
  , m_timeBinWidth(width)
  , m_larEmID(nullptr)
  , m_larFcalID(nullptr)
  , m_larHecID(nullptr)
  , m_hitCollectionName(std::move(hitCollectionName))
  , m_hitSourceName(SensitiveDetectorName)
{
  // Only one string causes a change in action
  if(type == "Uniform")
    m_timeBinType = LArG4SimpleSD::HitTimeBinUniform;
}

LArG4SimpleSD::LArG4SimpleSD(G4String a_name, StoreGateSvc* detStore,
                             std::string hitCollectionName)
  : G4VSensitiveDetector(std::move(a_name))
  , m_calculator(nullptr)
  , m_numberInvalidHits(0)
  , m_timeBinType(LArG4SimpleSD::HitTimeBinDefault)
  , m_timeBinWidth(2.5*CLHEP::ns)
  , m_larEmID (nullptr)
  , m_larFcalID (nullptr)
  , m_larHecID (nullptr)
  , m_hitCollectionName(std::move(hitCollectionName))
{
  // This should only be used when it's safe to do this retrieval
  const CaloIdManager* caloIdManager=nullptr;
  if ( detStore->retrieve(caloIdManager).isFailure() ){
    throw std::runtime_error("Could not retrieve Calo ID manager");
  }

  const LArEM_ID* larEmID = caloIdManager->getEM_ID();
  if(larEmID==nullptr)
    throw std::runtime_error("IFastSimDedicatedSD: Invalid LAr EM ID helper");

  const LArFCAL_ID* larFcalID = caloIdManager->getFCAL_ID();
  if(larFcalID==nullptr)
    throw std::runtime_error("IFastSimDedicatedSD: Invalid FCAL ID helper");

  const LArHEC_ID* larHecID = caloIdManager->getHEC_ID();
  if(larHecID==nullptr)
    throw std::runtime_error("IFastSimDedicatedSD: Invalid HEC ID helper");

  setupHelpers( larEmID, larFcalID, larHecID );
}

LArG4SimpleSD::~LArG4SimpleSD()
{
  if(verboseLevel>5 && m_numberInvalidHits>0) {
    G4cout << "Destructor: Sensitive Detector <" << SensitiveDetectorName << "> had " << m_numberInvalidHits
           << " G4Step energy deposits outside the region determined by its Calculator." << G4endl;
  }
}

void LArG4SimpleSD::Initialize(G4HCofThisEvent*)
{
  if (auto* hitContainer = getHitContainer()) {
    // Register before any hits arrive so finalization follows SD setup order.
    hitContainer->RegisterSource(m_hitSourceName);
  }
}

G4bool LArG4SimpleSD::ProcessHits(G4Step* a_step,G4TouchableHistory*)
{
  // If there's no energy, there's no hit.  (Aside: Isn't this energy
  // the same as the energy from the calculator?  Not necessarily.
  // The calculator may include detector effects such as
  // charge-collection which are not modeled by Geant4.)
  if(a_step->GetTotalEnergyDeposit() == 0.) return false;

  // Convert the G4Step into (eta,phi,sampling).
  // Check that hit was valid.  (It might be invalid if, for example,
  // it occurred outside the sensitive region.  If such a thing
  // happens, it means that the geometry definitions in the
  // detector-construction routine and the calculator do not agree.)
  std::vector<LArHitData> hits;

  if(!(m_calculator->Process(a_step, hits))) {
    m_numberInvalidHits++;
    return false;
  }

  // A calculator can determine that a given energy deposit results
  // in more than one hit in the simulation.  For each such hit...
  G4bool result = true;
#ifdef DEBUG_IDENTIFIERS
  const G4StepPoint* pre_step_point = a_step->GetPreStepPoint();
  const G4ThreeVector startPoint = pre_step_point->GetPosition();
  const G4StepPoint* post_step_point = a_step->GetPostStepPoint();
  const G4ThreeVector endPoint = post_step_point->GetPosition();
  const G4ThreeVector p = startPoint;
  const int side = (p.z()<0) ? -1 : 1;
#endif
  for(const auto &ihit : hits) {
#ifdef DEBUG_IDENTIFIERS
    if (ihit.id[2]*side<0 && std::abs(ihit.id[1])<4) {
      G4cout << SensitiveDetectorName << "::ProcessHits ERROR G4Step position "<< "(" << p.x() << ", " << p.y() << ", " << p.z() << ") is inconsistent with LArG4Identifier assigned:" << G4endl;
      G4cout << "     LArG4Identifier from " << m_calculator->name() << " tech : " << ihit.id[1] << ", barrel_ec : " << ihit.id[2]
             << ", sampling : " << ihit.id[3]
             << ",   region : " << ihit.id[4]
             << ",      eta : " << ihit.id[5]
             << ",      phi : " << ihit.id[6] << G4endl;
      G4cout << "     startPoint: (" << startPoint.x() << ", " << startPoint.y() << ", " << startPoint.z() << ")" << G4endl;
      G4cout << "       endPoint: (" << endPoint.x() << ", " << endPoint.y() << ", " << endPoint.z() << ")" << G4endl;
    }
#endif
    // Ported in from the old LArG4HitMerger code
    result = result && SimpleHit( ihit.id ,
                                  ihit.time,
                                  ihit.energy );
  }
  return result;
}

G4bool LArG4SimpleSD::SimpleHit( const LArG4Identifier& lar_id , G4double time , G4double energy )
{
  // Build the hit from the calculator results.
  Identifier id = ConvertID( lar_id );
  if (!id.is_valid()) return false;

  G4int timeBin = getTimeBin( time );

  auto* hitContainer = getHitContainer();
  if (!hitContainer) {
    return false;
  }
  hitContainer->AddHit(m_hitSourceName,
                       std::make_unique<LArHit>(id,energy,time),
                       timeBin);
  
  return true;
}


G4int LArG4SimpleSD::getTimeBin(G4double time) const
{
    G4int timeBin = INT_MAX;
    
    if(m_timeBinType==LArG4SimpleSD::HitTimeBinDefault)
    {
        if(time<0.)
            time=0.;
        if(time<10.)
            timeBin = int(time/2.5);
        else if(time<50.)
            timeBin = 4 + int((time-10.)/5.);
        else if(time<100.)
            timeBin = 12 + int((time-50.)/25.);
        else
            timeBin = 14;
    }
    else
    {
        // Uniform binning by 2.5 ns
        G4double timeBinf = time/m_timeBinWidth;
        
        if (timeBinf < G4double(INT_MAX))
            timeBin = G4int(timeBinf);
    }
    
    return timeBin;
} 


LArHitContainerBuilder* LArG4SimpleSD::getHitContainer() const
{
  auto* eventManager = G4EventManager::GetEventManager();
  if (!eventManager) {
    return nullptr;
  }

  auto* eventInfo =
    dynamic_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation());
  if (!eventInfo) {
    return nullptr;
  }

  std::shared_ptr<HitCollectionMap> hitCollections = eventInfo->GetHitCollectionMap();
  return hitCollections ? hitCollections->Find<LArHitContainerBuilder>(m_hitCollectionName)
                        : nullptr;
}

Identifier LArG4SimpleSD::ConvertID(const LArG4Identifier& a_ident) const
{
    Identifier id;
    
    if(a_ident[0]==4)
    {
        // is LAr
        if(a_ident[1]==1)
        {
            //is LAr EM
            try
            {
                id = m_larEmID->channel_id(a_ident[2],  // barrel_ec
                                           a_ident[3],  // sampling
                                           a_ident[4],  // region
                                           a_ident[5],  // eta
                                           a_ident[6]); // phi
            }
            catch (LArID_Exception& e)
            {
                G4cout << "ERROR ConvertID: LArEM_ID error code " << e.code() << " "
                << (std::string) e << G4endl;
            }
        }
        else if(a_ident[1]==2)
        {
            //is EM HEC
            try
            {
                id = m_larHecID->channel_id(a_ident[2],  // zSide
                                            a_ident[3],  // sampling
                                            a_ident[4],  // region
                                            a_ident[5],  // eta
                                            a_ident[6]); // phi
            }
            catch(LArID_Exception& e)
            {
                G4cout << "ERROR ConvertID: BuildHitCollections: LArHEC_ID error code " << e.code() << " "
                << (std::string) e << G4endl;
            }
        }
        else if(a_ident[1]==3)
        {
            // FCAL
            if(a_ident[3]>0)
            {
                //is EM FCAL
                try
                {
                    id = m_larFcalID->channel_id(a_ident[2],  // zSide
                                                 a_ident[3],  // sampling
                                                 a_ident[4],  // eta
                                                 a_ident[5]); // phi
                }
                catch(LArID_Exception& e)
                {
                    G4cout << "ERROR ConvertID:: LArFCAL_ID error code " << e.code() << " "
                    << (std::string) e
                    << G4endl;
                }
            }
            else
            {
              //is Mini FCAL
              G4cout << "ERROR ConvertID:: unsupported LArMiniFCAL Identifier"
                     << G4endl;
            }
        }
    }
    return id;
}
