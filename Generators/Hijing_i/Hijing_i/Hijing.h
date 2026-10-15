/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORMODULESHIJING_H
#define GENERATORMODULESHIJING_H

#include "GeneratorModules/GenModule.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "Hijing_i/HiParnt.h"
#include "Hijing_i/RanSeed.h"
#include "Hijing_i/HiMain1.h"
#include "Hijing_i/HiMain2.h"
#include "Hijing_i/HijJet1.h"
#include "Hijing_i/HijJet2.h"
#include "Hijing_i/HijJet4.h"
#include "Hijing_i/HiStrng.h"
#include "Hijing_i/HijCrdn.h"

// new to store hijing event parameters
#include "GeneratorObjects/HijingEventParams.h"

/**
   @class  Generators/Hijing.h

   @brief This code is used to get a HIJING Monte Carlo event.
   genInitialize() is used to read parameters
   callGenerator() makes the event
   genFinalize() writes log files etc
   fillEvt(GeneratorEvent* evt) passes the event to HepMC

   The output will be stored in the transient event store so it can be
   passed to the simulation.

   @author         Georgios Stavropoulos, October 2002

   Ewelina Lobodzinska (Jan. 2008) - doxygen docu
*/

class Hijing:public GenModule {
public:
  using GenModule::GenModule;
  virtual ~Hijing() = default;

  virtual StatusCode genInitialize() override;
  virtual StatusCode callGenerator() override;
  virtual StatusCode genFinalize() override;
  virtual StatusCode fillEvt(HepMC::GenEvent* evt) override;

protected:
  virtual CLHEP::HepLorentzVector randomizeVertex(CLHEP::HepRandomEngine* engine);

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999};

  // inputs to HIJSET (Hijing initialization) ...
  float m_efrm{0.};
  std::string         m_frame;
  std::string         m_proj;
  std::string         m_targ;
  int         m_iap{0};
  int         m_iat{0};
  int         m_izp{0};
  int         m_izt{0};
  // ... and HIJING (event generation)
  float m_bmin{0.};
  float m_bmax{0.};

  // Vertex shifting

  DoubleProperty m_x{this, "x", 0.};
  DoubleProperty m_y{this, "y", 0.};
  DoubleProperty m_z{this, "z", 0.};
  BooleanProperty m_sel{this, "selectVertex", false}; // Select vertex location (off for random)
  BooleanProperty m_spec{this, "keepSpectators", true}; // TRUE  will keep specator information (default)

  //Set internal randomization of vertices for beam gas gen (on or off).
  BooleanProperty m_rand{this, "randomizeVertices", false};
  // Allow randoms off the beamline (out to the pipe)
  BooleanProperty m_wide{this, "wide", false}; // True allows particles with x,y distributions
  BooleanProperty m_prand{this, "randomizeP", false}; // BPK randomizes the left<->right direction by mirroring momenta for beam gas
  BooleanProperty m_keepAllDecayVertices{this, "keepAllDecayVertices", true};

  // Limit settings
  DoubleProperty m_partonStoreMinPt{this, "partonStoreMinPt", 5.0};
  DoubleProperty m_vertexOffsetCut{this, "vertexOffsetCut", 1.0E-7};

  // Random numbers seed
  int  m_randomseed{0};
  std::vector<long int> m_seeds;

  // event counter
  int m_events{0};

  // Commands to setup hijing
  StringArrayProperty m_InitializeVector{this,
                                         "Initialize",
                                         {},
                                         "List of parameters for Hijing initialization",
                                         "GeneratorSettings<std::string>"};

  // Accessor to HIJING Options and parameters COMMON
  HiParnt m_hiparnt;

  // Accessor to HIJING Random number seed COMMON
  RanSeed m_ranseed;

  // Accessors to HIJING Event information COMMONS
  HiMain1 m_himain1;
  HiMain2 m_himain2;
  HijJet1 m_hijjet1;
  HijJet2 m_hijjet2;
  HijJet4 m_hijjet4;
  HiStrng m_histrng;
  HijCrdn m_hijcrdn;

  void        set_user_params(void);

  SG::WriteHandleKey<HijingEventParams> m_event_paramsKey{this, "HijingOutputKey", "Hijing_event_params"};
};

#endif
