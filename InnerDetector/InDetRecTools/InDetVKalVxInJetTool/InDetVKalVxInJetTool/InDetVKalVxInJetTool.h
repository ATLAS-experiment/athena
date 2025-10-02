/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///
/// InDetVKalVxInJetTool.h - Description
///
/**
   Tool for secondary vertex inside jet reconstruction.
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to xAOD::Vertex's of found secondary verteces.
   In case of failure pointer to Trk::VxSecVertexInfo is 0.
   

   Package creates a derivative object VxSecVKalVertexInfo which contains also additional variables
   see  Tracking/TrkEvent/VxSecVertex/VxSecVertex/VxSecVKalVertexInfo.h
   
    
   Additional returned values:
      Results    - vector of variables for b-tagging
                   [0] - secondary vertex mass
                   [1] - energy ratio   Esec/Ejet
		   [2] - number of 2-track vertices
		   [3] - pointers to "bad" tracks
		   [4] - pointers to identified V0 tracks

  Package has "single vertex" and "multiple vertices" modes of work.

  More details at

     https://twiki.cern.ch/twiki/bin/save/Atlas/BTagVrtSec

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch

-----------------------------------------------------------------------------*/



#ifndef _VKalVrt_InDetVKalVxInJetTool_H
#define _VKalVrt_InDetVKalVxInJetTool_H
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "StoreGate/ReadHandle.h"

//Remove in boost > 1.76 when the boost iterator issue
//is solved see ATLASRECTS-6358
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include "boost/graph/adjacency_list.hpp"
//
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODEventInfo/EventInfo.h"
//
//
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"
#include "InDetRecToolInterfaces/ISecVertexInJetFinder.h"
#include "InDetVKalVxInJetTool/InDetTrkInJetType.h"

class TH1D;
class TH2D;
class TH1F;
class TProfile;
class TTree;
class IChronoStatSvc;
class ITHistSvc;

namespace Trk{
  class TrkVKalVrtFitter;
  class IVertexFitter;
  class IVKalState;
}

class BeamPipeDetectorManager;

namespace InDetDD {
  class PixelDetectorManager;
}

typedef std::vector<double> dvect;

 
//------------------------------------------------------------------------
namespace InDet {


  struct workVectorArrxAOD{
        std::vector<const xAOD::TrackParticle*> listJetTracks;
        std::vector<const xAOD::TrackParticle*> tmpListTracks;
        std::vector<const xAOD::TrackParticle*> listSecondTracks;
        std::vector<const xAOD::TrackParticle*> TracksForFit;
        std::vector<const xAOD::TrackParticle*> InpTrk;
        std::vector< std::vector<const xAOD::TrackParticle*> >  FoundSecondTracks;
		     std::vector<const xAOD::TrackParticle*>    TrkFromV0; 
  };
  
  class InDetVKalVxInJetTool : 
    public AthAlgTool, virtual public ISecVertexInJetFinder{


  public:
       /* Constructor */
      InDetVKalVxInJetTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~InDetVKalVxInJetTool();


      StatusCode initialize();
      StatusCode finalize();

      Trk::VxSecVertexInfo* findSecVertex(const xAOD::Vertex & primaryVertex,
                                                const TLorentzVector & jetMomentum,
                                                const std::vector<const xAOD::IParticle*> & inputTracks) const;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

    private:

      struct DevTuple;
      struct Hists {
        StatusCode book (ITHistSvc& histSvc, const std::string& histDir);
        TTree* m_tuple{};
        DevTuple* m_curTup;
        TH1D* m_hb_massPiPi{};
        TH1D* m_hb_massPiPi1{};
        TH1D* m_hb_massPPi{};
        TH1D* m_hb_massEE{};
        TH1D* m_hb_totmassEE{};
        TH1D* m_hb_totmass2T0{};
        TH1D* m_hb_totmass2T1{};
        TH1D* m_hb_totmass2T2{};
        TH1D* m_hb_nvrt2{};
        TH1D* m_hb_ratio{};
        TH1D* m_hb_totmass{};
        TH1D* m_hb_impact{};
        TH1D* m_hb_impactR{};
        TH2D* m_hb_impactRZ{};
        TH1D* m_hb_trkD0{};
        TH1D* m_hb_ntrkjet{};
        TH1D* m_hb_impactZ{};
        TH1D* m_hb_r2d{};
        TH1D* m_hb_r1dc{};
        TH1D* m_hb_r2dc{};
        TH1D* m_hb_r3dc{};
        TH1D* m_hb_rNdc{};
        TH1D* m_hb_dstToMat{};
        TH1D* m_hb_jmom{};
        TH1D* m_hb_mom{};
        TH1D* m_hb_signif3D{};
        TH1D* m_hb_impV0{};
        TH1D* m_hb_sig3DTot{};
        TH1F* m_hb_goodvrtN{};
        TH1D* m_hb_distVV{};
        TH1D* m_hb_diffPS{};
        TH1D* m_hb_sig3D1tr{};
        TH1D* m_hb_sig3D2tr{};
        TH1D* m_hb_sig3DNtr{};
        TH1D* m_hb_trkPtMax{};
        TH1F* m_hb_rawVrtN{};
        TH1F* m_hb_lifetime{};
        TH1F* m_hb_trkPErr{};
        TH1F* m_hb_deltaRSVPV{};
//--
        TProfile * m_pr_NSelTrkMean{};
        TProfile * m_pr_effVrt2tr{};
        TProfile * m_pr_effVrt2trEta{};
        TProfile * m_pr_effVrt{};
        TProfile * m_pr_effVrtEta{};
      };
      std::unique_ptr<Hists> m_h;

    IntegerProperty m_cutSctHits{this, "CutSctHits", 4,
      "Remove track is it has less SCT hits"};
    IntegerProperty m_cutPixelHits{this, "CutPixelHits", 1,
      "Remove track is it has less Pixel hits"};
    IntegerProperty m_cutSiHits{this, "CutSiHits", 7,
      "Remove track is it has less Pixel+SCT hits"};
    IntegerProperty m_cutBLayHits{this, "CutBLayHits", 0,
      "Remove track is it has less B-layer hits"};
    IntegerProperty m_cutSharedHits{this, "CutSharedHits", 1000,
      "Reject final 2tr vertices if tracks have shared hits"};
    DoubleProperty m_cutPt{this, "CutPt", 700., "Track Pt selection cut"};
    DoubleProperty m_cutZVrt{this, "CutZVrt", 15., "Track Z impact selection cut"};
    DoubleProperty m_cutA0{this, "CutA0", 5., "Track A0 selection cut"};
    DoubleProperty m_cutChi2{this, "CutChi2", 5., "Track Chi2 selection cut"};
    DoubleProperty m_secTrkChi2Cut{this, "SecTrkChi2Cut", 10.,
      "Track - common secondary vertex association cut. Single Vertex Finder only"};
    DoubleProperty m_coneForTag{this, "ConeForTag", 0.4,
      "Cone around jet direction for track selection"};
    DoubleProperty m_sel2VrtChi2Cut{this, "Sel2VrtChi2Cut", 10.,
      "Cut on Chi2 of 2-track vertex for initial selection"};
    DoubleProperty m_sel2VrtSigCut{this, "Sel2VrtSigCut", 4.,
      "Cut on significance of 3D distance between initial 2-track vertex and PV"};
    DoubleProperty m_trkSigCut{this, "TrkSigCut", 2.,
      "Track 3D impact significance w/r primary vertex"};
    DoubleProperty m_a0TrkErrorCut{this, "A0TrkErrorCut", 1.,
      "Track A0 error cut"};
    DoubleProperty m_zTrkErrorCut{this, "ZTrkErrorCut", 5.,
      "Track Z impact error cut"};
    DoubleProperty m_cutBVrtScore{this, "CutBVrtScore", 0.015,
      "B vertex selection cut on 2track vertex score (probability-like) based on track classification"};
    DoubleProperty m_vrt2TrMassLimit{this, "Vrt2TrMassLimit", 4000.,
      "Maximal allowed mass for 2-track vertices"};

    BooleanProperty m_useFrozenVersion{this, "UseFrozenVersion", false,
      "Switch from default frozen version to the development/improved one"};
    BooleanProperty m_fillHist{this, "FillHist", false,
      "Fill technical histograms"};
    BooleanProperty m_existIBL{this, "ExistIBL", true,
      "Inform whether 3-layer or 4-layer detector is used"};

    IntegerProperty m_RobustFit{this, "RobustFit", 1,
      "Use vertex fit with RobustFit functional(VKalVrt) for common secondary vertex fit"};

    double m_beampipeR = 0.;
    double m_rLayerB = 0.;
    double m_rLayer1 = 0.;
    double m_rLayer2 = 0.;
    double m_rLayer3 = 0.;

    BooleanProperty m_useVertexCleaningPix{this, "useVertexCleaningPix", false,
      "Clean vertices requiring track pixel hit patterns according to vertex position"};
    BooleanProperty m_useVertexCleaningFMP{this, "useVertexCleaningFMP", false,
      "Clean vertices requiring track F(irst) M(easured) P(oints) matching to vertex position"};
    BooleanProperty m_rejectBadVertices{this, "rejectBadVertices", false,
      "Reject V0s after checking 3D PV impact"};
    BooleanProperty m_multiVertex{this, "MultiVertex", false,
      "Run Multiple Secondary Vertices in jet finder"};
    BooleanProperty m_multiWithPrimary{this, "MultiWithPrimary", false,
      "Find Multiple Secondary Vertices + primary vertex in jet. MultiVertex Finder only!"};
    BooleanProperty m_getNegativeTail{this, "getNegativeTail", false,
      "Allow secondary vertex behind the primary one (negative) w/r jet direction (not for multivertex!)"};
    BooleanProperty m_getNegativeTag{this, "getNegativeTag", false,
      "Return ONLY negative secondary vertices (not for multivertex!)"};
    BooleanProperty m_multiWithOneTrkVrt{this, "MultiWithOneTrkVrt", true,
      "Allow one-track-vertex addition to already found secondary vertices. MultiVertex Finder only!"};

    DoubleProperty m_vertexMergeCut{this, "VertexMergeCut", 3.,
      "To allow vertex merging for MultiVertex Finder"};
    DoubleProperty m_trackDetachCut{this, "TrackDetachCut", 6.,
      "To allow track from vertex detachment for MultiVertex Finder"};

    ToolHandle < Trk::IVertexFitter > m_fitter
      {this, "VertexFitterTool", "Trk::TrkVKalVrtFitter/VertexFitterTool"};
    Trk::TrkVKalVrtFitter* m_fitSvc{};
    ServiceHandle<IChronoStatSvc> m_timingProfile{this,"ChronoStatSvc","ChronoStatSvc"};

    bool m_useTrackClassificator = true;
    ToolHandle < IInDetTrkInJetType >  m_trackClassificator
      {this, "TrackClassTool", "InDet::InDetTrkInJetType"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      {this, "EventInfoName", "EventInfo"};

    bool m_useEtaDependentCuts = false;
    /** service to get cut values depending on different variable */
    ServiceHandle<InDet::IInDetEtaDependentCutsSvc> m_etaDependentCutsSvc
      {this, "InDetEtaDependentCutsSvc", ""};

    BooleanProperty m_useITkMaterialRejection
      {this, "useITkMaterialRejection", false,
       "Reject vertices from hadronic interactions in detector material using ITk layout"};
    const BeamPipeDetectorManager*       m_beamPipeMgr = nullptr;
    const InDetDD::PixelDetectorManager* m_pixelManager = nullptr;
    std::unique_ptr<TH2D> m_ITkPixMaterialMap;

    const double m_massPi  = ParticleConstants::chargedPionMassInMeV ;
    const double m_massP   = ParticleConstants::protonMassInMeV  ;
    const double m_massE   = ParticleConstants::electronMassInMeV  ;
    const double m_massK0  = ParticleConstants::KZeroMassInMeV  ;
    const double m_massLam = ParticleConstants::lambdaMassInMeV  ;
    const double m_massB   =5279.400  ;

//-------------------------------------------
//For ntuples (only for development/tuning!)

      static int notFromBC(int PDGID) ;
      static const xAOD::TruthParticle * getPreviousParent(const xAOD::TruthParticle * child, int & ParentPDG) ;
      static int getIdHF(const xAOD::TrackParticle* TP ) ;
      static int getG4Inter( const xAOD::TrackParticle* TP ) ;
      static int getMCPileup(const xAOD::TrackParticle* TP ) ;

      struct DevTuple 
     { 
       static const int maxNTrk=100;
       static const int maxNVrt=100;
       int nTrkInJet;
       float ewgt;
       float ptjet;
       float etajet;
       float phijet;
       float etatrk[maxNTrk];
       float p_prob[maxNTrk];
       float s_prob[maxNTrk];
       int   idMC[maxNTrk];
       float SigR[maxNTrk];
       float SigZ[maxNTrk];
       float   d0[maxNTrk];
       float   Z0[maxNTrk];
       float pTvsJet[maxNTrk];
       float  prodTJ[maxNTrk];
       float    wgtB[maxNTrk];
       float    wgtL[maxNTrk];
       float    wgtG[maxNTrk];
       float   sig3D[maxNTrk];
       int    chg[maxNTrk];
       int  nVrtT[maxNTrk];
       float TotM;
       int   nVrt;
       float VrtDist2D[maxNVrt];
       float VrtSig3D[maxNVrt];
       float VrtSig2D[maxNVrt];
       float VrtDR[maxNVrt];
       float VrtdRtt[maxNVrt];
       float VrtErrR[maxNVrt];
       float mass[maxNVrt];
       float Chi2[maxNVrt];
       int   itrk[maxNVrt];
       int   jtrk[maxNVrt];
       int badVrt[maxNVrt];
       int    ibl[maxNVrt];
       int     bl[maxNVrt];
       float fhitR[maxNVrt];
       int        NTHF;
       int   itHF[maxNVrt];
       //---
       int   nNVrt;
       float NVrtDist2D[maxNVrt];
       float NVrtSig3D[maxNVrt];
       int   NVrtNT[maxNVrt];
       int   NVrtTrkI[maxNVrt];
       float NVrtM[maxNVrt];
       float NVrtChi2[maxNVrt];
       float NVrtMaxW[maxNVrt];
       float NVrtAveW[maxNVrt];
       float NVrtDR[maxNVrt];
     };

     struct Vrt2Tr 
     {   int i=0, j=0;
         int badVrt=0;
         Amg::Vector3D     fitVertex;
         TLorentzVector    momentum;
         long int   vertexCharge;
         std::vector<double> errorMatrix;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double signif3D=0.;
         double signif3DProj=0.;
         double signif2D=0.;
         double chi2=0.;
         double dRSVPV=-1.;
     };


// For multivertex version only

      using compatibilityGraph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS>;
      float m_chiScale[11]{};
      struct WrkVrt 
     {   bool Good=true;
         std::deque<long int> selTrk;
         Amg::Vector3D     vertex;
         TLorentzVector    vertexMom;
         long int   vertexCharge{};
         std::vector<double> vertexCov;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double chi2{};
         int nCloseVrt=0;
         double dCloseVrt=1000000.;
	 double projectedVrt=0.;
         int detachedTrack=-1;
      };


//   Private technical functions
//
//
      xAOD::Vertex*      getVrtSec(const std::vector<const xAOD::TrackParticle*> & inpTrk,
                                   const xAOD::Vertex                            & primVrt,
                                   const TLorentzVector                          & jetDir,
                                   std::vector<double>                           & results,
                                   std::vector<const xAOD::TrackParticle*>       & selSecTrk,
                                   std::vector<const xAOD::TrackParticle*>       & trkFromV0,
				   int & nRefPVTrk,
                                   compatibilityGraph_t                          & compatibilityGraph) const;

      std::vector<xAOD::Vertex*> getVrtSecMulti(
        workVectorArrxAOD*,
        const xAOD::Vertex& primVrt,
        const TLorentzVector& jetDir,
        std::vector<double>& results,
        compatibilityGraph_t& compatibilityGraph) const;

      static void  trackClassification(std::vector<WrkVrt> *wrkVrtSet, 
                                std::vector< std::deque<long int> > *trkInVrt) ;

      static double MaxOfShared(std::vector<WrkVrt> *WrkVrtSet, 
                         std::vector< std::deque<long int> > *trkInVrt,
			 long int & selectedTrack,
			 long int & selectedVertex) ;
      void removeTrackFromVertex(std::vector<WrkVrt> *wrkVrtSet, 
                                 std::vector< std::deque<long int> > *trkInVrt,
				 long int & selectedTrack,
				 long int & selectedVertex) const;
//
//

      void printWrkSet(const std::vector<WrkVrt> * WrkSet, const std::string& name ) const;


      StatusCode cutTrk(const std::unordered_map<std::string,double>& TrkVarDouble,
                        const std::unordered_map<std::string,int>& TrkVarInt,
			float evtWgt=1.) const;
      static double coneDist(const AmgVector(5) & , const TLorentzVector & ) ;
//
// Gives correct mass assignment in case of nonequal masses
      static double massV0( std::vector< std::vector<double> >& trkAtVrt, double massP, double massPi ) ;
      static int findMax( std::vector<double>& chi2PerTrk, std::vector<float>&  rank) ;


      TLorentzVector totalMom(const std::vector<const Trk::Perigee*>& inpTrk) const; 
      static TLorentzVector totalMom(const std::vector<const xAOD::TrackParticle*>& inpTrk) ; 
      TLorentzVector momAtVrt(const std::vector<double>& inpTrk) const; 
      static double           pTvsDir(const Amg::Vector3D &Dir, const std::vector<double>& inpTrk) ; 
      static double           vrtRadiusError(const Amg::Vector3D & secVrt, const std::vector<double>  & vrtErr) ;

      bool  insideMatLayer(float ,float ) const;
      void  fillVrtNTup( std::vector<Vrt2Tr> & all2TrVrt) const;
      void  fillNVrtNTup(std::vector<WrkVrt> & vrtSet, std::vector< std::vector<float> > & trkScore,
                         const xAOD::Vertex   & primVrt, const TLorentzVector & jetDir)const;

      TLorentzVector getBDir( const xAOD::TrackParticle* trk1,
                              const xAOD::TrackParticle* trk2,
                              const xAOD::Vertex    & primVrt,
			      Amg::Vector3D &V1, Amg::Vector3D &V2) const;

      static int   nTrkCommon( std::vector<WrkVrt> *WrkVrtSet, int V1, int V2) ;
      double minVrtVrtDist( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) const;
      static double minVrtVrtDistNext( std::vector<WrkVrt> *WrkVrtSet, int & V1, int & V2) ;
      static bool isPart( std::deque<long int> test, std::deque<long int> base) ;
      static void clean1TrVertexSet(std::vector<WrkVrt> *WrkVrtSet) ;
      static double jetProjDist(Amg::Vector3D &SecVrt, const xAOD::Vertex &primVrt, const TLorentzVector &JetDir) ;

      double vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double>& VrtErr,double& Signif ) const;
      double vrtVrtDist2D(const xAOD::Vertex & primVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double>& VrtErr,double& Signif ) const;
      double vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & SecVrt, 
                                  const std::vector<double>& SecVrtErr, const TLorentzVector & JetDir) const;
      double vrtVrtDist(const Amg::Vector3D & Vrt1, const std::vector<double>& VrtErr1,
                        const Amg::Vector3D & Vrt2, const std::vector<double>& VrtErr2) const;
 
      template <class Particle>
      void disassembleVertex(std::vector<WrkVrt> *WrkVrtSet, int iv, 
                             std::vector<const Particle*>  AllTracks,
                             Trk::IVKalState& istate) const;
					  
      static double projSV_PV(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Jet) ;

      double rankBTrk(double TrkPt, double JetPt, double Signif) const;
 

      static const Trk::Perigee* getPerigee( const xAOD::TrackParticle* ) ;
      std::vector<const Trk::Perigee*> GetPerigeeVector( const std::vector<const Trk::TrackParticleBase*>& ) const;


      template <class Trk>
      double fitCommonVrt(std::vector<const Trk*>& listSecondTracks,
                          std::vector<float>   & trkRank,
                          const xAOD::Vertex   & primVrt,
                          const TLorentzVector & jetDir,
                          std::vector<double>  & inpMass, 
                          Amg::Vector3D        & fitVertex,
                          std::vector<double>  & errorMatrix,
                          TLorentzVector       & momentum,
           std::vector< std::vector<double> >  & trkAtVrt) const; 

      template <class Trk>
      void removeEntryInList(std::vector<const Trk*>& , std::vector<float>&, int) const;
      template <class Trk>
      void removeDoubleEntries(std::vector<const Trk*>& ) const;

      template <class Particle>
      StatusCode refitVertex( std::vector<WrkVrt> *WrkVrtSet, int selectedVertex,
                              std::vector<const Particle*> & selectedTracks,
                              Trk::IVKalState& istate,
                              bool ifCovV0) const;

      template <class Particle>
      double refitVertex( WrkVrt &Vrt,std::vector<const Particle*> & SelectedTracks,
                          Trk::IVKalState& istate,
                          bool ifCovV0) const;

      template <class Particle>
      double mergeAndRefitVertices( std::vector<WrkVrt> *WrkVrtSet, int V1, int V2, WrkVrt & newvrt,
                                    std::vector<const Particle*> & AllTrackList,
                                    Trk::IVKalState& istate) const;
      template <class Particle>
      void   mergeAndRefitOverlapVertices( std::vector<WrkVrt> *WrkVrtSet, int V1, int V2,
                                           std::vector<const Particle*> & AllTrackLis,
                                           Trk::IVKalState& istate) const;

      template <class Particle>
      double  improveVertexChi2( std::vector<WrkVrt> *WrkVrtSet, int V, std::vector<const Particle*> & AllTracks,
                                 Trk::IVKalState& istate,
                                 bool ifCovV0) const;

     int   selGoodTrkParticle( const std::vector<const xAOD::TrackParticle*>& inpPart,
                                const xAOD::Vertex                           & primVrt,
                                const TLorentzVector                         & jetDir,
                                      std::vector<const xAOD::TrackParticle*>& selPart,
                                float evtWgt=1.) const;


      template <class Trk>
      int select2TrVrt(std::vector<const Trk*>  & SelectedTracks,
                        std::vector<const Trk*>  & TracksForFit,
                        const xAOD::Vertex       & primVrt,
                        const TLorentzVector     & JetDir,
                        std::vector<double>      & InpMass, 
                        int                      & nRefPVTrk,
                        std::vector<const Trk*>  & TrkFromV0,
                        std::vector<const Trk*>  & ListSecondTracks,
                        compatibilityGraph_t     & compatibilityGraph,
                        float evtWgt = 1) const;

     static Amg::MatrixX  makeVrtCovMatrix( std::vector<double> & ErrorMatrix ) ;


//////////////////////////////////////////////////////////////////////////////////////////
//   Needed for TrackParticle<->TrackParticleBase story!!!!
//
//     template <class Track>
//     StatusCode VKalVrtFitFastBase(const std::vector<const Track*>& listPart,Amg::Vector3D& Vertex) const;
     StatusCode VKalVrtFitFastBase(const std::vector<const xAOD::TrackParticle*>& listPart,Amg::Vector3D& Vertex, Trk::IVKalState& istate) const;

     template <class Track>
     bool  check2TrVertexInPixel( const Track* p1, const Track* p2, Amg::Vector3D &, std::vector<double> &) const;
     template <class Track>
     bool  check1TrVertexInPixel( const Track* p1, Amg::Vector3D &, std::vector<double> & ) const;

     void  getPixelLayers(const xAOD::TrackParticle* Part, int &blHit, int &l1Hit, int &l2Hit, int &nLay) const;
     static void  getPixelDiscs(const xAOD::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) ;
     void  getPixelProblems(const xAOD::TrackParticle* Part, int &splshIBL, int &splshBL ) const;


     StatusCode VKalVrtFitBase(const std::vector<const xAOD::TrackParticle*> & listPart,
                                Amg::Vector3D&                 Vertex,
                                TLorentzVector&                Momentum,
                                long int&                      Charge,
                                std::vector<double>&           ErrorMatrix,
                                std::vector<double>&           Chi2PerTrk,
                                std::vector< std::vector<double> >& TrkAtVrt,
                                double& Chi2,
                                Trk::IVKalState& istate,
                                bool ifCovV0) const;

     StatusCode GetTrkFitWeights(std::vector<double> & wgt,
                                 const Trk::IVKalState& istate) const;

     Hists& getHists() const;
   };


  template <class Trk>
  void InDetVKalVxInJetTool::removeEntryInList(std::vector<const Trk*>& ListTracks, std::vector<float> & rank, int Outlier) const
  {
    if(Outlier < 0 ) return;
    if(Outlier >= (int)ListTracks.size() ) return;
    ListTracks.erase( ListTracks.begin()+Outlier);
    rank.erase( rank.begin()+Outlier);
  }     

  template <class Trk>
  void InDetVKalVxInJetTool::removeDoubleEntries(std::vector<const Trk*>& ListTracks) const
  {
    typename std::vector<const Trk*>::iterator   TransfEnd;
    sort(ListTracks.begin(),ListTracks.end());
    TransfEnd =  unique(ListTracks.begin(),ListTracks.end());
    ListTracks.erase( TransfEnd, ListTracks.end());
  }     

  struct clique_visitor
  {
    clique_visitor(std::vector< std::vector<int> > & input): m_allCliques(input){ input.clear();}
    
    template <typename Clique, typename Graph>
    void clique(const Clique& clq, Graph& )
    { 
      std::vector<int> new_clique(0);
      for(auto i = clq.begin(); i != clq.end(); ++i) new_clique.push_back(*i);
      m_allCliques.push_back(new_clique);
    }

    std::vector< std::vector<int> > & m_allCliques;

  };

   template <class Track>
   bool InDetVKalVxInJetTool::check1TrVertexInPixel( const Track* p1, Amg::Vector3D &FitVertex, std::vector<double> &VrtCov)
   const
   {
        int blTrk=0, blP=0, l1Trk=0, l1P=0, l2Trk=0, nLays=0; 
        getPixelLayers( p1, blTrk , l1Trk, l2Trk, nLays );
        getPixelProblems(p1, blP, l1P );
        double radiusError=vrtRadiusError(FitVertex, VrtCov);
        double xvt=FitVertex.x();
        double yvt=FitVertex.y();
        double R=std::hypot(xvt, yvt);
        if(R < m_rLayerB-radiusError){           // Inside B-layer
          if( blTrk<1  && l1Trk<1 )  return false;
          if(  nLays           <2 )   return false;  // Less than 2 layers on track 0
          return true;
        }else if(R > m_rLayerB+radiusError){     // Outside b-layer
          if( blTrk>0 && blP==0 ) return false;  // Good hit in b-layer is present
       }
// 
// L1 and L2 are considered only if vertex is in acceptance
//
        if(std::abs(FitVertex.z())<400.){
          if(R < m_rLayer1-radiusError) {        // Inside 1st-layer
             if( l1Trk<1  && l2Trk<1 )     return false;  // Less than 1 hits on track 0
             return true;
          }else if(R > m_rLayer1+radiusError) {  // Outside 1st-layer
             if( l1Trk>0 && l1P==0 )       return false;  //  Good L1 hit is present
          }
          
          if(R < m_rLayer2-radiusError) {        // Inside 2nd-layer
             if( l2Trk==0 )  return false;           // At least one L2 hit must be present
          }
        } else {
          int d0Trk=0, d1Trk=0, d2Trk=0; 
          getPixelDiscs( p1, d0Trk , d1Trk, d2Trk );
          if( d0Trk+d1Trk+d2Trk ==0 )return false;
        }
        return true;
   }

}  //end namespace

#endif
