/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArStraightAbsorbers.h"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4Trap.hh"
#include "LArG4Code/LArVG4DetectorParameters.h"

const LArStraightAbsorbers* LArStraightAbsorbers::GetInstance(const std::string& strDetector)
{
  static const LArStraightAbsorbers instance(strDetector);
  return &instance;
}

LArStraightAbsorbers::LArStraightAbsorbers(const std::string& strDetector)
{
  const LArVG4DetectorParameters* parameters = LArVG4DetectorParameters::GetInstance();
  if (parameters->GetValue("LArEMBPhiAtCurvature",0)>0.)  m_parity=0;  // first wave goes up
  else                                                    m_parity=1;  // first wave goes down

  static const std::string prefix = strDetector.empty() ? "" : strDetector+"::";
  static const PhysicalVolumeAccessor pva(prefix+"LAr::EMB::STAC",
                                          prefix+"LAr::EMB::ThinAbs::Straight");

  for (int stackid=0; stackid<14; stackid++) {
    for (int cellid=0; cellid<1024; cellid++) {
      initXYCentAbs(pva, stackid,cellid);
      initHalfLength(pva, stackid,cellid);
      const double slant = SlantAbs(pva, stackid,cellid);
      sincos(slant, &m_sinu[cellid][stackid], &m_cosu[cellid][stackid]);
    }
  }
}

void LArStraightAbsorbers::initXYCentAbs(const PhysicalVolumeAccessor& theAbsorbers, int stackid, int cellid)
{
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theAbsorbers.GetPhysicalVolume(id);
  if (!pv) {
    m_xcent[cellid][stackid] = 0;
    m_ycent[cellid][stackid] = 0;
    return;
  }
  const G4ThreeVector& tv=pv->GetTranslation();
  const G4VPhysicalVolume *pv2=theAbsorbers.GetPhysicalVolume(1000000+id);
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
    const double xl1=trap->GetYHalfLength1();
    const double xl2=trap2->GetYHalfLength1();
    m_xcent[cellid][stackid] = (tv.x()*xl1+tv2.x()*xl2)/(xl1+xl2);
    m_ycent[cellid][stackid] = (tv.y()*xl1+tv2.y()*xl2)/(xl1+xl2);
  }
}

double LArStraightAbsorbers::SlantAbs(const PhysicalVolumeAccessor& theAbsorbers, int stackid, int cellid) const
{
  // both stackid and cellid start from 0 in the following code
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theAbsorbers.GetPhysicalVolume(id);
  if (!pv) return 0.;
  const G4RotationMatrix *rm=pv->GetRotation();
  double Slant = (stackid%2 ==m_parity) ? 180*CLHEP::deg-(rm->thetaY()):(rm->thetaY())-180*CLHEP::deg;
  if((stackid%2 == m_parity) && (rm->phiY() > 0)) Slant = 360.*CLHEP::deg - Slant;
  if((stackid%2 == (1-m_parity)) && (rm->phiY() < 0)) Slant = - Slant;
  return Slant;
}

void LArStraightAbsorbers::initHalfLength(const PhysicalVolumeAccessor& theAbsorbers, int stackid, int cellid)
{
  double l = 0.;
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theAbsorbers.GetPhysicalVolume(id);
  if (pv) {
    const G4LogicalVolume* lv = pv->GetLogicalVolume();
    const G4Trap* trap = static_cast<G4Trap*> (lv->GetSolid());
    const G4VPhysicalVolume *pv2=theAbsorbers.GetPhysicalVolume(1000000+id);
    if (!pv2) l = trap->GetYHalfLength1();
    else {
      const G4LogicalVolume* lv2 = pv2->GetLogicalVolume();
      const G4Trap* trap2 = static_cast<G4Trap*> (lv2->GetSolid());
      l = trap->GetYHalfLength1()+trap2->GetYHalfLength1();
    }
  }
  m_halflength[cellid][stackid] = l;
}

