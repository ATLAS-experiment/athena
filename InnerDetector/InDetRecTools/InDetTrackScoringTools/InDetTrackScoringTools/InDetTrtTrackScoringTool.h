/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRTTRACKSCORINGTOOL_H
#define INDETTRTTRACKSCORINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkEventPrimitives/TrackScore.h"
#include "TrkToolInterfaces/ITrackScoringTool.h"
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
// Access to mu
#include "LumiBlockComps/ILumiBlockMuTool.h"
#include "TrkParameters/TrackParameters.h"
#include <string>
#include <vector>

namespace Trk {
class Track;
class TrackSummary;
}

class TRT_ID;

namespace InDet {

class ITrtDriftCircleCutTool;

/**Concrete implementation of the ITrackScoringTool pABC*/
class InDetTrtTrackScoringTool
  : virtual public Trk::ITrackScoringTool
  , public AthAlgTool
{

public:
  InDetTrtTrackScoringTool(const std::string&,
                           const std::string&,
                           const IInterface*);
  virtual ~InDetTrtTrackScoringTool() = default;
  virtual StatusCode initialize() override;
  /** check track selections independent from TrackSummary */
  virtual bool passBasicSelections( const Trk::Track& track ) const override;

  /** create a score based on how good the passed track is*/
  virtual Trk::TrackScore score(const Trk::Track& track, bool checkBasicSel) const override;

  /** create a score based on how good the passed TrackSummary is*/
  virtual Trk::TrackScore simpleScore(
    const Trk::Track& track,
    const Trk::TrackSummary& trackSum) const override;

  Trk::TrackScore TRT_ambigScore(const Trk::Track& track,
                                 const Trk::TrackSummary& trackSum) const;

private:
  void setupTRT_ScoreModifiers();

  /** Decide whether standalone TRT tracks pass the minimum hit requirement. */
  bool isGoodTRT(const Trk::Track&) const;

  // Get Eta bin for eta-dependent TRT-track cuts
  unsigned int getEtaBin(const Trk::Perigee& perigee) const;

  // Get eta- and mu-dependent nTRT cut for TRT track
  double getMuDependentNtrtMinCut(unsigned int eta_bin) const;

  /**ID TRT helper*/
  const TRT_ID* m_trtId;

  // these are used for ScoreModifiers
  int m_maxSigmaChi2 = -1, m_maxTrtRatio = -1, m_maxTrtFittedRatio = -1;

  std::vector<double> m_factorSigmaChi2, m_factorTrtRatio,
    m_factorTrtFittedRatio;

  std::vector<double> m_boundsSigmaChi2, m_boundsTrtRatio,
    m_boundsTrtFittedRatio;

  /** use the scoring tuned to Ambiguity processing or not */
  BooleanProperty m_useAmbigFcn{this, "useAmbigFcn", true};
  BooleanProperty m_useSigmaChi2{this, "useSigmaChi2", false};

  /**holds the scores assigned to each Trk::SummaryType from the track's
   * Trk::TrackSummary*/
  std::vector<Trk::TrackScore> m_summaryTypeScore;

  /** Returns minimum number of expected TRT drift circles depending on eta. */
  ToolHandle<ITrtDriftCircleCutTool> m_selectortool
    {this, "DriftCircleCutTool", "InDet::InDetTrtDriftCircleCutTool"};

  // Read handle for conditions object to get the field cache
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey{
    this,
    "AtlasFieldCacheCondObj",
    "fieldCondObj",
    "Name of the Magnetic Field conditions object key"
  };

  ToolHandle<ILumiBlockMuTool> m_lumiBlockMuTool{
    this,
    "LuminosityTool",
    "LumiBlockMuTool/LumiBlockMuTool",
    "Luminosity Tool"
  };

  /** cuts for selecting good tracks*/
  IntegerProperty m_minTRTonTrk
    {this, "minTRTonTrk", 15, "minimum number of TRT hits"};
  DoubleProperty m_maxEta{this, "maxEta", 2.1, "maximal Eta cut"};
  DoubleProperty m_ptmin{this, "PtMin", 1.0, "Minimum Pt"};
  BooleanProperty m_parameterization
    {this, "UseParameterization", true, "use parameterization to cut instead of custom cut"};
  BooleanProperty m_oldLogic
    {this, "OldTransitionLogic", false, "use old transition hit logic"};
  DoubleProperty m_minTRTprecision
    {this, "minTRTPrecisionFraction", 0.5, "minimum fraction of TRT precision hits"};

  DoubleArrayProperty m_TRTTrksEtaBins
    {this, "TRTTrksEtaBins",
     {999., 999., 999., 999., 999., 999., 999., 999., 999., 999.},
     "Eta bins (10 expected) for TRT-only track cuts"};
  DoubleArrayProperty m_TRTTrksMinTRTHitsThresholds
    {this, "TRTTrksMinTRTHitsThresholds",
     {0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
     "Eta-binned nTRT cut for TRT-only cuts"};
  DoubleArrayProperty m_TRTTrksMinTRTHitsMuDependencies
    {this, "TRTTrksMinTRTHitsMuDependencies",
     {0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
     "Eta-bined Mu-dependent component for nTRT cut"};
};

}
#endif
