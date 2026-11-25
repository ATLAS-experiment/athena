/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// InDetRecStatisticsAlg.h
// Authors: Sven Vahsen, Lauren Tompkins
///////////////////////////////////////////////////////////////////

#ifndef INDETRECSTATISTICS_TrackStatistics_H
#define INDETRECSTATISTICS_TrackStatistics_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TrkTrack/TrackCollection.h"
#include "TrackStatHelper.h"
#include "Identifier/Identifier.h"
#include "TrkToolInterfaces/IPRDtoTrackMapTool.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkToolInterfaces/ITruthToTrack.h"
#include "TrkToolInterfaces/IUpdator.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "GeneratorObjects/McEventCollection.h"

#include <vector>
#include <atomic>

// forward declarations

class AtlasDetectorID;
class PixelID;
class SCT_ID;
class TRT_ID;
class IdDictManager;
class GenParticle;

namespace Trk {
  class ITrackSummaryTool;
  class TrackSummary;
}

namespace InDet {

    class InDetRecStatisticsAlg:public AthReentrantAlgorithm {


 /**
  @class InDetRecStatisticsAlg
  This class is used to compute statistics to compare multiple track finding algorithms.  It prints a statistics table which shows efficiencies and fake rates for input track collections.
  @author lauren.tompkin@cern.ch, sven.vahsen@cern.ch
  */

     public:
      /**Default Constructor*/
      using AthReentrantAlgorithm::AthReentrantAlgorithm;
      /** Initialization of services, track collections, creates TrackStatHelper for each Track Collection*/
      StatusCode    initialize();
      /**Calculation of statistics*/
      StatusCode    execute(const EventContext &ctx) const;
      /**Statistics table printed, collection cleared*/
      StatusCode    finalize();
      /**Get various services such as StoreGate, dictionaries, detector managers etc.*/
      StatusCode    getServices();
      /** Clear statistics counters, called before each track collection is processed*/
      StatusCode    resetStatistics();
      /** Print tracking statistics calculated with TrackStatHelper */
      void          printStatistics();
      /** Print track statistics for all and low proability tracks */
      void          printTrackSummary   (MsgStream &out, enum eta_region );
      


     private:

      template <typename T_Int=int>
      struct Counter;
      using CounterLocal = Counter<int>;

      /** Select charged,stable particles which pass pt and eta cuts for analysis*/
      /** Classify truth tracks as primary, secondary or truncated */
      void          selectGenSignal     (const McEventCollection*,
					 std::vector <std::pair<HepMC::ConstGenParticlePtr,int> > &,
					 unsigned int,
                                         unsigned int,
                                         CounterLocal &counter) const;

      /**Select for analysis reconstructed tracks passing Pt and eta cuts*/
      void          selectRecSignal     (const TrackCollection*,
					 std::vector <const Trk::Track *> & ,
					 std::vector <const Trk::Track *> &,
                                         CounterLocal &counter) const;



      std::vector <class TrackStatHelper *> m_SignalCounters;//!< Vector of TrackStatHelper objects, one for each track collection
      const AtlasDetectorID*                m_idHelper{};          //!< Used to find out the sub-det from PRD->identify().
      const PixelID*                        m_pixelID{}; //!< get pixel layer from hit ID
      const SCT_ID*                         m_sctID{}; //!< get sct layer from hit ID
      const TRT_ID*                         m_trtID{}; //!< get trt layer from hit ID
      // added to check TRT existence (SLHC geo check)
      const IdDictManager * m_idDictMgr{};
      PublicToolHandle<Trk::ITruthToTrack>              m_truthToTrack{this, "TruthToTrackTool", "Trk::TruthToTrack"}; //!< tool to create track parameters from a gen particle
      PublicToolHandle<Trk::IExtendedTrackSummaryTool>  m_trkSummaryTool{this, "SummaryTool", "Trk::TrackSummaryTool/InDetTrackSummaryTool"};  //!< tool to get track summary information from track 

      PublicToolHandle<Trk::IUpdator>                   m_updatorHandle{this, "UpdatorTool", "Trk::KalmanUpdator/TrkKalmanUpdator", "Measurement updator to calculate unbiased track states"};            //!< Tool handle of updator for unbiased states
      Trk::IUpdator*                              m_updator{};                  //!< updator for unbiased states
      PublicToolHandle<Trk::IResidualPullCalculator>    m_residualPullCalculator{this, "ResidualPullCalculatorTool", "Trk::ResidualPullCalculator/ResidualPullCalculator", "Tool to calculate residuals and pulls"};   //!< The residual and pull calculator tool handle

      // steering parameters of Algorithm
      SG::ReadHandleKey<McEventCollection>  m_McTrackCollection_key{this, "McTrackCollectionKey", "TruthEvent"};
      PublicToolHandle<Trk::ITrackSelectorTool> m_trackSelectorTool{this, "TrackSelectorTool", "InDet::InDetDetailedTrackSelectorTool"};
     
      Gaudi::Property<bool> m_UseTrackSummary{this, "UseTrackSummary", true};//!< Flag to print detailed statistics for each track collection
      Gaudi::Property<bool> m_printSecondary{this, "PrintSecondary", false}; //!< Flag to print hit information for secondary tracks
      Gaudi::Property<float> m_minPt{this, "minPt", 1000};//!< Minimum Pt cut for tracks used by the algorithm
      Gaudi::Property<float> m_maxEta{this, "maxEta", 4.2}; //!< Maximum Eta cut for tracks used by the algorithm
      Gaudi::Property<float> m_maxEtaBarrel{this, "maxEtaBarrel", 0.8}; //!< define max eta of barrel region
      Gaudi::Property<float> m_maxEtaTransition{this, "maxEtaTransition", 1.6}; //!< define max eta of transition region
      Gaudi::Property<float> m_maxEtaEndcap{this, "maxEtaEndcap", 2.5}; //!< define max eta of eta region
      Gaudi::Property<float> m_fakeTrackCut{this, "fakeTrackCut", 0.9}; //!< Maximum probability for which a track will be considered a fake.
      Gaudi::Property<float> m_fakeTrackCut2{this, "fakeTrackCut2", 0.7};//!< Second definition of maximum probability for which a track will be considered a fake
      Gaudi::Property<float> m_matchTrackCut{this, "matchTrackCut", 0.5};//!< Minimum number of hits from a truth track to be considered a matched reco track
      //      float         m_maxRIndet;
      Gaudi::Property<float> m_maxRStartPrimary{this, "maxRStartPrimary", 25.0*CLHEP::mm};//!< Maximum R of start vertex to be considered primary.
      Gaudi::Property<float> m_maxRStartSecondary{this, "maxRStartSecondary", 360.0*CLHEP::mm};//!< Maximum R of start vertex to be considered secondary.
      Gaudi::Property<float> m_maxZStartPrimary{this, "maxZStartPrimary", 200.0*CLHEP::mm};//!< Maximum Z of start vertex to be considered primary.
      Gaudi::Property<float> m_maxZStartSecondary{this, "maxZStartSecondary", 2000.0*CLHEP::mm};//!< Maximum Z of start vertex to be considered secondary.
      Gaudi::Property<float> m_minREndPrimary{this, "minREndPrimary", 400.0*CLHEP::mm};//!< If track has end vertex, this is min R of end vertex to be considered primary.
      Gaudi::Property<float> m_minREndSecondary{this, "minREndSecondary", 1000.0*CLHEP::mm};//!< If track has end vertex, this is min R of end vertex to be considered secondary.
      Gaudi::Property<float> m_minZEndPrimary{this, "minZEndPrimary", 2300.0*CLHEP::mm};//!< If track has end vertex, this is min Z of end vertex to be considered primary.
      Gaudi::Property<float> m_minZEndSecondary{this, "minZEndSecondary", 3200.0*CLHEP::mm};//!< If track has end vertex, this is min Z of end vertex to be considered secondary.
      Gaudi::Property<bool> m_useTrackSelection{this, "UseTrackSelection", false}; //!< Use track selector tool
      Gaudi::Property<bool> m_doTruth{this, "DoTruth", true};  //!< Use truth information
      Gaudi::Property<float> m_minEtaFORWARD{this, "minEtaFORWARD", 2.5};
      Gaudi::Property<float> m_maxEtaFORWARD{this, "maxEtaFORWARD", 4.2};
      SG::ReadHandleKeyArray<TrackCollection>        m_RecTrackCollection_keys{this, "TrackCollectionKeys", {}};
      SG::ReadHandleKeyArray<TrackTruthCollection>   m_TrackTruthCollection_keys{this, "TrackTruthCollectionKeys", {}};

      mutable std::atomic<bool>      m_pullWarning    {}; //!< warn only once, if pull cannot be calculated
      mutable std::atomic<bool>      m_UpdatorWarning {}; //!< warn only once, if unbiased track states can not be calculated
      mutable std::atomic<int>       m_isUnbiased{0}  ;  //!< if can get unbiased residuals

      mutable std::atomic<long> m_events_processed{0};//!< number of events processed
         // statistics counters
      enum ECounter {
         kN_rec_tracks_without_perigee, //!< number of tracks w/o perigee
         kN_unknown_hits,               //!< number of hits without track
         kN_rec_tracks_processed,       //!< number of reconstructed tracks processed
         kN_gen_tracks_processed,       //!< number of generated tracks processed
         kN_spacepoints_processed,      //!< number of space points processed
         kNCounter
      };

      template <typename T_Int>
      struct Counter {
         Counter() { reset(); }
         void reset() {
            for (unsigned int i=0; i<kNCounter; ++i) { m_counter[i]=0; }
         }
         template <typename T_IntB>
         Counter &operator+=(const Counter<T_IntB> &b) {
            for (unsigned int i=0; i<kNCounter; ++i) { m_counter[i] += b.m_counter[i]; }
            return *this;
         }
         T_Int m_counter[kNCounter];
      };

      mutable Counter<std::atomic<long> > m_counter ATLAS_THREAD_SAFE;
      /** Calculate pull from residual, track and hit error.*/
      static float calculatePull(const float, const float, const float);   //vv
      /** Get Unbiased Track Parameters */
      const Trk::TrackParameters *  getUnbiasedTrackParameters(const Trk::TrackParameters* , const Trk::MeasurementBase* );
      Identifier  getIdentifier(const Trk::MeasurementBase* measurement );
    };

} // close of namespace

#endif // INDETRECSTATISTICS_TrackStatistics_H
