/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    TrackAnalysisPlotsMgr.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    19 June 2023
**/

/// local includes
#include "TrackAnalysisPlotsMgr.h"
#include "TrackAnalysisCollections.h"
#include "ITrackMatchingLookup.h"
#include "OfflineObjectDecorHelper.h"
#include "TrackParametersHelper.h"

/// Gaudi include(s)
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"


/// -------------------
/// --- Constructor ---
/// -------------------
IDTPM::TrackAnalysisPlotsMgr::TrackAnalysisPlotsMgr(
    const std::string& dirName,
    const std::string& anaTag,
    const std::string& chain,
    PlotMgr* pParent ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_anaTag( anaTag ), m_chain( chain ),
        m_directory( dirName ) { }


/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::TrackAnalysisPlotsMgr::initialize()
{
  ATH_MSG_DEBUG( "Initialising in directory: " << m_directory );

  /// load trkAnaDefSvc 
  m_trkAnaDefSvc = Gaudi::svcLocator()->service( "TrkAnaDefSvc"+m_anaTag );
  ATH_CHECK( m_trkAnaDefSvc.isValid() );

  /// Summary plots
  if( m_trkAnaDefSvc->plotTrackMultiplicities() ) {
    m_plots_summary = std::make_unique< SummaryPlots >(
        this, "Tracks/Multiplicities", m_anaTag,
        m_trkAnaDefSvc->doTrigNavigation() );
  }

  /// Track parameters plots
  if( m_trkAnaDefSvc->plotTrackParameters() ) {
    m_plots_trkParam_vsTest = std::make_unique< TrackParametersPlots >(
        this, "Tracks/Parameters", m_anaTag, m_trkAnaDefSvc->testTag() );
    m_plots_trkParam_vsRef = std::make_unique< TrackParametersPlots >(
        this, "Tracks/Parameters", m_anaTag, m_trkAnaDefSvc->referenceTag() );
  } 

  /// Track multiplicity plots
  if( m_trkAnaDefSvc->plotTrackMultiplicities() ) {
    m_plots_nTracks_vsTest = std::make_unique< NtracksPlots >(
        this, "Tracks/Multiplicities", m_anaTag, m_trkAnaDefSvc->testTag(),
        m_trkAnaDefSvc->doTrigNavigation(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    m_plots_nTracks_vsRef = std::make_unique< NtracksPlots >(
        this, "Tracks/Multiplicities", m_anaTag, m_trkAnaDefSvc->referenceTag(),
        m_trkAnaDefSvc->doTrigNavigation() );
  }

  /// Efficiency plots
  if( m_trkAnaDefSvc->plotEfficiencies() ) {
    m_plots_eff_vsTest = std::make_unique< EfficiencyPlots >(
        this, "Tracks/Efficiencies/Purities", m_anaTag, m_trkAnaDefSvc->testTag() );
    m_plots_eff_vsRef = std::make_unique< EfficiencyPlots >(
        this, "Tracks/Efficiencies", m_anaTag, m_trkAnaDefSvc->referenceTag(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    if( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
      m_plots_eff_vsTruth = std::make_unique< EfficiencyPlots >(
          this, "Tracks/Efficiencies", m_anaTag, "truth" );
    }
  }

  /// Technical efficiency plots
  if( m_trkAnaDefSvc->plotTechnicalEfficiencies()) {
    m_plots_tech_eff_vsTest = std::make_unique< EfficiencyPlots >(
        this, "Tracks/Efficiencies/Technical/Purities", m_anaTag, m_trkAnaDefSvc->testTag());
    m_plots_tech_eff_vsRef = std::make_unique< EfficiencyPlots >(
        this, "Tracks/Efficiencies/Technical", m_anaTag, m_trkAnaDefSvc->referenceTag(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    if( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
      m_plots_tech_eff_vsTruth = std::make_unique< EfficiencyPlots >(
          this, "Tracks/Efficiencies/Technical", m_anaTag, "truth" );
    }
  }

  /// Resolution plots
  if( m_trkAnaDefSvc->plotResolutions() ) {
    m_plots_resolution = std::make_unique< ResolutionPlots >(
        this, "Tracks/Resolutions", m_anaTag, 
        m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->referenceTag(),
        m_trkAnaDefSvc->resolutionMethod() );
  }

  /// Fake Rate plots (only if reference is Truth)
  if( m_trkAnaDefSvc->plotFakeRates() and m_trkAnaDefSvc->isReferenceTruth() ) {
    m_plots_fakeRate = std::make_unique< FakeRatePlots >(
        this, "Tracks/FakeRates", m_anaTag, m_trkAnaDefSvc->testTag(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    if ( not m_trkAnaDefSvc->unlinkedAsFakes() ) {
      m_plots_missingTruth = std::make_unique< FakeRatePlots >(
          this, "Tracks/FakeRates/Unlinked", m_anaTag, m_trkAnaDefSvc->testTag(),
          true, m_trkAnaDefSvc->hasFullPileupTruth() );
    }
  }

  /// Duplicate Rate plots
  if( m_trkAnaDefSvc->plotDuplicateRates() ) {
    m_plots_duplRate = std::make_unique< DuplicateRatePlots >(
        this, "Tracks/Duplicates", m_anaTag, m_trkAnaDefSvc->referenceTag(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
  }

  /// Hits on tracks plots
  /// -- all tracks
  if( m_trkAnaDefSvc->plotHitsOnTracks() and not m_trkAnaDefSvc->isTestTruth() ) {
    m_plots_hitsOnTrk_vsTest = std::make_unique< HitsOnTracksPlots >(
        this, "Tracks/HitsOnTracks", m_anaTag,
        m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->isITk(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
  }
  if( m_trkAnaDefSvc->plotHitsOnTracksReference() and not m_trkAnaDefSvc->isReferenceTruth() ) {
    m_plots_hitsOnTrk_vsRef = std::make_unique< HitsOnTracksPlots >(
        this, "Tracks/HitsOnTracks", m_anaTag,
        m_trkAnaDefSvc->referenceTag(), m_trkAnaDefSvc->isITk() );
  }
  /// -- matched tracks
  if( m_trkAnaDefSvc->plotHitsOnMatchedTracks() and not m_trkAnaDefSvc->isTestTruth() ) {
    m_plots_hitsOnMatchedTrk = std::make_unique< HitsOnTracksPlots >(
        this, "Tracks/Resolutions/HitsOnTracks", m_anaTag,
        m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->isITk(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    m_plots_hitsOnMatchedTrk_vsRef = std::make_unique< HitsOnTracksPlots >(
        this, "Tracks/Resolutions/HitsOnTracks", m_anaTag,
        m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->referenceTag(), m_trkAnaDefSvc->isITk() );
  }
  /// -- fake and unlinked tracks
  if( m_trkAnaDefSvc->plotHitsOnFakeTracks() and m_trkAnaDefSvc->isReferenceTruth() ) {
    m_plots_hitsOnFakeTrk = std::make_unique< HitsOnTracksPlots >(
        this, "Tracks/FakeRates/HitsOnTracks", m_anaTag,
        m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->isITk(),
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    if ( not m_trkAnaDefSvc->unlinkedAsFakes() ) {
      m_plots_hitsOnUnlinkedTrk = std::make_unique< HitsOnTracksPlots >(
          this, "Tracks/FakeRates/Unlinked/HitsOnTracks", m_anaTag,
          m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->isITk(),
          true, m_trkAnaDefSvc->hasFullPileupTruth() );
    }
  }

  /// Offline electron plots
  if( m_trkAnaDefSvc->plotOfflineElectrons() ) {
    m_plots_offEle = std::make_unique< OfflineElectronPlots >(
        this, "Tracks/Parameters", m_anaTag );
    if( m_trkAnaDefSvc->plotEfficiencies() ) {
      m_plots_eff_vsOffEle = std::make_unique< OfflineElectronPlots >(
          this, "Tracks/Efficiencies", m_anaTag, true );
    }
  }

  /// Vertex parameters plots
  if( m_trkAnaDefSvc->plotVertexParameters() ) {
    /// Vertices parameters plots
    m_plots_vtxParam_vsTest = std::make_unique< VertexParametersPlots >(
        this, "Vertices/AllPrimary/Parameters", m_anaTag,
        m_trkAnaDefSvc->testTag(),
        not m_trkAnaDefSvc->isTestTruth() ); // do associated tracks plots for reco only
    m_plots_vtxParam_vsRef = std::make_unique< VertexParametersPlots >(
        this, "Vertices/AllPrimary/Parameters", m_anaTag,
        m_trkAnaDefSvc->referenceTag(),
        not m_trkAnaDefSvc->isReferenceTruth() ); // do associated tracks plots for reco only

    /// Vertices multiplicity plots
    m_plots_nVtxParam_vsTest = std::make_unique< VertexParametersPlots >(
        this, "Vertices/AllPrimary/Parameters", m_anaTag,
        m_trkAnaDefSvc->testTag(), false,
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
    m_plots_nVtxParam_vsRef = std::make_unique< VertexParametersPlots >(
        this, "Vertices/AllPrimary/Parameters", m_anaTag,
        m_trkAnaDefSvc->referenceTag(), false,
        true, m_trkAnaDefSvc->hasFullPileupTruth() );
  } 

  /// intialize PlotBase
  ATH_CHECK( PlotMgr::initialize() );

  return StatusCode::SUCCESS;
}


/// --------------------------
/// ------ General fill ------
/// --------------------------
StatusCode IDTPM::TrackAnalysisPlotsMgr::fill(
    TrackAnalysisCollections& trkAnaColls, float weight )
{
  float actualMu = trkAnaColls.eventInfo() ?
                   trkAnaColls.eventInfo()->actualInteractionsPerCrossing() : -1.;
  float truthMu = trkAnaColls.truthPileupEventContainer() ?
                  static_cast< float >( trkAnaColls.truthPileupEventContainer()->size() ) : -1.;

  /// Plots w.r.t. test tracks quantities
  if( m_trkAnaDefSvc->isTestTruth() ) {
    ATH_CHECK( fillPlotsTest(
        trkAnaColls.testTruthVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(),
        trkAnaColls.testTruthVertexVec( TrackAnalysisCollections::InRoI ),
        truthMu, actualMu, weight ) );
  } else {
    ATH_CHECK( fillPlotsTest(
        trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(),
        trkAnaColls.testRecoVertexVec( TrackAnalysisCollections::InRoI ),
        truthMu, actualMu, weight ) );
  } 

  /// Plots w.r.t. reference tracks quantities
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    ATH_CHECK( fillPlotsReference(
        trkAnaColls.refTruthVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(),
        trkAnaColls.refTruthVertexVec( TrackAnalysisCollections::InRoI ),
        truthMu, actualMu, weight ) );
  } else {
    ATH_CHECK( fillPlotsReference(
        trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(),
        trkAnaColls.refRecoVertexVec( TrackAnalysisCollections::InRoI ),
        truthMu, actualMu, weight ) );
  } 

  /// Plots w.r.t. truth quantities (for EFTruthMatch only)
  if( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
    ATH_CHECK( fillPlotsTruth(
        trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.truthPartVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(),
        truthMu, actualMu, weight ) );
  }

  /// Track and vertex multiplicity plots
  std::vector< size_t > testTrackCounts = trkAnaColls.testTrackCounts();
  std::vector< size_t > refTrackCounts = trkAnaColls.refTrackCounts();

  std::vector< size_t > testVertexCounts = trkAnaColls.testVertexCounts();
  std::vector< size_t > refVertexCounts = trkAnaColls.refVertexCounts();

  if( m_plots_summary ) {
    ATH_CHECK( m_plots_summary->fillPlots(
        testTrackCounts, refTrackCounts,
        trkAnaColls.isNewRoI(), weight ) );
  }

  if( m_plots_nTracks_vsTest ) {
    ATH_CHECK( m_plots_nTracks_vsTest->fillPlots(
        testTrackCounts, testVertexCounts,
        truthMu, actualMu, weight ) );
  }

  if( m_plots_nTracks_vsRef ) {
    ATH_CHECK( m_plots_nTracks_vsRef->fillPlots(
        refTrackCounts, refVertexCounts,
        truthMu, actualMu, weight ) );
  }

  return StatusCode::SUCCESS;
}


/// ------------------------------
/// --- Fill plots w.r.t. test ---
/// ------------------------------
template< typename PARTICLE, typename VERTEX >
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest(
    const std::vector< const PARTICLE* >& particles,
    const ITrackMatchingLookup& matches,
    const std::vector< const VERTEX* >& vertices,
    float truthMu, float actualMu, float weight )
{
  /// Selected particles loop
  for( const PARTICLE* particle : particles ) {
    /// track parameters plots
    if( m_plots_trkParam_vsTest ) {
      ATH_CHECK( m_plots_trkParam_vsTest->fillPlots( *particle, weight ) );
    }

    /// hits on tracks plots
    if( m_plots_hitsOnTrk_vsTest ) {
      ATH_CHECK( m_plots_hitsOnTrk_vsTest->fillPlots( *particle, truthMu, actualMu, weight ) );
    }

    bool isMatched = matches.isTestMatched( *particle );

    /// efficiency plots
    if( m_plots_eff_vsTest ) {
      ATH_CHECK( m_plots_eff_vsTest->fillPlots(
          *particle, isMatched, truthMu, actualMu, weight ) );
    }

    /// technical efficiency plots 
    if( m_plots_tech_eff_vsTest ) {
      if (  m_trkAnaDefSvc->isTestTruth() and
            isReconstructable( *particle,
                               m_trkAnaDefSvc->minSilHits(),
                               m_trkAnaDefSvc->etaBins() ) ) {
        ATH_CHECK( m_plots_tech_eff_vsTest->fillPlots(
            *particle, isMatched, truthMu, actualMu, weight ) );
      }
      else if (  m_trkAnaDefSvc->isReferenceTruth() ) {
        bool isTechMatched = isMatched ?
            isReconstructable( *(matches.getMatchedRefTruth( *particle )),
                               m_trkAnaDefSvc->minSilHits(),
                               m_trkAnaDefSvc->etaBins() ) : false;
        ATH_CHECK( m_plots_tech_eff_vsTest->fillPlots(
            *particle, isTechMatched, truthMu, actualMu, weight ) );
      }
      else if ( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
        const xAOD::TruthParticle* linkedTruth = getLinkedTruth(
          *particle, m_trkAnaDefSvc->truthProbCut() );
        bool isTechMatched = isMatched ? 
            isReconstructable( *linkedTruth,
                               m_trkAnaDefSvc->minSilHits(),
                               m_trkAnaDefSvc->etaBins() ) : false;
        ATH_CHECK( m_plots_tech_eff_vsTest->fillPlots(
            *particle, isTechMatched, truthMu, actualMu, weight ) );
      }
    }

    /// resolution plots
    if( m_plots_resolution ) {
      if( isMatched ) {
        if( m_trkAnaDefSvc->isReferenceTruth() ) {
	        ATH_CHECK( m_plots_resolution->fillPlots(
            *particle, *(matches.getMatchedRefTruth( *particle )), weight ) );
        } else {
	        ATH_CHECK( m_plots_resolution->fillPlots(
            *particle, *(matches.getMatchedRefTrack( *particle )), weight ) );
        }
      }
    }

    /// hits on matched tracks plots
    if( m_plots_hitsOnMatchedTrk and m_plots_hitsOnMatchedTrk_vsRef and isMatched ) {
      ATH_CHECK( m_plots_hitsOnMatchedTrk->fillPlots( *particle, truthMu, actualMu, weight ) );
      if( m_trkAnaDefSvc->isReferenceTruth() ) {
        ATH_CHECK( m_plots_hitsOnMatchedTrk_vsRef->fillPlots(
          *particle, *(matches.getMatchedRefTruth( *particle )), truthMu, actualMu, weight ) );
      } else {
        ATH_CHECK( m_plots_hitsOnMatchedTrk_vsRef->fillPlots(
          *particle, *(matches.getMatchedRefTrack( *particle )), truthMu, actualMu, weight ) );
      }
    }

    /// fake rate plots (and hits on fake plots)
    bool isUnlinked = isUnlinkedTruth( *particle );
    bool notTruthMatched = getTruthMatchProb( *particle ) < 0.5;
    if ( isUnlinked ) {
      if( m_plots_missingTruth ) {
        ATH_CHECK( m_plots_missingTruth->fillPlots( *particle, notTruthMatched, truthMu, actualMu, weight ) );
      }
      if( m_plots_hitsOnUnlinkedTrk ) {
        ATH_CHECK( m_plots_hitsOnUnlinkedTrk->fillPlots( *particle, truthMu, actualMu, weight ) ); 
      }
    }

    bool doFakes = m_trkAnaDefSvc->unlinkedAsFakes() ? true : not isUnlinked;
    if( doFakes and m_plots_fakeRate ) {
      bool isFake = isFakeTruth( *particle, m_trkAnaDefSvc->truthProbCut(),
                                 m_trkAnaDefSvc->unlinkedAsFakes() );
      ATH_CHECK( m_plots_fakeRate->fillPlots( *particle, isFake, truthMu, actualMu, weight ) );
      if( m_plots_hitsOnFakeTrk and isFake ) {
        ATH_CHECK( m_plots_hitsOnFakeTrk->fillPlots( *particle, truthMu, actualMu, weight ) );
      }
    }

    /// offline electron plots (Offline is always either test or reference)
    if( m_trkAnaDefSvc->isTestOffline() ) {
      if( m_plots_offEle ) {
        ATH_CHECK( m_plots_offEle->fillPlots( *particle, false, weight ) );
      }
      if( m_plots_eff_vsOffEle ) {
        ATH_CHECK( m_plots_eff_vsOffEle->fillPlots( *particle, isMatched, weight ) );
      }
    }

  } // close loop over particles

  /// Selected vertices loop
  int nGoodVertices(0);
  for( const VERTEX* vertex : vertices ) {
    /// skip dummy vertex
    if( vertexType( *vertex ) == xAOD::VxType::NoVtx ) {
      ATH_MSG_DEBUG( "Found Dummy vertex. Skipping" );
      continue;
    }
    nGoodVertices++;

    /// getting vertex-associated tracks and their weights
    std::vector< const PARTICLE* > vtxTracks{};
    std::vector< float > vtxTrackWeights{};
    if( not getVertexTracksAndWeights(
          *vertex, vtxTracks, vtxTrackWeights,
          particles, m_trkAnaDefSvc->useSelectedVertexTracks() ) ) {
      ATH_MSG_WARNING( "Problem when retrieving vertex-assocciated tracks" );
      if( not vtxTracks.empty() ) {
        ATH_MSG_WARNING( "Invalid associated track links found. Check your input format." );
      }
    }

    /// vertex parameters plots
    if( m_plots_vtxParam_vsTest ) {
      ATH_CHECK( m_plots_vtxParam_vsTest->fillPlots( *vertex, vtxTracks, vtxTrackWeights, weight ) );
    }
  } /// close loop over vertices

  /// Vertices multiplicity plots
  if( m_plots_nVtxParam_vsTest ) {
    ATH_CHECK( m_plots_nVtxParam_vsTest->fillPlots( nGoodVertices, truthMu, actualMu, weight ) );
  }

  return StatusCode::SUCCESS;
}

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest< xAOD::TrackParticle, xAOD::Vertex >(
    const std::vector< const xAOD::TrackParticle* >& particles,
    const ITrackMatchingLookup& matches,
    const std::vector< const xAOD::Vertex* >& vertices,
    float truthMu, float actualMu, float weight );

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest< xAOD::TruthParticle, xAOD::TruthVertex >(
    const std::vector< const xAOD::TruthParticle* >& particles,
    const ITrackMatchingLookup& matches,
    const std::vector< const xAOD::TruthVertex* >& vertices,
    float truthMu, float actualMu, float weight );


/// -----------------------------------
/// --- Fill plots w.r.t. reference ---
/// -----------------------------------
template< typename PARTICLE, typename VERTEX >
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference(
    const std::vector< const PARTICLE* >& particles,
    const ITrackMatchingLookup& matches,
    const std::vector< const VERTEX* >& vertices,
    float truthMu, float actualMu, float weight )
{

  for( const PARTICLE* particle : particles ) {

    /// track parameters plots
    if( m_plots_trkParam_vsRef ) {
      ATH_CHECK( m_plots_trkParam_vsRef->fillPlots( *particle, weight ) );
    }

    /// hits on tracks plots
    if( m_plots_hitsOnTrk_vsRef ) {
      ATH_CHECK( m_plots_hitsOnTrk_vsRef->fillPlots( *particle, truthMu, actualMu, weight ) );
    }

    bool isMatched = matches.isRefMatched( *particle );

    /// efficiency plots
    if( m_plots_eff_vsRef ) {
      ATH_CHECK( m_plots_eff_vsRef->fillPlots(
          *particle, isMatched, truthMu, actualMu, weight ) );
    }

    /// technical efficiency plots 
    if( m_plots_tech_eff_vsRef ) {
      if( m_trkAnaDefSvc->isReferenceTruth() and 
          isReconstructable( *particle, m_trkAnaDefSvc->minSilHits(), m_trkAnaDefSvc->etaBins() ) )
      {
        ATH_CHECK( m_plots_tech_eff_vsRef->fillPlots(
            *particle, isMatched, truthMu, actualMu, weight ) );
      }
      else if (  m_trkAnaDefSvc->isTestTruth() ) { 

        bool isTechMatched = false;

        if (isMatched) {
          for ( const xAOD::TruthParticle *thisTruth : (matches.getMatchedTestTruths( *particle ))) {
            if ( isReconstructable( *thisTruth, m_trkAnaDefSvc->minSilHits(), m_trkAnaDefSvc->etaBins() ) ) {
              isTechMatched = true;
              break;
            }
          }
        }
        ATH_CHECK( m_plots_tech_eff_vsRef->fillPlots(
            *particle, isTechMatched, truthMu, actualMu, weight ) );
      }

      else if ( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
        const xAOD::TruthParticle* linkedTruth = getLinkedTruth(
          *particle, m_trkAnaDefSvc->truthProbCut() );
        bool isTechMatched = isMatched ? 
            isReconstructable( *linkedTruth, m_trkAnaDefSvc->minSilHits(), m_trkAnaDefSvc->etaBins() ) : false;
        ATH_CHECK( m_plots_tech_eff_vsRef->fillPlots(
            *particle, isTechMatched, truthMu, actualMu, weight ) );
      }
    }

    /// duplicate rate plots
    if( m_plots_duplRate ) {
      unsigned int nMatched = m_trkAnaDefSvc->isTestTruth() ?
                              matches.getMatchedTestTruths( *particle ).size() :
                              matches.getMatchedTestTracks( *particle ).size();

      ATH_CHECK( m_plots_duplRate->fillPlots(
          *particle, nMatched, truthMu, actualMu, weight ) );
    }
 
    /// offline electron plots (Offline is always either test or reference)
    if( m_trkAnaDefSvc->isReferenceOffline() ) {
      if( m_plots_offEle ) {
        ATH_CHECK( m_plots_offEle->fillPlots( *particle, false, weight ) );
      }
      if( m_plots_eff_vsOffEle ) {
        ATH_CHECK( m_plots_eff_vsOffEle->fillPlots( *particle, isMatched, weight ) );
      }
    }

  } // close loop over particles

  /// Selected vertices loop
  int nGoodVertices(0);
  for( const VERTEX* vertex : vertices ) {
    /// skip dummy vertex
    if( vertexType( *vertex ) == xAOD::VxType::NoVtx ) {
      ATH_MSG_DEBUG( "Found Dummy vertex. Skipping" );
      continue;
    }
    nGoodVertices++;

    /// getting vertex-associated tracks and their weights
    std::vector< const PARTICLE* > vtxTracks{};
    std::vector< float > vtxTrackWeights{};
    if( not getVertexTracksAndWeights(
          *vertex, vtxTracks, vtxTrackWeights,
          particles, m_trkAnaDefSvc->useSelectedVertexTracks() ) ) {
      ATH_MSG_WARNING( "Problem when retrieving vertex-assocciated tracks" );
      if( not vtxTracks.empty() ) {
        ATH_MSG_WARNING( "Invalid associated track links found. Check your input format." );
      }
    }

    /// vertex parameters plots
    if( m_plots_vtxParam_vsRef ) {
      ATH_CHECK( m_plots_vtxParam_vsRef->fillPlots( *vertex, vtxTracks, vtxTrackWeights, weight ) );
    }
  } /// close loop over vertices

  /// Vertices multiplicity plots
  if( m_plots_nVtxParam_vsRef ) {
    ATH_CHECK( m_plots_nVtxParam_vsRef->fillPlots( nGoodVertices, truthMu, actualMu, weight ) );
  }

  return StatusCode::SUCCESS;
}

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference< xAOD::TrackParticle, xAOD::Vertex >(
    const std::vector< const xAOD::TrackParticle* >& particles,
    const ITrackMatchingLookup& matches,
    const std::vector< const xAOD::Vertex* >& vertices,
    float truthMu, float actualMu, float weight );

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference< xAOD::TruthParticle, xAOD::TruthVertex >(
    const std::vector< const xAOD::TruthParticle* >& particles,
    const ITrackMatchingLookup& matches,
    const std::vector< const xAOD::TruthVertex* >& vertices,
    float truthMu, float actualMu, float weight );


/// ------------------------------
/// --- Fill plots w.r.t. truth ---
/// ------------------------------
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsTruth(
    const std::vector< const xAOD::TrackParticle* >& testTracks,
    const std::vector< const xAOD::TrackParticle* >& refTracks,
    const std::vector< const xAOD::TruthParticle* >& truths,
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight )
{

  for( const xAOD::TruthParticle* thisTruth : truths ) {

    bool isMatched( false );  // test track matched to reference track through EFTruthMatch method
    bool refMatched( false ); // reference track matched to thisTruth

    /// Loop over reference tracks to look for a reference matched to thisTruth
    for( const xAOD::TrackParticle* thisTrack : refTracks ) {
      const xAOD::TruthParticle* linkedTruth = getLinkedTruth(
          *thisTrack, m_trkAnaDefSvc->truthProbCut() );
      if( not linkedTruth ) {
        ATH_MSG_WARNING( "Unlinked track!!" );
        continue;
      }
      if( thisTruth == linkedTruth ) {
        refMatched = true;
        break;
      }
    } // close loop over reference tracks

    /// Fill the histogram only if a matched reference is found
    if ( not refMatched ) continue;
    
    else {

      /// Loop over test tracks to look for a test matched to thisTruth
      for( const xAOD::TrackParticle* thisTrack : testTracks ) {
        const xAOD::TruthParticle* linkedTruth = getLinkedTruth(
            *thisTrack, m_trkAnaDefSvc->truthProbCut() );
        if( not linkedTruth ) {
          ATH_MSG_WARNING( "Unlinked track!!" );
          continue;
        }
        if( thisTruth == linkedTruth ) {
          isMatched = matches.isTestMatched( *thisTrack ); // Check if this test is matched to reference with EFTruthMatch
          break;
        }
      } // close loop over test tracks
      
      /// efficiency plots (for EFTruthMatch only)
      if( m_plots_eff_vsTruth ) {
        ATH_CHECK( m_plots_eff_vsTruth->fillPlots(
            *thisTruth, isMatched, truthMu, actualMu, weight ) );
      }

      /// technical efficiency plots (for EFTruthMatch only)
      if( m_plots_tech_eff_vsTruth ) {
        if (isReconstructable( *thisTruth, m_trkAnaDefSvc->minSilHits(), m_trkAnaDefSvc->etaBins() )) {
            ATH_CHECK( m_plots_tech_eff_vsTruth->fillPlots(
            *thisTruth, isMatched , truthMu, actualMu, weight ) );
        }
      }
    }
  } // close loop over truth particles

  return StatusCode::SUCCESS;
}
