/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// File:  Generators/FlowAfterburner/AddFlowByShifting.h
// Description:
//    This code is used to introduce particle flow
//    to particles from generated events
//    It works by modifying phi angles of particles
//    according to requested flow type and magnitude
//
//    It takes from SG a container of tracks as input
//    and registers in SG a new container with modified tracks on output
//
//    It currently uses Hijing generator specific class HijingEventParams
//    with truth event parameters information
//
// AuthorList:
// Andrzej Olszewski: Initial Code February 2006

#ifndef ADDFLOWBYSHIFTING_H
#define ADDFLOWBYSHIFTING_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
//
#include "AthenaKernel/IAthRNGSvc.h"
#include "AtlasHepMC/GenParticle.h" //typedef GenParticlePtr
#include "AtlasHepMC/GenVertex.h" //GenParticlePtr



class HijingEventParams;
namespace CLHEP{
  class HepRandomEngine;
}
class TGraph;

class AddFlowByShifting:public AthAlgorithm {
public:
  AddFlowByShifting(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  //functions used for root finding when using the "exact"(and not the "approximate")  method
  static double vn_func           (double x, void *params);


private:
  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName, const EventContext& ctx) const;

  double SetParentToRanPhi(HepMC::GenParticlePtr parent, CLHEP::HepRandomEngine *rndmEngine);
  double AddFlowToParent(HepMC::GenParticlePtr parent,
                         const HijingEventParams *hijing_pars);
  void   MoveDescendantsToParent(HepMC::GenParticlePtr parent, double phishift);


  // flow functions to set the vn values
  void (AddFlowByShifting::*m_flow_function)     (double b, double eta, double pt);//function pointer which is set to one of the functions below
  void jjia_minbias_new       (double b, double eta, double pt);
  void jjia_minbias_new_v2only(double b, double eta, double pt);
  void fixed_vn               (double b, double eta, double pt);
  void fixed_v2               (double b, double eta, double pt);
  void jjia_minbias_old       (double b, double eta, double pt);
  void ao_test                (double b, double eta, double pt);
  void custom_vn              (double b, double eta, double pt);
  void p_Pb_cent_eta_indep    (double b, double eta, double pt); //for p_Pb
  void OO_eta_indep           (double b, double eta, double pt); //for OO

  TGraph *m_graph_fluc{};//TGraph storing the v2_RP/delta Vs b_imp
  void Set_EbE_Fluctuation_Multipliers(HepMC::GenVertexPtr mainvtx, float b, CLHEP::HepRandomEngine *rndmEngine);

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  // Setable Properties:-
  StringProperty  m_inkey              {this, "McTruthKey"        , "GEN_EVENT"       }; //FIXME use Read/WriteHandles
  StringProperty  m_outkey             {this, "McFlowKey"         , "FLOW_EVENT"      }; //FIXME use Read/WriteHandles
  IntegerProperty m_ranphi_sw          {this, "RandomizePhi"      , 0                 };
  StringProperty  m_flow_function_name {this, "FlowFunctionName"  , "jjia_minbias_new"};
  StringProperty  m_flow_implementation{this, "FlowImplementation", "exact"           };
  int   m_flow_implementation_type{0};
  BooleanProperty m_flow_fluctuations  {this, "FlowFluctuations"  , false             };
  IntegerProperty m_floweta_sw         {this, "FlowEtaSwitch"     , 0                 };
  FloatProperty   m_flow_maxeta        {this, "FlowMaxEtaCut"     , 10.0              };
  FloatProperty   m_flow_mineta        {this, "FlowMinEtaCut"     , 0.f               };
  IntegerProperty m_flowpt_sw          {this, "FlowPtSwitch"      , 0                 };
  FloatProperty   m_flow_maxpt         {this, "FlowMaxPtCut"      , 1000000.f         };
  FloatProperty   m_flow_minpt         {this, "FlowMinPtCut"      , 0.f               };
  IntegerProperty m_flowb_sw           {this, "FlowBSwitch"       , 0                 };//currently not used
  FloatProperty   m_custom_v1          {this, "custom_v1"         , 0.f               };
  FloatProperty   m_custom_v2          {this, "custom_v2"         , 0.f               };
  FloatProperty   m_custom_v3          {this, "custom_v3"         , 0.f               };
  FloatProperty   m_custom_v4          {this, "custom_v4"         , 0.f               };
  FloatProperty   m_custom_v5          {this, "custom_v5"         , 0.f               };
  FloatProperty   m_custom_v6          {this, "custom_v6"         , 0.f               };

  int   m_particles_processed{0};

  enum Harmonic{
    NumHar=6,
    v1=0,
    v2=1,
    v3=2,
    v4=3,
    v5=4,
    v6=5,
  };
  float m_psi_n            [Harmonic::NumHar];
  float m_v_n              [Harmonic::NumHar];
  float m_EbE_Multiplier_vn[Harmonic::NumHar];
};

#endif
