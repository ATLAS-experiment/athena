/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////
// Markus Elsing
/////////////////////////////////

#ifndef INDETAMBISCORINGTOOL_H
#define INDETAMBISCORINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "InDetRecToolInterfaces/IInDetEtaDependentCutsSvc.h"
#include "InDetRecToolInterfaces/ITrtDriftCircleCutTool.h"
#include "TrkCaloClusterROI/ROIPhiRZContainer.h"
#include "TrkEventPrimitives/TrackScore.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkToolInterfaces/ITrackScoringTool.h"
#include "TrkParameters/TrackParameters.h"
#include "AthenaKernel/CLASS_DEF.h"
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"
#include <vector>
#include <string>
#include "BeamSpotConditionsData/BeamSpotData.h"

namespace Trk {
  class Track;
  class TrackSummary;

}
class EventContext;

namespace InDet {

/**Concrete implementation of the ITrackScoringTool pABC*/
class InDetAmbiScoringTool : virtual public Trk::ITrackScoringTool,  
                             public AthAlgTool
{

 public:
  InDetAmbiScoringTool(const std::string&,const std::string&,const IInterface*);
  virtual ~InDetAmbiScoringTool () = default;
  virtual StatusCode initialize() override;
  /** check track selections independent from TrackSummary */
  virtual bool passBasicSelections( const Trk::Track& track ) const override;

  /** create a score based on how good the passed track is*/
  virtual Trk::TrackScore score( const Trk::Track& track, bool checkBasicSel ) const override;
  
  /** create a score based on how good the passed TrackSummary is*/
  virtual Trk::TrackScore simpleScore( const Trk::Track& track, const Trk::TrackSummary& trackSum ) const override;
  Trk::TrackScore  ambigScore( const Trk::Track& track, const Trk::TrackSummary& trackSum ) const;
  

 private:
  void setupScoreModifiers();
  
  
    /** Check if the cluster is compatible with a EM cluster*/
  bool isEmCaloCompatible(const Trk::Track& track, const EventContext& ctx) const;
  
  
  //these are used for ScoreModifiers
  int m_maxDblHoles = -1, m_maxPixHoles = -1, m_maxSCT_Holes = -1,
    m_maxHits = -1, m_maxSigmaChi2 = -1, m_maxTrtRatio = -1,
    m_maxTrtFittedRatio = -1, m_maxB_LayerHits = -1, m_maxPixelHits = -1,
    m_maxPixLay = -1,  m_maxGangedFakes = -1;
  std::vector<double> m_factorDblHoles, m_factorPixHoles, m_factorSCT_Holes,  m_factorHits,
    m_factorSigmaChi2, m_factorB_LayerHits, m_factorPixelHits, m_factorPixLay, m_factorGangedFakes;
  std::vector<double> m_boundsSigmaChi2,
    m_boundsTrtRatio, m_factorTrtRatio, m_boundsTrtFittedRatio, m_factorTrtFittedRatio;

  /** Returns minimum number of expected TRT drift circles depending on eta. */
  ToolHandle<ITrtDriftCircleCutTool> m_selectortool
    {this, "DriftCircleCutTool", "InDet::InDetTrtDriftCircleCutTool"};
  
  /**holds the scores assigned to each Trk::SummaryType from the track's Trk::TrackSummary*/
  std::vector<Trk::TrackScore>           m_summaryTypeScore;
  
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
  
  ToolHandle<Trk::IExtrapolator> m_extrapolator
    {this, "Extrapolator", "Trk::Extrapolator"};

  // Read handle for conditions object to get the field cache
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};

  
  /** use the scoring tuned to Ambiguity processing or not */
  BooleanProperty m_useAmbigFcn{this, "useAmbigFcn", true};
  BooleanProperty m_useTRT_AmbigFcn{this, "useTRT_AmbigFcn", false};
  BooleanProperty m_useSigmaChi2{this, "useSigmaChi2", false};

  BooleanProperty m_usePixel{this, "usePixel", true};
  BooleanProperty m_useSCT{this, "useSCT", true};

  /** cuts for selecting good tracks*/
  DoubleProperty m_minPt{this, "minPt", 500., "minimal Pt cut"};
  DoubleProperty m_maxEta{this, "maxEta", 2.7, "maximal Eta cut"};
  DoubleProperty m_maxRPhiImp
    {this, "maxRPhiImp", 10., "maximal RPhi impact parameter cut"};
  DoubleProperty m_maxZImp
    {this, "maxZImp", 250., "maximal z impact parameter cut"};

  IntegerProperty m_minSiClusters
    {this, "minSiClusters", 7, "minimal number of Si clusters"};
  IntegerProperty m_maxDoubleHoles
    {this, "maxDoubleHoles", 2, "maximum number of SCT double holes"};
  IntegerProperty m_maxSiHoles
    {this, "maxSiHoles", 5, "max number of Silicon (Pixel+SCT) holes"};
  IntegerProperty m_maxPixelHoles
    {this, "maxPixelHoles", 5, "max number of Pixel holes"};
  IntegerProperty m_maxSctHoles
    {this, "maxSCTHoles", 5, "max number of SCT holes"};
  IntegerProperty m_minTRTonTrk
    {this, "minTRTonTrk", 9, "minimum number of TRT hits"};
  DoubleProperty m_minTRTprecision
    {this, "minTRTPrecisionFraction", 0.5, "minimum fraction of TRT precision hits"};
  IntegerProperty m_minPixel
    {this, "minPixel", 0, "minimum number of pixel clusters"};

  DoubleProperty m_maxRPhiImpEM
    {this, "maxRPhiImpEM", 50., "maximal RPhi impact parameter cut track that match EM clusters"};
  BooleanProperty m_useEmClusSeed{this, "doEmCaloSeed", true};
  FloatProperty m_phiWidthEm{this, "phiWidthEM", 0.075};
  FloatProperty m_etaWidthEm{this, "etaWidthEM", 0.05};

  SG::ReadHandleKey<ROIPhiRZContainer> m_caloClusterROIKey
     {this, "EMROIPhiRZContainer", "", "Name of the calo cluster ROIs in Phi,R,Z parameterization"};

  /** use the ITk scoring tuned to Ambiguity processing or not */
  BooleanProperty m_useITkAmbigFcn{this, "useITkAmbigFcn", false};

  /** ITk eta-dependent cuts*/
  ServiceHandle<IInDetEtaDependentCutsSvc> m_etaDependentCutsSvc{this, "InDetEtaDependentCutsSvc", ""};

};


} // namespace InDet


#endif 
