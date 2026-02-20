/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

#include "AsgMessaging/Check.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "InDetAccessor.h"

#ifndef XAOD_ANALYSIS
#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkTrack/Track.h"
#include "VxVertex/Vertex.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkEventPrimitives/FitQuality.h"
#endif

#include <memory>
#include <array>

using namespace InDetAccessor;

namespace {
   template <unsigned int n_summary_types>
   std::array<xAOD::SummaryType, n_summary_types> summaryArray( std::array<xAOD::SummaryType, n_summary_types> summary_types) {return summary_types; }

   template <class Trk_Helper, unsigned int n_summary_types>
   class  MinTRTHitsCut {
   public:
      MinTRTHitsCut( double maxTrtEtaAcceptance,
                     double maxEtaForTrtHitCuts,
                     int min_n_hits,
                     std::array<xAOD::SummaryType, n_summary_types> summary_types)
         : m_maxTrtEtaAcceptance(maxTrtEtaAcceptance),
           m_maxEtaForTrtHitCuts(maxEtaForTrtHitCuts),
           m_minNHits(min_n_hits),
           m_summaryTypes(summary_types)
      {}

      uint8_t nHits(Trk_Helper helper, const asg::AsgMessaging &msgHelper) const {
         return getSummarySum(helper,msgHelper,m_summaryTypes);
      }
      bool operator()(Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
         double absEta = std::abs( helper.eta( msgHelper) );
         return      (absEta <= m_maxTrtEtaAcceptance || absEta > m_maxEtaForTrtHitCuts) || nHits(helper,msgHelper) >=m_minNHits;
      }
   private:
      double m_maxTrtEtaAcceptance;
      double m_maxEtaForTrtHitCuts;
      int m_minNHits;
      std::array<xAOD::SummaryType, n_summary_types> m_summaryTypes;
   };

   // return the first bin whose value is lower or equal than the given value
   // the bins have to be in ascending order;
   template <typename T>
   unsigned int findBin(const std::vector<T> &bins, T value) {
      if (bins.empty() || bins[0]>value) return bins.size();
      unsigned int bin_i=bins.size();
      while ( value < bins[--bin_i]) {};
      return bin_i;
   }
   // return true if the given bins are in ascending order
   template <typename T>
   bool checkOrder(const std::vector<T> &bins) {
      if (bins.empty()) return true;
      for (unsigned int bin_i=1; bin_i<bins.size(); ++bin_i) {
         if (bins[bin_i-1]>=bins[bin_i]) return false;
      }
      return true;
   }
}

InDet::InDetTrackSelectionTool::InDetTrackSelectionTool(const std::string& name)
  : asg::AsgTool(name)
  , m_acceptInfo( "InDetTrackSelection" )
#ifndef XAOD_ANALYSIS
#endif // XAOD_ANALYSIS
{

  // set the cut selection: default is "no cut"
  setCutLevelPrivate(InDet::CutLevel::NoCut);

#ifndef XAOD_STANDALONE
  declareInterface<IInDetTrackSelectionTool>(this);
#endif

}

// we must define the destructor in order to use forward-declaration with unique_ptr
InDet::InDetTrackSelectionTool::~InDetTrackSelectionTool() = default;

/// Initializes the selection tool, setting up the cut objects as well as any additional tools it needs.
/// 
/// In athena, this is called automatically after the user has set the properties and so
/// it must set the cut level without overwriting any additional cuts that have been set.
/// 
/// In standalone root, the cut levels can be set in a more or less intuitive way. After setting
/// the cut levels and providing any additional configuration, this initialize() function must
/// be called explicitly.
/// 
StatusCode InDet::InDetTrackSelectionTool::initialize() {
  // Make sure we haven't already been initialized - this would be a sign of improper usage.
  if (m_isInitialized) {
    ATH_MSG_ERROR( "Tool has already been initialized. This is illegitimate." );
    ATH_MSG_ERROR( "This call to initialize() will do nothing." );
    return StatusCode::SUCCESS;
  }

  // Greet the user:
  ATH_CHECK( asg::AsgTool::initialize() );

  // if the CutLevel string is set to something recognizable,
  // then do a soft set on the cuts (i.e. not overwriting those already set)
  if (!m_cutLevel.empty()) {
    std::unordered_map<std::string, InDet::CutLevel>::const_iterator it_mapCutLevel = s_mapCutLevel.find(m_cutLevel);
    if ( it_mapCutLevel == s_mapCutLevel.end() ) {
      ATH_MSG_ERROR( "The string \"" << m_cutLevel << "\" is not recognized as a cut level. No cuts will be changed." );
      ATH_MSG_ERROR( "Cut level options are:" );
      for (const auto& opt : s_mapCutLevel) {
	ATH_MSG_ERROR( "\t" << opt.first );
      }
    } else {
      ATH_MSG_DEBUG( "Cut level set to \"" << it_mapCutLevel->first << "\"." );
      ATH_MSG_DEBUG( "This will not overwrite other cuts that have been set.");
      setCutLevelPrivate( it_mapCutLevel->second, false );
    }
  }

#ifndef XAOD_ANALYSIS
  if (m_initTrkTools) {
    m_trackSumToolAvailable = false;
    if (!m_trackSumTool.empty()) {
      ATH_CHECK( m_trackSumTool.retrieve() );
      ATH_MSG_DEBUG( "Track summary tool retrieved." );
      m_trackSumToolAvailable = true;
    }
    ATH_CHECK( m_extrapolator.retrieve() );
    ATH_MSG_DEBUG( "Retrieved tool " << m_extrapolator );
  } else {
    m_extrapolator.disable();
    m_trackSumTool.disable();
  }
#endif // XAOD_ANALYSIS

  // Need messaging helper for cut functions
  m_msgHelper = std::make_unique<asg::AsgMessaging>(this) ;


#ifndef XAOD_ANALYSIS
  // setup cut functions for Trk::Tracks
  ATH_CHECK( setupCuts<0>(m_trkTrackCuts) );
  ATH_CHECK( setupCuts<0>(m_trackParticleCuts) );
#else
  ATH_CHECK( setupCuts<1>(m_trackParticleCuts) );
#endif
  // setup cut functions for xAOD::TrackParticles
  for (const auto& cutFamily : m_trackParticleCuts) {
    const std::string& cutFamilyName = cutFamily.first;
    // lock(m_mutex) is not needed because this is inside of non-const initialize method.
    m_numTracksPassedCuts.push_back(0);
    if (m_acceptInfo.addCut( cutFamilyName, "Selection of tracks according to " + cutFamilyName ) < 0) {
      ATH_MSG_ERROR( "Failed to add cut family " << cutFamilyName << " because the TAccept object is full." );
      return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Adding cut family " << cutFamilyName);
  }

  m_isInitialized = true;

  return StatusCode::SUCCESS;
}

namespace {
  template <typename T>
  inline T sqr(T a) { return a *a;}
}
template <int VERBOSE, class Trk_Helper>
StatusCode InDet::InDetTrackSelectionTool::setupCuts(std::map< std::string, std::vector< std::function<bool(Trk_Helper helper, const asg::AsgMessaging &msgHelper)> > > &trackCuts) {
   // track parameter cuts
  if (m_minPt > 0.) {
     if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum Pt: " << m_minPt << " MeV" );
     trackCuts["Pt"].push_back( [minPt = m_minPt](Trk_Helper helper, const asg::AsgMessaging &msgHelper) -> bool { return helper.pt(msgHelper) >= minPt; } );
  }
  if (maxDoubleIsSet(m_maxAbsEta)) {
     if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum |Eta|: " << m_maxAbsEta );
     trackCuts["Eta"].push_back( [maxAbsEta = m_maxAbsEta](Trk_Helper helper, const asg::AsgMessaging &msgHelper) { return std::abs(helper.eta(msgHelper)) <= maxAbsEta; } );
  }
  if (m_minP > 0.) {
     if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum P: " << m_minP << " MeV" );
     trackCuts["P"].push_back( [maxInvP = 1./m_minP](Trk_Helper helper, const asg::AsgMessaging &msgHelper) { return std::abs(helper.qOverP(msgHelper)) <= maxInvP; } );
  }
  if (maxDoubleIsSet(m_maxD0)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum d0: " << m_maxD0 << " mm" );
    trackCuts["D0"].push_back( [maxD0 = m_maxD0](Trk_Helper helper, const asg::AsgMessaging &msgHelper) { return std::abs(helper.d0(msgHelper)) <= maxD0; } );
  }
  if (maxDoubleIsSet(m_maxZ0)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum z0: " << m_maxZ0 << " mm");
    trackCuts["Z0"].push_back( [maxZ0 = m_maxZ0](Trk_Helper helper, const asg::AsgMessaging &msgHelper) { return std::abs(helper.z0(msgHelper)) <= maxZ0; } );
  }
  if (maxDoubleIsSet(m_maxZ0SinTheta)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum z0*sin(theta): " << m_maxZ0SinTheta << " mm" );
    trackCuts["Z0SinTheta"].push_back([maxZ0SinTheta = m_maxZ0SinTheta](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return std::abs( helper.z0(msgHelper) * std::sin(helper.theta(msgHelper))) <= maxZ0SinTheta;
    });
  }
  if (maxDoubleIsSet(m_maxSigmaD0)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum uncertainty on d0: " << m_maxSigmaD0 << " mm" );
    trackCuts["D0"].push_back([maxSigmaD0Squared = sqr(m_maxSigmaD0.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getDefiningParametersCov(helper,msgHelper, InDetAccessor::d0,InDetAccessor::d0) <= maxSigmaD0Squared;
    });
  }
  if (maxDoubleIsSet(m_maxSigmaZ0)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum uncertainty on z0: " << m_maxSigmaZ0 << " mm" );
    trackCuts["Z0"].push_back([maxSigmaZ0Squared = sqr(m_maxSigmaZ0.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getDefiningParametersCov(helper,msgHelper, InDetAccessor::z0,InDetAccessor::z0) <= maxSigmaZ0Squared;
    });
  }
  if (maxDoubleIsSet(m_maxSigmaZ0SinTheta)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum uncertainty on z0*sin(theta): "
                                 << m_maxSigmaZ0SinTheta << " mm" );
    trackCuts["Z0SinTheta"].push_back([maxSigmaZ0SinThetaSquared = sqr(m_maxSigmaZ0SinTheta.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double theta =    helper.theta(msgHelper);
       double sinTheta = std::sin(theta);
       double cosTheta = std::cos(theta);
       double z0 =       helper.z0(msgHelper);

       return (  sqr(z0)*sqr(cosTheta)     * getDefiningParametersCov(helper,msgHelper, InDetAccessor::theta,InDetAccessor::theta)
               + 2*z0   *sinTheta*cosTheta * getDefiningParametersCov(helper,msgHelper, InDetAccessor::theta,InDetAccessor::z0)
               + sqr(sinTheta)             * getDefiningParametersCov(helper,msgHelper, InDetAccessor::z0,   InDetAccessor::z0) ) <= maxSigmaZ0SinThetaSquared;
    });
  }
  if (maxDoubleIsSet(m_maxD0overSigmaD0)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum significance on d0: " << m_maxD0overSigmaD0 );
    trackCuts["D0"].push_back([maxD0overSigmaD0Squared = sqr(m_maxD0overSigmaD0.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return sqr(helper.d0(msgHelper)) <= maxD0overSigmaD0Squared * getDefiningParametersCov(helper,msgHelper, InDetAccessor::d0,InDetAccessor::d0);
    });
  }
  if (maxDoubleIsSet(m_maxZ0overSigmaZ0)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum significance on z0: " << m_maxZ0overSigmaZ0 );
    trackCuts["Z0"].push_back([maxZ0overSigmaZ0Squared = sqr(m_maxZ0overSigmaZ0.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) -> bool {
       return sqr(helper.z0(msgHelper)) <= maxZ0overSigmaZ0Squared * getDefiningParametersCov(helper,msgHelper, InDetAccessor::z0,InDetAccessor::z0);
    });
  }
  if (maxDoubleIsSet(m_maxZ0SinThetaoverSigmaZ0SinTheta)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum significance on z0*sin(theta): "
                                 << m_maxZ0SinThetaoverSigmaZ0SinTheta );
    trackCuts["Z0SinTheta"].push_back([maxZ0SinThetaoverSigmaZ0SinThetaSquared = sqr(m_maxZ0SinThetaoverSigmaZ0SinTheta.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {

       double theta =    helper.theta(msgHelper);
       double sinTheta = std::sin(theta);
       double cosTheta = std::cos(theta);
       double z0 =       helper.z0(msgHelper);

       return sqr(z0*sinTheta) <=
          maxZ0SinThetaoverSigmaZ0SinThetaSquared * (  sqr(z0)*sqr(cosTheta)        * getDefiningParametersCov(helper,msgHelper, InDetAccessor::theta,InDetAccessor::theta)
                                                     + 2*z0   *sinTheta*cosTheta    * getDefiningParametersCov(helper,msgHelper, InDetAccessor::theta,InDetAccessor::z0)
                                                     + sqr(sinTheta)                * getDefiningParametersCov(helper,msgHelper, InDetAccessor::z0,   InDetAccessor::z0));
    });
  }

  // hit cuts
  if (m_minNInnermostLayerHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum hits from innermost pixel layer: " << m_minNInnermostLayerHits );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "    (Track will pass if no hit is expected.)" );
    trackCuts["InnermostLayersHits"].push_back([minNInnermostLayerHits = m_minNInnermostLayerHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return (   getSummary(helper, msgHelper, xAOD::numberOfInnermostPixelLayerHits) >= minNInnermostLayerHits
               || getSummary(helper, msgHelper, xAOD::expectInnermostPixelLayerHit)    == 0);
    });
  }
  if (m_minNNextToInnermostLayerHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum hits from next to innermost pixel layer: " << m_minNNextToInnermostLayerHits );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "    (Track will pass if no hit is expected.)" );
    trackCuts["InnermostLayersHits"].push_back([minNNextToInnermostLayerHits = m_minNNextToInnermostLayerHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return (   getSummary(helper, msgHelper, xAOD::numberOfNextToInnermostPixelLayerHits) >= minNNextToInnermostLayerHits
               || getSummary(helper, msgHelper, xAOD::expectNextToInnermostPixelLayerHit)    == 0);
    });
  }
  if (m_minNBothInnermostLayersHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum hits from both innermost pixel layers: " << m_minNBothInnermostLayersHits );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "    (If a layer has no hits but one is not expected, the" );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "     number of hits in that layer will be taken to be 1.)" );
    if (m_minNBothInnermostLayersHits > 2) {
      ATH_MSG_WARNING( "A value of minNBothInnermostLayersHits above 2 does not make sense." );
      ATH_MSG_WARNING( "  Use 1 for \"or\" or 2 for \"and\"." );
    }
    trackCuts["InnermostLayersHits"].push_back([minNBothInnermostLayersHits = m_minNBothInnermostLayersHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return (   std::max(  getSummary(helper, msgHelper, xAOD::numberOfInnermostPixelLayerHits),
                             static_cast<uint8_t>( !getSummary(helper, msgHelper, xAOD::expectInnermostPixelLayerHit) ))
                 +std::max(  getSummary(helper, msgHelper, xAOD::numberOfNextToInnermostPixelLayerHits),
                             static_cast<uint8_t>( !getSummary(helper, msgHelper, xAOD::expectNextToInnermostPixelLayerHit )))
                  >= minNBothInnermostLayersHits);
    });
  }
  if (m_useMinBiasInnermostLayersCut > 0 ) { // less than zero indicates this cut is manually turned off
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  An innermost layer hit is required if expected, otherwise" );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "    a next-to-innermost layer hit is required if it is expected." );
    trackCuts["InnermostLayersHits"].push_back([](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       uint8_t expected_innermost_pixel_layer_hit = getSummary(helper, msgHelper, xAOD::expectInnermostPixelLayerHit);
       return    (expected_innermost_pixel_layer_hit >  0  && getSummary(helper, msgHelper, xAOD::numberOfInnermostPixelLayerHits)>=1)
              || (expected_innermost_pixel_layer_hit == 0  && (   getSummary(helper, msgHelper, xAOD::expectNextToInnermostPixelLayerHit)==0
                                                               || getSummary(helper, msgHelper, xAOD::numberOfNextToInnermostPixelLayerHits)>=1));
    });
  }
  if (maxIntIsSet(m_maxNInnermostLayerSharedHits)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum shared hits in innermost pixel layer: " << m_maxNInnermostLayerSharedHits );
    trackCuts["InnermostLayersHits"].push_back([maxNInnermostLayerSharedHits = m_maxNInnermostLayerSharedHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getSummary(helper, msgHelper, xAOD::numberOfInnermostPixelLayerSharedHits) <= maxNInnermostLayerSharedHits;
    });
  }
  if (m_minNPixelHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum pixel hits: " << m_minNPixelHits );
    trackCuts["PixelHits"].push_back( [minNPixelHits = m_minNPixelHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummarySum<2,Trk_Helper>(helper, msgHelper, {xAOD::numberOfPixelHits,xAOD::numberOfPixelDeadSensors}) >= minNPixelHits;
    });
  }
  if (m_minNPixelHitsPhysical > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum physical pixel hits (i.e. dead sensors do not count): "
                                 << m_minNPixelHitsPhysical );
    trackCuts["PixelHits"].push_back([minNPixelHitsPhysical = m_minNPixelHitsPhysical](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfPixelHits) >= minNPixelHitsPhysical;
    });
  }
  if (maxIntIsSet(m_maxNPixelHoles)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum pixel holes: " << m_maxNPixelHoles );
    trackCuts["PixelHits"].push_back([maxNPixelHoles = m_maxNPixelHoles](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfPixelHoles) <= maxNPixelHoles;
    });
  }
  if (maxIntIsSet(m_maxNPixelSharedHits)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum pixel shared hits: " << m_maxNPixelSharedHits );
    trackCuts["PixelHits"].push_back([maxNPixelSharedHits = m_maxNPixelSharedHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfPixelSharedHits) <= maxNPixelSharedHits;
    });
  }
  if (m_minNSctHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum SCT hits: " << m_minNSctHits );
    trackCuts["SctHits"].push_back( [minNSctHits = m_minNSctHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummarySum<2,Trk_Helper>(helper, msgHelper, {xAOD::numberOfSCTHits, xAOD::numberOfSCTDeadSensors}) >= minNSctHits;
    });
  }
  if (m_minNSctHitsPhysical > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum physical SCT hits (i.e. dead sensors do not count): "
                                 << m_minNSctHitsPhysical );
    trackCuts["SctHits"].push_back([minNSctHitsPhysical = m_minNSctHitsPhysical](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfSCTHits) >= minNSctHitsPhysical;
    });
  }
  if (maxIntIsSet(m_maxNSctHoles)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum SCT holes: " << m_maxNSctHoles );
    trackCuts["SctHits"].push_back([maxNSctHoles = m_maxNSctHoles](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfSCTHoles) <= maxNSctHoles;
    });
  }
  if (maxIntIsSet(m_maxNSctSharedHits)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum SCT shared hits: " << m_maxNSctSharedHits );
    trackCuts["SctHits"].push_back([maxNSctSharedHits = m_maxNSctSharedHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfSCTSharedHits) <= maxNSctSharedHits;
    });
  }
  if (maxIntIsSet(m_maxNSctDoubleHoles)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum SCT double holes: " << m_maxNSctDoubleHoles );
    trackCuts["SctHits"].push_back([maxNSctDoubleHoles = m_maxNSctDoubleHoles](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper, xAOD::numberOfSCTDoubleHoles) <= maxNSctDoubleHoles ;
    });
  }
  if (m_minNSiHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum silicon (pixel + SCT) hits: " << m_minNSiHits );
    trackCuts["SiHits"].push_back([minNSiHits = m_minNSiHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getSummarySum<4,Trk_Helper>(helper, msgHelper, {xAOD::numberOfPixelHits,
                                                              xAOD::numberOfSCTHits,
                                                              xAOD::numberOfPixelDeadSensors,
                                                              xAOD::numberOfSCTDeadSensors} )    >= minNSiHits ;
    });
  }
  if (m_minNSiHitsPhysical > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum physical silicon hits (i.e. dead sensors do not count): "
                                 << m_minNSiHitsPhysical );
    trackCuts["SiHits"].push_back([minNSiHitsPhysical = m_minNSiHitsPhysical](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelHits, xAOD::numberOfSCTHits} )   >= minNSiHitsPhysical ;
    });
  }
  if (maxIntIsSet(m_maxNSiHoles)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum silicon holes: " << m_maxNSiHoles );
    trackCuts["SiHits"].push_back([maxNSiHoles = m_maxNSiHoles](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelHoles, xAOD::numberOfSCTHoles} )   <= maxNSiHoles ;
    });
  }
  if (maxIntIsSet(m_maxNSiSharedHits)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum silicon shared hits: " << m_maxNSiSharedHits );
    trackCuts["SiHits"].push_back([maxNSiSharedHits = m_maxNSiSharedHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelSharedHits, xAOD::numberOfSCTSharedHits} )   <= maxNSiSharedHits ;
    });
  }
  if (m_maxOneSharedModule) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  No more than one shared module:" );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "    i.e. max 1 shared pixel hit or" );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "    2 shared SCT hits, and not both." );
    trackCuts["SiHits"].push_back([](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummary(helper, msgHelper,xAOD::numberOfPixelSharedHits )
              + getSummary(helper, msgHelper,xAOD::numberOfSCTSharedHits )/2  <= 1  ;
    });
  }
  if (m_minNSiHitsIfSiSharedHits > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum silicon hits if the track has shared hits: "
                                 << m_minNSiHitsIfSiSharedHits );
    trackCuts["SiHits"].push_back([minNSiHitsIfSiSharedHits = m_minNSiHitsIfSiSharedHits](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return    getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelSharedHits, xAOD::numberOfSCTSharedHits} ) == 0
              || getSummarySum<4,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelHits,
                                                                xAOD::numberOfSCTHits,
                                                                xAOD::numberOfPixelDeadSensors,
                                                                xAOD::numberOfSCTDeadSensors}) >=minNSiHitsIfSiSharedHits;
    });
  }
  if (maxDoubleIsSet(m_minEtaForStrictNSiHitsCut)
      && m_minNSiHitsAboveEtaCutoff > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Require " << m_minNSiHitsAboveEtaCutoff
                                 << " silicon hits above eta = " << m_minEtaForStrictNSiHitsCut );
    trackCuts["SiHits"].push_back([minEtaForStrictNSiHitsCut = m_minEtaForStrictNSiHitsCut,
                                   minNSiHitsAboveEtaCutoff  = m_minNSiHitsAboveEtaCutoff](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return    std::abs(helper.eta(msgHelper)) <= minEtaForStrictNSiHitsCut
              || getSummarySum<4,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelHits,
                                                                xAOD::numberOfSCTHits,
                                                                xAOD::numberOfPixelDeadSensors,
                                                                xAOD::numberOfSCTDeadSensors})   >= minNSiHitsAboveEtaCutoff;
    });
  }
  if (m_useExperimentalInnermostLayersCut) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Zero pixel holes allowed, except one pix hole is allowed if there is a physical IBL hit and a BLayer hit is expected but there is no BLayer hit." );
    trackCuts["PixHits"].push_back([](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       uint8_t pixel_holes = getSummary(helper, msgHelper,xAOD::numberOfPixelHoles);
       return  pixel_holes == 0 || ( pixel_holes<=1 && getSummary(helper, msgHelper,xAOD::numberOfInnermostPixelLayerHits)       >= 1
                                                    && getSummary(helper, msgHelper,xAOD::expectNextToInnermostPixelLayerHit)    > 0
                                                    && getSummary(helper, msgHelper,xAOD::numberOfNextToInnermostPixelLayerHits) == 0 );
    });
  }
#ifndef XAOD_ANALYSIS
  if (m_minNSiHitsMod > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum modified Si hits (2*pix + sct) (does not include dead sensors)= "
                                 << m_minNSiHitsMod );
    trackCuts["SiHits"].push_back([minNSiHitsMod = m_minNSiHitsMod](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return   getSummarySum<3,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelHits,
                                                               xAOD::numberOfPixelHits, // pixel hits count twice in this definition
                                                               xAOD::numberOfSCTHits})   >= minNSiHitsMod;
    });
  }
  if (m_minNSiHitsModTop > 0 || m_minNSiHitsModBottom > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum modified Si hits in top half = "
                                 << m_minNSiHitsModTop );
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum modified Si hits in bottom half = "
                                 << m_minNSiHitsModBottom );
    trackCuts["SiHits"].push_back([minNSiHitsModTop    = m_minNSiHitsModTop,
                                   minNSiHitsModBottom = m_minNSiHitsModBottom](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       auto [top,bottom] = getSiHitsTopBottom(helper, msgHelper);
       return top  >= minNSiHitsModTop && bottom >= minNSiHitsModBottom;
    });
  }
#endif
  if (m_maxEtaForTrtHitCuts > 0. && m_maxTrtEtaAcceptance < m_maxEtaForTrtHitCuts) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  -- TRT hit cuts applied above eta = " << m_maxTrtEtaAcceptance
		  << " and below eta = " << m_maxEtaForTrtHitCuts << " --" );
    if (m_minNTrtHits > 0) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Minimum TRT hits outside eta acceptance: " << m_minNTrtHits );
      trackCuts["TrtHits"].push_back( MinTRTHitsCut<Trk_Helper,1>( m_maxTrtEtaAcceptance, m_maxEtaForTrtHitCuts, m_minNTrtHits,
                                                                   {xAOD::numberOfTRTHits} ));
    }
    if (m_minNTrtHitsPlusOutliers > 0) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Minimum TRT hits outside eta acceptance including outliers: " << m_minNTrtHitsPlusOutliers );
      trackCuts["TrtHits"].push_back( MinTRTHitsCut<Trk_Helper,2>( m_maxTrtEtaAcceptance,m_maxEtaForTrtHitCuts, m_minNTrtHitsPlusOutliers,
                                                                   {xAOD::numberOfTRTHits, xAOD::numberOfTRTOutliers} ));
    }
    if (m_minNTrtHighThresholdHits > 0) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Minimum TRT hits outside eta acceptance above high energy threshold: "
		    << m_minNTrtHighThresholdHits );
      trackCuts["TrtHits"].push_back( MinTRTHitsCut<Trk_Helper,1>( m_maxTrtEtaAcceptance,m_maxEtaForTrtHitCuts, m_minNTrtHighThresholdHits,
                                                                   {xAOD::numberOfTRTHighThresholdHits} ));
    }
    if (m_minNTrtHighThresholdHitsPlusOutliers > 0) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Minimum TRT hits outside eta acceptance above high energy threshold including outliers: "
		    << m_minNTrtHighThresholdHitsPlusOutliers );
      trackCuts["TrtHits"].push_back( MinTRTHitsCut<Trk_Helper,2>( m_maxTrtEtaAcceptance,m_maxEtaForTrtHitCuts, m_minNTrtHighThresholdHitsPlusOutliers,
                                                                   {xAOD::numberOfTRTHighThresholdHits, xAOD::numberOfTRTHighThresholdOutliers} ));
    }
    if (maxDoubleIsSet(m_maxTrtHighEFraction)) { // I think this condition could be instead that it is between 0 and 1
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Maximum ratio of high threshold to regular TRT hits outside eta acceptance: "
		    << m_maxTrtHighEFraction);
      trackCuts["TrtHits"].push_back([maxTrtEtaAcceptance = m_maxTrtEtaAcceptance,
                                      maxEtaForTrtHitCuts = m_maxEtaForTrtHitCuts,
                                      maxTrtHighEFraction = m_maxTrtHighEFraction](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
         double absEta = std::abs( helper.eta( msgHelper) );
         return      (absEta <= maxTrtEtaAcceptance || absEta > maxEtaForTrtHitCuts)
                ||                            getSummary(helper, msgHelper,xAOD::numberOfTRTHighThresholdHits )
                     <= maxTrtHighEFraction * getSummary(helper, msgHelper,xAOD::numberOfTRTHits );
      });
    }
    if (maxDoubleIsSet(m_maxTrtHighEFractionWithOutliers)) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Maximum ratio of high threshold to regular TRT hits above eta acceptance including outliers: "
		    << m_maxTrtHighEFractionWithOutliers);
      trackCuts["TrtHits"].push_back([maxTrtEtaAcceptance             = m_maxTrtEtaAcceptance,
                                      maxEtaForTrtHitCuts             = m_maxEtaForTrtHitCuts,
                                      maxTrtHighEFractionWithOutliers = m_maxTrtHighEFractionWithOutliers](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
         double absEta = std::abs( helper.eta( msgHelper) );
         return      (absEta <= maxTrtEtaAcceptance || absEta > maxEtaForTrtHitCuts)
            ||                                      (  getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfTRTHighThresholdHits, xAOD::numberOfTRTHighThresholdOutliers} ))
               <= maxTrtHighEFractionWithOutliers * (  getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfTRTHits,              xAOD::numberOfTRTOutliers} ));
      });
    }
    if (m_maxTrtOutlierFraction < 1. && m_maxTrtOutlierFraction >= 0.) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    Maximum fraction of TRT hits that are outliers: " << m_maxTrtOutlierFraction );
      trackCuts["TrtHits"].push_back([maxTrtEtaAcceptance   = m_maxTrtEtaAcceptance,
                                      maxEtaForTrtHitCuts   = m_maxEtaForTrtHitCuts,
                                      maxTrtOutlierFraction = m_maxTrtOutlierFraction](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
         double absEta = std::abs( helper.eta( msgHelper) );
         uint8_t trt_outliers = getSummary(helper, msgHelper,xAOD::numberOfTRTOutliers );
         return     ( absEta <= maxTrtEtaAcceptance || absEta > maxEtaForTrtHitCuts)
                 || trt_outliers <= maxTrtOutlierFraction  * (  trt_outliers + getSummary(helper, msgHelper,xAOD::numberOfTRTHits) );
      });
    }
  }

  // fit quality cuts
  if (m_useEtaDependentMaxChiSq) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Using pre-defined eta-dependent maximum chi squared (no longer recommended)." );
    trackCuts["FitQuality"].push_back([](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double eta = helper.eta(msgHelper);
       double fit_chi_square = getFitChiSquare(helper,msgHelper);
       double fit_ndof = getFitNDoF(helper,msgHelper);
       if (std::abs( eta) < 1.9) {
          return fit_chi_square <= fit_ndof * ( 4.4 + 0.32*sqr(eta) );
       }
       else {
          return fit_chi_square <= fit_ndof * ( 26.9 - 19.6978*std::abs(eta) + 4.4534*sqr(eta) );
       }
    });
  }
  if (maxDoubleIsSet(m_maxChiSq)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum chi squared: " << m_maxChiSq );
    trackCuts["FitQuality"].push_back([maxChiSq = m_maxChiSq](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getFitChiSquare(helper,msgHelper) <= maxChiSq;
    });
  }
  if (maxDoubleIsSet(m_maxChiSqperNdf)) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Maximum chi squared per degree of freedom: " << m_maxChiSqperNdf );
    trackCuts["FitQuality"].push_back([maxChiSqperNdf = m_maxChiSqperNdf](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getFitChiSquare(helper,msgHelper)  <= maxChiSqperNdf * getFitNDoF(helper,msgHelper);
    });
  }
  if (m_minProb > 0.) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum chi squared probability: " << m_minProb );
    trackCuts["FitQuality"].push_back([minProb = m_minProb](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return TMath::Prob( getFitChiSquare(helper,msgHelper),getFitNDoF(helper,msgHelper))  >= minProb;
    });
  }
  if (maxDoubleIsSet(m_minPtForProbCut) && m_minProbAbovePtCutoff > 0.) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum chi-sq probability of " << m_minProbAbovePtCutoff
                                 << " above pt of " << m_minPtForProbCut*1e-3 << " GeV." );
    trackCuts["FitQuality"].push_back([minPtForProbCut      = m_minPtForProbCut,
                                       minProbAbovePtCutoff = m_minProbAbovePtCutoff](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return    helper.pt(msgHelper) <= minPtForProbCut
              || TMath::Prob( getFitChiSquare(helper,msgHelper),getFitNDoF(helper,msgHelper))  >= minProbAbovePtCutoff;
    });
  }

  // dE/dx cuts
  if (m_minNUsedHitsdEdx > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum used hits for dEdx: " << m_minNUsedHitsdEdx );
    trackCuts["dEdxHits"].push_back([minNUsedHitsdEdx = m_minNUsedHitsdEdx](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getNumberOfUsedHitsdEdx(helper,msgHelper) >= minNUsedHitsdEdx;
    });
  }
  if (m_minNOverflowHitsdEdx > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum IBL overflow hits for dEdx: " << m_minNOverflowHitsdEdx );
    trackCuts["dEdxHits"].push_back([minNOverflowHitsdEdx = m_minNOverflowHitsdEdx](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       return getNumberOfIBLOverflowsdEdx(helper,msgHelper) >= minNOverflowHitsdEdx;
    });
  }
  if (m_minEProbabilityHT > 0) {
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  Minimum high threshold electron probability: " << m_minEProbabilityHT );
    if (m_eProbHTonlyForXe) {
      if constexpr(VERBOSE>0) ATH_MSG_INFO( "    (only applied on tracks where all TRT hits are Xenon)" );
      trackCuts["eProbHT"].push_back([minEProbabilityHT = m_minEProbabilityHT](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
         return     getSummarySum<2,Trk_Helper>(helper, msgHelper, {xAOD::numberOfTRTHits, xAOD::numberOfTRTOutliers})
                  > getSummary(helper, msgHelper, xAOD::numberOfTRTXenonHits)
               || getEProbabilityHT(helper,msgHelper) >= minEProbabilityHT;
      });
    } else {
       trackCuts["eProbHT"].push_back([minEProbabilityHT = m_minEProbabilityHT](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
          return  getEProbabilityHT(helper,msgHelper) >= minEProbabilityHT;
       });
    }
  }

  if (!m_vecEtaCutoffsForSiHitsCut.empty() || !m_vecMinNSiHitsAboveEta.empty()) {
    auto cutSize = m_vecEtaCutoffsForSiHitsCut.size();
    if (cutSize != m_vecMinNSiHitsAboveEta.size()) {
       ATH_MSG_ERROR( "Eta cutoffs and Silicon hit cuts must be vectors of the same length." );
       return StatusCode::FAILURE;
    }
    if constexpr(VERBOSE>0) {
       for (size_t i_cut=0; i_cut<cutSize-1; ++i_cut) {
          ATH_MSG_INFO( "  for " << m_vecEtaCutoffsForSiHitsCut[i_cut]
                        << " < eta < " << m_vecEtaCutoffsForSiHitsCut[i_cut+1]
                        << " ,Silicon hits  >= " << m_vecMinNSiHitsAboveEta[i_cut] );
       }
    }
    if (!checkOrder(m_vecEtaCutoffsForSiHitsCut.value())) {
       ATH_MSG_ERROR( "Eta values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  for eta > " << m_vecEtaCutoffsForSiHitsCut[cutSize-1]
					  << " ,Silicon hits >= " << m_vecMinNSiHitsAboveEta[cutSize-1] );

    trackCuts["SiHits"].push_back([p_vecEtaCutoffsForSiHitsCut = &std::as_const(m_vecEtaCutoffsForSiHitsCut.value()),
                                   p_vecMinNSiHitsAboveEta     = &std::as_const(m_vecMinNSiHitsAboveEta.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double abs_eta = std::abs(helper.eta(msgHelper));
       unsigned int bin_i = findBin(*p_vecEtaCutoffsForSiHitsCut, abs_eta);
       return    bin_i >= p_vecMinNSiHitsAboveEta->size()
              || abs_eta>5.0
              || getSummarySum<4,Trk_Helper>(helper, msgHelper,{xAOD::numberOfPixelHits,
                                                                xAOD::numberOfSCTHits,
                                                                xAOD::numberOfPixelDeadSensors,
                                                                xAOD::numberOfSCTDeadSensors})   >= (*p_vecMinNSiHitsAboveEta)[bin_i];
    });
  }

  if (!m_vecEtaCutoffsForPtCut.empty() || !m_vecMinPtAboveEta.empty()) {
    auto cutSize = m_vecEtaCutoffsForPtCut.size();
    if (cutSize != m_vecMinPtAboveEta.size()) {
       ATH_MSG_ERROR( "Eta cutoffs and pT cuts must be vectors of the same length." );
      return StatusCode::FAILURE;
    }
    if constexpr(VERBOSE>0) {
       for (size_t i_cut=0; i_cut<cutSize-1; ++i_cut) {
          ATH_MSG_INFO( "  for " << m_vecEtaCutoffsForPtCut[i_cut]
                        << " < eta < " << m_vecEtaCutoffsForPtCut[i_cut+1]
                        << " ,transverse momentum >= " << m_vecMinPtAboveEta[i_cut] );
       }
    }
    if (!checkOrder(m_vecEtaCutoffsForPtCut.value())) {
       ATH_MSG_ERROR( "Eta values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  for eta > " << m_vecEtaCutoffsForPtCut[cutSize-1]
                               << " ,transverse momentum >= " << m_vecMinPtAboveEta[cutSize-1] );
    trackCuts["Pt"].push_back([p_vecEtaCutoffsForPtCut = &std::as_const(m_vecEtaCutoffsForPtCut.value()),
                               p_vecMinPtAboveEta = &std::as_const(m_vecMinPtAboveEta.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double abs_eta = std::abs(helper.eta(msgHelper));
       unsigned int bin_i = findBin(*p_vecEtaCutoffsForPtCut, abs_eta);
       return  bin_i >= p_vecMinPtAboveEta->size() || abs_eta > 5.0 || helper.pt(msgHelper)  >= (*p_vecMinPtAboveEta)[bin_i];
    });
  }

  if (!m_vecPtCutoffsForSctHitsCut.empty() || !m_vecMinNSctHitsAbovePt.empty()) {
    auto cutSize = m_vecPtCutoffsForSctHitsCut.size();
    if (cutSize != m_vecMinNSctHitsAbovePt.size()) {
       ATH_MSG_ERROR( "Pt cutoffs and SCT hit cuts must be vectors of the same length." );
       return StatusCode::FAILURE;
    }
    if constexpr(VERBOSE>0) {
       for (size_t i_cut=0; i_cut<cutSize-1; ++i_cut) {
          ATH_MSG_INFO( "  for " << m_vecPtCutoffsForSctHitsCut[i_cut]
                        << " < pt < " << m_vecPtCutoffsForSctHitsCut[i_cut+1]
                        << " MeV,\tSCT hits >= " << m_vecMinNSctHitsAbovePt[i_cut] );
       }
    }
    if (!checkOrder(m_vecPtCutoffsForSctHitsCut.value())) {
       ATH_MSG_ERROR( "Pt values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if constexpr(VERBOSE>0) ATH_MSG_INFO( "  for pt > " << m_vecPtCutoffsForSctHitsCut[cutSize-1]
                               << " MeV,\t\tSCT hits >= " << m_vecMinNSctHitsAbovePt[cutSize-1] );
    trackCuts["SctHits"].push_back([p_vecPtCutoffsForSctHitsCut = &std::as_const(m_vecPtCutoffsForSctHitsCut.value()),
                                    p_vecMinNSctHitsAbovePt     = &std::as_const(m_vecMinNSctHitsAbovePt.value())](Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double pt = helper.pt(msgHelper);
       unsigned int bin_i = findBin(*p_vecPtCutoffsForSctHitsCut, pt);
       return     bin_i >= p_vecPtCutoffsForSctHitsCut->size()
               || getSummarySum<2,Trk_Helper>(helper, msgHelper,{xAOD::numberOfSCTHits,
                                                                 xAOD::numberOfSCTDeadSensors}) >= (*p_vecMinNSctHitsAbovePt)[bin_i];
    });
  }

  if (!m_vecPtCutoffsForZ0SinThetaCut.empty()  ||
      !m_vecEtaCutoffsForZ0SinThetaCut.empty() ||
      !m_vecvecMaxZ0SinThetaAboveEtaPt.empty()) {
    auto etaSize = m_vecEtaCutoffsForZ0SinThetaCut.size();
    auto ptSize  = m_vecPtCutoffsForZ0SinThetaCut.size();
    if (etaSize != m_vecvecMaxZ0SinThetaAboveEtaPt.size()) {
      ATH_MSG_ERROR( "Eta cutoffs and Z0SinTheta cuts must be vectors of the same length." );
      return StatusCode::FAILURE;
    }
    for (size_t i_size=0; i_size<etaSize-1; ++i_size) {
      if (ptSize != m_vecvecMaxZ0SinThetaAboveEtaPt[i_size].size()) {
        ATH_MSG_ERROR( "Pt cutoffs and Z0SinTheta cuts must be vectors of the same length." );
        return StatusCode::FAILURE;
      }
    }

    std::stringstream pTRangeBuffer;
    std::copy(m_vecPtCutoffsForZ0SinThetaCut.begin(), m_vecPtCutoffsForZ0SinThetaCut.end(), std::ostream_iterator<double>(pTRangeBuffer, ", "));
    std::string pTString=pTRangeBuffer.str();
    if constexpr(VERBOSE>0) ATH_MSG_INFO("Z0SinTheta cuts (<=) for pT above "<<pTString.substr(0, pTString.size()-2)<<"MeV, respectively:");
    for (size_t i_cut_eta=0; i_cut_eta<etaSize; ++i_cut_eta) 
    {
      std::stringstream etaRangeBuffer;
      etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForZ0SinThetaCut[i_cut_eta] << " < |#eta| < ";
      if(i_cut_eta!=etaSize-1) etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForZ0SinThetaCut[i_cut_eta+1];
      else                     etaRangeBuffer << std::setprecision(2) << std::fixed <<m_maxAbsEta;

      std::stringstream cutBuffer;
      std::copy(m_vecvecMaxZ0SinThetaAboveEtaPt[i_cut_eta].begin(), m_vecvecMaxZ0SinThetaAboveEtaPt[i_cut_eta].end(), std::ostream_iterator<double>(cutBuffer, ", "));
      std::string cutString=cutBuffer.str();

      if constexpr(VERBOSE>0) ATH_MSG_INFO(" for "<<etaRangeBuffer.str()<<": "<<cutString.substr(0, cutString.size()-2));
    }

    if (!checkOrder(m_vecEtaCutoffsForZ0SinThetaCut.value())) {
       ATH_MSG_ERROR( "Eta values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if (!checkOrder(m_vecPtCutoffsForZ0SinThetaCut.value())) {
       ATH_MSG_ERROR( "Pt values not in ascending order." );
       return StatusCode::FAILURE;
    }

    trackCuts["Z0SinTheta"].push_back([p_vecEtaCutoffsForZ0SinThetaCut = &std::as_const(m_vecEtaCutoffsForZ0SinThetaCut.value()),
                                       p_vecPtCutoffsForZ0SinThetaCut = &std::as_const(m_vecPtCutoffsForZ0SinThetaCut.value()),
                                       p_vecvecMaxZ0SinThetaAboveEtaPt = &std::as_const(m_vecvecMaxZ0SinThetaAboveEtaPt.value())] (Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double eta = helper.eta(msgHelper);
       unsigned int bin_eta = findBin(*p_vecEtaCutoffsForZ0SinThetaCut, std::fabs(eta));
       double pt = helper.pt(msgHelper);
       unsigned int bin_pt = findBin(*p_vecPtCutoffsForZ0SinThetaCut, pt);
       return     bin_eta >= p_vecEtaCutoffsForZ0SinThetaCut->size()
               || bin_pt >= p_vecPtCutoffsForZ0SinThetaCut->size()
               || std::fabs(helper.z0(msgHelper) * std::sin(helper.theta(msgHelper))) <= (*p_vecvecMaxZ0SinThetaAboveEtaPt)[bin_eta][bin_pt];
    });
  }

  if (!m_vecPtCutoffsForD0Cut.empty()  ||
      !m_vecEtaCutoffsForD0Cut.empty() ||
      !m_vecvecMaxD0AboveEtaPt.empty()) {
    auto etaSize = m_vecEtaCutoffsForD0Cut.size();
    auto ptSize  = m_vecPtCutoffsForD0Cut.size();
    if (etaSize != m_vecvecMaxD0AboveEtaPt.size()) {
      ATH_MSG_ERROR( "Eta cutoffs and D0 cuts must be vectors of the same length." );
      return StatusCode::FAILURE;
    }
    for (size_t i_size=0; i_size<etaSize-1; ++i_size) {
      if (ptSize != m_vecvecMaxD0AboveEtaPt[i_size].size()) {
        ATH_MSG_ERROR( "Pt cutoffs and D0 cuts must be vectors of the same length." );
        return StatusCode::FAILURE;
      }
    }

    std::stringstream pTRangeBuffer;
    std::copy(m_vecPtCutoffsForD0Cut.begin(), m_vecPtCutoffsForD0Cut.end(), std::ostream_iterator<double>(pTRangeBuffer, ", "));
    std::string pTString=pTRangeBuffer.str();
    if constexpr(VERBOSE>0) ATH_MSG_INFO("D0 cuts (<=) for pT above "<<pTString.substr(0, pTString.size()-2)<<"MeV, respectively:");
    for (size_t i_cut_eta=0; i_cut_eta<etaSize; ++i_cut_eta) 
    {
      std::stringstream etaRangeBuffer;
      etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForD0Cut[i_cut_eta] << " < |#eta| < ";
      if(i_cut_eta!=etaSize-1) etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForD0Cut[i_cut_eta+1];
      else                     etaRangeBuffer << std::setprecision(2) << std::fixed <<m_maxAbsEta;

      std::stringstream cutBuffer;
      std::copy(m_vecvecMaxD0AboveEtaPt[i_cut_eta].begin(), m_vecvecMaxD0AboveEtaPt[i_cut_eta].end(), std::ostream_iterator<double>(cutBuffer, ", "));
      std::string cutString=cutBuffer.str();

      if constexpr(VERBOSE>0) ATH_MSG_INFO(" for "<<etaRangeBuffer.str()<<": "<<cutString.substr(0, cutString.size()-2));
    }

    if (!checkOrder(m_vecEtaCutoffsForD0Cut.value())) {
       ATH_MSG_ERROR( "Eta values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if (!checkOrder(m_vecPtCutoffsForD0Cut.value())) {
       ATH_MSG_ERROR( "Pt values not in ascending order." );
       return StatusCode::FAILURE;
    }

    trackCuts["D0"].push_back([p_vecEtaCutoffsForD0Cut = &std::as_const(m_vecEtaCutoffsForD0Cut.value()),
                               p_vecPtCutoffsForD0Cut = &std::as_const(m_vecPtCutoffsForD0Cut.value()),
                               p_vecvecMaxD0AboveEtaPt = &std::as_const(m_vecvecMaxD0AboveEtaPt.value())] (Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double eta = helper.eta(msgHelper);
       unsigned int bin_eta = findBin(*p_vecEtaCutoffsForD0Cut, std::fabs(eta));
       double pt = helper.pt(msgHelper);
       unsigned int bin_pt = findBin(*p_vecPtCutoffsForD0Cut, pt);
       return     bin_eta >= p_vecEtaCutoffsForD0Cut->size()
               || bin_pt >= p_vecPtCutoffsForD0Cut->size()
               || std::fabs(helper.d0(msgHelper)) <= (*p_vecvecMaxD0AboveEtaPt)[bin_eta][bin_pt];
    });
  }

  if (!m_vecPtCutoffsForSctHolesCut.empty()  ||
      !m_vecEtaCutoffsForSctHolesCut.empty() ||
      !m_vecvecMaxSctHolesAboveEtaPt.empty()) {
    auto etaSize = m_vecEtaCutoffsForSctHolesCut.size();
    auto ptSize  = m_vecPtCutoffsForSctHolesCut.size();
    if (etaSize != m_vecvecMaxSctHolesAboveEtaPt.size()) {
      ATH_MSG_ERROR( "Eta cutoffs and SctHoles cuts must be vectors of the same length." );
      return StatusCode::FAILURE;
    }
    for (size_t i_size=0; i_size<etaSize-1; ++i_size) {
      if (ptSize != m_vecvecMaxSctHolesAboveEtaPt[i_size].size()) {
        ATH_MSG_ERROR( "Pt cutoffs and SctHoles cuts must be vectors of the same length." );
        return StatusCode::FAILURE;
      }
    }

    std::stringstream pTRangeBuffer;
    std::copy(m_vecPtCutoffsForSctHolesCut.begin(), m_vecPtCutoffsForSctHolesCut.end(), std::ostream_iterator<double>(pTRangeBuffer, ", "));
    std::string pTString=pTRangeBuffer.str();
    if constexpr(VERBOSE>0) ATH_MSG_INFO("SctHoles cuts (<=) for pT above "<<pTString.substr(0, pTString.size()-2)<<"MeV, respectively:");
    for (size_t i_cut_eta=0; i_cut_eta<etaSize; ++i_cut_eta) 
    {
      std::stringstream etaRangeBuffer;
      etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForSctHolesCut[i_cut_eta] << " < |#eta| < ";
      if(i_cut_eta!=etaSize-1) etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForSctHolesCut[i_cut_eta+1];
      else                     etaRangeBuffer << std::setprecision(2) << std::fixed <<m_maxAbsEta;

      std::stringstream cutBuffer;
      std::copy(m_vecvecMaxSctHolesAboveEtaPt[i_cut_eta].begin(), m_vecvecMaxSctHolesAboveEtaPt[i_cut_eta].end(), std::ostream_iterator<double>(cutBuffer, ", "));
      std::string cutString=cutBuffer.str();

      if constexpr(VERBOSE>0) ATH_MSG_INFO(" for "<<etaRangeBuffer.str()<<": "<<cutString.substr(0, cutString.size()-2));
    }

    if (!checkOrder(m_vecEtaCutoffsForSctHolesCut.value())) {
       ATH_MSG_ERROR( "Eta values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if (!checkOrder(m_vecPtCutoffsForSctHolesCut.value())) {
       ATH_MSG_ERROR( "Pt values not in ascending order." );
       return StatusCode::FAILURE;
    }

    trackCuts["SctHits"].push_back([p_vecEtaCutoffsForSctHolesCut = &std::as_const(m_vecEtaCutoffsForSctHolesCut.value()),
                                    p_vecPtCutoffsForSctHolesCut = &std::as_const(m_vecPtCutoffsForSctHolesCut.value()),
                                    p_vecvecMaxSctHolesAboveEtaPt = &std::as_const(m_vecvecMaxSctHolesAboveEtaPt.value())] (Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double eta = helper.eta(msgHelper);
       unsigned int bin_eta = findBin(*p_vecEtaCutoffsForSctHolesCut, std::fabs(eta));
       double pt = helper.pt(msgHelper);
       unsigned int bin_pt = findBin(*p_vecPtCutoffsForSctHolesCut, pt);
       return     bin_eta >= p_vecEtaCutoffsForSctHolesCut->size()
               || bin_pt >= p_vecPtCutoffsForSctHolesCut->size()
               || getSummary(helper, msgHelper, xAOD::numberOfSCTHoles) <= (*p_vecvecMaxSctHolesAboveEtaPt)[bin_eta][bin_pt];
      });
  }

  if (!m_vecPtCutoffsForSctHitsPlusDeadCut.empty()  ||
      !m_vecEtaCutoffsForSctHitsPlusDeadCut.empty() ||
      !m_vecvecMinSctHitsPlusDeadAboveEtaPt.empty()) {
    auto etaSize = m_vecEtaCutoffsForSctHitsPlusDeadCut.size();
    auto ptSize  = m_vecPtCutoffsForSctHitsPlusDeadCut.size();
    if (etaSize != m_vecvecMinSctHitsPlusDeadAboveEtaPt.size()) {
      ATH_MSG_ERROR( "Eta cutoffs and SctHitsPlusDead cuts must be vectors of the same length." );
      return StatusCode::FAILURE;
    }
    for (size_t i_size=0; i_size<etaSize-1; ++i_size) {
      if (ptSize != m_vecvecMinSctHitsPlusDeadAboveEtaPt[i_size].size()) {
        ATH_MSG_ERROR( "Pt cutoffs and SctHitsPlusDead cuts must be vectors of the same length." );
        return StatusCode::FAILURE;
      }
    }

    std::stringstream pTRangeBuffer;
    std::copy(m_vecPtCutoffsForSctHitsPlusDeadCut.begin(), m_vecPtCutoffsForSctHitsPlusDeadCut.end(), std::ostream_iterator<double>(pTRangeBuffer, ", "));
    std::string pTString=pTRangeBuffer.str();
    if constexpr(VERBOSE>0) ATH_MSG_INFO("SctHitsPlusDead cuts (>=) for pT above "<<pTString.substr(0, pTString.size()-2)<<"MeV, respectively:");
    for (size_t i_cut_eta=0; i_cut_eta<etaSize; ++i_cut_eta) 
    {
      std::stringstream etaRangeBuffer;
      etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForSctHitsPlusDeadCut[i_cut_eta] << " < |#eta| < ";
      if(i_cut_eta!=etaSize-1) etaRangeBuffer << std::setprecision(2) << std::fixed << m_vecEtaCutoffsForSctHitsPlusDeadCut[i_cut_eta+1];
      else                     etaRangeBuffer << std::setprecision(2) << std::fixed <<m_maxAbsEta;

      std::stringstream cutBuffer;
      std::copy(m_vecvecMinSctHitsPlusDeadAboveEtaPt[i_cut_eta].begin(), m_vecvecMinSctHitsPlusDeadAboveEtaPt[i_cut_eta].end(), std::ostream_iterator<double>(cutBuffer, ", "));
      std::string cutString=cutBuffer.str();

      if constexpr(VERBOSE>0) ATH_MSG_INFO(" for "<<etaRangeBuffer.str()<<": "<<cutString.substr(0, cutString.size()-2));
    }

    if (!checkOrder(m_vecEtaCutoffsForSctHitsPlusDeadCut.value())) {
       ATH_MSG_ERROR( "Eta values not in ascending order." );
       return StatusCode::FAILURE;
    }
    if (!checkOrder(m_vecPtCutoffsForSctHitsPlusDeadCut.value())) {
       ATH_MSG_ERROR( "Pt values not in ascending order." );
       return StatusCode::FAILURE;
    }

    trackCuts["SctHits"].push_back([p_vecEtaCutoffsForSctHitsPlusDeadCut = &std::as_const(m_vecEtaCutoffsForSctHitsPlusDeadCut.value()),
                                    p_vecPtCutoffsForSctHitsPlusDeadCut = &std::as_const(m_vecPtCutoffsForSctHitsPlusDeadCut.value()),
                                    p_vecvecMinSctHitsPlusDeadAboveEtaPt = &std::as_const(m_vecvecMinSctHitsPlusDeadAboveEtaPt.value())] (Trk_Helper helper, const asg::AsgMessaging &msgHelper) {
       double eta = helper.eta(msgHelper);
       unsigned int bin_eta = findBin(*p_vecEtaCutoffsForSctHitsPlusDeadCut, std::fabs(eta));
       double pt = helper.pt(msgHelper);
       unsigned int bin_pt = findBin(*p_vecPtCutoffsForSctHitsPlusDeadCut, pt);
       return     bin_eta >= p_vecEtaCutoffsForSctHitsPlusDeadCut->size()
               || bin_pt >= p_vecPtCutoffsForSctHitsPlusDeadCut->size()
               || getSummarySum<2,Trk_Helper>(helper, msgHelper, {xAOD::numberOfSCTHits, xAOD::numberOfSCTDeadSensors}) >= (*p_vecvecMinSctHitsPlusDeadAboveEtaPt)[bin_eta][bin_pt];
      });
  }

  return StatusCode::SUCCESS;
}


template <class Trk_Helper>
asg::AcceptData InDet::InDetTrackSelectionTool::accept(Trk_Helper helper,
                                                       const std::map< std::string, std::vector< std::function<bool(Trk_Helper helper, const asg::AsgMessaging &msgHelper)> > > &trackCuts) const {
  if (!m_isInitialized) {
    if (!m_warnInit) {
      ATH_MSG_WARNING( "Tool is not initialized! Calling accept() will not be very helpful." );
      m_warnInit = true;
    }
  }

  asg::AcceptData acceptData(&m_acceptInfo);
  bool passAll = true;

  // loop over all cuts
  UShort_t cutFamilyIndex = 0;
  for ( const auto& cutFamily : trackCuts ) {
    bool pass = true;

    for ( const auto& cut : cutFamily.second ) {
      if (! cut(helper, *m_msgHelper) ) {
	pass = false;
	break;
      }
    }

    // @TODO really always run through all cuts even if passed is false ?
    passAll &= pass;
    acceptData.setCutResult( cutFamilyIndex, pass );
    cutFamilyIndex++;
  }

  {
     // lock access to m_numTracksPassedCuts
     std::lock_guard<std::mutex> lock(m_mutex);
     for (unsigned int idx=0; idx < trackCuts.size(); ++idx) {
        assert(idx<m_numTracksPassedCuts.size());
        m_numTracksPassedCuts[idx] += acceptData.getCutResult(idx);
     }
  }
  if (passAll) m_numTracksPassed++;
  m_numTracksProcessed++;

  return acceptData;

}


StatusCode InDet::InDetTrackSelectionTool::finalize()
{
  if (!m_isInitialized) {
    ATH_MSG_ERROR( "You are attempting to finalize a tool that has not been initialized()." );
  }

#ifdef XAOD_ANALYSIS
  if (m_numTracksProcessed == 0) {
    ATH_MSG_INFO( "No tracks processed in selection tool." );
    return StatusCode::SUCCESS;
  }
  
  // Only printed out for analysis
  ATH_MSG_INFO( m_numTracksPassed << " / " << m_numTracksProcessed << " = "
		<< m_numTracksPassed*100./m_numTracksProcessed << "% passed all cuts." );
  for (const auto& cutFamily : m_trackParticleCuts) {
    // lock(m_mutex) is not needed because this is inside of non-const finalize method.
    uint64_t numPassed = m_numTracksPassedCuts.at(m_acceptInfo.getCutPosition(cutFamily.first));
    ATH_MSG_INFO( numPassed << " = " << numPassed*100./m_numTracksProcessed << "% passed "
		  << cutFamily.first << " cut." );
  }
#endif

  return StatusCode::SUCCESS;
}


/// Accessor function to the tool's Root::TAccept object, which stores the result of
/// the last accept(...) call. The TAccept object itself is expensive to copy,
/// and so should be accessed by const reference.
/// 
const asg::AcceptInfo& InDet::InDetTrackSelectionTool::getAcceptInfo() const
{  
  return m_acceptInfo;
}


/// Function that returns the result of the cut through the xAOD::IParticle interface.
/// 
/// This is included to implement the AsgSelectionTool interface. It is not the recommended
/// way of using the tool. If the IParticle is an xAOD::TrackParticle, then it will return
/// the result of the explicitly xAOD::TrackParticle version below, with no vertex provided.
/// 
/// @param p An xAOD::IParticle pointer. If it is of type xAOD::TrackParticle, then it
///          will be cast as such and funnelled into the corresponding function. If it
///          is of some different type, then the particle will not pass.
/// 
asg::AcceptData
InDet::InDetTrackSelectionTool::accept( const xAOD::IParticle* p ) const
{
  
  asg::AcceptData acceptData(&m_acceptInfo); 
  // Check if this is a track:
  if( p->type() != xAOD::Type::TrackParticle ) {
    ATH_MSG_ERROR( "accept(...) Function received a non-track" );
    return acceptData;
  }
    
  // Cast it to a track (we have already checked its type so we do not have to dynamic_cast):
  const xAOD::TrackParticle* trk = static_cast< const xAOD::TrackParticle* >( p );
    
  // Let the specific function do the work:
  return accept( *trk, nullptr );
}


/// Function that returns the result of the cuts applied to a track in the xAOD framework.
/// 
/// The workhorse function of the tool that checks whether or not a track passes,
/// and also can provide some information about why the track failed when it does.
/// First it accesses all the data it needs from the track (but does not retrieve
/// info it won't need to check) by looping over its InDet::TrackAccessors. It then
/// loops over its cut families, each of which is composed of one or more
/// InDet::TrackCuts, storing whether or not the track passes all the corresponding
/// cuts. This design allows functionality to be added to the tool without increasing
/// overhead in the main loop for users who don't need it. This eliminates the need
/// for several multiple versions of the tool.
/// 
/// @param trk The xAOD::TrackParticle in question.
/// @param vtx An optional pointer to a xAOD::Vertex that can be provided if the user
///            wishes to cut on z0 with respect to the vertex's position. Note that
///            this does not affect d0 cuts. If a vertex is not provided then the z0
///            cuts are performed with respect to the beamspot. More information can
///            be found at the InDetTrackingDC14 TWiki.
/// @returns A asg::AcceptData object that can be treated as a boolean for a simple
///          pass/fail. It does store more detailed information about which cuts
///          passed and which failed, by organizing the results into cut families.
///          For example, TAccept::getCutResult("Z0SinTheta") will be true if the
///          track passed cuts on Z0*Sin(theta), its uncertainty, and its
///          significance.
/// 
asg::AcceptData InDet::InDetTrackSelectionTool::accept( const xAOD::TrackParticle& trk,
                                                        const xAOD::Vertex* vtx ) const
{
  asg::AcceptData acceptData(&m_acceptInfo);
  if (m_trackParticleCuts.empty()) {
     m_numTracksPassed++;
     m_numTracksProcessed++;
     return acceptData;
  }

  InDetAccessor::TrackParticleHelper track_helper(trk,vtx);
  return accept( track_helper, m_trackParticleCuts);
}

#ifndef XAOD_ANALYSIS
/// Function that returns the result of the cuts to a Trk::Track.
///
/// This version of accept() is included to allow the tool to be used
/// in full-blown athena on Trk::Tracks. It is supposed to have identical
/// functionality to the user as the xAOD version above, but of course it
/// does need to access track information differently.
/// 
asg::AcceptData
InDet::InDetTrackSelectionTool::accept( const Trk::Track& track,
					const Trk::Vertex* vertex ) const
{
  if (!m_isInitialized) ATH_MSG_WARNING( "Tool is not initialized! Calling accept() will not be very helpful." );

  asg::AcceptData acceptData(&m_acceptInfo);
  if (m_trkTrackCuts.empty()) {
     m_numTracksPassed++;
     m_numTracksProcessed++;
     return acceptData;
  }

  const Trk::TrackParameters* perigee = track.perigeeParameters();

  if ( perigee == nullptr || !perigee->covariance() ) {
    ATH_MSG_WARNING( "Track preselection: Zero pointer to parameterbase* received (most likely a track without perigee). This track will not pass any cuts." );
    return acceptData;
  }

  std::unique_ptr<const Trk::TrackParameters> paramsAtVertex;
  if (vertex) {
    Trk::PerigeeSurface perigeeSurface(vertex->position());
    paramsAtVertex =
      m_extrapolator->extrapolate(Gaudi::Hive::currentContext(),
                                  *perigee,
                                  perigeeSurface,
                                  Trk::anyDirection,
                                  true,
                                  track.info().particleHypothesis());
    perigee = paramsAtVertex.get();
  }

  if ( perigee == nullptr || !perigee->covariance() ) {
    ATH_MSG_INFO( "Track preselection: cannot make a measured perigee. This track will not pass any cuts." );
    if (!m_initTrkTools)
      ATH_MSG_INFO( "The user should set \"UseTrkTrackTools\" to true if they want the extrapolation tool to try to get a perigee." );
    return acceptData;
  }


  std::unique_ptr<Trk::TrackSummary> cleanup_summary;
  const Trk::TrackSummary* summary = track.trackSummary();
  if (summary == nullptr && m_trackSumToolAvailable) {
     cleanup_summary = m_trackSumTool->summary(Gaudi::Hive::currentContext(),track);
     summary = cleanup_summary.get();
  }
  if (summary == nullptr) {
    ATH_MSG_INFO( "Track preselection: cannot get a track summary. This track will not pass any cuts." );
    if (!m_initTrkTools)
      ATH_MSG_INFO( "The Trk::Track tools were not set to be initialized. The user should set the property \"UseTrkTrackTools\" to true if they wish to use the summary tool." );
    return acceptData;
  }

  InDetAccessor::TrkTrackHelper track_helper(track,*summary, perigee);
  acceptData = accept( track_helper, m_trkTrackCuts);
  return acceptData;
}

#endif // XAOD_ANALYSIS

/// Function to set the cut level within standalone ROOT
/// 
/// This function can be used to set the cut selection of the tool to a
/// pre-defined level, as definied in the twiki for
/// InDetTrackingPerformanceGuidelines. It is left public for use in
/// standalone ROOT. In athena, this should be set through the jobOptions
/// via the "CutLevel" property instead.
///
/// @param level The CutLevel enumeration that picks the preset level
///              to set the cuts to.
/// @param overwrite A boolean (default true) that indicates whether
///                  to force an overwrite of each cut. If false, this
///                  function does not change the cut levels if they
///                  have been altered from their default (no cut) value.
///
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
void InDet::InDetTrackSelectionTool::setCutLevel(InDet::CutLevel level, bool overwrite )
{
#ifndef XAOD_STANDALONE
  ATH_MSG_WARNING( "InDetTrackSelectionTool::setCutLevel() is not designed to be called manually in Athena." );
  ATH_MSG_WARNING( "It may not behave as intended. Instead, configure it in the job options through the CutLevel property." );
#endif // XAOD_STANDALONE
  if (!m_cutLevel.empty()) {
    ATH_MSG_WARNING( "Cut level already set to " << m_cutLevel << ".  Calling setCutLevel() is not expected." );
  }
  setCutLevelPrivate(level, overwrite);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

void InDet::InDetTrackSelectionTool::setCutLevelPrivate(InDet::CutLevel level, bool overwrite)
{
  switch (level) {
  case CutLevel::NoCut :
    if (m_isInitialized) {
      // this check is in here so that it only happens once per call to setCutLevel,
      // but will still warn if only the private version is called somehow.
      ATH_MSG_WARNING( "Trying to set cut level while the tool is already initialized." );
      ATH_MSG_WARNING( "This will almost certainly not exhibit intended behavior." );
    }
    if (overwrite) {
      // minimum cuts will default to -1, so if a user wishes to remove a cut
      //   from a preset level they can do so by setting the minimum to zero.
      // maximum cuts can be removed from a preset level by setting them negative.
      m_minPt = -1.; // in MeV
      m_minP = -1.;
      m_maxAbsEta = LOCAL_MAX_DOUBLE;
      m_maxZ0SinTheta = LOCAL_MAX_DOUBLE; // in mm
      m_maxZ0 = LOCAL_MAX_DOUBLE;
      m_maxD0 = LOCAL_MAX_DOUBLE;
      m_maxSigmaD0 = LOCAL_MAX_DOUBLE;
      m_maxSigmaZ0 = LOCAL_MAX_DOUBLE;
      m_maxSigmaZ0SinTheta = LOCAL_MAX_DOUBLE;
      m_maxD0overSigmaD0 = LOCAL_MAX_DOUBLE;
      m_maxZ0overSigmaZ0 = LOCAL_MAX_DOUBLE;
      m_maxZ0SinThetaoverSigmaZ0SinTheta = LOCAL_MAX_DOUBLE;
      m_minNInnermostLayerHits = -1;
      m_minNNextToInnermostLayerHits = -1;
      m_minNBothInnermostLayersHits = -1;
      m_maxNInnermostLayerSharedHits = LOCAL_MAX_INT;
      m_useMinBiasInnermostLayersCut = 0;
      m_minNPixelHits = -1;
      m_minNPixelHitsPhysical = -1;
      m_maxNPixelSharedHits = LOCAL_MAX_INT;
      m_maxNPixelHoles = LOCAL_MAX_INT;
      m_minNSctHits = -1;
      m_minNSctHitsPhysical = -1;
      m_maxNSctHoles = LOCAL_MAX_INT;
      m_maxNSctSharedHits = LOCAL_MAX_INT;
      m_maxNSctDoubleHoles = LOCAL_MAX_INT;
      m_minNSiHits = -1;
      m_minNSiHitsPhysical = -1;
      m_maxNSiSharedHits = LOCAL_MAX_INT;
      m_minNSiHitsIfSiSharedHits = -1;
      m_maxNSiHoles = LOCAL_MAX_INT;
      m_minEtaForStrictNSiHitsCut = LOCAL_MAX_DOUBLE;
      m_minNSiHitsAboveEtaCutoff = -1;
      m_maxOneSharedModule = false;
      m_maxTrtEtaAcceptance = LOCAL_MAX_DOUBLE;
      m_maxEtaForTrtHitCuts = -1.; // this is really a minimum eta above which cuts are not applied
      m_minNTrtHits = -1;
      m_minNTrtHitsPlusOutliers = -1;
      m_minNTrtHighThresholdHits = -1;
      m_minNTrtHighThresholdHitsPlusOutliers = -1;
      m_maxTrtHighEFraction = LOCAL_MAX_DOUBLE;
      m_maxTrtHighEFractionWithOutliers = LOCAL_MAX_DOUBLE;
      m_maxTrtOutlierFraction = LOCAL_MAX_DOUBLE;
      m_maxChiSq = LOCAL_MAX_DOUBLE;
      m_minProb = -1.;
      m_maxChiSqperNdf = LOCAL_MAX_DOUBLE;
      m_minPtForProbCut = LOCAL_MAX_DOUBLE;
      m_minProbAbovePtCutoff = -1.;
      m_useEtaDependentMaxChiSq = false;
      m_minNUsedHitsdEdx = -1;
      m_minNOverflowHitsdEdx = -1;
      m_minEProbabilityHT = -1.;
      m_eProbHTonlyForXe = false;
#ifndef XAOD_ANALYSIS
      m_minNSiHitsMod = -1;
      m_minNSiHitsModTop = -1;
      m_minNSiHitsModBottom = -1;
#endif
      m_vecEtaCutoffsForSiHitsCut = std::vector<double>();
      m_vecMinNSiHitsAboveEta = std::vector<int>();
      m_vecEtaCutoffsForPtCut = std::vector<double>();
      m_vecMinPtAboveEta = std::vector<double>();
      m_vecPtCutoffsForSctHitsCut = std::vector<double>();
      m_vecMinNSctHitsAbovePt = std::vector<int>();
      m_vecEtaCutoffsForZ0SinThetaCut = std::vector<double>();
      m_vecPtCutoffsForZ0SinThetaCut = std::vector<double>();
      m_vecvecMaxZ0SinThetaAboveEtaPt = std::vector<std::vector<double>>();
      m_vecEtaCutoffsForD0Cut = std::vector<double>();
      m_vecPtCutoffsForD0Cut = std::vector<double>();
      m_vecvecMaxD0AboveEtaPt = std::vector<std::vector<double>>();
      m_vecEtaCutoffsForSctHolesCut = std::vector<double>();
      m_vecPtCutoffsForSctHolesCut = std::vector<double>();
      m_vecvecMaxSctHolesAboveEtaPt = std::vector<std::vector<double>>();
      m_vecEtaCutoffsForSctHitsPlusDeadCut = std::vector<double>();
      m_vecPtCutoffsForSctHitsPlusDeadCut = std::vector<double>();
      m_vecvecMinSctHitsPlusDeadAboveEtaPt = std::vector<std::vector<double>>();
    }
    break;
  case CutLevel::Loose :
    setCutLevelPrivate(CutLevel::NoCut, overwrite); // if hard overwrite, reset all cuts first. will do nothing if !overwrite
    // change the cuts if a hard overwrite is asked for or if the cuts are unset
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_minNSiHits < 0) m_minNSiHits = 8;
    m_maxOneSharedModule = true;
    if (overwrite || m_maxNSiHoles >= LOCAL_MAX_INT) m_maxNSiHoles = 2;
    if (overwrite || m_maxNPixelHoles >= LOCAL_MAX_INT) m_maxNPixelHoles = 1;
    break;
  case CutLevel::LoosePrimary :
    setCutLevelPrivate(CutLevel::NoCut, overwrite); // implement loose cuts first
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_minNSiHits < 0) m_minNSiHits = 8;
    m_maxOneSharedModule = true;
    if (overwrite || m_maxNSiHoles >= LOCAL_MAX_INT) m_maxNSiHoles = 2;
    if (overwrite || m_maxNPixelHoles >= LOCAL_MAX_INT) m_maxNPixelHoles = 1;
    if (overwrite || m_minNSiHitsIfSiSharedHits < 0) m_minNSiHitsIfSiSharedHits = 10;
    break;
  case CutLevel::TightPrimary :
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_minNSiHits < 0) m_minNSiHits = 9;
    m_maxOneSharedModule = true;
    if (overwrite || m_maxNSiHoles >= LOCAL_MAX_INT) m_maxNSiHoles = 2;
    if (overwrite || m_maxNPixelHoles >= LOCAL_MAX_INT) m_maxNPixelHoles = 0;
    if (overwrite || m_minEtaForStrictNSiHitsCut >= LOCAL_MAX_DOUBLE) m_minEtaForStrictNSiHitsCut = 1.65;
    if (overwrite || m_minNSiHitsAboveEtaCutoff < 0) m_minNSiHitsAboveEtaCutoff = 11;
    if (overwrite || m_minNBothInnermostLayersHits < 0) m_minNBothInnermostLayersHits = 1;
    break;
  case CutLevel::LooseMuon :
    setCutLevelPrivate(CutLevel::NoCut, overwrite); // reset cuts unless we are doing a soft set
    if (overwrite || m_minNPixelHits < 0) m_minNPixelHits = 1;
    if (overwrite || m_minNSctHits < 0) m_minNSctHits = 5;
    if (overwrite || m_maxNSiHoles >= LOCAL_MAX_INT) m_maxNSiHoles = 2;
    if (overwrite || m_maxTrtEtaAcceptance >= LOCAL_MAX_DOUBLE ) m_maxTrtEtaAcceptance = 0.1;
    if (overwrite || m_maxEtaForTrtHitCuts < 0.) m_maxEtaForTrtHitCuts = 1.9;
    if (overwrite || m_minNTrtHitsPlusOutliers < 0) m_minNTrtHitsPlusOutliers = 6;
    if (overwrite || m_maxTrtOutlierFraction >= LOCAL_MAX_DOUBLE) m_maxTrtOutlierFraction = 0.9;
    break;
  case CutLevel::LooseElectron :
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_minNSiHits < 0) m_minNSiHits = 7;
    if (overwrite || m_minNPixelHits < 0) m_minNPixelHits = 1;
    break;
  case CutLevel::LooseTau :
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_minPt < 0.0) m_minPt = 1000.0;
    if (overwrite || m_minNSiHits < 0) m_minNSiHits = 7;
    if (overwrite || m_minNPixelHits < 0) m_minNPixelHits = 2;
    if (overwrite || m_maxD0 >= LOCAL_MAX_DOUBLE) m_maxD0 = 1.0;
    if (overwrite || m_maxZ0 >= LOCAL_MAX_DOUBLE) m_maxZ0 = 1.5;
    break;
  case CutLevel::MinBias :
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_useMinBiasInnermostLayersCut >= 0) m_useMinBiasInnermostLayersCut = 1; // if this is less than 0, it is turned off
    if (overwrite || m_minNPixelHits < 0) m_minNPixelHits = 1;
    if (overwrite || m_minNSctHits < 0) m_minNSctHits = 6;
    if (overwrite || m_minProbAbovePtCutoff < 0.) {
      m_minPtForProbCut = 10000.;
      m_minProbAbovePtCutoff = .01;
    }
    if (overwrite || m_maxD0 >= LOCAL_MAX_DOUBLE) m_maxD0 = 1.5;
    if (overwrite || m_maxZ0SinTheta >= LOCAL_MAX_DOUBLE) m_maxZ0SinTheta = 1.5;
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_minPt < 0.) m_minPt = 500.0;
    break;
  case CutLevel::HILoose:
    // HILoose is similar to MinBias, but not identical
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_useMinBiasInnermostLayersCut >= 0) m_useMinBiasInnermostLayersCut = 1;
    if (overwrite || m_minNPixelHits < 0) m_minNPixelHits = 1;
    if (overwrite || (m_vecPtCutoffsForSctHitsCut.empty()
		      && m_vecMinNSctHitsAbovePt.empty())) {
      m_vecPtCutoffsForSctHitsCut = std::vector<double>({0.0, 300.0, 400.0});
      m_vecMinNSctHitsAbovePt = std::vector<int>({2, 4, 6});
    }
    if (overwrite || m_maxD0 >= LOCAL_MAX_DOUBLE) m_maxD0 = 1.5;
    if (overwrite || m_maxZ0SinTheta >= LOCAL_MAX_DOUBLE) m_maxZ0SinTheta = 1.5;
    break;
  case CutLevel::HITight:
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    // HITight is like HILoose but we require 8 SCT hits and 2 pixel hits
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_useMinBiasInnermostLayersCut >= 0) m_useMinBiasInnermostLayersCut = 1;
    if (overwrite || m_minNPixelHits < 0) m_minNPixelHits = 2;
    if (overwrite || (m_vecPtCutoffsForSctHitsCut.empty()
		      && m_vecMinNSctHitsAbovePt.empty())) {
      m_vecPtCutoffsForSctHitsCut = std::vector<double>({0.0, 300.0, 400.0});
      m_vecMinNSctHitsAbovePt = std::vector<int>({4, 6, 8});
    }
    if (overwrite || m_maxD0 >= LOCAL_MAX_DOUBLE) m_maxD0 = 1.0;
    if (overwrite || m_maxZ0SinTheta >= LOCAL_MAX_DOUBLE) m_maxZ0SinTheta = 1.0;
    if (overwrite || m_maxChiSqperNdf >= LOCAL_MAX_DOUBLE) m_maxChiSqperNdf = 6.0;
    break;
  case CutLevel::HILooseOptimized:
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_maxNPixelHoles >= LOCAL_MAX_INT) m_maxNPixelHoles = 0;
    if (overwrite || m_useMinBiasInnermostLayersCut >= 0) m_useMinBiasInnermostLayersCut = 1;
    if (overwrite || (m_vecEtaCutoffsForZ0SinThetaCut.empty() &&
                      m_vecPtCutoffsForZ0SinThetaCut.empty()  &&
                      m_vecvecMaxZ0SinThetaAboveEtaPt.empty())){
      m_vecEtaCutoffsForZ0SinThetaCut = std::vector<double>({0.0, 1.1, 1.6, 2.0});
      m_vecPtCutoffsForZ0SinThetaCut  =
	std::vector<double>({500,  600,  700,  800,  900,  1000,  1500,
	    2000, 2500, 3000, 5000, 8000, 12000});
      m_vecvecMaxZ0SinThetaAboveEtaPt =
	std::vector<std::vector<double>>({{2.10, 2.15, 6.00, 5.00, 3.10, 2.00, 1.75, 1.60, 1.43, 1.40, 1.05, 0.65, 0.60},
					  {1.44, 1.47, 1.50, 1.55, 1.62, 1.45, 1.45, 1.78, 1.73, 1.50, 1.20, 0.97, 0.53},
					  {1.40, 1.45, 1.50, 1.46, 1.41, 1.37, 1.25, 1.50, 1.50, 1.36, 1.10, 0.85, 0.52},
					  {1.51, 1.70, 1.70, 1.71, 1.71, 1.53, 1.54, 1.49, 1.36, 1.20, 0.95, 0.60, 0.55}});
    }
    if (overwrite || (m_vecEtaCutoffsForD0Cut.empty() &&
                      m_vecPtCutoffsForD0Cut.empty()  &&
                      m_vecvecMaxD0AboveEtaPt.empty())){
      m_vecEtaCutoffsForD0Cut = std::vector<double>({0.0, 1.1, 1.6, 2.0});
      m_vecPtCutoffsForD0Cut  =
	std::vector<double>({500,  600,  700,  800,  900,  1000,  1500,
	    2000, 2500, 3000, 5000, 8000, 12000});
      m_vecvecMaxD0AboveEtaPt =
	std::vector<std::vector<double>>({{0.81, 0.90, 0.94, 0.92, 0.90, 0.75, 0.65, 0.63, 0.62, 0.60, 0.63, 0.50, 0.55},
					  {1.00, 0.98, 0.98, 0.92, 0.90, 0.69, 0.67, 0.86, 0.88, 0.88, 0.88, 0.87, 1.06},
					  {1.19, 1.15, 1.10, 1.08, 1.03, 0.94, 0.85, 0.97, 0.97, 0.96, 0.95, 0.92, 1.04},
					  {1.33, 1.23, 1.21, 1.15, 1.15, 1.07, 0.94, 0.97, 0.97, 0.97, 0.98, 1.10, 1.10}});
    }
    break;
  case CutLevel::HITightOptimized:
    setCutLevelPrivate(CutLevel::NoCut, overwrite);
    if (overwrite || m_maxAbsEta >= LOCAL_MAX_DOUBLE) m_maxAbsEta = 2.5;
    if (overwrite || m_maxNPixelHoles >= LOCAL_MAX_INT) m_maxNPixelHoles = 0;
    if (overwrite || m_useMinBiasInnermostLayersCut >= 0) m_useMinBiasInnermostLayersCut = 1;
    if (overwrite || (m_vecEtaCutoffsForZ0SinThetaCut.empty() &&
                      m_vecPtCutoffsForZ0SinThetaCut.empty()  &&
                      m_vecvecMaxZ0SinThetaAboveEtaPt.empty())){
      m_vecEtaCutoffsForZ0SinThetaCut = std::vector<double>({0.0, 1.1, 1.6, 2.0});
      m_vecPtCutoffsForZ0SinThetaCut  = std::vector<double>({500,  600,  700,  800,  900,  1000,  1500,
	  2000, 2500, 3000, 5000, 8000, 12000});
      m_vecvecMaxZ0SinThetaAboveEtaPt =
	std::vector<std::vector<double>>({{0.62, 0.70, 0.82, 0.87, 0.74, 0.61, 0.50, 0.48, 0.46, 0.45, 0.30, 0.24, 0.23},
					  {0.51, 0.53, 0.53, 0.53, 0.52, 0.43, 0.28, 0.27, 0.28, 0.30, 0.24, 0.22, 0.13},
					  {0.91, 0.89, 0.87, 0.55, 0.59, 0.37, 0.39, 0.31, 0.34, 0.35, 0.30, 0.30, 0.20},
					  {0.76, 0.71, 0.69, 0.48, 0.48, 0.47, 0.46, 0.42, 0.38, 0.32, 0.28, 0.20, 0.15}});
    }
    if (overwrite || (m_vecEtaCutoffsForD0Cut.empty() &&
                      m_vecPtCutoffsForD0Cut.empty()  &&
                      m_vecvecMaxD0AboveEtaPt.empty())){
      m_vecEtaCutoffsForD0Cut = std::vector<double>({0.0, 1.1, 1.6, 2.0});
      m_vecPtCutoffsForD0Cut  = std::vector<double>({500,  600,  700,  800,  900,  1000,  1500,
	  2000, 2500, 3000, 5000, 8000, 12000});
      m_vecvecMaxD0AboveEtaPt =
	std::vector<std::vector<double>>({{0.34, 0.39, 0.47, 0.49, 0.55, 0.47, 0.44, 0.21, 0.19, 0.17, 0.12, 0.14, 0.15},
					  {0.32, 0.32, 0.33, 0.33, 0.33, 0.27, 0.16, 0.15, 0.13, 0.15, 0.13, 0.16, 0.20},
					  {0.95, 0.91, 0.88, 0.35, 0.37, 0.24, 0.26, 0.22, 0.23, 0.24, 0.19, 0.19, 0.23},
					  {0.68, 0.67, 0.65, 0.42, 0.42, 0.36, 0.35, 0.31, 0.27, 0.26, 0.27, 0.28, 0.30}});
    }
    if (overwrite || (m_vecEtaCutoffsForSctHolesCut.empty() &&
                      m_vecPtCutoffsForSctHolesCut.empty()  &&
                      m_vecvecMaxSctHolesAboveEtaPt.empty())){
      m_vecEtaCutoffsForSctHolesCut = std::vector<double>({0.0, 1.1, 1.6, 2.0});
      m_vecPtCutoffsForSctHolesCut  = std::vector<double>({500,  600,  700,  800,  900,  1000,  1500,
	  2000, 2500, 3000, 5000, 8000, 12000});
      m_vecvecMaxSctHolesAboveEtaPt =
	std::vector<std::vector<double>>({{0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
					  {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
					  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
					  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}});
    }
    if (overwrite || (m_vecEtaCutoffsForSctHitsPlusDeadCut.empty() &&
                      m_vecPtCutoffsForSctHitsPlusDeadCut.empty()  &&
                      m_vecvecMinSctHitsPlusDeadAboveEtaPt.empty())){
      m_vecEtaCutoffsForSctHitsPlusDeadCut = std::vector<double>({0.0, 1.1, 1.6, 2.0});
      m_vecPtCutoffsForSctHitsPlusDeadCut  = std::vector<double>({500,  600,  700,  800,  900,  1000,  1500,
	  2000, 2500, 3000, 5000, 8000, 12000});
      m_vecvecMinSctHitsPlusDeadAboveEtaPt =
	std::vector<std::vector<double>>({{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
					  {0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 0, 0, 0},
					  {8, 8, 8, 7, 7, 6, 6, 6, 6, 6, 0, 0, 0},
					  {7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}});
    }
    break;
  default:
    ATH_MSG_ERROR("CutLevel not recognized. Cut selection will remain unchanged.");
    break;
  }      
}

// initialize the map from strings to cut levels
const std::unordered_map<std::string, InDet::CutLevel>
InDet::InDetTrackSelectionTool::s_mapCutLevel =
  {
    {"NoCut", InDet::CutLevel::NoCut},
    {"Loose", InDet::CutLevel::Loose},
    {"LoosePrimary", InDet::CutLevel::LoosePrimary},
    {"TightPrimary", InDet::CutLevel::TightPrimary},
    {"LooseMuon", InDet::CutLevel::LooseMuon},
    {"LooseElectron", InDet::CutLevel::LooseElectron},
    {"LooseTau", InDet::CutLevel::LooseTau},
    {"MinBias", InDet::CutLevel::MinBias},
    {"HILoose", InDet::CutLevel::HILoose},
    {"HITight", InDet::CutLevel::HITight},
    {"HILooseOptimized", InDet::CutLevel::HILooseOptimized},
    {"HITightOptimized", InDet::CutLevel::HITightOptimized}
  };
