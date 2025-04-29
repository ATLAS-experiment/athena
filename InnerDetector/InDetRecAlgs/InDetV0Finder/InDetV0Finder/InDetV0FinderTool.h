/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                          InDetV0FinderTool.h  -  Description
                             -------------------
    begin   : 30-11-2014
    authors : Evelina Bouhova-Thacker (Lancater University)
    email   : e.bouhova@cern.ch
    changes :

 ***************************************************************************/

#ifndef INDETV0FINDERTOOL_H
#define INDETV0FINDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODEventInfo/EventInfo.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "TrkParameters/TrackParameters.h"
#include "GeoPrimitives/GeoPrimitives.h" //Needed for Amg::Vector3D
#include "GaudiKernel/IPartPropSvc.h"
#include <atomic>

#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "MVAUtils/BDT.h"
#include <TTree.h>
#include <TFile.h>
#include "PathResolver/PathResolver.h"

/**
   The InDetV0FinderTool reads in the TrackParticle container from StoreGate,
   if useorigin = True only tracks not associated to a primary vertex are used.
   There are options to use silicon tracks only (default) or include Si+TRT and TRT+TRT.

   Pairs of tracks passing pre-selection (using InDetTrackSelectorTool) are
   fitted to a common vertex using TrkV0Fitter or TrkVKalVrtFitter (useV0Fitter = False).

   InDetV0FinderTool can take as input a vertex (or a collection of vertices)
   w.r.t which the V0s can be selected to minimise combinatorics.
     - if such a vertex is provided, tracks are used if fabs(d0/sig_d0) > d0_cut (= 2.)
     - if useVertexCollection = True all vertices in the collection are used
     - if useVertexCollection = False
       - if trkSelPV = True either a primary vertex, if provided, or the beam spot are used
       - if trkSelPV = False (default) all track combinations are used.

   The unconstrained vertex fit is attempted if the radius of the starting point is < maxsxy (= 1000 mm)
   and at least one invariant mass at the starting point is in the allowed range:
   uksmin < m(pipi) < uksmax or ulamin < m(ppi) < ulamax or ulamin < m(pip) < ulamax

   V0s are kept if the cumulative chi2 probability of the unconstrained fit is > minVertProb (= 0.0001)

   If doSimpleV0 = True all vertices that pass these cuts are stored in V0UnconstrVertices.

   If doSimpleV0 = False (default) mass constrained fits are attempted if
     - the invariant mass of the unconstrained V0 is in the allowed range:
       ksmin < m(pipi) < ksmax, lamin < m(ppi), m(pip) < lamax
       and the error on the invariant mass is < errmass (= 100 MeV)
     - if an input vertex (collection) is provided the unconstrained V0 is required to
       have an impact parameter w.r.t the vertex < vert_a0xy_cut (= 3 mm) in xy and
       < vert_a0z_cut (= 15 mm) in z, the cosine of the angle between the V0 momentum and
       the direction from the input vertex to the V0 vertex is > 0,
       Lxy w.r.t the vertex is < vert_lxy_cut (= 500 mm) and Lxy/sigma(Lxy) > vert_lxy_sig (= 2)

   Mass constrainedV0s are kept if the cumulative chi2 probability of the fit is > minVertProb (= 0.0001)

   For successful mass constrained fits a conversion fit is also attempted and if successful,
   the corresponding unconstrained V0 is decorated with the invariant mass, its error and
   the vertex probability of the conversion fit.

   The links between the unconstrained V0 and the successful mass constrained V0s are stored.
*/


/* Forward declarations */

namespace Trk
{
  class TrkV0VertexFitter;
  class V0Tools;
}

namespace HepPDT{
  class ParticleDataTable;
}

namespace InDet
{
  static const InterfaceID IID_InDetV0FinderTool("InDetV0FinderTool", 1, 0);

  class InDetV0FinderTool:  public AthAlgTool
  {
  public:
    InDetV0FinderTool(const std::string& t, const std::string& n, const IInterface*  p);
    ~InDetV0FinderTool();
    StatusCode initialize();
    StatusCode finalize();

    static const InterfaceID& interfaceID() { return IID_InDetV0FinderTool;}

    StatusCode performSearch(xAOD::VertexContainer* v0Container,
                             xAOD::VertexContainer* ksContainer,
                             xAOD::VertexContainer* laContainer,
                             xAOD::VertexContainer* lbContainer,
                             const xAOD::Vertex* vertex,
			     const xAOD::VertexContainer* vertColl, const EventContext& ctx
			     ) const;

  //protected:
  private:
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleKey { this, "TrackParticleCollection", "InDetTrackParticles",
                                                                         "key for retrieval of TrackParticles" };

    PublicToolHandle<Trk::IVertexFitter> m_iVertexFitter {
      this, "VertexFitterTool", "Trk::V0VertexFitter"};
    PublicToolHandle<Trk::IVertexFitter> m_iVKVertexFitter{
      this, "VKVertexFitterTool", "Trk::TrkVKalVrtFitter"};
    PublicToolHandle<Trk::IVertexFitter> m_iKshortFitter{
      this, "KshortFitterTool", "Trk::TrkVKalVrtFitter"};
    PublicToolHandle<Trk::IVertexFitter> m_iLambdaFitter{
      this, "LambdaFitterTool", "Trk::TrkVKalVrtFitter"};
    PublicToolHandle<Trk::IVertexFitter> m_iLambdabarFitter{
      this, "LambdabarFitterTool", "Trk::TrkVKalVrtFitter"};
    PublicToolHandle<Trk::IVertexFitter> m_iGammaFitter{
      this, "GammaFitterTool", "Trk::TrkVKalVrtFitter"};
    PublicToolHandle<Trk::V0Tools> m_V0Tools{
      this, "V0Tools", "Trk::V0Tools"};
    PublicToolHandle<Reco::ITrackToVertex> m_trackToVertexTool{
      this, "TrackToVertexTool", "Reco::TrackToVertex"};
    PublicToolHandle<Trk::ITrackSelectorTool> m_trkSelector{
      this, "TrackSelectorTool", "InDet::TrackSelectorTool"};
    PublicToolHandle<InDet::VertexPointEstimator> m_vertexEstimator{
      this, "VertexPointEstimator", "InDet::VertexPointEstimator"};
    PublicToolHandle<Trk::IExtrapolator> m_extrapolator{
      this, "Extrapolator", "Trk::Extrapolator"};

    const HepPDT::ParticleDataTable *m_particleDataTable = nullptr;

    BooleanProperty m_doSimpleV0{this, "doSimpleV0", false};            //!< = true equivalent to the old InDetSimpleV0Finder (false)
    BooleanProperty m_useorigin{this, "useorigin", true};               //!< = true only using tracks that have no vertex association (true)
    BooleanProperty m_samesign{this, "AddSameSign", false};             //!< = true select tracks with same sign (false)
    BooleanProperty m_pv{this, "trkSelPV", false};                      //!< = true select tracks wrt primary vertex (false)
    BooleanProperty m_use_vertColl{this, "useVertexCollection", false}; //!< = true select tracks wrt a vertex collection (false)
    BooleanProperty m_useTRTplusTRT{this, "useTRTplusTRT", false};      //!< = use TRT+TRT pairs (true)
    BooleanProperty m_useTRTplusSi{this, "useTRTplusSi", false};        //!< = use TRT+Si pairs (true)
    BooleanProperty m_useV0Fitter{this, "useV0Fitter", false};          //!< = true if using TrkV0Fitter, = false if using VKalVert (true)
    BooleanProperty m_use_innerPixHits{this, "use_innerPixHits", false}; //!< = true select allows tracks with no innermost pixel layer hits to always pass d0 significance cut (false)
    BooleanProperty m_useBDT{this, "useBDT", false};                     //!< = true uses BDT selections in place of rectangular pointAtVertex + minVertProb
    BooleanProperty m_useTrkSel{this, "use_TrackSelector", true};      //!< = true uses TrackSelectorTool
    
    IntegerProperty m_masses{this, "masses", 1};                        //!< = 1 if using PDG values, = 2 if user set (1)
    DoubleProperty m_masspi{this, "masspi", 139.57};                    //!< pion mass (139.57 MeV)
    DoubleProperty m_massp{this, "massp", 938.272};                     //!< proton mass (938.272 MeV)
    DoubleProperty m_masse{this, "masse", 0.510999};                    //!< electron mass (0.510999 MeV)
    DoubleProperty m_massK0S{this, "massK0S", 497.672};                 //!< Kshort mass (497.672 MeV)
    DoubleProperty m_massLambda{this, "massLambda", 1115.68};           //!< Lambda mass (1115.68 MeV)
    DoubleProperty m_ptTRT{this, "ptTRT", 700.};                        //!< Minimum pT for TRT tracks (700. MeV)
    DoubleProperty m_maxsxy{this, "maxsxy", 1000.};                     //!< Maximum Rxy of starting point (1000. mm)
    DoubleProperty m_uksmin{this, "uksmin", 400.};                      //!< min Kshort mass, unconstrained fit (400. MeV)
    DoubleProperty m_uksmax{this, "uksmax", 600.};                      //!< max Kshort mass, unconstrained fit (600. MeV)
    DoubleProperty m_ulamin{this, "ulamin", 1000.};                     //!< min Lambda mass, unconstrained fit (1000. MeV)
    DoubleProperty m_ulamax{this, "ulamax", 1200.};                     //!< max Lambda mass, unconstrained fit (1200. MeV)
    DoubleProperty m_ksmin{this, "ksmin", 400.};                        //!< min Kshort mass (400. MeV)
    DoubleProperty m_ksmax{this, "ksmax", 600.};                        //!< max Kshort mass (600. MeV)
    DoubleProperty m_lamin{this, "lamin", 1000.};                       //!< min Lambda mass (1000. MeV)
    DoubleProperty m_lamax{this, "lamax", 1200.};                       //!< max Lambda mass (1200. MeV)
    DoubleProperty m_errmass{this, "errmass", 100.};                    //!< Maximum mass error (100. MeV)
    DoubleProperty m_minVertProb{this, "minVertProb", 0.0001};          //!< Minimum vertex probability (0.0001)
    DoubleProperty m_minConstrVertProb{this, "minConstrVertProb", 0.0001}; //!< Minimum vertex probability for constrained fit (0.0001)
    DoubleProperty m_d0_cut{this, "d0_cut", 2.};                        //!< track d0 significance wrt a vertex (>2.)
    DoubleProperty m_max_d0_cut{this, "max_d0_cut", 999999.};           //!< track |d0| wrt a vertex (<999999.)
    DoubleProperty m_max_z0_cut{this, "max_z0_cut", 999999.};           //!< track |z0| wrt a vertex (<999999.)    
    DoubleProperty m_vert_lxy_sig{this, "vert_lxy_sig", 2.};            //!< V0 lxy significance wrt a vertex (>2.)
    DoubleProperty m_vert_lxy_cut{this, "vert_lxy_cut", 500.};          //!< V0 lxy V0 lxy  (<500.)
    DoubleProperty m_vert_a0xy_cut{this, "vert_a0xy_cut", 3.};          //!< V0 |a0xy| wrt a vertex (<3.)
    DoubleProperty m_vert_a0z_cut{this, "vert_a0z_cut", 15.};           //!< V0 |a0z| wrt a vertex (<15.)
    DoubleProperty m_vert_cos_cut{this, "vert_cos_cut", 0.};            //!< V0 cos(theta) angle between displacement and momentum (>0.)
    DoubleProperty m_BDTCut{this, "BDTCut", -1};                        //!< BDT Score threshold

    StringProperty m_BDTFile{this, "BDTFile", "XGBModelBetterVertex.root"};    //!< Filename of mvaUtils model file, located in /InDetV0FinderTool/BDT/v1/


    mutable std::atomic<unsigned int>  m_events_processed{};
    mutable std::atomic<unsigned int>  m_V0s_stored{};
    mutable std::atomic<unsigned int>  m_Kshort_stored{};
    mutable std::atomic<unsigned int>  m_Lambda_stored{};
    mutable std::atomic<unsigned int>  m_Lambdabar_stored{};
    mutable std::atomic<unsigned int>  m_Gamma_stored{};


    void SGError(const std::string& errService) const;

    static double invariantMass(const Trk::TrackParameters* per1, const Trk::TrackParameters* per2, double m1, double m2) ;

    bool doFit(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2, Amg::Vector3D &startingPoint, const EventContext& ctx) const;

    bool d0Pass(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2, const xAOD::VertexContainer * vertColl, const EventContext& ctx) const;
    bool d0Pass(const xAOD::TrackParticle* track1, const xAOD::VertexContainer * vertColl, const EventContext& ctx) const;
    bool d0Pass(const xAOD::TrackParticle* track1, const xAOD::Vertex * vertex, const EventContext& ctx) const;
    bool d0Pass(const xAOD::TrackParticle* track1, const Amg::Vector3D& vertex, const EventContext& ctx) const;

    bool pointAtVertex(const xAOD::Vertex* v0, const xAOD::Vertex* PV, float &score) const;
    bool pointAtVertexColl(xAOD::Vertex* v0, const xAOD::VertexContainer * vertColl, float &score) const;

    bool doMassFit(xAOD::Vertex* vxCandidate, int pdgID) const;

    xAOD::Vertex* massFit(int pdgID, const std::vector<const xAOD::TrackParticle*> &pairV0, const Amg::Vector3D &vertex) const;

    const Trk::TrkV0VertexFitter* m_concreteVertexFitter = nullptr;

    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexKey { this, "VertexContainer", "PrimaryVertices",
	                                                   "primary vertex container" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0LinksDecorkeyks;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0LinksDecorkeylb;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0LinksDecorkeylbb;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0_ksLinksDecorkey;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0_laLinksDecorkey;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0_lbLinksDecorkey;

    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_gfit;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_gmass;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_gmasserr;
    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_mDecor_gprob;

    SG::WriteDecorHandleKey<xAOD::VertexContainer> m_v0_BDTScore;

    std::unique_ptr<MVAUtils::BDT> m_BDT;

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
    SG::ReadDecorHandleKeyArray<xAOD::EventInfo> m_beamSpotDecoKey{this, "BeamSpotDecoKeys" ,{}, "Add the scheduler dependencies on the beamspot information"};
    Gaudi::Property<bool>       m_useBeamSpotCond { this, "UseBeamSpotCond", false };
    // V0 candidate output container name (same calling alg)
    Gaudi::Property<std::string>       m_v0Key { this, "V0ContainerName", "V0Candidates", "V0 container name (same calling alg)" };
    Gaudi::Property<std::string>       m_ksKey { this, "KshortContainerName", "KshortCandidates", "Ks container name (same calling alg)" };
    Gaudi::Property<std::string>       m_laKey { this, "LambdaContainerName", "LambdaCandidates",
                                                              "Lambda container name (same calling alg)" };
    Gaudi::Property<std::string>       m_lbKey { this, "LambdabarContainerName", "LambdabarCandidates", 
                                                              "Lambdabar container name (same calling alg)" };
    Gaudi::Property<int>       m_maxPV { this, "MaxPV", 999999 };
    SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_RelinkContainers{this, "RelinkTracks", {}, "Track Containers if they need to be relinked through indirect use" };
    ElementLink<xAOD::TrackParticleContainer> makeLink(const xAOD::TrackParticle*, const std::vector<const xAOD::TrackParticleContainer*>&) const;

    ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};
  };

}//end of namespace InDet

#endif

