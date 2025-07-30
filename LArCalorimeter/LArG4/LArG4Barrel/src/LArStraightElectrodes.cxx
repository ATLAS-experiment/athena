/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArStraightElectrodes.h"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4Trap.hh"
#include "LArG4Code/LArVG4DetectorParameters.h"


const LArStraightElectrodes* LArStraightElectrodes::GetInstance(const std::string& strDetector)
{
  static const LArStraightElectrodes instance(strDetector);
  return &instance;
}


LArStraightElectrodes::LArStraightElectrodes(const std::string& strDetector)
{
  const LArVG4DetectorParameters* parameters = LArVG4DetectorParameters::GetInstance();
  if (parameters->GetValue("LArEMBPhiAtCurvature",0)>0.)  m_parity=0;  // first wave goes up
  else                                                    m_parity=1;  // first wave goes down

  static const std::string prefix = strDetector.empty() ? "" : strDetector+"::";
  static const PhysicalVolumeAccessor pva(prefix+"LAr::EMB::STAC",
                                          prefix+"LAr::EMB::Electrode::Straight");

  for (int stackid=0; stackid<14; stackid++) {
    for (int cellid=0; cellid<1024; cellid++) {
      initXYCentEle(pva, stackid, cellid);
      initHalfLength(pva, stackid, cellid);
      const double slant = SlantEle(pva, stackid, cellid);
      sincos(slant, &m_sinu[cellid][stackid], &m_cosu[cellid][stackid]);
    }
  }
}


void LArStraightElectrodes::initXYCentEle(const PhysicalVolumeAccessor& theElectrodes, int stackid, int cellid)
{
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theElectrodes.GetPhysicalVolume(id);
  if (!pv) {
    m_xcent[cellid][stackid] = 0;
    m_ycent[cellid][stackid] = 0;
    return;
  }

  const G4ThreeVector& tv=pv->GetTranslation();
  const G4VPhysicalVolume *pv2=theElectrodes.GetPhysicalVolume(1000000+id);
  if (!pv2) {
    m_xcent[cellid][stackid] = tv.x();
    m_ycent[cellid][stackid] = tv.y();
  }
  else {
    const G4ThreeVector& tv2=pv2->GetTranslation();
    const G4LogicalVolume* lv = pv->GetLogicalVolume();
    const G4Trap* trap = static_cast<G4Trap*> (lv->GetSolid());
    const G4LogicalVolume* lv2 = pv2->GetLogicalVolume();
    const G4Trap* trap2 = static_cast<G4Trap*> (lv2->GetSolid());
    double xl1=trap->GetYHalfLength1();
    double xl2=trap2->GetYHalfLength1();
    m_xcent[cellid][stackid] = (tv.x()*xl1+tv2.x()*xl2)/(xl1+xl2);
    m_ycent[cellid][stackid] = (tv.y()*xl1+tv2.y()*xl2)/(xl1+xl2);
  }
}


double LArStraightElectrodes::SlantEle(const PhysicalVolumeAccessor& theElectrodes, int stackid, int cellid) const
{
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theElectrodes.GetPhysicalVolume(id);
  if (!pv) return 0.;
  const G4RotationMatrix *rm=pv->GetRotation();
  double Slant = (stackid%2 ==m_parity) ? 180*CLHEP::deg-(rm->thetaY()):(rm->thetaY())-180*CLHEP::deg;
  if((stackid%2 == m_parity) && (rm->phiY() > 0)) Slant = 360.*CLHEP::deg - Slant;
  if((stackid%2 == (1-m_parity)) && (rm->phiY() < 0)) Slant = - Slant;
  return Slant;
}


void LArStraightElectrodes::initHalfLength(const PhysicalVolumeAccessor& theElectrodes, int stackid, int cellid)
{
  double l = 0.;
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theElectrodes.GetPhysicalVolume(id);
  if (!pv){
    l = 0.;
  } else {
    const G4LogicalVolume* lv = pv->GetLogicalVolume();
    const G4Trap* trap = static_cast<G4Trap*> (lv->GetSolid());
    const G4VPhysicalVolume *pv2=theElectrodes.GetPhysicalVolume(1000000+id);
    if (!pv2){ 
      l = trap->GetYHalfLength1();
    } else {
      const G4LogicalVolume* lv2 = pv2->GetLogicalVolume();
      const G4Trap* trap2 = static_cast<G4Trap*> (lv2->GetSolid());
      l = trap->GetYHalfLength1()+trap2->GetYHalfLength1();
    }
  }
  m_halflength[cellid][stackid] = l;
}

