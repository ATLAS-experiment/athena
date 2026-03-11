/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_STABLEDELTARMATCHINGTOOL_H
#define INDETTRACKPERFMON_STABLEDELTARMATCHINGTOOL_H

/**
 * @file   StableDeltaRMatchingTool.h
 * @author Harry Simpson <harry.simpson@cern.ch>
 * @date   30 March 2024
 * @brief  Derived tool to perform Delta R matching of tracks based on the stable marriage algorithm
 */

/// Athena include(s)
#include "AsgTools/AsgTool.h"

/// Local include(s)
#include "InDetTrackPerfMon/StableMatchingBase.h"
#include "InDetTrackPerfMon/ITrackMatchingTool.h"

namespace IDTPM {

  template<typename T, typename R>
    class StableDeltaRMatchingTool : public StableMatchingBase<T,R> {
  public:
    /// Constructor
    StableDeltaRMatchingTool(const std::string& name)
      : StableMatchingBase<T,R>(name) {}    

    /// Initialize
    virtual StatusCode initialize() override {
      ATH_MSG_DEBUG("Initializing " << this->name());
      ATH_CHECK(asg::AsgTool::initialize());

      if (m_dRmax < 0) {
        ATH_MSG_ERROR("No DeltaRMax criteria requested");
        return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
    }

  protected:

    virtual float distance(const T& t, const R& r) const override {
      float dR = deltaR(t, r);
      // Return -1 if distance exceeds threshold
      return (dR > m_dRmax) ? -1 : dR;
    }

    FloatProperty m_dRmax { this, "dRmax", 0.05, "Maximum DeltaR cone size for DeltaR-matching (disabled if <0)" };
  };

  /// --------------------------------------------
  /// ------ Track -> Track Stable Matching ------
  /// --------------------------------------------
  class StableDeltaRMatchingTool_trk :
    public StableDeltaRMatchingTool<xAOD::TrackParticle, xAOD::TrackParticle>,
    public virtual ITrackMatchingTool
  {
  public:
    ASG_TOOL_CLASS(StableDeltaRMatchingTool_trk, ITrackMatchingTool);

    /// Constructor
    StableDeltaRMatchingTool_trk(const std::string& name)
      : StableDeltaRMatchingTool<xAOD::TrackParticle, xAOD::TrackParticle>(name) {}

    /// General matching method, via TrackAnalysisCollections    
    virtual StatusCode match(TrackAnalysisCollections& trkAnaColls,
                             const std::string& chainRoIName,
                             const std::string& roiStr) const override;

    /// Specific matching methods, via test/reference vectors

    /// --------------------------------------------
    /// ------ Track -> Track Stable Matching ------
    /// --------------------------------------------

    virtual StatusCode match(const std::vector<const xAOD::TrackParticle*>& vTest,
                             const std::vector<const xAOD::TrackParticle*>& vRef,
                             ITrackMatchingLookup& matches) const override {
      ATH_MSG_DEBUG("Doing Track->Track stable DeltaR matching");
      ATH_CHECK(matchVectors(vTest, vRef, matches));
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching (disabled)
    virtual StatusCode match(const std::vector<const xAOD::TrackParticle*>& /*vTest*/,
			     const std::vector<const xAOD::TruthParticle*>& /*vRef*/,
			     ITrackMatchingLookup& /*matches*/) const override {
      ATH_MSG_WARNING("Track->Truth matching not supported by this tool.");
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching (disabled)
    virtual StatusCode match(const std::vector<const xAOD::TruthParticle*>& /*vTest*/,
			     const std::vector<const xAOD::TrackParticle*>& /*vRef*/,
			     ITrackMatchingLookup& /*matches*/) const override {
      ATH_MSG_WARNING("Truth->Track matching not supported by this tool.");
      return StatusCode::SUCCESS;
    }
  }; 

  /// --------------------------------------------
  /// ------ Track -> Truth Stable Matching ------
  /// --------------------------------------------
  class StableDeltaRMatchingTool_trkTruth :
    public StableDeltaRMatchingTool<xAOD::TrackParticle, xAOD::TruthParticle>,
    public virtual ITrackMatchingTool
  {
  public:
    ASG_TOOL_CLASS(StableDeltaRMatchingTool_trkTruth, ITrackMatchingTool);

    /// Constructor
    StableDeltaRMatchingTool_trkTruth(const std::string& name)
      : StableDeltaRMatchingTool<xAOD::TrackParticle, xAOD::TruthParticle>(name) {}

    /// General matching method, via TrackAnalysisCollections    
    virtual StatusCode match(TrackAnalysisCollections& trkAnaColls,
                             const std::string& chainRoIName,
                             const std::string& roiStr) const override;

    /// Specific matching methods, via test/reference vectors

    /// track -> track matching (disabled)   
    virtual StatusCode match(const std::vector<const xAOD::TrackParticle*>& /*vTest*/,
                             const std::vector<const xAOD::TrackParticle*>& /*vRef*/,
                             ITrackMatchingLookup& /*matches*/) const override {
      ATH_MSG_WARNING("Track->Track matching not supported by this tool.");
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching 
    virtual StatusCode match(const std::vector<const xAOD::TrackParticle*>& vTest,
			     const std::vector<const xAOD::TruthParticle*>& vRef,
			     ITrackMatchingLookup& matches) const override {
      ATH_MSG_DEBUG("Doing Track->Truth stable DeltaR matching.");
      ATH_CHECK(matchVectors(vTest, vRef, matches));
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching (disabled)
    virtual StatusCode match(const std::vector<const xAOD::TruthParticle*>& /*vTest*/,
			     const std::vector<const xAOD::TrackParticle*>& /*vRef*/,
			     ITrackMatchingLookup& /*matches*/) const override {
      ATH_MSG_WARNING("Truth->Track matching not supported by this tool.");
      return StatusCode::SUCCESS;
    }
  };

  /// --------------------------------------------
  /// ------ Truth -> Track Stable Matching ------
  /// --------------------------------------------
  class StableDeltaRMatchingTool_truthTrk :
    public StableDeltaRMatchingTool<xAOD::TruthParticle, xAOD::TrackParticle>,
    public virtual ITrackMatchingTool
  {
  public:
    ASG_TOOL_CLASS(StableDeltaRMatchingTool_truthTrk, ITrackMatchingTool);

    /// Constructor
    StableDeltaRMatchingTool_truthTrk(const std::string& name)
      : StableDeltaRMatchingTool<xAOD::TruthParticle, xAOD::TrackParticle>(name) {}

    /// General matching method, via TrackAnalysisCollections    
    virtual StatusCode match(TrackAnalysisCollections& trkAnaColls,
                             const std::string& chainRoIName,
                             const std::string& roiStr) const override;

    /// Specific matching methods, via test/reference vectors

    /// track -> track matching (disabled)   
    virtual StatusCode match(const std::vector<const xAOD::TrackParticle*>& /*vTest*/,
                             const std::vector<const xAOD::TrackParticle*>& /*vRef*/,
                             ITrackMatchingLookup& /*matches*/) const override {
      ATH_MSG_WARNING("Track->Track matching not supported by this tool.");
      return StatusCode::SUCCESS;
    }

    /// track -> truth matching (disabled)
    virtual StatusCode match(const std::vector<const xAOD::TrackParticle*>& /*vTest*/,
			     const std::vector<const xAOD::TruthParticle*>& /*vRef*/,
			     ITrackMatchingLookup& /*matches*/) const override {
      ATH_MSG_WARNING("Track->Truth matching not supported by this tool.");
      return StatusCode::SUCCESS;
    }

    /// truth -> track matching 
    virtual StatusCode match(const std::vector<const xAOD::TruthParticle*>& vTest,
			     const std::vector<const xAOD::TrackParticle*>& vRef,
			     ITrackMatchingLookup& matches) const override {
      ATH_MSG_DEBUG("Doing Truth->Track stable DeltaR matching.");
      ATH_CHECK(matchVectors(vTest, vRef, matches));
      return StatusCode::SUCCESS;
    }
  };

} // end of namespace IDTPM

#endif // > !INDETTRACKPERFMON_STABLEDELTARMATCHINGTOOL_H

