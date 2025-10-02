/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArBarrelCalculator.hh
// The Cell Identifier for the EM Barrel readout cells

// Adapted from code written by Gaston Parrour
// Adaptation by Sylvain Negroni

// 12-Jul-2002 WGS: Added LArG4Identifier.

// 18-03-2005  G.Unal: major revision to include new current maps
#ifndef LARBARRELCALCULATOR_H
#define LARBARRELCALCULATOR_H

#include "LArG4Code/LArG4Identifier.h"
#include "LArG4Code/LArCalculatorSvcImp.h"
#include "ILArBarrelGeometry.h"
#include "MapEta.h"

#include <stdexcept>
#include <vector>
#include <string>
#include <memory>

class G4Step;
class AccMap;

class LArG4BirksLaw;

class LArBarrelCalculator : public LArCalculatorSvcImp
{
public:

  LArBarrelCalculator(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  LArBarrelCalculator (const LArBarrelCalculator&) = delete;
  LArBarrelCalculator& operator= (const LArBarrelCalculator&) = delete;

  void initializeForSDCreation() override final { m_geometry->initializeForSDCreation(); };

  virtual G4bool Process(const G4Step* a_step, std::vector<LArHitData>& hdata) const override final;

  // Check if the current hitTime is in-time
  virtual G4bool isInTime(G4double hitTime) const override final
  {
    return !(std::fabs(hitTime) > m_OOTcut);
  }

  //
  // get functions:
  //
  virtual G4float OOTcut() const override final { return m_OOTcut; }

private:
  
  ServiceHandle<ILArBarrelGeometry> m_geometry{this, "GeometryCalculator", "LArBarrelGeometry"};
  const AccMap*   m_accmap{nullptr};
  std::unique_ptr<MapEta>   m_etamap1{};
  std::unique_ptr<MapEta>   m_etamap2{};
  std::unique_ptr<MapEta>   m_etamap3{};

  // RUN Options
  Gaudi::Property<bool> m_IflCur{this, "EMBCurr", true};
  Gaudi::Property<bool> m_IflMapTrans{this, "EMBEtaTrans", true};
  Gaudi::Property<bool> m_IflXtalk{this, "EMBXtalk", true};

  Gaudi::Property<double> m_dstep{this, "EMBdstep", .2*CLHEP::mm};

  const LArG4BirksLaw* m_birksLaw{nullptr};
  Gaudi::Property<bool>  m_doHV{this, "EMBHVEnable", false};

  // global EMBarrel dimensions
  double m_etaMaxBarrel{0.};
  double m_zMinBarrel{0.};
  double m_zMaxBarrel{0.};
  // global Accordion dimensions
  double m_rMinAccordion{0.};
  double m_rMaxAccordion{0.};
  // half thickness of absorber and electrode
  double m_ThickAbs{0.};
  double m_ThickEle{0.};
  // GU 11/06/2003  total number of cells in phi
  int m_NCellTot{0};
  int m_NCellMax{0};
  // to handle small difference (mostly phi wrapping and +-z symmetry)
  // between atlas and test beam
  bool m_testbeam{false};


  // Hv values
  //   0,1 = positive/negative barrel
  //   0->1023 = electrode number
  //   0->6 = eta region number (0.2 granularity) (1.2 to 1.475 in same eta bin)
  //   0,1 = below, above the electrode (according to phi in global Atlas frame)

  double m_hv[2][1024][7][2]{};

  G4bool FiducialCuts(G4double,G4double,G4double) const;

  void InitHV();
  double ScaleHV(double, double, double, double) const;

};

#endif // LARBARRELCALCULATOR_H
