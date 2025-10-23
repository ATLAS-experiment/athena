/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4H6COLDTCMod0Calculator.h"
#include "LArG4H6COLDTCMod0ChannelMap.h"

// ATLAS LAr includes
#include "LArG4Code/LArG4Identifier.h"

// Geant4 includes
#include "G4Version.hh"
#include "G4NavigationHistory.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4StepPoint.hh"
#include "G4Step.hh"
#include "globals.hh"

#include <cstdlib>
#include <fstream>
#include <exception>

#include <cmath>
#include <limits>

// constructor
LArG4H6COLDTCMod0Calculator::LArG4H6COLDTCMod0Calculator(const std::string& name, ISvcLocator* pSvcLocator)
  : LArCalculatorSvcImp(name, pSvcLocator)
{
}

StatusCode LArG4H6COLDTCMod0Calculator::initialize()
{
  // get channel map
  m_channelMap = LArG4H6COLDTCMod0ChannelMap(m_innerActiveRadius,
                                             m_outerActiveRadius,
                                             m_areaActive,
                                             m_phiModuleStart,
                                             m_phiModuleEnd,
                                             8);

  return StatusCode::SUCCESS;
}

// hit processing
G4bool LArG4H6COLDTCMod0Calculator::Process(const G4Step* a_step, std::vector<LArHitData>& hdata) const
{
  hdata.clear();
  LArHitData larhit;
  // Given a G4Step, determine the cell identifier.

  // 29-Mar-2002 WGS: this method now returns a boolean.  If it's
  // true, the hit is valid; if it's false, there was some problem
  // with the hit and it should be ignored.

  // First, get the energy.
  larhit.energy = a_step->GetTotalEnergyDeposit() * a_step->GetTrack()->GetWeight();

  // Find out how long it took the energy to get here.
  G4StepPoint* pre_step_point = a_step->GetPreStepPoint();
  G4StepPoint* post_step_point = a_step->GetPostStepPoint();
  G4double timeOfFlight = (pre_step_point->GetGlobalTime() +
                           post_step_point->GetGlobalTime()) * 0.5;
  G4ThreeVector startPoint = pre_step_point->GetPosition();
  G4ThreeVector endPoint   = post_step_point->GetPosition();
  G4ThreeVector p = (startPoint + endPoint) * 0.5;

  // Determine if the hit was in-time.
  larhit.time = timeOfFlight/Units::ns - p.mag()/Units::c_light/Units::ns;

  // get local coordinates
  G4ThreeVector theLocalPoint =  pre_step_point->GetTouchable()->GetHistory()->GetTopTransform().TransformPoint(p);

  /////////////////////////////////////
  //
  // For the cold tailcatcher:
  //
  // (1) liquid argon gaps are sensitive detectors
  // (2) only lateral segmentation, no longitudinal
  // (3) strategy: find (x,y,z) in local coordinate system first, then
  //     calculate channel identifier in 4 phi bins (phi range [4,7]) and
  //     8 radial bins (range [0,7]) (consult with Seligman on coordinate
  //     system transformation!)
  //
  /////////////////////////////////////
  // Strip the name of the volume:
  G4String hitVolume = pre_step_point->GetTouchable()->GetVolume(0)->GetName();
#if G4VERSION_NUMBER < 1100
  if(hitVolume.contains("::") ) {
#else
  if(G4StrUtil::contains(hitVolume,"::") ) {
#endif
           int last = hitVolume.rfind(':');
            hitVolume.erase(0,last+1);
  }
  int volnum = 0;
  if( hitVolume == "Gap") {
      volnum = pre_step_point->GetTouchable()->GetVolume(0)->GetCopyNo()-1;
  }else if(hitVolume == "Active") {
      volnum = pre_step_point->GetTouchable()->GetVolume(2)->GetCopyNo()-1;
  }else if (hitVolume == "Absorber"){
      volnum = int((theLocalPoint.z() + m_fullModuleDepth/2.)/(m_fullModuleDepth/8.));
  }else if (hitVolume == "Electrode") {
      volnum = pre_step_point->GetTouchable()->GetVolume(1)->GetCopyNo()-1;
  }

  G4int etaIndex = m_channelMap.getRBin(theLocalPoint);
  G4int phiIndex = m_channelMap.getPhiBin(theLocalPoint);
  if(volnum/2 == 0) {
      phiIndex = int(phiIndex/4) + 8;
  } else {
      etaIndex = int(etaIndex/4) + 8;
  }
  // zSide is negative if z<0.
  G4int zSide = p.z() < 0 ? -2 : 2;
  G4int sampling = m_FCalSampling;

  /////////////////////////////////////////////////
  //
  // Cold TailCatcher:
  //
  // sampling = 3   // fake FCal3
  // etaIndex = (radial bin #)
  // phiIndex = (azimuthal bin #)
  //
  /////////////////////////////////////////////////

  larhit.id.clear();
  if((etaIndex > 7 && phiIndex > 7) ||
     (etaIndex <=7 && phiIndex <= 7)) {
    std::cout<<"LArCOLDTCMod0Calculator::Process: Bad identifier !!!"<<std::endl;
    return false;
  } else {

    // Append the values to the empty identifier.
    larhit.id << 4          // LArCalorimeter  (same for cold TC)
	      << 3          // LArFCAL         (same for cold TC)
	      << zSide      // EndCap          (same for cold TC)
	      << sampling   // FCal Module #   (3 for cold TC)
	      << etaIndex   //                   (see above)
	      << phiIndex;  //                   (see above)
    hdata.push_back(std::move(larhit));
    return true;
  }
}
