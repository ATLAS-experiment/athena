/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// NewVrtSecInclusiveTool.h - Description
//
/*
   Tool for inclusive secondary vertex reconstruction
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to xAOD::Vertex's of found secondary verteces.
   In case of failure pointer to Trk::VxSecVertexInfo is 0.
   

   Tool creates a derivative object VxSecVKalVertexInfo which contains also additional variables
   see  Tracking/TrkEvent/VxSecVertex/VxSecVertex/VxSecVKalVertexInfo.h
   

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch

-----------------------------------------------------------------------------*/



#ifndef _VKalVrt_NewVrtSecInclusiveTool_H
#define _VKalVrt_NewVrtSecInclusiveTool_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
//Remove in boost > 1.76 when the boost iterator issue
//is solved see ATLASRECTS-6358
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include "boost/graph/adjacency_list.hpp"
//
#include "xAODTruth/TruthEventContainer.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "NewVrtSecInclusiveTool/IVrtInclusive.h"
#include "StoreGate/WriteDecorHandleKey.h"

class TH1D;
class TH2D;
class TH1F;
class TProfile;
class TTree;
class ITHistSvc;

namespace Trk{
  class TrkVKalVrtFitter;
  class IVertexFitter;
  class IVKalState;
}
 
namespace MVAUtils { class BDT; }

 
//------------------------------------------------------------------------
namespace Rec {

  struct workVectorArrxAOD{
        std::vector<const xAOD::TrackParticle*> listSelTracks;  // Selected tracks after quality cuts
        std::vector<const xAOD::TrackParticle*> tmpListTracks;
        std::vector<const xAOD::TrackParticle*> inpTrk;         // All tracks provided to tool
        double beamX=0.;
        double beamY=0.;
        double beamZ=0.;
        double tanBeamTiltX=0.;
        double tanBeamTiltY=0.;
  };

  class NewVrtSecInclusiveTool : public AthAlgTool, virtual public IVrtInclusive
  {


  public:
       /* Constructor */
      NewVrtSecInclusiveTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~NewVrtSecInclusiveTool();


      StatusCode initialize();
      StatusCode finalize();



      std::unique_ptr<Trk::VxSecVertexInfo> findAllVertices(const std::vector<const xAOD::TrackParticle*> & inputTracks,
                                                                                     const xAOD::Vertex & primaryVertex) const final;
//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

    private:

      double m_w_1{};
      struct DevTuple;
      struct Hists {
        StatusCode book (ITHistSvc& histSvc, const std::string& histDir);
        TTree* m_tuple{};
        DevTuple*  m_curTup;
        TH1D* m_hb_massPiPi{};
        TH1D* m_hb_massPiPi1{};
        TH1D* m_hb_massPPi{};
        TH1D* m_hb_massEE{};
        TH1D* m_hb_nvrt2{};
        TH1D* m_hb_ratio{};
        TH1D* m_hb_totmass{};
        TH1D* m_hb_impact{};
        TH1D* m_hb_impactR{};
        TH2D* m_hb_impactRZ{};
        TH1D* m_hb_trkD0{};
        TH1D* m_hb_trkZ{};
        TH1F* m_hb_ntrksel{};
        TH1F* m_hb_ntrkInput{};
        TH1F* m_hb_trkSelect{};
        TH1D* m_hb_impactZ{};
        TH1D* m_hb_r2d{};
        TH1D* m_hb_signif3D{};
        TH1D* m_hb_impV0{};
        TH1D* m_hb_sig3DTot{};
        TH1F* m_hb_goodvrtN{};
        TH1F* m_hb_goodvrt1N{};
        TH1D* m_hb_distVV{};
        TH1D* m_hb_diffPS{};
        TH1D* m_hb_sig3D1tr{};
        TH1D* m_hb_sig3D2tr{};
        TH1D* m_hb_sig3DNtr{};
        TH1F* m_hb_rawVrtN{};
        TH1F* m_hb_cosSVMom{};
        TH1F* m_hb_etaSV{};
        TH1F* m_hb_fakeSVBDT{};
      };
      std::unique_ptr<Hists> m_h;
//--
      Gaudi::Property<long int> m_cutSctHits{this, "CutSctHits",    4 ,  "Remove track if it has less SCT hits" };
      Gaudi::Property<long int> m_cutPixelHits{this, "CutPixelHits",  2, "Remove track if it has less Pixel hits"};
      Gaudi::Property<long int> m_cutTRTHits{this, "CutTRTHits",    10,   "Remove track if it has less TRT hits"};
      Gaudi::Property<long int> m_cutSiHits{this, "CutSiHits",     8,    "Remove track if it has less Pixel+SCT hits"  };
      Gaudi::Property<long int> m_cutBLayHits{this, "CutBLayHits",   0,  "Remove track if it has less B-layer hits"   };
      Gaudi::Property<long int> m_cutSharedHits{this, "CutSharedHits", 1,"Reject final 2tr vertices if tracks have shared hits" };

      Gaudi::Property<double> m_cutPt{this, "CutPt",         500.,     "Track Pt selection cut"  };
      Gaudi::Property<double> m_cutD0Min{this, "CutD0Min",      0.,  "Track minimal D0 selection cut"  };
      Gaudi::Property<double> m_cutD0Max{this, "CutD0Max",      10.,  "Track maximal D0 selection cut"  };
      Gaudi::Property<double> m_maxZVrt{this, "MaxZVrt",       15.,   "Track Z impact selection max"};
      Gaudi::Property<double> m_minZVrt{this, "MinZVrt",       0.,   "Track Z impact selection min"};
      Gaudi::Property<double> m_cutChi2{this, "CutChi2",       5.,   "Track Chi2 selection cut" };
      Gaudi::Property<double> m_trkSigCut{this, "TrkSigCut",     2.0, "Track 3D impact significance w/r primary vertex. Should be >=AntiPileupSigRCut" };

      Gaudi::Property<float> m_vrtMassLimit{this, "VrtMassLimit",   5500.,   "Maximal allowed mass for found vertices" };
      Gaudi::Property<float> m_vrt2TrMassLimit{this, "Vrt2TrMassLimit",4000.,"Maximal allowed mass for 2-track vertices" };
      Gaudi::Property<float> m_vrt2TrPtLimit{this, "Vrt2TrPtLimit",  5.e5,  "Maximal allowed Pt for 2-track vertices. Calibration limit" };

      Gaudi::Property<double> m_sel2VrtProbCut{this, "Sel2VrtProbCut",    0.02, "Cut on probability of 2-track vertex for initial selection"  };
      Gaudi::Property<double> m_globVrtProbCut{this, "GlobVrtProbCut",    0.005, "Cut on probability of any vertex for final selection"  };
      Gaudi::Property<double> m_maxSVRadiusCut{this, "MaxSVRadiusCut",    140., "Cut on maximal radius of SV (def = Pixel detector size)"  };
      Gaudi::Property<double> m_selVrtSigCut{this, "SelVrtSigCut",      3.0,  "Cut on significance of 3D distance between vertex and PV"  };
      Gaudi::Property<float> m_antiPileupSigRCut{this, "AntiPileupSigRCut", 2.0,  "Upper cut on significance of 2D distance between beam and perigee"  };
      Gaudi::Property<float> m_dRdZRatioCut{this, "dRdZRatioCut",      0.25,  "Cut on dR/dZ ratio to remove pileup tracks"  };
      Gaudi::Property<float> m_v2tIniBDTCut{this, "v2tIniBDTCut",      -0.6,  "Initial BDT cut for 2track vertices selection "  };
      Gaudi::Property<float> m_v2tFinBDTCut{this, "v2tFinBDTCut",      0.,  "Final BDT cut for 2track vertices selection "  };
      Gaudi::Property<float> m_fastZSVCut{this, "FastZSVCut",        15.,  "Cut to remove SV candidates based on fast SV estimation. To save full fit CPU."  };
      Gaudi::Property<float> m_cosSVPVCut{this, "cosSVPVCut",        0.,  "Cut on cos of angle between SV-PV and full vertex momentum"  };

      Gaudi::Property<bool> m_fillHist{this, "FillHist",   false, "Fill technical histograms"  };

      Gaudi::Property<bool> m_do2TrkIBLChecks {this, "do2TrkIBLChecks", true, "IBL and B-layer hit requrirements based on the position of 2-track DV." };

      Gaudi::Property<bool> m_useVertexCleaning{this, "useVertexCleaning",  true,    "Clean vertices by requiring pixel hit presence according to vertex position" };

      Gaudi::Property<float> m_twoTrkVtxFormingD0Cut{this, "TwoTrkVtxFormingD0Cut", 0.0,  "Minimum two-track forming vertex d0 cut."};

      Gaudi::Property<bool> m_multiWithOneTrkVrt{this, "MultiWithOneTrkVrt", true,"Allow one-track-vertex addition to already found secondary vertices"};

      Gaudi::Property<float> m_vertexMergeCut{this, "VertexMergeCut",	  4., "To allow vertex merging for MultiVertex Finder" };

      Gaudi::Property<float> m_beampipeR{this, "BeampipeR",	  24.3, "Radius of the beampipe material for aggressive material rejection" };
      Gaudi::Property<float> m_firstPixelLayerR{this, "FirstPixelLayerR",	  32.0, "Radius of the first Pixel layer" };
      Gaudi::Property<float> m_removeTrkMatSignif{this, "removeTrkMatSignif", 0., "Significance of Vertex-TrackingMaterial distance for removal. No removal if <=0." };

      Gaudi::Property<std::string> m_calibFileName{this, "CalibFileName", "Fake2TrVertexReject.MVA.v02.root", " MVA calibration file for 2-track fake vertices removal" };

      std::unique_ptr<MVAUtils::BDT> m_SV2T_BDT;

      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

      ToolHandle<Trk::IExtrapolator>  m_extrapolator{this,"ExtrapolatorName","Trk::Extrapolator/Extrapolator", "Name of the extrapolator tool"};
      ToolHandle<Trk::TrkVKalVrtFitter>  m_fitSvc{this, "VertexFitterTool", "Trk::TrkVKalVrtFitter/VertexFitterTool", "Name of the Vertex Fitter tool"};
      ToolHandle< Reco::ITrackToVertex >  m_trackToVertexTool{this, "TrackToVertexTool", "Reco::TrackToVertex/TrackToVertex", "Name of the TrackToVertex tool"};

      Gaudi::Property<std::string> m_augString {this, "AugmentingVersionString", "_NVSI", "Augmentation version string"};

      double m_massPi {};
      double m_massP {};
      double m_massE{};
      double m_massK0{};
      double m_massLam{};
      std::string m_instanceName;

      SG::AuxElement::Decorator<char> m_is_selected;
      SG::AuxElement::Decorator<char> m_is_svtrk_final;
      SG::AuxElement::Decorator<float> m_pt_wrtSV;
      SG::AuxElement::Decorator<float> m_eta_wrtSV;
      SG::AuxElement::Decorator<float> m_phi_wrtSV;
      SG::AuxElement::Decorator<float> m_d0_wrtSV;
      SG::AuxElement::Decorator<float> m_z0_wrtSV;
      SG::AuxElement::Decorator<float> m_errP_wrtSV;
      SG::AuxElement::Decorator<float> m_errd0_wrtSV;
      SG::AuxElement::Decorator<float> m_errz0_wrtSV;
      SG::AuxElement::Decorator<float> m_chi2_toSV;

//=======================================================================================
// Functions and structure below are for algorithm development, debugging and calibration
// NOT USED IN PRODUCTION!

     static int notFromBC(int PDGID) ;
     static const xAOD::TruthParticle * getPreviousParent(const xAOD::TruthParticle * child, int & ParentPDG) ;
     int getIdHF(const xAOD::TrackParticle* TP ) const;
     static int getG4Inter( const xAOD::TrackParticle* TP ) ;
     static int getMCPileup(const xAOD::TrackParticle* TP ) ;

     struct DevTuple 
     { 
       static constexpr int maxNTrk=100;
       static constexpr int maxNVrt=100;
       int   nTrk;
       float pttrk[maxNTrk];
       float d0trk[maxNTrk];
       float etatrk[maxNTrk];
       float Sig3D[maxNTrk];
       float dRdZrat[maxNTrk];
       int   idHF[maxNTrk];
       int   trkTRT[maxNTrk];
       int   n2Vrt;
       int   VrtTrkHF[maxNVrt];
       int   VrtTrkI[maxNVrt];
       int   VrtCh[maxNVrt];
       int   VrtIBL[maxNVrt];
       int   VrtBL[maxNVrt];
       int   VrtDisk[maxNVrt];
       float VrtDist2D[maxNVrt];
       float VrtSig3D[maxNVrt];
       float VrtSig2D[maxNVrt];
       float VrtM[maxNVrt];
       float VrtZ[maxNVrt];
       float VrtPt[maxNVrt];
       float VrtEta[maxNVrt];
       float VrtBDT[maxNVrt];
       float VrtProb[maxNVrt];
       float VrtHR1[maxNVrt];
       float VrtHR2[maxNVrt];
       float VrtDZ[maxNVrt];
       float VrtCosSPM[maxNVrt];
       float VMinPtT[maxNVrt];
       float VMinS3DT[maxNVrt];
       float VMaxS3DT[maxNVrt];
       float VSigMat[maxNVrt];
       //---
       int   nNVrt;
       int   NVrtTrk[maxNVrt];
       int   NVrtTrkHF[maxNVrt];
       int   NVrtTrkI[maxNVrt];
       int   NVrtCh[maxNVrt];
       int   NVrtIBL[maxNVrt];
       int   NVrtBL[maxNVrt];
       float NVrtM[maxNVrt];
       float NVrtPt[maxNVrt];
       float NVrtEta[maxNVrt];
       float NVrtCosSPM[maxNVrt];
       float NVMinPtT[maxNVrt];
       float NVMinS3DT[maxNVrt];
       float NVMaxS3DT[maxNVrt];
       float NVrtDist2D[maxNVrt];
       float NVrtSig3D[maxNVrt];
       float NVrtSig2D[maxNVrt];
       float NVrtProb[maxNVrt];
       float NVrtBDT[maxNVrt];
       float NVrtHR1[maxNVrt];
       float NVrtHR2[maxNVrt];
     };
//
// End of development stuff
//============================================================


     struct Vrt2Tr 
     {
         Amg::Vector3D     fitVertex;
         TLorentzVector    momentum;
         long int   vertexCharge;
         std::vector<double> errorMatrix;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double chi2=0.;
     };


// For multivertex version only

      using compatibilityGraph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS>;
      float m_chiScale[11]{};
      struct WrkVrt 
      {  bool Good=true;
         std::deque<long int> selTrk;
         Amg::Vector3D     vertex;
         TLorentzVector    vertexMom;
         long int   vertexCharge{};
         std::vector<double> vertexCov;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double chi2{};
         double projectedVrt=0.;
         int detachedTrack=-1;
         double BDT=1.1;
      };


//   Private technical functions
//
//
      std::vector<xAOD::Vertex*> getVrtSecMulti(  workVectorArrxAOD * inpParticlesxAOD, const xAOD::Vertex  & primVrt,
                                                  compatibilityGraph_t& compatibilityGraph ) const;


      void printWrkSet(const std::vector<WrkVrt> * WrkSet, const std::string &name ) const;

//
// Gives correct mass assignment in case of nonequal masses
      static double massV0(const std::vector< std::vector<double> >& TrkAtVrt, double massP, double massPi ) ;


      TLorentzVector momAtVrt(const std::vector<double>& inpTrk) const; 
      static double  vrtRadiusError(const Amg::Vector3D & secVrt, const std::vector<double>  & vrtErr) ;

      static int   nTrkCommon( std::vector<WrkVrt> *WrkVrtSet, int indexV1, int indexV2) ;
      double minVrtVrtDist( std::vector<WrkVrt> *WrkVrtSet, int & indexV1, int & indexV2, std::vector<double> & check) const;
      static bool isPart( const std::deque<long int>& test, std::deque<long int> base) ;
      static std::vector<double> estimVrtPos( int nTrk, std::deque<long int> &selTrk, std::map<long int,std::vector<double>> & vrt) ;

      static double vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                  const std::vector<double>& vrtErr,double& signif ) ;
      static double vrtVrtDist2D(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                  const std::vector<double>& vrtErr,double& signif ) ;
      static double vrtVrtDist(const Amg::Vector3D & vrt1, const std::vector<double>& vrtErr1,
                        const Amg::Vector3D & vrt2, const std::vector<double>& vrtErr2) ;
      static double PntPntDist(const Amg::Vector3D & Vrt1, const Amg::Vector3D & Vrt2) ;


      static double projSV_PV(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Direction) ;
      static double MomProjDist(const Amg::Vector3D & SV, const xAOD::Vertex & PV, const TLorentzVector & Direction) ;

      double distToMatLayerSignificance(Vrt2Tr & Vrt) const;

      double refitVertex( WrkVrt &Vrt,std::vector<const xAOD::TrackParticle*> & SelectedTracks,
                          Trk::IVKalState& istate,
                          bool ifCovV0) const;

      static int mostHeavyTrk(WrkVrt V, std::vector<const xAOD::TrackParticle*> AllTracks) ;
      double refineVerticesWithCommonTracks( WrkVrt &v1, WrkVrt &v2, std::vector<const xAOD::TrackParticle*> & allTrackList,
                                                        Trk::IVKalState& istate) const;
      double mergeAndRefitVertices( WrkVrt & v1, WrkVrt & v2, WrkVrt & newvrt,
                                    std::vector<const xAOD::TrackParticle*> & AllTrackList,
                                    Trk::IVKalState& istate, int robKey =0) const;

      double  improveVertexChi2( WrkVrt &vertex, std::vector<const xAOD::TrackParticle*> & allTracks,
                                 Trk::IVKalState& istate,
                                 bool ifCovV0) const;

      void selGoodTrkParticle( workVectorArrxAOD * xAODwrk,
                               const xAOD::Vertex  & primVrt) const;



      void select2TrVrt(std::vector<const xAOD::TrackParticle*> & SelectedTracks, const xAOD::Vertex  & primVrt,
                        std::map<long int,std::vector<double>> & vrt,
                        compatibilityGraph_t& compatibilityGraph) const;


     static void  getPixelDiscs   (const xAOD::TrackParticle* Part, int &d0Hit, int &d1Hit, int &d2Hit) ;
     static int   getIBLHit(const xAOD::TrackParticle* Part) ;
     static int   getBLHit(const xAOD::TrackParticle* Part) ;

     Hists& getHists() const;
   };


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

}  //end namespace

#endif
