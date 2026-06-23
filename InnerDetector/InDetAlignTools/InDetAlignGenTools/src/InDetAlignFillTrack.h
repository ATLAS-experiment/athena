/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGENTOOLS_INDETALIGNFILLTRACK_H
#define INDETALIGNGENTOOLS_INDETALIGNFILLTRACK_H
// ================================================
// InDetAlignFillTrack
// ================================================
//
// InDetAlignFillTrack.h
// Headerfile for InDetAlignFillTrack
//
// Carlos Escobar, started 27/12/2007
//
// AthAlgTool to create store Trk::Track and MCTrack information in a ntuple

#include <string>

#include "TrkTrack/TrackCollection.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ToolHandle.h"

#include "InDetAlignGenTools/IInDetAlignFillTrack.h"
#include "TrkToolInterfaces/ITruthToTrack.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"

namespace Trk {
  class Track;
}

class INTupleSvc;


class InDetAlignFillTrack: public extends<AthAlgTool, IInDetAlignFillTrack> {
 public:
  InDetAlignFillTrack(const std::string& type, const std::string& name,
		      const IInterface* parent);
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual StatusCode FillTrack() override;

  virtual int GetTrks() const override { return m_totaltrks; }
  virtual int GetTrkHits() const override { return m_totalhits; }
  virtual int GetTrkPixHits() const override { return m_totalPixhits; }
  virtual int GetTrkSCTHits() const override { return m_totalSCThits; }
  virtual int GetTrkTRTHits() const override { return m_totalTRThits; }

 private:
  ServiceHandle<INTupleSvc> m_ntupleSvc{this,"NTupleSvc","NTupleSvc"};

  // member functions
  void bookNtuple();
  void bookUpNtuple();
  void bookLowNtuple();
  void bookMatchingNtuple();
  int dumpTrackCol(const EventContext& ctx, const TrackCollection*);
  int dumpTrackCol(const EventContext& ctx, const TrackCollection*, const std::string&);
  void dumpTrack(const EventContext& ctx, int, const Trk::Track*, const std::string&);
  void dumpPerigee(const Trk::TrackParameters*, int);
  StatusCode dumpMatching(const TrackCollection*,const TrackCollection*);

  // properties

  // Truth information
  BooleanProperty m_doTruth{this,"doTruth",false};       //!< switch on/off the truth information
  StringProperty m_TruthTrkCol{this,"TruthTrkCol","TrackTruthCollection"};

  StringProperty m_inputCol{this,"InputTrkCol","Tracks"};
  StringProperty m_inputUpCol{this,"InputUpTrkCol",""};
  StringProperty m_inputLowCol{this,"InputLowTrkCol",""};

  // cosmic segments matching
  BooleanProperty m_doMatching{this,"doMatching",true};  //!< switch on/off the matching information
  FloatProperty m_matchedRcut{this,"dRCut",100.};
  FloatProperty m_mindR{this,"minimumdR",10000.};

  // Ntuple
  StringProperty m_ntupleName{this,"NtupleName","/NTUPLES/GLOBFILE"};

  // variables
  int m_totaltrks{0};
  int m_totalhits{0};
  int m_totalPixhits{0};
  int m_totalSCThits{0};
  int m_totalTRThits{0};
  int m_totalUptrks{0};
  int m_totalUphits{0};
  int m_totalUpPixhits{0};
  int m_totalUpSCThits{0};
  int m_totalUpTRThits{0};
  int m_totalLowtrks{0};
  int m_totalLowhits{0};
  int m_totalLowPixhits{0};
  int m_totalLowSCThits{0};
  int m_totalLowTRThits{0};
  int m_events{0};

  // tool handles
  ToolHandle < Trk::ITruthToTrack > m_truthToTrack{this
    , "TruthToTrackTool"
    , "Trk::TruthToTrack"
    , "tool to produce perigee track parameters from generated parameters"};

  ToolHandle < Trk::IExtrapolator > m_extrapolator{this
    , "ExtrapolationTool"
    , "Trk::Extrapolator/CosmicsExtrapolator"
    , "tool to extrapolate tracks"};

  ToolHandle <Trk::ITrackParticleCreatorTool> m_particleCreator{this
    , "TrackParticleCreatorTool"
    , "Trk::TrackParticleCreatorTool/TrackParticleCreatorTool"
    , "tool to build TrackParticle"};

  NTuple::Item<long> m_nt_ntracks;            //!< number of tracks
  NTuple::Item<long> m_nt_nUptracks;          //!< number of Up tracks
  NTuple::Item<long> m_nt_nLowtracks;         //!< number of Low tracks
  NTuple::Item<long> m_nt_nmctracks;          //!< number of mc tracks
  NTuple::Item<long> m_nt_matchingTrk;        //!< matching tracks
  // ----------------------------------------------------------------------
  // Trk::Track parameters
  NTuple::Array<float> m_nt_Trk_d0;      //!< d0 parameter
  NTuple::Array<float> m_nt_Trk_z0;      //!< z0 parameter
  NTuple::Array<float> m_nt_Trk_phi0;    //!< phi0 parameter
  NTuple::Array<float> m_nt_Trk_theta0;  //!< theta0 parameter
  NTuple::Array<float> m_nt_Trk_qoverp;  //!< q/p parameter
  NTuple::Array<float> m_nt_Trk_pt;      //!< pt parameter
  // ----------------------------------------------------------------------

  // ----------------------------------------------------------------------
  NTuple::Array<long> m_nt_Trk_nHits;      //!< number of hits
  NTuple::Array<long> m_nt_Trk_nhitspix;   //!< number of Pixel hits 
  NTuple::Array<long> m_nt_Trk_nhitssct;   //!< number of SCT hits
  NTuple::Array<long> m_nt_Trk_nhitstrt;   //!<  number of TRT hits

  NTuple::Array<long> m_nt_Trk_nsharedPixels; //!< number of Pixel shared hits 
  NTuple::Array<long> m_nt_Trk_nsharedSCT;    //!< number of SCT shared hits 
  NTuple::Array<long> m_nt_Trk_nshared;       //!< number of shared hits 

  NTuple::Array<long> m_nt_Trk_nholesPixels;  //!< number of Pixel holes 
  NTuple::Array<long> m_nt_Trk_nholesSCT;     //!< number of SCT holes 
  NTuple::Array<long> m_nt_Trk_nholes;        //!< number of holes 

  NTuple::Array<float> m_nt_Trk_chi2;      //!< number of chi2 
  NTuple::Array<long> m_nt_Trk_ndof;       //!< number of ndof 
  NTuple::Array<float> m_nt_Trk_chi2Prob;  //!< number of chi2 probability 
 // ----------------------------------------------------------------------

  // ----------------------------------------------------------------------
  // Trk::Track parameters
  NTuple::Array<float> m_nt_Trk_d0_Up;      //!< d0 parameter (Up track)
  NTuple::Array<float> m_nt_Trk_z0_Up;      //!< z0 parameter (Up track)
  NTuple::Array<float> m_nt_Trk_phi0_Up;    //!< phi0 parameter (Up track)
  NTuple::Array<float> m_nt_Trk_theta0_Up;  //!< theta0 parameter (Up track)
  NTuple::Array<float> m_nt_Trk_qoverp_Up;  //!< q/p parameter (Up track)
  NTuple::Array<float> m_nt_Trk_pt_Up;      //!< pt parameter (Up track)
  // ----------------------------------------------------------------------

  // ----------------------------------------------------------------------
  NTuple::Array<long> m_nt_Trk_nHits_Up;      //!< number of hits (Up track)
  NTuple::Array<long> m_nt_Trk_nhitspix_Up;   //!< number of Pixel hits (Up track)
  NTuple::Array<long> m_nt_Trk_nhitssct_Up;   //!< number of SCT hits (Up track)
  NTuple::Array<long> m_nt_Trk_nhitstrt_Up;   //!<  number of TRT hits (Up track)

  NTuple::Array<long> m_nt_Trk_nsharedPixels_Up; //!< number of Pixel shared hits (Up track)
  NTuple::Array<long> m_nt_Trk_nsharedSCT_Up;    //!< number of SCT shared hits (Up track)
  NTuple::Array<long> m_nt_Trk_nshared_Up;       //!< number of shared hits (Up track)

  NTuple::Array<long> m_nt_Trk_nholesPixels_Up;  //!< number of Pixel holes (Up track)
  NTuple::Array<long> m_nt_Trk_nholesSCT_Up;     //!< number of SCT holes (Up track)
  NTuple::Array<long> m_nt_Trk_nholes_Up;        //!< number of holes (Up track)

  NTuple::Array<float> m_nt_Trk_chi2_Up;      //!< number of chi2 (Up track) 
  NTuple::Array<long> m_nt_Trk_ndof_Up;       //!< number of ndof (Up track) 
  NTuple::Array<float> m_nt_Trk_chi2Prob_Up;  //!< number of chi2 probability (Up track) 
 // ----------------------------------------------------------------------

  // ----------------------------------------------------------------------
  // Trk::Track parameters
  NTuple::Array<float> m_nt_Trk_d0_Low;      //!< d0 parameter (Low track)
  NTuple::Array<float> m_nt_Trk_z0_Low;      //!< z0 parameter (Low track)
  NTuple::Array<float> m_nt_Trk_phi0_Low;    //!< phi0 parameter (Low track)
  NTuple::Array<float> m_nt_Trk_theta0_Low;  //!< theta0 parameter (Low track)
  NTuple::Array<float> m_nt_Trk_qoverp_Low;  //!< q/p parameter (Low track)
  NTuple::Array<float> m_nt_Trk_pt_Low;      //!< pt parameter (Low track)
  // ----------------------------------------------------------------------

  // ----------------------------------------------------------------------
  NTuple::Array<long> m_nt_Trk_nHits_Low;      //!< number of hits (Low track)
  NTuple::Array<long> m_nt_Trk_nhitspix_Low;   //!< number of Pixel hits (Low track)
  NTuple::Array<long> m_nt_Trk_nhitssct_Low;   //!< number of SCT hits (Low track)
  NTuple::Array<long> m_nt_Trk_nhitstrt_Low;   //!<  number of TRT hits (Low track)

  NTuple::Array<long> m_nt_Trk_nsharedPixels_Low; //!< number of Pixel shared hits (Low track)
  NTuple::Array<long> m_nt_Trk_nsharedSCT_Low;    //!< number of SCT shared hits (Low track)
  NTuple::Array<long> m_nt_Trk_nshared_Low;       //!< number of shared hits (Low track)

  NTuple::Array<long> m_nt_Trk_nholesPixels_Low;  //!< number of Pixel holes (Low track)
  NTuple::Array<long> m_nt_Trk_nholesSCT_Low;     //!< number of SCT holes (Low track)
  NTuple::Array<long> m_nt_Trk_nholes_Low;        //!< number of holes (Low track)
  
  NTuple::Array<float> m_nt_Trk_chi2_Low;      //!< number of chi2 (Low track) 
  NTuple::Array<long> m_nt_Trk_ndof_Low;       //!< number of ndof (Low track) 
  NTuple::Array<float> m_nt_Trk_chi2Prob_Low;  //!< number of chi2 probability (Low track) 
  // ----------------------------------------------------------------------
  
  // ----------------------------------------------------------------------
  NTuple::Array<float> m_nt_mc_trkistruth;    //!< Has the Track an associated truth track?
  
  // generated particle parameters
  NTuple::Array<float> m_nt_mc_Trk_genParticlePt;  //!< generated pt
  NTuple::Array<float> m_nt_mc_Trk_genParticleEta; //!< generated eta 
  NTuple::Array<float> m_nt_mc_Trk_genParticlePhi; //!< generated phi
  
  // MonteCarlo Track parameters
  NTuple::Array<float> m_nt_mc_Trk_d0;        //!< MonteCarlo d0 parameter
  NTuple::Array<float> m_nt_mc_Trk_z0;        //!< MonteCarlo z0 parameter
  NTuple::Array<float> m_nt_mc_Trk_phi0;      //!< MonteCarlo phi0 parameter
  NTuple::Array<float> m_nt_mc_Trk_theta0;    //!< MonteCarlo theta0 parameter
  NTuple::Array<float> m_nt_mc_Trk_eta;       //!< MonteCarlo eta parameter
  NTuple::Array<float> m_nt_mc_Trk_qoverp;    //!< MonteCarlo q/p parameter
  NTuple::Array<float> m_nt_mc_Trk_qoverpt;   //!< MonteCarlo q/pt parameter
    
  NTuple::Array<float> m_nt_mc_Trk_pt;        //!< MonteCarlo pt parameter
  NTuple::Array<float> m_nt_mc_Trk_charge;    //!< MonteCarlo charge parameter
  NTuple::Array<float> m_nt_mc_Trk_prob;      //!< MonteCarlo prob parameter
  NTuple::Array<float> m_nt_mc_Trk_pdg;       //!< MonteCarlo pdg parameter
  
  NTuple::Array<float> m_nt_mc_Trk_vtxX;      //!< MonteCarlo Vertex.X parameter
  NTuple::Array<float> m_nt_mc_Trk_vtxY;      //!< MonteCarlo Vertex.Y parameter
  NTuple::Array<float> m_nt_mc_Trk_vtxZ;      //!< MonteCarlo Vertex.Z parameter
  // ----------------------------------------------------------------------


  // ----------------------------------------------------------------------
  // Matching Trk::Track_Up and Trk::Track_Low
  NTuple::Array<float> m_nt_Trk_delta_d0;      //!< d0 parameter
  NTuple::Array<float> m_nt_Trk_delta_z0;      //!< z0 parameter
  NTuple::Array<float> m_nt_Trk_delta_phi0;    //!< phi0 parameter
  //**
  NTuple::Array<float> m_nt_Trk_delta_theta0;  //!< theta parameter
  NTuple::Array<float> m_nt_Trk_delta_eta;     //!< eta parameter
  NTuple::Array<float> m_nt_Trk_delta_qoverpt; //!< q/pt parameter
  NTuple::Array<float> m_nt_Trk_delta_pt;      //!< pt parameter
  NTuple::Array<float> m_nt_Trk_delta_charge;  //!< charge parameter
  // ----------------------------------------------------------------------

};

#endif // INDETALIGNGENTOOLS_INDETALIGNFILLTRACK_H
