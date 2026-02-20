// -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSELECTIONTOOL_INDETTRACKSELECTIONTOOL_H
#define INDETTRACKSELECTIONTOOL_INDETTRACKSELECTIONTOOL_H

// Local include(s):
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
// Framework include(s):
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "AsgMessaging/AsgMessaging.h"
#ifndef XAOD_ANALYSIS
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#endif

#include <atomic>
#include <limits>
#include <map>
#include <mutex>
#include <unordered_map>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdint>

namespace InDetAccessor {

   class TrackParticleHelper;

#ifndef XAOD_ANALYSIS
   class TrkTrackHelper;
#endif
}

namespace InDet {

  // forward declaration of helper classes
  class TrackAccessor;
  class TrackCut;

   /// Implementation of the track selector tool
   ///
  class InDetTrackSelectionTool :
    public virtual IInDetTrackSelectionTool,
    public asg::AsgTool {
    
    friend class TrackCut;

    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS2( InDetTrackSelectionTool,
		     IAsgSelectionTool,
		     InDet::IInDetTrackSelectionTool )
    
  public:
    /// Constructor for standalone usage
    InDetTrackSelectionTool( const std::string& name );

    // The default destructor is OK but it must be defined in the
    // implementation file in order to forward declare with unique_ptr
    ~InDetTrackSelectionTool();

    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{
    
    /// Function initialising the tool
    virtual StatusCode initialize() override;
    /// Function finalizing the tool
    virtual StatusCode finalize() override;
    
    /// @}
    
    /// @name Function(s) implementing the IAsgSelectionTool interface
    /// @{
    
    /// Get an object describing the "selection steps" of the tool
    virtual const asg::AcceptInfo& getAcceptInfo() const override;
    
    /// Get the decision using a generic IParticle pointer
    virtual  asg::AcceptData accept( const xAOD::IParticle* ) const override;
    
    /// @}
    
    /// @name Function(s) implementing the IInDetTrackSelectionTool interface
    /// @{
    
    /// Get the decision for a specific track object
    virtual  asg::AcceptData accept( const xAOD::TrackParticle& track,
					 const xAOD::Vertex* vertex = nullptr ) const override;

#ifndef XAOD_ANALYSIS
    virtual  asg::AcceptData accept( const Trk::Track& track,
					 const Trk::Vertex* vertex = nullptr ) const override;
#endif
    
    /// @}

    virtual void setCutLevel( InDet::CutLevel level, bool overwrite = true ) override
      __attribute__ ((deprecated("For consistency with the athena interface, the cut level is best set through the \"CutLevel\" property.")));

  private:
    bool m_isInitialized = false; // flag whether or not the tool has been initialized, to check erroneous use cases.
    mutable std::atomic_bool m_warnInit = false; // flag to keep track of whether we have warned about a lack of initialization

    // this is the setCutLevel function that actually does the work, so that it doesn't warn if called in athena.
    void setCutLevelPrivate( InDet::CutLevel level, bool overwrite = true );

    // helper method to setup the cut functions for TrackParticles and Trk::Tracks
    template <int VERBOSE, class Trk_Helper>
    StatusCode setupCuts(std::map< std::string, std::vector< std::function<bool(Trk_Helper helper, const asg::AsgMessaging &msgHelper)> > > &trackCuts);

    template <class Trk_Helper>
    asg::AcceptData accept(Trk_Helper helper, const std::map< std::string, std::vector< std::function<bool(Trk_Helper helper, const asg::AsgMessaging &msgHelper)> > > &trackCuts) const;

    std::unordered_map< std::string, std::shared_ptr<TrackAccessor> > m_trackAccessors; //!< list of the accessors that need to be run for each track

    std::unique_ptr<asg::AsgMessaging> m_msgHelper;
    std::map< std::string, std::vector< std::function<bool(InDetAccessor::TrackParticleHelper helper, const asg::AsgMessaging &msgHelper)> > >
           m_trackParticleCuts; //!< First element is the name of the cut family, second element is the set of cuts
#ifndef XAOD_ANALYSIS
    std::map< std::string, std::vector< std::function<bool(InDetAccessor::TrkTrackHelper helper, const asg::AsgMessaging &msgHelper)> > >
           m_trkTrackCuts; //!< First element is the name of the cut family, second element is the set of cuts
#endif

    mutable std::atomic<uint64_t> m_numTracksProcessed = 0; //!< a counter of the number of tracks proccessed
    mutable std::atomic<uint64_t> m_numTracksPassed = 0; //!< a counter of the number of tracks that passed all cuts
    mutable std::vector<uint64_t> m_numTracksPassedCuts ATLAS_THREAD_SAFE; //!< tracks the number of tracks that passed each cut family. Guarded by m_mutex
    mutable std::mutex m_mutex;

    static inline bool maxDoubleIsSet(double cutValue) {return cutValue < InDet::InDetTrackSelectionTool::LOCAL_MAX_DOUBLE && cutValue >= 0.;}
    static inline bool maxIntIsSet(int cutValue){return cutValue < InDet::InDetTrackSelectionTool::LOCAL_MAX_INT && cutValue >= 0;}
    constexpr static double LOCAL_MAX_DOUBLE = 1.0e16;
    constexpr static int LOCAL_MAX_INT = std::numeric_limits<int>::max();

    Gaudi::Property<double> m_minPt{this, "minPt", -1., "Minimum transverse momentum"};
    Gaudi::Property<double> m_minP{this, "minP", -1., "Minimum momentum"};

    Gaudi::Property<double> m_maxAbsEta
      {this, "maxAbsEta", LOCAL_MAX_DOUBLE, "Maximum magnitude of pseudorapidity"};
    Gaudi::Property<double> m_maxZ0SinTheta
      {this, "maxZ0SinTheta", LOCAL_MAX_DOUBLE, "Maximum |z0|*sin(theta)"};
    Gaudi::Property<double> m_maxZ0
      {this, "maxZ0", LOCAL_MAX_DOUBLE, "Maximum longitudinal separation"};
    Gaudi::Property<double> m_maxD0
      {this, "maxD0", LOCAL_MAX_DOUBLE, "Maximum transvers separation"};
    Gaudi::Property<double> m_maxSigmaD0
      {this, "maxSigmaD0", LOCAL_MAX_DOUBLE, "Maximum error on d0"};
    Gaudi::Property<double> m_maxSigmaZ0
      {this, "maxSigmaZ0", LOCAL_MAX_DOUBLE, "Maximum error on z0"};
    Gaudi::Property<double> m_maxSigmaZ0SinTheta
      {this, "maxSigmaZ0SinTheta", LOCAL_MAX_DOUBLE, "Maximum error on z0*sin(theta)"};
    Gaudi::Property<double> m_maxD0overSigmaD0
      {this, "maxD0overSigmaD0", LOCAL_MAX_DOUBLE, "Significance cut on |d0|"};
    Gaudi::Property<double> m_maxZ0overSigmaZ0
      {this, "maxZ0overSigmaZ0", LOCAL_MAX_DOUBLE, "Significance cut on |z0|"};
    Gaudi::Property<double> m_maxZ0SinThetaoverSigmaZ0SinTheta
      {this, "maxZ0SinThetaoverSigmaZ0SinTheta", LOCAL_MAX_DOUBLE,
       "Significance cut on |z0*sin(theta)|"};

    Gaudi::Property<int> m_minNInnermostLayerHits
      {this, "minNInnermostLayerHits", -1,
       "Required hits on the innermost pixel layer"};
    Gaudi::Property<int> m_minNNextToInnermostLayerHits
      {this, "minNNextToInnermostLayerHits", -1,
       "Required hits on the next to innermost pixel layer"};
    Gaudi::Property<int> m_minNBothInnermostLayersHits
      {this, "minNBothInnermostLayersHits", -1,
       "Required hits on two innermost pixel layers"};
    Gaudi::Property<int> m_maxNInnermostLayerSharedHits
      {this, "maxNInnermostLayerSharedHits", LOCAL_MAX_INT,
       "Maximum shared hits in innermost pixel layer"};
    Gaudi::Property<int> m_useMinBiasInnermostLayersCut
      {this, "useMinBiasInnermostLayersCut", 0,
       "IBL hit if expected, otherwise next layer hit if expected"};
    Gaudi::Property<int> m_minNSiHits
      {this, "minNSiHits", -1, "Minimum silicon (pixel + SCT) hits"};
    Gaudi::Property<int> m_maxNSiSharedHits
      {this, "maxNSiSharedHits", LOCAL_MAX_INT,
       "Maximum silicon (pixel + SCT) sensors shared with other track"};
    Gaudi::Property<int> m_minNSiHitsIfSiSharedHits
      {this, "minNSiHitsIfSiSharedHits", -1,
       "Minimum number of silicon hits if there are any shared silicon hits"};
    Gaudi::Property<int> m_maxNSiHoles
      {this, "maxNSiHoles", LOCAL_MAX_INT, "Maximum silicon (pixel + SCT) holes"};
    Gaudi::Property<int> m_minNPixelHits
      {this, "minNPixelHits", -1, "Required pixel hits"};
    Gaudi::Property<int> m_maxNPixelSharedHits
      {this, "maxNPixelSharedHits", LOCAL_MAX_INT,
       "Maximum pixels shared with other tracks"};
    Gaudi::Property<int> m_maxNPixelHoles
      {this, "maxNPixelHoles", LOCAL_MAX_INT,
       "Maximum number of missed layers in pixel"};

    Gaudi::Property<double> m_minEtaForStrictNSiHitsCut
      {this, "minEtaForStrictNSiHitsCut", LOCAL_MAX_DOUBLE,
       "Eta cutoff for strict silicon hits cut"};
    Gaudi::Property<int> m_minNSiHitsAboveEtaCutoff
      {this, "minNSiHitsAboveEtaCutoff", -1,
       "Minimum silicon hits at large pseudorapidity"};
    Gaudi::Property<bool> m_maxOneSharedModule
      {this, "maxOneSharedModule", false,
       "Allow only 1 shared pixel hit or 2 shared SCT hits, not both"};
    Gaudi::Property<bool> m_useEtaDependentMaxChiSq
      {this, "useEtaDependentMaxChiSq", false,
       "Whether or not to use the eta-dependent chi squared per degree of freedom cut"};

    Gaudi::Property<int> m_minNSiHitsPhysical
      {this, "minNSiHitsPhysical", -1,
       "Minimum physical silicon hits (i.e. dead sensors do not count)"};
    Gaudi::Property<int> m_minNPixelHitsPhysical
      {this, "minNPixelHitsPhysical", -1, "Minimum physical pixel hits"};
    Gaudi::Property<int> m_minNSctHitsPhysical
      {this, "minNSctHitsPhysical", -1, "Minimum physical SCT hits"};
    Gaudi::Property<int> m_minNSctHits
      {this, "minNSctHits", -1, "Minimum SCT hits"};
    Gaudi::Property<int> m_maxNSctSharedHits
      {this, "maxNSctSharedHits", LOCAL_MAX_INT,
       "Maximum SCT hits shared with other track"};
    Gaudi::Property<int> m_maxNSctHoles
      {this, "maxNSctHoles", LOCAL_MAX_INT, "Maximum SCT holes"};
    Gaudi::Property<int> m_maxNSctDoubleHoles
      {this, "maxNSctDoubleHoles", LOCAL_MAX_INT, "Maximum SCT double holes"};

    Gaudi::Property<double> m_maxTrtEtaAcceptance
      {this, "maxTrtEtaAcceptance", LOCAL_MAX_DOUBLE,
       "Maximum eta that ignores TRT hit cuts"};
    Gaudi::Property<double> m_maxEtaForTrtHitCuts
      {this, "maxEtaForTrtHitCuts", -1.,
       "Eta above which TRT hit cuts are not applied."};
    Gaudi::Property<int> m_minNTrtHits{this, "minNTrtHits", -1, "Minimum TRT hits"};
    Gaudi::Property<int> m_minNTrtHitsPlusOutliers
      {this, "minNTrtHitsPlusOutliers", -1, "Minimum TRT hits including outliers"};
    Gaudi::Property<int> m_minNTrtHighThresholdHits
      {this, "minNTrtHighThresholdHits", -1, "Minimum high E TRT hits"};
    Gaudi::Property<int> m_minNTrtHighThresholdHitsPlusOutliers
      {this, "minNTrtHighThresholdHitsPlusOutliers", -1,
       "Minimum high E TRT hits including outliers"};
    Gaudi::Property<double> m_maxTrtHighEFraction
      {this, "maxTrtHighEFraction", LOCAL_MAX_DOUBLE,
       "Maximum TRT hits that are above high energy threshold"};
    Gaudi::Property<double> m_maxTrtHighEFractionWithOutliers
      {this, "maxTrtHighEFractionWithOutliers", LOCAL_MAX_DOUBLE,
       "Maximum TRT hits that are above high energy threshold including outliers"};
    Gaudi::Property<double> m_maxTrtOutlierFraction
      {this, "maxTrtOutlierFraction", LOCAL_MAX_DOUBLE,
       "Maximum fraction of TRT outliers over TRT hits plus outliers"};

    Gaudi::Property<double> m_maxChiSq
      {this, "maxChiSq", LOCAL_MAX_DOUBLE, "Maximum chi squared"};
    Gaudi::Property<double> m_maxChiSqperNdf
      {this, "maxChiSqperNdf", LOCAL_MAX_DOUBLE,
       "Maximum chi squared per degree of freedom"};
    Gaudi::Property<double> m_minProb{this, "minProb", -1., "Minimum p(chi^2, Ndof)"};
    Gaudi::Property<double> m_minPtForProbCut
      {this, "minPtForProbCut", LOCAL_MAX_DOUBLE,
       "Minimum pt for chi-sq probability cut"};
    Gaudi::Property<double> m_minProbAbovePtCutoff
      {this, "minProbAbovePtCutoff", -1.,
       "Minimum chi-sq probability above a pt cutoff"};

    Gaudi::Property<int> m_minNUsedHitsdEdx
      {this, "minNUsedHitsdEdx", -1, "Minimum hits used for dEdx"};
    Gaudi::Property<int> m_minNOverflowHitsdEdx
      {this, "minNOverflowHitsdEdx", -1, "Minimum overflow hits in IBL for dEdx"};
    Gaudi::Property<bool> m_eProbHTonlyForXe
      {this, "eProbHTonlyForXe", false,
       "Flag whether to apply the eProbabilityHT cut only when all TRT hits are Xenon"};
    Gaudi::Property<double> m_minEProbabilityHT
      {this, "minEProbabilityHT", -1.,
       "Minimum High Threshold electron probability"};

    Gaudi::Property<bool> m_useExperimentalInnermostLayersCut
      {this, "useExperimentalInnermostLayersCut", false,
       "Use the experimental cut on pixel holes"};

#ifndef XAOD_ANALYSIS
    Gaudi::Property<int> m_minNSiHitsMod
      {this, "minNSiHitsMod", -1,
       "Minimum number of Si hits, with pixel hits counting twice"};
    Gaudi::Property<int> m_minNSiHitsModTop
      {this, "minNSiHitsModTop", -1,
       "Min number of Si hits on top half (pixel counting twice)"};
    Gaudi::Property<int> m_minNSiHitsModBottom
      {this, "minNSiHitsModBottom", -1,
       "Min number of Si hits on bottom half (pixel counting twice)"};
#endif

    Gaudi::Property<std::vector<double>> m_vecEtaCutoffsForSiHitsCut
      {this, "vecEtaCutoffsForSiHitsCut", {},
       "Minimum eta cutoffs for each Silicon hit cut"};
    Gaudi::Property<std::vector<int>> m_vecMinNSiHitsAboveEta
      {this, "vecMinNSiHitsAboveEta", {},
       "Minimum Silicon hits above each eta cutoff"};
    Gaudi::Property<std::vector<double>> m_vecEtaCutoffsForPtCut
      {this, "vecEtaCutoffsForPtCut", {}, "Minimum eta cutoffs for each pT cut"};
    Gaudi::Property<std::vector<double>> m_vecMinPtAboveEta
      {this, "vecMinPtAboveEta", {},
       "Minimum transverse momentum above each eta cutoff"};

    Gaudi::Property<std::vector<double>> m_vecPtCutoffsForSctHitsCut
      {this, "vecPtCutoffsForSctHitsCut", {},
       "Minimum pt cutoffs for each SCT hits"};
    Gaudi::Property<std::vector<int>> m_vecMinNSctHitsAbovePt
      {this, "vecMinNSctHitsAbovePt", {},
       "Minimum SCT hits above each pt cutoff"};

    Gaudi::Property<std::vector<double>> m_vecEtaCutoffsForZ0SinThetaCut
      {this, "vecEtaCutoffsForZ0SinThetaCut", {},
       "Minimum eta cutoffs for each Z0SinTheta value"};
    Gaudi::Property<std::vector<double>> m_vecPtCutoffsForZ0SinThetaCut
      {this, "vecPtCutoffsForZ0SinThetaCut", {},
       "Minimum pt cutoffs for each Z0SinTheta value"};
    Gaudi::Property<std::vector<std::vector<double>>> m_vecvecMaxZ0SinThetaAboveEtaPt
      {this, "vecvecMaxZ0SinThetaAboveEtaPt", {},
       "Maximum Z0SinTheta value above each eta and pT cutoff"};

    Gaudi::Property<std::vector<double>> m_vecEtaCutoffsForD0Cut
      {this, "vecEtaCutoffsForD0Cut", {}, "Minimum eta cutoffs for each D0 value"};
    Gaudi::Property<std::vector<double>> m_vecPtCutoffsForD0Cut
      {this, "vecPtCutoffsForD0Cut", {}, "Minimum pt cutoffs for each D0 value"};
    Gaudi::Property<std::vector<std::vector<double>>> m_vecvecMaxD0AboveEtaPt
      {this, "vecvecMaxD0AboveEtaPt", {},
       "Maximum D0 value above each eta and pT cutoff"};

    Gaudi::Property<std::vector<double>> m_vecEtaCutoffsForSctHolesCut
      {this, "vecEtaCutoffsForSctHolesCut", {},
       "Minimum eta cutoffs for each SctHoles value"};
    Gaudi::Property<std::vector<double>> m_vecPtCutoffsForSctHolesCut
      {this, "vecPtCutoffsForSctHolesCut", {},
       "Minimum pt cutoffs for each SctHoles value"};
    Gaudi::Property<std::vector<std::vector<double>>> m_vecvecMaxSctHolesAboveEtaPt
      {this, "vecvecMaxSctHolesAboveEtaPt", {},
       "Maximum SctHoles value above each eta and pT cutoff"};

    Gaudi::Property<std::vector<double>> m_vecEtaCutoffsForSctHitsPlusDeadCut
      {this, "vecEtaCutoffsForSctHitsPlusDeadCut", {},
       "Minimum eta cutoffs for each SctHitsPlusDead value"};
    Gaudi::Property<std::vector<double>> m_vecPtCutoffsForSctHitsPlusDeadCut
      {this, "vecPtCutoffsForSctHitsPlusDeadCut", {},
       "Minimum pt cutoffs for each SctHitsPlusDead value"};
    Gaudi::Property<std::vector<std::vector<double>>> m_vecvecMinSctHitsPlusDeadAboveEtaPt
      {this, "vecvecMinSctHitsPlusDeadAboveEtaPt", {},
       "Minimum SctHitsPlusDead value above each eta and pT cutoff"};
    
    /// Object used to store the last decision
    asg::AcceptInfo m_acceptInfo; //!< Object that stores detailed selection information

    // to set to a pre-defined cut level in Athena, we need to save the cut level
    // as a string so we can do a soft set in initialize()
    Gaudi::Property<std::string> m_cutLevel{this, "CutLevel", ""}; //!< The string version of the cut level so that it can be set via jobOptions

    // we need a map from strings (for use in Athena) to the CutLevel enum
    static const std::unordered_map<std::string, CutLevel> s_mapCutLevel;

#ifndef XAOD_ANALYSIS
    Gaudi::Property<bool> m_initTrkTools{this, "UseTrkTrackTools", false, "Whether to initialize the Trk::Track tools"};
    bool m_trackSumToolAvailable = false; //!< Whether the summary tool is available
    ToolHandle<Trk::ITrackSummaryTool> m_trackSumTool
      {this, "TrackSummaryTool", "Trk::TrackSummaryTool/TrackSummaryTool"};
    ToolHandle<Trk::IExtrapolator> m_extrapolator
      {this, "Extrapolator", "Trk::Extrapolator/Extrapolator"};

#endif // XAOD_ANALYSIS

  }; // class InDetTrackSelectionTool

} // namespace InDet

#endif // INDETTRACKSELECTIONTOOL_INDETTRACKSELECTIONTOOL_H
