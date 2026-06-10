/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// VKalVrt.h
//
#ifndef VRTSECINCLUSIVE_VRTSECINCLUSIVE_H
#define VRTSECINCLUSIVE_VRTSECINCLUSIVE_H


#include "VrtSecInclusive/Constants.h"

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthContainers/Decorator.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
//
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

// for truth
#include "GeneratorObjects/McEventCollection.h"

#include "TrkToolInterfaces/ITruthToTrack.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkDetDescrInterfaces/IVertexMapper.h"
#include "GaudiKernel/ServiceHandle.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"

// xAOD Classes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Electron.h"

// Normal STL and physical vectors
#include <vector>
#include <deque>
#include <functional>
#include <optional>
#include <map>
#include <cstdint> //for uint8_t


/** Forward declarations **/

class TH1;

namespace Trk {
  class ITruthToTrack;
  class ITrackToVertex;
  class ITrackToVertexIPEstimator;
  class IExtrapolator;
  class IVertexMapper;
  struct MappedVertex;
}

namespace VKalVrtAthena {

  namespace GeoModel { enum GeoModel { Run1=1, Run2=2 }; }

  class IntersectionPos;
  class IntersectionPos_barrel;
  class IntersectionPos_endcap;

  class NtupleVars;
}


namespace VKalVrtAthena {

  class VrtSecInclusive : public AthAlgorithm {
  public:
    /** Standard Athena-Algorithm Constructor */
    VrtSecInclusive(const std::string& name, ISvcLocator* pSvcLocator);

    /** Default Destructor */
    virtual ~VrtSecInclusive() override;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode initEvent();

  private:
    StatusCode defineDummyCollections(const EventContext& ctx);
    StatusCode dummyVertexContainer(const EventContext& ctx,
				    const SG::WriteHandleKey<xAOD::VertexContainer>& handleKey);
    /////////////////////////////////////////////////////////
    //
    //  Member Variables
    //

    // JO: GeoModel
    Gaudi::Property<int> m_geoModel{this, "GeoModel", VKalVrtAthena::GeoModel::Run2};

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackLocation{this, "TrackLocation", "InDetTrackParticles"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_MuonLocation{this, "MuonLocation",  "Muons"};
    SG::ReadHandleKey<xAOD::ElectronContainer> m_ElectronLocation{this, "ElectronLocation", "Electrons"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_PrimVrtLocation{this, "PrimVrtLocation", "PrimaryVertices"};
    Gaudi::Property<std::string> m_truthParticleContainerName{this, "McParticleContainer", "TruthParticles"};
    Gaudi::Property<std::string> m_mcEventContainerName{this, "MCEventContainer", "TruthEvents"};
    Gaudi::Property<std::string> m_augVerString{this, "AugmentingVersionString", "_VSI"};
    Gaudi::Property<std::string> m_truthParticleFilter{this, "TruthParticleFilter", "Rhadron"};// Either "", "Kshort", "Rhadron", "HNL", "HadInt", "Bhadron"

    Gaudi::Property<std::string> m_all2trksVerticesContainerName{this, "All2trkVerticesContainerName", "All2TrksVertices"};
    Gaudi::Property<std::string> m_secondaryVerticesContainerName{this, "SecondaryVerticesContainerName", "SecondaryVertices"};

    // common feature flags
    Gaudi::Property<bool> m_doTruth{this, "DoTruth", false};
    Gaudi::Property<bool> m_FillHist{this, "FillHist", false};
    Gaudi::Property<bool> m_FillNtuple{this, "FillNtuple", false};
    Gaudi::Property<bool> m_FillIntermediateVertices{this, "FillIntermediateVertices", false};
    Gaudi::Property<bool> m_doIntersectionPos{this, "DoIntersectionPos", false};
    Gaudi::Property<bool> m_doMapToLocal{this, "DoMapToLocal", false};
    Gaudi::Property<bool> m_extrapPV{this, "ExtrapPV", false}; //extrapolate reco and prim truth trks to PV (for testing only)

    Gaudi::Property<bool> m_passThroughTrackSelection{this, "PassThroughTrackSelection", false };

    Gaudi::Property<bool> m_doFastMode{this, "DoFastMode", false}; // flag for running in rapid finder mode instead of using graph

    // track selection conditions
    Gaudi::Property<unsigned int> m_SelTrkMaxCutoff{this, "SelTrkMaxCutoff", 50}; // max number of tracks
    Gaudi::Property<bool> m_SAloneTRT{this, "DoSAloneTRT", false}; // SAlone = "standalone"

    /* impact parameters */
    Gaudi::Property<bool> m_do_PVvetoCut{this, "do_PVvetoCut", true};
    Gaudi::Property<bool> m_do_d0Cut{this, "do_d0Cut", true};
    Gaudi::Property<bool> m_do_z0Cut{this, "do_z0Cut", true};
    Gaudi::Property<bool> m_do_d0errCut{this, "do_d0errCut", false};
    Gaudi::Property<bool> m_do_z0errCut{this, "do_z0errCut", false};
    Gaudi::Property<bool> m_do_d0signifCut{this, "do_d0signifCut", false};
    Gaudi::Property<bool> m_do_z0signifCut{this, "do_z0signifCut", false};

    Gaudi::Property<bool> m_ImpactWrtBL{this, "ImpactWrtBL", true}; // false option is going to be deprecated
    Gaudi::Property<double> m_d0TrkPVDstMinCut{this, "a0TrkPVDstMinCut", 0. }; // in [mm]
    Gaudi::Property<double> m_d0TrkPVDstMaxCut{this, "a0TrkPVDstMaxCut", 1000.}; // in [mm]
    Gaudi::Property<double> m_d0TrkPVSignifCut{this, "a0TrkPVSignifCut", 0.}; // in [mm]
    Gaudi::Property<double> m_z0TrkPVDstMinCut{this, "zTrkPVDstMinCut", 0.}; // in [mm]
    Gaudi::Property<double> m_z0TrkPVDstMaxCut{this, "zTrkPVDstMaxCut", 1000.}; // in [mm]
    Gaudi::Property<double> m_z0TrkPVSignifCut{this, "zTrkPVSignifCut", 0.}; // in unit of sigma
    Gaudi::Property<double> m_d0TrkErrorCut{this, "TrkA0ErrCut", 10000}; // in [mm]
    Gaudi::Property<double> m_z0TrkErrorCut{this, "TrkZErrCut", 20000}; // in [mm]
    Gaudi::Property<double> m_twoTrkVtxFormingD0Cut{this, "twoTrkVtxFormingD0Cut", 1.}; // in [mm]

    /* pT anc chi2 */
    Gaudi::Property<double> m_TrkChi2Cut{this, "TrkChi2Cut", 3.}; // in terms of chi2 / ndof
    Gaudi::Property<double> m_TrkPtCut{this, "TrkPtCut", 1000.}; // low pT threshold. in [MeV]

    /* hit requirements */
    Gaudi::Property<bool> m_doTRTPixCut{this, "doTRTPixCut", false, "mode for R-hadron displaced vertex"}; // Kazuki
    Gaudi::Property<int> m_CutSctHits{this, "CutSctHits", 0};
    Gaudi::Property<int> m_CutPixelHits{this, "CutPixelHits", 0};
    Gaudi::Property<int> m_CutSiHits{this, "CutSiHits", 0};
    Gaudi::Property<int> m_CutBLayHits{this, "CutBLayHits", 0};
    Gaudi::Property<int> m_CutSharedHits{this, "CutSharedHits", 0};
    Gaudi::Property<int> m_CutTRTHits{this, "CutTRTHits", 0}; // Kazuki
    Gaudi::Property<int> m_CutTightSCTHits{this, "CutTightSCTHits", 7};
    Gaudi::Property<int> m_CutTightTRTHits{this, "CutTightTRTHits", 20};

    /* track extrpolator{this, }; 1==VKalGetImpact, 2==m_trackToVertexTool*/
    Gaudi::Property<int> m_trkExtrapolator{this, "TrkExtrapolator", 2};

    // Vertex reconstruction
    Gaudi::Property<bool> m_doPVcompatibilityCut{this, "DoPVcompatibility", true};
    Gaudi::Property<bool> m_doTightPVcompatibilityCut{this, "DoTightPVcompatibility", false};
    Gaudi::Property<bool> m_removeFakeVrt{this, "RemoveFake2TrkVrt", true};
    Gaudi::Property<bool> m_removeFakeVrtLate{this, "DoDelayedFakeReject", false};
    Gaudi::Property<bool> m_doReassembleVertices{this, "doReassembleVertices", false};
    Gaudi::Property<bool> m_doMergeByShuffling{this, "doMergeByShuffling", false};
    Gaudi::Property<bool> m_doSuggestedRefitOnMerging{this, "doSuggestedRefitOnMerging", true};
    Gaudi::Property<bool> m_doMagnetMerging{this, "doMagnetMerging", true}; // sub-option of doMergeByShuffling-2
    Gaudi::Property<bool> m_doWildMerging{this, "doWildMerging", true}; // sub-option of doMergeByShuffling-3
    Gaudi::Property<bool> m_doMergeFinalVerticesDistance{this, "doMergeFinalVerticesDistance", false}; // Kazuki
    Gaudi::Property<bool> m_doAssociateNonSelectedTracks{this, "doAssociateNonSelectedTracks", false};
    Gaudi::Property<bool> m_doFinalImproveChi2{this, "doFinalImproveChi2", false};
    Gaudi::Property<double> m_pvCompatibilityCut{this, "PVcompatibilityCut", -20.}; // in [mm]
    Gaudi::Property<double> m_SelVrtChi2Cut{this, "SelVrtChi2Cut", 4.5}; // in terms of chi2 / ndof
    Gaudi::Property<double> m_VertexMergeFinalDistCut{this, "VertexMergeFinalDistCut", 1.}; // in [mm] // Kazuki
    Gaudi::Property<double> m_VertexMergeFinalDistScaling{this, "VertexMergeFinalDistScaling", 0.}; // in [1/mm]
    Gaudi::Property<double> m_VertexMergeCut{this, "VertexMergeCut", 3};
    Gaudi::Property<double> m_TrackDetachCut{this, "TrackDetachCut", 6};

    Gaudi::Property<bool> m_doTwoTrSoftBtag{this, "DoTwoTrSoftBtag", false};
    Gaudi::Property<double> m_twoTrVrtAngleCut{this, "TwoTrVrtAngleCut", -10};
    Gaudi::Property<double> m_twoTrVrtMinDistFromPV{this, "TwoTrVrtMinDistFromPVCut", 0.};

    // When truncateWrkVertices set to true, maxWrkVertices is the maximum
    // number of potential vertices to process, used to truncate the list
    // in rare caes where several thousands are found, to avoid algorithm
    // timeout issues
    Gaudi::Property<bool> m_truncateWrkVertices{this, "TruncateListOfWorkingVertices", true};
    Gaudi::Property<size_t> m_maxWrkVertices{this, "MaxNumberOfWorkingVertices", 1500};

    Gaudi::Property<double> m_associateMinDistanceToPV{this, "associateMinDistanceToPV", 0.5};
    Gaudi::Property<double> m_associateMaxD0Signif{this, "associateMaxD0Signif", 5.}; // wrt. DV in unit of sigma
    Gaudi::Property<double> m_associateMaxZ0Signif{this, "associateMaxZ0Signif", 5.}; // wrt. DV in unit of sigma
    Gaudi::Property<double> m_associatePtCut{this, "associatePtCut", 0.}; // in [MeV]
    Gaudi::Property<double> m_associateChi2Cut{this, "associateChi2Cut", 20.};

    Gaudi::Property<double> m_reassembleMaxImpactParameterD0{this, "reassembleMaxImpactParameterD0",  1.}; // wrt. DV in [mm]
    Gaudi::Property<double> m_reassembleMaxImpactParameterZ0{this, "reassembleMaxImpactParameterZ0",  5.}; // wrt. DV in [mm]
    Gaudi::Property<double> m_mergeByShufflingMaxSignificance{this, "mergeByShufflingMaxSignificance", 100.}; // in unit of sigma
    Gaudi::Property<double> m_mergeByShufflingAllowance{this, "mergeByShufflingAllowance", 4.}; // in unit of sigma

    Gaudi::Property<double> m_improveChi2ProbThreshold{this, "improveChi2ProbThreshold", 1.e-4};

    // vertexing using muons (test implementation)
    Gaudi::Property<bool> m_doSelectTracksFromMuons{this, "doSelectTracksFromMuons", false};
    Gaudi::Property<bool> m_doRemoveCaloTaggedMuons{this, "doRemoveCaloTaggedMuons", false};
    Gaudi::Property<bool> m_doSelectTracksFromElectrons{this, "doSelectTracksFromElectrons", false};
    Gaudi::Property<bool> m_doSelectIDAndGSFTracks{this, "doSelectIDAndGSFTracks", false};
    Gaudi::Property<bool> m_doRemoveNonLeptonVertices{this, "doRemoveNonLeptonVertices", false};

    // vertexing using disapperaing track
    Gaudi::Property<bool> m_doDisappearingTrackVertexing{this, "doDisappearingTrackVertexing", false};
    Gaudi::Property<double> m_twoTrVrtMaxPerigeeDist{this, "twoTrVrtMaxPerigeeDist", 50}; // in [mm]
    Gaudi::Property<double> m_twoTrVrtMinRadius{this, "twoTrVrtMinRadius", 50}; // in [mm]

    // When doSelectTracksWithLRTCuts is set to true, the addtional track cuts
    // be applied to the selected tracks to reduce the number of fake tracks in
    // the selected track collected. These cuts are inspired by the improvments that
    // were implmented for LRT Run 3.
    Gaudi::Property<bool> m_doSelectTracksWithLRTCuts {this, "doSelectTracksWithLRTCuts", false};

    // Additional dressing option
    Gaudi::Property<bool> m_doAugmentDVimpactParametersToMuons{this, "doAugmentDVimpactParametersToMuons", false};     // potentially useful for DV + muon search
    Gaudi::Property<bool> m_doAugmentDVimpactParametersToElectrons{this, "doAugmentDVimpactParametersToElectrons", false}; // potentially useful for analyses involving electrons

    // MC truth
    Gaudi::Property<double> m_mcTrkResolution{this, "MCTrackResolution", 0.06}; // see getTruth for explanation
    Gaudi::Property<double> m_TruthTrkLen{this, "TruthTrkLen", 1000}; // in [mm]

    // Indicates give-up modes during vertexing
    // 0 if no errors occured
    // 1 if too few selected tracks
    // 2 if too many selected tracks
    // 3 if wrkVertices container is truncated
    // 4 if any uncaught exception is raised at the top level of VSI execute()
    int m_vertexingStatus = 0;

    // xAOD Accessors
    const xAOD::VertexContainer* m_primaryVertices{};
    const xAOD::Vertex* m_thePV{};
    std::vector<const xAOD::TrackParticle*> m_selectedTracks;
    std::vector<const xAOD::TrackParticle*> m_associatedTracks;
    std::vector<const xAOD::TrackParticle*> m_leptonicTracks;
    std::vector<double>  m_BeamPosition;

    /////////////////////////////////////////////////////////
    //
    //  Athena JobOption Properties
    //

    ToolHandle <Trk::ITrkVKalVrtFitter> m_fitSvc{this, "VertexFitterTool", "Trk::TrkVKalVrtFitter", " Private TrkVKalVrtFitter"};       // VKalVrtFitter tool
    PublicToolHandle <Trk::ITruthToTrack> m_truthToTrack{this, "TruthToTrack", "Trk::TruthToTrack/InDetTruthToTrack"}; // tool to create trkParam from genPart

    /** get a handle on the Track to Vertex tool */
    PublicToolHandle< Reco::ITrackToVertex > m_trackToVertexTool{this, "TrackToVertexTool", "Reco::TrackToVertex"};
    PublicToolHandle<Trk::ITrackToVertexIPEstimator> m_trackToVertexIPEstimatorTool{this, "TrackToVertexIPEstimatorTool", "Trk::TrackToVertexIPEstimator/TrackToVertexIPEstimator"};
    PublicToolHandle<Trk::IExtrapolator> m_extrapolator{this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};
    PublicToolHandle<Trk::IVertexMapper> m_vertexMapper{this, "VertexMapper", ""};

    /** Condition service **/
    ToolHandle<IInDetConditionsTool> m_pixelCondSummaryTool{this, "PixelConditionsSummaryTool", "PixelConditionsSummaryTool", "Tool to retrieve Pixel Conditions summary"};
    ToolHandle<IInDetConditionsTool> m_sctCondSummaryTool{this, "InDetSCT_ConditionsSummaryTool", "SCT_ConditionsSummaryTool/InDetSCT_ConditionsSummaryTool", "Tool to retrieve SCT conditions summary"};

    const AtlasDetectorID* m_atlasId{};
    const PixelID* m_pixelId{};
    const SCT_ID*  m_sctId{};

    Gaudi::Property<std::string> m_checkPatternStrategy{this, "CheckHitPatternStrategy", "Classical", "Either Classical or Extrapolation"};
    using PatternStrategyFunc = bool (VrtSecInclusive::*) ( const xAOD::TrackParticle *trk, const Amg::Vector3D& vertex );
    std::map<std::string, PatternStrategyFunc> m_patternStrategyFuncs;

    // AuxElement decorators
    std::optional< SG::Decorator< char > > m_decor_isSelected;
    std::optional< SG::Decorator< char > > m_decor_isAssociated;
    std::optional< SG::Decorator< char > > m_decor_is_svtrk_final;
    std::map< unsigned, SG::Decorator<float> > m_trkDecors;

    /** Read/Write Handle Keys **/
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey", "EventInfo", "EventInfo name"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_vertexKey {this, "VertexKey", "", "Vertex key"};
    SG::WriteHandleKey<xAOD::VertexContainer> m_twoTrksVertexKey {this, "TwoTracksVertexKey", "", "Two Tracks Vertex key"};
    std::map<std::string, SG::WriteHandleKey<xAOD::VertexContainer>> m_intermediateVertexKey;
    
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_vertexingStatusKey {this, "VertexingStatusKey", m_eventInfoKey, ""};
    
    using IPDecoratorType = SG::AuxElement::Decorator< std::vector< std::vector<float> > >;
    std::vector< IPDecoratorType > m_ipDecors;

    using VertexELType = SG::AuxElement::Decorator< std::vector<ElementLink< xAOD::VertexContainer > > >;
    std::optional< VertexELType > m_decor_svLink;

    //////////////////////////////////////////////////////////////////////////////////////
    //
    // define ntuple variables here
    //

    // The standard AANT, CollectionTree, is bare bones
    TTree      *m_tree_Vert{};
    std::unique_ptr<NtupleVars> m_ntupleVars;

    // Histograms for stats
    std::map<std::string, TH1*> m_hists;


    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Private member functions
    //

    // for event info to new ntuple (used to go by default in CollectionTree)
    void declareProperties();

    StatusCode addEventInfo();
    StatusCode setupNtupleVariables();
    StatusCode setupNtuple();
    StatusCode clearNtupleVariables();
    StatusCode deleteNtupleVariables();
    StatusCode processPrimaryVertices();
    StatusCode fillAANT_SelectedBaseTracks();
    StatusCode fillAANT_SecondaryVertices( xAOD::VertexContainer* );

    //
    struct WrkVrt {
      bool isGood = false;                            //! flagged true for good vertex candidates
      std::deque<long int> selectedTrackIndices;      //! list if indices in TrackParticleContainer for selectedBaseTracks
      std::deque<long int> associatedTrackIndices;    //! list if indices in TrackParticleContainer for associatedTracks
      Amg::Vector3D        vertex;                    //! VKalVrt fit vertex position
      TLorentzVector       vertexMom;                 //! VKalVrt fit vertex 4-momentum
      std::vector<double>  vertexCov;                 //! VKalVrt fit covariance
      double               Chi2 = 0;                  //! VKalVrt fit chi2 result
      double               Chi2_core = 0;             //! VKalVrt fit chi2 result
      std::vector<double>  Chi2PerTrk;                //! list of VKalVrt fit chi2 for each track
      long int             Charge = 0;                //! total charge of the vertex
      std::vector< std::vector<double> > TrkAtVrt;    //! list of track parameters wrt the reconstructed vertex
      unsigned long        closestWrkVrtIndex = 0;    //! stores the index of the closest WrkVrt in std::vector<WrkVrt>
      double               closestWrkVrtValue = 0;    //! stores the value of some observable to the closest WrkVrt ( observable = e.g. significance )

      inline double ndof() const { return 2.0*( selectedTrackIndices.size() + associatedTrackIndices.size() ) - 3.0; }
      inline double ndof_core() const { return 2.0*( selectedTrackIndices.size() ) - 3.0; }
      inline unsigned nTracksTotal() const { return selectedTrackIndices.size() + associatedTrackIndices.size(); }
      inline double fitQuality() const { return Chi2 / ndof(); }
    };


    using Detector = int;
    using Bec      = int;
    using Layer    = int;
    using Flag     = int;
    using ExtrapolatedPoint   = std::tuple<const TVector3, Detector, Bec, Layer, Flag>;
    using ExtrapolatedPattern = std::vector< ExtrapolatedPoint >;
    using PatternBank         = std::map<const xAOD::TrackParticle*, std::pair< std::unique_ptr<ExtrapolatedPattern>, std::unique_ptr<ExtrapolatedPattern> > >;

    PatternBank m_extrapolatedPatternBank;

    std::vector< std::pair<int, int> > m_incomp;

    // the map used by printWrkSet
    std::map<const xAOD::TruthVertex*, bool> m_matchMap;

    ////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// Vertexing Algorithm Member Functions
    ///

    /** select tracks which become seeds for vertex finding */
    void selectTrack( const xAOD::TrackParticle* );
    StatusCode selectTracksInDet(const EventContext& ctx);
    StatusCode selectTracksFromMuons(const EventContext& ctx);
    StatusCode selectTracksFromElectrons(const EventContext& ctx);
    StatusCode selectInDetAndGSFTracks(const EventContext& ctx);

    using TrackSelectionAlg = StatusCode (VrtSecInclusive::*)(const EventContext&);
    std::vector<TrackSelectionAlg> m_trackSelectionAlgs;

    /** track selection */
    using CutFunc = bool (VrtSecInclusive::*) ( const xAOD::TrackParticle* ) const;
    std::vector<CutFunc> m_trackSelectionFuncs;

    /** track-by-track selection strategies */
    bool selectTrack_notPVassociated ( const xAOD::TrackParticle* ) const;
    bool selectTrack_pTCut           ( const xAOD::TrackParticle* ) const;
    bool selectTrack_chi2Cut         ( const xAOD::TrackParticle* ) const;
    bool selectTrack_hitPattern      ( const xAOD::TrackParticle* ) const;
    bool selectTrack_hitPatternTight ( const xAOD::TrackParticle* ) const;
    bool selectTrack_d0Cut           ( const xAOD::TrackParticle* ) const;
    bool selectTrack_z0Cut           ( const xAOD::TrackParticle* ) const;
    bool selectTrack_d0errCut        ( const xAOD::TrackParticle* ) const;
    bool selectTrack_z0errCut        ( const xAOD::TrackParticle* ) const;
    static bool selectTrack_d0signifCut     ( const xAOD::TrackParticle* ) ;
    static bool selectTrack_z0signifCut     ( const xAOD::TrackParticle* ) ;
    bool selectTrack_LRTR3Cut        ( const xAOD::TrackParticle* ) const;

    /** related to the graph method and verte finding */
    StatusCode extractIncompatibleTrackPairs( const EventContext& ctx, std::vector<WrkVrt>* );
    StatusCode findNtrackVertices( const EventContext& ctx, std::vector<WrkVrt>* );
    StatusCode rearrangeTracks( const EventContext& ctx, std::vector<WrkVrt>* );

    /** attempt to merge vertices when all tracks of a vertex A is close to vertex B in terms of impact parameter */
    StatusCode reassembleVertices( const EventContext& ctx, std::vector<WrkVrt>* );

    /** attempt to merge splitted vertices when they are significantly distant
        due to the long-tail behavior of the vertex reconstruction resolution */
    StatusCode mergeByShuffling( const EventContext& ctx, std::vector<WrkVrt>* );

    /** attempt to merge vertices by lookng at the distance between two vertices */
    StatusCode mergeFinalVertices( const EventContext& ctx, std::vector<WrkVrt>* ); // Kazuki

    /** in addition to selected tracks, associate as much tracks as possible */
    StatusCode associateNonSelectedTracks( const EventContext& ctx, std::vector<WrkVrt>* );

    /** finalization of the vertex and store to xAOD::VertexContainer */
    StatusCode refitAndSelectGoodQualityVertices( const EventContext& ctx, std::vector<WrkVrt>* );

    /** get secondary vertex impact parameters **/
    bool getSVImpactParameters(const EventContext& ctx, const xAOD::TrackParticle* trk, const Amg::Vector3D& vertex, std::vector<double>& impactParameters, std::vector<double>& impactParErrors);

    enum TrkParameter    { k_d0=0, k_z0=1, k_theta=2, k_phi=3, k_qOverP=4 ,k_nTP=5 };
    enum TrkParameterUnc { k_d0d0=0, k_z0z0=1, k_nTPU=2 };

    using vertexingAlg = StatusCode (VrtSecInclusive::*)( const EventContext&, std::vector<WrkVrt>* );
    std::vector< std::pair<std::string, vertexingAlg> > m_vertexingAlgorithms;
    unsigned m_vertexingAlgorithmStep = 0U;


    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Supporting utility functions

    /** print the contents of reconstructed vertices */
    void printWrkSet(const std::vector<WrkVrt> *WrkVrtSet, const std::string& name);

    /** refit the vertex. */
    StatusCode refitVertex( const EventContext&, WrkVrt& );
    StatusCode refitVertex( WrkVrt&, Trk::IVKalState& istate );

    /** refit the vertex with suggestion */
    StatusCode refitVertexWithSuggestion( const EventContext& ctx, WrkVrt&, const Amg::Vector3D& );
    StatusCode refitVertexWithSuggestion( WrkVrt&, const Amg::Vector3D&, Trk::IVKalState& istate );

    /** attempt to improve the vertex chi2 by removing the most-outlier track one by one until
        the vertex chi2 satisfies a certain condition. */
    double improveVertexChi2( const EventContext&, WrkVrt& );

    static void removeTrackFromVertex(std::vector<WrkVrt>*,
                                      std::vector< std::deque<long int> > *,
                                      const long int & ,const long int & );

    StatusCode disassembleVertex(const EventContext& ctx, std::vector<WrkVrt> *, const unsigned& vertexIndex );

    void trackClassification(std::vector< WrkVrt >* , std::map< long int, std::vector<long int> >& );

    double findWorstChi2ofMaximallySharedTrack(std::vector<WrkVrt>*, std::map< long int, std::vector<long int> >&, long int & ,long int & );

    /** returns the number of tracks commonly present in both vertices */
    static size_t nTrkCommon( std::vector<WrkVrt> *WrkVrtSet, const std::pair<unsigned, unsigned>& pairIndex ) ;

    /** calculate the significance (Mahalanobis distance) between two reconstructed vertices */
    double significanceBetweenVertices( const WrkVrt&, const WrkVrt& ) const;

    /** calculate the physical distance */
    double distanceBetweenVertices( const WrkVrt&, const WrkVrt& ) const;

    using AlgForVerticesPair = double (VrtSecInclusive::*)( const WrkVrt&, const WrkVrt& ) const;

    /** returns the pair of vertices that give minimum in terms of some observable (e.g. distance, significance) */
    double findMinVerticesPair( std::vector<WrkVrt>*, std::pair<unsigned, unsigned>&, const AlgForVerticesPair& );

    /** returns the next pair of vertices that give next-to-minimum distance significance */
    static double findMinVerticesNextPair( std::vector<WrkVrt>*, std::pair<unsigned, unsigned>& );

    /** the 2nd vertex is merged into the 1st vertex. A destructive operation. */
    StatusCode mergeVertices( const EventContext& ctx, WrkVrt& destination, WrkVrt& source );

    enum mergeStep { RECONSTRUCT_NTRK, REASSEMBLE, SHUFFLE1, SHUFFLE2, SHUFFLE3, FINAL };

    typedef struct track_summary_properties {
      uint8_t numIBLHits;
      uint8_t numBLayerHits;
      uint8_t numPixelLayer1_Hits;
      uint8_t numPixelLayer2_Hits;
      uint8_t numPixelDisk0_Hits;
      uint8_t numPixelDisk1_Hits;
      uint8_t numPixelDisk2_Hits;
      uint8_t numPixelHits;
      uint8_t numSctBarrelLayer0_Hits;
      uint8_t numSctBarrelLayer1_Hits;
      uint8_t numSctBarrelLayer2_Hits;
      uint8_t numSctBarrelLayer3_Hits;
      uint8_t numSctEC0_Hits;
      uint8_t numSctEC1_Hits;
      uint8_t numSctEC2_Hits;
      uint8_t numSctEC3_Hits;
      uint8_t numSctEC4_Hits;
      uint8_t numSctHits;
      uint8_t numTrtHits;
    } track_summary;

    /** retrieve the track hit information */
    static void fillTrackSummary( track_summary& summary, const xAOD::TrackParticle *trk );

    ExtrapolatedPattern* extrapolatedPattern( const xAOD::TrackParticle*, enum Trk::PropDirection );

    bool patternCheck    ( const uint32_t& pattern, const Amg::Vector3D& vertex );
    static bool patternCheckRun1( const uint32_t& pattern, const Amg::Vector3D& vertex );
    static bool patternCheckRun2( const uint32_t& pattern, const Amg::Vector3D& vertex );

    bool patternCheckOuterOnly    ( const uint32_t& pattern, const Amg::Vector3D& vertex );
    static bool patternCheckRun1OuterOnly( const uint32_t& pattern, const Amg::Vector3D& vertex );
    static bool patternCheckRun2OuterOnly( const uint32_t& pattern, const Amg::Vector3D& vertex );

    /** A classical method with hard-coded geometry */
    bool checkTrackHitPatternToVertex( const xAOD::TrackParticle *trk, const Amg::Vector3D& vertex );

    /** A classical method with hard-coded geometry */
    bool checkTrackHitPatternToVertexOuterOnly( const xAOD::TrackParticle *trk, const Amg::Vector3D& vertex );

    /** New method with track extrapolation */
    bool checkTrackHitPatternToVertexByExtrapolation( const xAOD::TrackParticle *trk, const Amg::Vector3D& vertex );

    /** New method with track extrapolation */
    bool checkTrackHitPatternToVertexByExtrapolationAssist( const xAOD::TrackParticle *trk, const Amg::Vector3D& vertex );

    /** Flag false if the consistituent tracks are not consistent with the vertex position */
    bool passedFakeReject( const Amg::Vector3D& FitVertex, const xAOD::TrackParticle *itrk, const xAOD::TrackParticle *jtrk );

    /** Remove inconsistent tracks from vertices */
    void removeInconsistentTracks( WrkVrt& );

    template<class Track> void getIntersection(Track *trk, std::vector<IntersectionPos*>& layers, const Trk::Perigee* per);
    template<class Track> void setIntersection(Track *trk, IntersectionPos *bec, const Trk::Perigee* per);

    /** monitor the intermediate status of vertexing */
    StatusCode monitorVertexingAlgorithmStep( const EventContext& ctx,
					      std::vector<WrkVrt>*, const std::string& name, bool final = false );

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Truth Information Algorithms Member Functions
    //
    //

    static const xAOD::TruthParticle *getTrkGenParticle(const xAOD::TrackParticle*) ;

    StatusCode categorizeVertexTruthTopology( xAOD::Vertex *vertex );

    void dumpTruthInformation();

    std::vector<const xAOD::TruthVertex*> m_tracingTruthVertices;

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Additional augmentation
    //
    //

    template<class LeptonFlavor>
    StatusCode augmentDVimpactParametersToLeptons( const EventContext& ctx, const std::string& containerName );

    /** lock decorations at the end of the algorithm */
    void lockTrackDecorations( const xAOD::TrackParticle* trk, bool onlySelection ) const;
    void lockLeptonDecorations( const SG::AuxVectorData* cont ) const;
    StatusCode lockTrackDecorations( bool onlySelection, const EventContext& ctx ) const;

    std::unordered_map<std::string, bool> m_vertexCollectionsDefinitions;
  };

} // end of namespace bracket


  // This header file contains the definition of member templates
#include "details/Utilities.h"


#endif /* VRTSECINCLUSIVE_VRTSECINCLUSIVE_H */
