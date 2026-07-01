/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


/**
@class CosmicGenerator




@brief  Cosmic generator.  The output will be stored in the transient event store so it can be passed to the simulation.

@author 
   W. Seligman: Initial Code 08-Nov-2002,
        based on work by M. Shapiro and I. Hinchliffe


 Modification for increasing efficiency of muon hitting the detector:
                     H. Ma.    March 17, 2006 
 Property: ExzCut: 	
	if true, the method exzCut(...) will be called to apply a 
               energy dependent position cut on the surface.
               This rejects low energy muons at large distance. 
   Property: RMax
               Used by exzCut to reject non-projective muons, which are 
               too far out on the surface


 Modifications to accomodate Pixel EndCap C Cosmic Test needs
      Marian Zdrazil   June 7, 2006   mzdrazil@lbl.gov

Modifications to accomodate replacement of Pixel EndCap C by a Pixel EndCap A
      Marian Zdrazil   November 24, 2006  mzdrazil@lbl.gov

It is easier and actually more useful to leave the EndCap A
in the vertical position (the way it is positioned in the ATLAS detector)
 instead of rotating it clockwise by 90deg which corresponds to the
 placement during the Pixel EndCap A cosmic test in SR1 in November 2006.
 This is why we will generate cosmic muons coming from the positive Z-axis 
 direction better than rotating the whole setup in PixelGeoModel.

 Modifications July 3rd 2007, Rob McPherson
     - Fix mu+/mu- bug (always present in Athena versions)
     - Fix sign of Py (since tag CosmicGenerator-00-00-21, muons only upward-going) 

 Optimize selection of events passed to Geant4 for full simulation:
 - cut on energy based on pathlength in rock
 - reweighting of generated cosmic rays
 - geometrical cut in plane of pixel detector
      Juerg Beringer   November 2007      JBeringer@lgl.gov
      Robert Cahn      November 2007      RNCahn@lbl.gov

*/



#ifndef GENERATORMODULESCOSMICGEN_H
#define GENERATORMODULESCOSMICGEN_H

#include "GeneratorModules/GenModule.h"
#include "GeneratorModules/GenData.h"

#include "CLHEP/Vector/LorentzVector.h"
#include "AtlasHepMC/Polarization.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>



class CosmicGun;

class CosmicGenerator:public GenModule {

public:
  CosmicGenerator(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~CosmicGenerator() = default;
  virtual StatusCode genInitialize();
  virtual StatusCode callGenerator();
  virtual StatusCode genFinalize();
  virtual StatusCode fillEvt(HepMC::GenEvent* evt);

  CLHEP::HepLorentzVector generateVertex(void);
  CLHEP::HepLorentzVector generateVertexReweighted(void);

  /// Static pointer to random number generator for use by
  static CLHEP::HepRandomEngine* COSMIC_RANDOM_ENGINE;

private:
  // Migration to MeV and mm units: all conversions are done in this interface
  // to the CosmicGun. The CosmicGun itself uses GeV units internally - to call
  // the fortran code.
  //

  static constexpr float m_GeV = 1000.f; // FIXME Take from SystemOfUnits.h header?
  static constexpr float m_mm = 10.f; // FIXME Take from SystemOfUnits.h header?

  // event counter, used for event ID
  int m_events{0};
  int m_rejected{0};
  int m_accepted{0};
  std::vector<int> m_pdgCode;

  IntegerProperty m_dsid{this, "Dsid", 999999, "Dataset ID number"};
  FloatProperty m_emin{this, "emin", 10.*m_GeV};
  FloatProperty m_emax{this, "emax", 100.*m_GeV};
  FloatProperty m_ctcut{this, "ctcut", 0.35};
  FloatProperty m_xlow{this, "xvert_low", 0.*m_mm};
  FloatProperty m_xhig{this, "xvert_hig", 10.*m_mm};
  FloatProperty m_zlow{this, "zvert_low", 0.*m_mm};
  FloatProperty m_zhig{this, "zvert_hig", 10.*m_mm};
  FloatProperty m_yval{this, "yvert_val", 81.*m_mm};
  FloatProperty m_IPx{this, "IPx", 0.f};
  FloatProperty m_IPy{this, "IPy", 0.f};
  FloatProperty m_IPz{this, "IPz", 0.f};
  FloatProperty m_radius{this, "Radius", 0.f};
  FloatProperty m_zpos{this, "Zposition", 14500.f};
  FloatProperty m_tmin{this, "tmin", 0.f};
  FloatProperty m_tmax{this, "tmax", 0.f};
  BooleanProperty m_cavOpt{this, "OptimizeForCavern", false};
  IntegerProperty m_srOneOpt{this, "OptimizeForSR1", 0}; // Not a bool??
  BooleanProperty m_srOnePixECOpt{this, "OptimizeForSR1PixelEndCap", false};
  BooleanProperty m_swapYZAxis{this, "SwapYZAxis", false};
  BooleanProperty m_muonECOpt{this, "OptimizeForMuonEndCap", false};
  IntegerProperty m_printEvent{this, "PrintEvent", 10};
  IntegerProperty m_printMod{this, "PrintMod", 100};

  FloatProperty m_thetamin{this, "ThetaMin", 0.f};
  FloatProperty m_thetamax{this, "ThetaMax", 1.f};
  FloatProperty m_phimin{this, "PhiMin", -1.*M_PI};
  FloatProperty m_phimax{this, "PhiMax", M_PI};

  bool m_readfile{false};
  StringProperty m_infile{this, "eventfile", "NONE"};
  std::ifstream    m_ffile;

  // Event scalars, three-vectors, and four-vectors:
  std::vector<CLHEP::HepLorentzVector> m_fourPos;
  std::vector<CLHEP::HepLorentzVector> m_fourMom;
  CLHEP::Hep3Vector m_center;
  std::vector<HepMC::Polarization> m_polarization;

  // Energy dependent position cut for muons to reach the detector.
  bool exzCut(const CLHEP::Hep3Vector& pos,const CLHEP::HepLorentzVector& p);

  // property for calling exzCut
  BooleanProperty m_exzCut{this, "ExzCut", false};
  // maximum r used in exzCut
  FloatProperty m_rmax{this, "RMax", 10000000.f};

  // Calculation of pathlength in rock and whether cosmic ray is aimed towards
  // the pixel detector
  double pathLengthInRock(double xgen, double ygen, double zgen, double theta, double phi);
  bool pointsAtPixels(double xgen, double ygen, double zgen, double theta, double phi);

  // New optimization options (November 2007)
  BooleanProperty m_doPathlengthCut{this, "doPathLengthCut", false};
  BooleanProperty m_doAimedAtPixelsCut{this, "doAimedAtPixelsCut", false};
  BooleanProperty m_doReweighting{this, "doReweighting", false};
  DoubleProperty m_energyCutThreshold{this, "energyCutThreshold", 1.0};
  DoubleProperty m_ysurface{this, "ysurface", 81.*m_mm};
  DoubleProperty m_rvertmax{this, "rvert_max", 300.*m_mm}; // replaces rectangle in case of reweighting
  DoubleProperty m_pixelplanemaxx{this, "pixelplane_maxx", 1150.};
  DoubleProperty m_pixelplanemaxz{this, "pixelplane_maxz", 1650.};
  std::shared_ptr<GenData> m_gendata{nullptr};
};

#endif
