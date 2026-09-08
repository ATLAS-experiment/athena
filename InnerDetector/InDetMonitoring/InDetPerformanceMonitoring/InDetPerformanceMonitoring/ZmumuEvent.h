/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDPERFMON_ZMUMUEVENT_H
#define IDPERFMON_ZMUMUEVENT_H

//==============================================================================
// Include files...
//==============================================================================
#include "CLHEP/Vector/LorentzVector.h"
#include "InDetPerformanceMonitoring/MuonSelector.h"

#include "InDetPerformanceMonitoring/EventAnalysis.h"
#include "InDetPerformanceMonitoring/PerfMonServices.h"

#include "AsgTools/ToolHandle.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include <array>
//==============================================================================
// Forward class declarations...
//==============================================================================
class TrackParticle;

//==============================================================================
// Class declaration...
//==============================================================================
class ZmumuEvent : public EventAnalysis
{
 public:
  ZmumuEvent();
  virtual ~ZmumuEvent();

  enum
  {
    MUON1,
    MUON2,
    MUON3_OVR1,
    MUON4_OVR2,
    NUM_MUONS
  };

  enum
  {
    CENTRAL,
    FORWARD,
    BACKWARD,
    UNDEF
  };

  enum ZTYPE
  {
    MS,     // Just the muon system ( uncorrected )
    ME,     // The adjusted muon system properties ( corr. for cal. )
    ID,     // Using the ID system.
    CB,     // Using both the muon & ID system.
    NUM_TYPES
  };

  virtual void Init();
  // virtual bool Reco ATLAS_NOT_REENTRANT ();
  virtual bool Reco (int theLumiBlock = 0);

  // Public access methods
  inline void                        doIsoSelection (bool doIso)          { m_xMuonID.doIsoSelection(doIso);  }
  inline void                        doIPSelection (bool doIPsel)         { m_xMuonID.doIPSelection(doIPsel); }
  inline void                        doMCPSelection (bool doMCP)          { m_xMuonID.doMCPSelection(doMCP);  }
  inline bool                        AcceptEvent()                        { return m_passedSelectionCuts;   }
  void                               finalize();
  inline unsigned int                getAcceptedEvents ()                 { return m_acceptedEventCount; }      
  const xAOD::Muon*                  getCombMuon(  unsigned int uPart )   { return (uPart < NUM_MUONS) ? m_pxRecMuon[uPart] : NULL;  }
  const xAOD::TrackParticle*         getIDTrack (  unsigned int uPart )   { return (uPart < NUM_MUONS) ? m_pxIDTrack[uPart] : NULL;  }
  inline double                      GetInvMass ()                        {return m_DiMuonPairInvMass;}
  const float&                       getLeptonOpeningAngle( ZTYPE eType ) { return m_fMuonDispersion[eType]; }
  const xAOD::TrackParticle*         getLooseIDTk ATLAS_NOT_REENTRANT ( unsigned int uPart );
  const xAOD::TrackParticle*         getMSTrack (  unsigned int uPart )   { return (uPart < NUM_MUONS) ? m_pxMSTrack[uPart] : NULL;  }
  unsigned int                       getNegMuon( ZTYPE eType );
  inline unsigned int                getNumberOfTaggedMuons()             { return m_numberOfFullPassMuons; }
  unsigned int                       getPosMuon( ZTYPE eType );
  float                              getPtImbalance( ZTYPE eType );
  const std::string                  getRegion() const ;
  inline unsigned int                getTestedMuonCount()                 { return m_testedMuonCount; }
  int                                getZCharge( ZTYPE eType );
  const float&                       getZEta(  ZTYPE eType )              { return m_fZEtaDir[eType];        }
  const float&                       getZMass( ZTYPE eType )              { return m_fInvariantMass[eType];  }
  const float&                       getZPhi(  ZTYPE eType )              { return m_fZPhiDir[eType];        }
  const float&                       getZPt(   ZTYPE eType )              { return m_fZPt[eType];            }
  void                               OrderMuonList ();
  inline void                        setDebugMode (bool debug)            { m_doDebug=debug; }
  inline void                        SetMaxLumiBlock (int newlumiblock)   { m_maxGoodLumiBlock = newlumiblock; }
  inline void                        SetMinLumiBlock (int newlumiblock)   { m_minGoodLumiBlock = newlumiblock; }
  inline void                        SetMuonPtCut (double newvalue)       { m_xMuonID.SetPtCut(newvalue); }
  inline void                        SetMuonQuality (const std::string& newname) { m_xMuonID.SetMuonQualityRequirement(newname); }

  inline void                        SetMassWindowLow (double newvalue)   { m_MassWindowLow = newvalue; }
  inline void                        SetMassWindowHigh (double newvalue)  { m_MassWindowHigh = newvalue; }
  void                               SetLeadingMuonPtCut (double newvalue); 
  void                               SetSecondMuonPtCut (double newvalue); 
  inline void                        SetOpeningAngleCut (double newvalue) {m_OpeningAngleCut = newvalue;}
  inline void                        SetZ0GapCut (double newvalue)        {m_Z0GapCut = newvalue;}
  inline void                        SetSkipMSCheck (bool value)          {m_skipMScheck = value;}

  inline void                        setContainer ( PerfMonServices::CONTAINERS container) { m_container = container; };
  inline void SetMuonSelectionTool ( const ToolHandle<CP::IMuonSelectionTool> & mst ) { m_muonSelectionTool = mst;  m_xMuonID.SetCustomMuonSelectionTool (mst); };

 protected:
  virtual void BookHistograms();

 private:
  typedef EventAnalysis PARENT;

  // Private methods
  void  Clear();
  bool  EventSelection (ZTYPE eType);
  void  ReconstructKinematics();
  bool  RecordMuon( const xAOD::Muon* pxMuon );

  // Active mu-cuts for the analysis
  MuonSelector            m_xMuonID;
  PerfMonServices::CONTAINERS m_container;
  ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool;

  // Tag Setup variables
  unsigned int m_uMuonTags;
  unsigned int m_uTrackMatch;
  bool m_bLooseMatch;
  double m_etaCut;
  double m_DiMuonPairInvMass = 0.0;

  double m_LeadingMuonPtCut;
  double m_SecondMuonPtCut;
  double m_MassWindowLow;
  double m_MassWindowHigh;
  double m_OpeningAngleCut;
  double m_Z0GapCut;
  bool   m_skipMScheck;
  int    m_minGoodLumiBlock;
  int    m_maxGoodLumiBlock;
 
  bool m_doDebug;
  // Member variables : Mostly to store relevant muon data for quick access.
  unsigned int     m_numberOfFullPassMuons{};
  bool             m_passedSelectionCuts{};
  int              m_analyzedEventCount{};
  unsigned int     m_eventsWithoutEnoughMuonsCount{};
  unsigned int     m_eventsWithEnoughMuonsCount{};
  unsigned int     m_acceptedEventCount{};
  unsigned int     m_testedMuonCount{};
  unsigned int     m_acceptedMuonCount{};
  unsigned int     m_eventselectioncount_toofewmuons{};
  unsigned int     m_eventselectioncount_notallmuonsfilled{};
  unsigned int     m_eventselectioncount_morethantwomuons{};
  unsigned int     m_eventselectioncount_ptofleadingmuon{};
  unsigned int     m_eventselectioncount_ptofsecondmuon{};
  unsigned int     m_eventselectioncount_masswindow{};
  unsigned int     m_eventselectioncount_openingangle{};
  unsigned int     m_eventselectioncount_dimuoncharge{};
  unsigned int     m_eventselectioncount_goodlumiblock{};


  const            xAOD::Muon*      m_pxRecMuon[NUM_MUONS]{};
  const            xAOD::TrackParticle*  m_pxMETrack[NUM_MUONS]{};  // Pointer to muon spectro ( corr. )
  const            xAOD::TrackParticle*  m_pxMSTrack[NUM_MUONS]{};      // Pointer to muon spectro
  const            xAOD::TrackParticle*  m_pxIDTrack[NUM_MUONS]{};       // Pointer to ID track

  // Keep kinematic information on the Z
  using Arrayf = std::array<float, NUM_TYPES>;
  Arrayf m_fZPt{};
  Arrayf m_fZEtaDir{};
  Arrayf m_fZPhiDir{};
  Arrayf m_fInvariantMass{};
  Arrayf m_fMuonDispersion{};

  // Graphs
  enum HISTOS_1D
  {
    ZMASS_MUON, ZMASS_MUONADJ, ZMASS_TRACK, ZMASS_COMB,
    NUM_1HISTOS
  };

  // muon selector configuration
  bool m_SelectMuonByIso{};
  bool m_SelectMuonByIP{};

  // selected muon identifiers
  int m_muon1 = 0;
  int m_muon2 = 0;

};
//==============================================================================
#endif
