/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetDetailedTrackSelectorTool_InDetDetailedTrackSelectorTool_H
#define InDetDetailedTrackSelectorTool_InDetDetailedTrackSelectorTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"
#include "InDetRecToolInterfaces/IInDetTestPixelLayerTool.h"


#include "TrkParameters/TrackParameters.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexFwd.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "xAODEventInfo/EventInfo.h"
// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

#include "CLHEP/Units/SystemOfUnits.h"

/**
 * @file InDetDetailedTrackSelectorTool.h
 * @class InDetDetailedTrackSelectorTool
 *
 * @brief TrackSelector for general use
 *
 * The option for anyDirection:
 * Required for cosmic tracks:  mixture of tracks with different reo logic.
 * Default is the extrapolation oppositeToMomentum, which is required for collision tracks.
 *
 * @author Giacinto Piacquadio <giacinto.piacquadio@physik.uni-freiburg.de>
 * @author Kirill Prokofiev <Kirill.Prokofiev@cern.ch>
 * @author Daniel Kollar <daniel.kollar@cern.ch>
 */



namespace Trk
{
  class ITrackSummaryTool;
  class IExtrapolator;
  class Vertex;
  class RecVertex;
  class FitQuality;
  class TrackSummary;
  class Track;
  class TrackParticleBase;
}

namespace InDet
{
  class ITrtDriftCircleCutTool;

  class InDetDetailedTrackSelectorTool : virtual public Trk::ITrackSelectorTool, public AthAlgTool
  {

  public:

    enum Grade { Undefined, Good, Shared, nbGrades };

    StatusCode initialize();

    StatusCode finalize();

    InDetDetailedTrackSelectorTool(const std::string& t, const std::string& n, const IInterface*  p);

    ~InDetDetailedTrackSelectorTool();

    bool decision(const Trk::Track& track,const Trk::Vertex* vertex) const;

    bool decision(const Trk::TrackParticleBase& track,const Trk::Vertex* vertex) const;

    bool decision(const xAOD::TrackParticle& track,const xAOD::Vertex* vertex) const;

  private:
    int getCount( const xAOD::TrackParticle& tp, xAOD::SummaryType type ) const {
      uint8_t val;
      if( !tp.summaryValue(val,type) ) return 0;
      return val > 0 ? val : 0;
    }
    bool decision(const Trk::Perigee* track,const AmgSymMatrix(3)* covariancePosition) const;
    bool decision(const Trk::FitQuality*  TrkQuality) const;
    bool decision(double chi2, int ndf ) const;
    bool decision(const Trk::TrackSummary* summary,
		  const xAOD::TrackParticle* tp,
		  bool useSharedHitInfo,
		  bool useTrtHitInfo,
		  const Trk::Perigee* track,
		  const int nHitTrt,
		  const int nHitTrtPlusOutliers) const;

    bool preselectionBeforeExtrapolation(const Trk::Perigee & myPerigee) const;
    Amg::Vector3D getPosOrBeamSpot(const xAOD::Vertex*) const;

    DoubleProperty m_pTMin{this, "pTMin", 1.*CLHEP::GeV, "min. pT: |pT|>pTMin"};
    DoubleProperty m_pMin{this, "pMin", 0., "min. p = pT/cos(theta): |p| > pMin"};
    DoubleProperty m_IPd0Max{this, "IPd0Max", 2.*CLHEP::mm, "max. d0: |d0|<d0Max"};
    DoubleProperty m_IPz0Max{this, "IPz0Max", 1.5*CLHEP::mm, "max. z0: |z0*sin(theta)|<z0Max"};
    DoubleProperty m_z0Max{this, "z0Max", 9999.*CLHEP::mm, "max. z0: |z0|<z0Max"};
    DoubleProperty m_sigIPd0Max{this, "sigIPd0Max", 999.*CLHEP::mm, "max d0 error"};
    DoubleProperty m_sigIPz0Max
      {this, "sigIPz0Max", 999.*CLHEP::mm, "max (error only due to z0)*sin(theta)"};
    DoubleProperty m_d0significanceMax
      {this, "d0significanceMax", -1., "max IP significance d0 (-1 switches it off)"};
    DoubleProperty m_z0significanceMax
      {this, "z0significanceMax", -1., "max IP significance z0 (-1 switches it off)"};
    DoubleProperty m_etaMax{this, "etaMax", 9999., "max. pseudo-rapidity"};

    //<! if false the following cuts are ignored
    BooleanProperty m_useTrackSummaryInfo{this, "useTrackSummaryInfo", true};
    IntegerProperty m_nHitBLayer
      {this, "nHitBLayer", 1, "at least n hits in Blayer"};
    IntegerProperty m_nHitPix{this, "nHitPix", 2, "at least n hits in pixels"};
    IntegerProperty m_nHitSct{this, "nHitSct", 0, "at least n hits in SCT"};
    IntegerProperty m_nHitSi{this, "nHitSi", 7, "at least n hits in pixels+SCT"};
    IntegerProperty m_nHitPixPhysical
      {this, "nHitPixPhysical", 0, "at least n physical hits in pixel"};
    IntegerProperty m_nHitSiPhysical
      {this, "nHitSiPhysical", 3, "at least n physical hits in pixel+SCT"};
    IntegerProperty m_nHitTrt{this, "nHitTrt", 0, "at least n hits in TRT"};
    IntegerProperty m_nHitTrtPlusOutliers
      {this, "nHitTrtPlusOutliers", 0,
       "at least n hits in TRT (including outliers)"};

    //<! for selecting electrons (soft E-ID)
    IntegerProperty m_nHitTrtHighE
      {this, "nHitTrtHighE", 0, "at least n high threshold hits in TRT"};
    IntegerProperty m_nHitTrtPlusOutliersHighE
      {this, "nHitTrtPlusOutliersHighE", 0,
       "at least n high threshold hits in TRT (including outliers)"};
    //<! for rejecting electrons (tau-ID)
    DoubleProperty m_nHitTrtHighEFraction
      {this, "nHitTrtHighEFractionMax", 999., "maximum x fraction of transition hits in TRT"};
    DoubleProperty m_nHitTrtHighEFractionWithOutliers
      {this, "nHitTrtHighEFractionWithOutliersMax", 999.,
       "maximum x fraction of transition hits in TRT (including outliers)"};

    DoubleProperty m_TrtMaxEtaAcceptance
      {this, "TrtMaxEtaAcceptance", 999.,
       "limit of eta regions where trt hits are expected"};

     //<! if false the following cuts are ignored
    BooleanProperty m_useSharedHitInfo{this, "useSharedHitInfo", false};
    IntegerProperty m_nSharedBLayer
      {this, "nSharedBLayer", 0, "max. number of shared hits in B layer"};
    IntegerProperty m_nSharedPix
      {this, "nSharedPix", 0, "max. number of shared hits in pixels"};
    IntegerProperty m_nSharedSct
      {this, "nSharedSct", 1, "max. number of shared hits in SCT"};
    IntegerProperty m_nSharedSi
      {this, "nSharedSi", 999, "max. number of shared hits in pixels+SCT"};

    IntegerProperty m_nHoles
      {this, "nHoles", 999, "max. number of holes in pixel+SCT"};
    IntegerProperty m_nDoubleHoles
      {this, "nDoubleHoles", 999, "max number of double-holes in SCT"};
    IntegerProperty m_nHolesPix
      {this, "nHolesPixel", 999, "max. number of holes in pixels"};
    IntegerProperty m_nHolesSct
      {this, "nHolesSct", 999, "max. number of holes in SCT"};

     //<! if false the following cuts are ignored
    BooleanProperty m_useTrackQualityInfo{this, "useTrackQualityInfo", true};
    DoubleProperty m_fitChi2{this, "fitChi2", 99999., "max. fit chi2"};
    DoubleProperty m_fitProb{this, "fitProb", -1., "min. fit chi2 probability"};
    DoubleProperty m_fitChi2OnNdfMax{this, "fitChi2OnNdfMax", 999., "max. fitchi2/ndf"};

    DoubleProperty m_scaleMinHitTrt
      {this, "scaleMinHitTrt", 1.,
       "scale the eta dependent minimum number of TRT hits; scaling is only applied if m_addToMinHitTrt==0"};
    IntegerProperty m_addToMinHitTrt
      {this, "addToMinHitTrt", 0,
       "add to/subtract from eta dependent minimum nimber of TRT hits"};
    DoubleProperty m_scaleMinHitTrtWithOutliers
      {this, "scaleMinHitTrtWithOutliers", 1.,
       "scale the eta dependent minimum number of TRT hits + outliers; scaling is only applied if m_addToMinHitTrtWithOutliers==0"};
    IntegerProperty m_addToMinHitTrtWithOutliers
      {this, "addToMinHitTrtWithOutliers", 0,
       "add to/subtract from eta dependent minimum nimber of TRT hits + outliers"};

    BooleanProperty m_usePreselectionCuts{this, "usePreselectionCuts", false};
    DoubleProperty m_d0MaxPreselection{this, "d0MaxPreselection", 10.};

    BooleanProperty m_useEtaDepententMinHitTrt
      {this, "useEtaDepententMinHitTrt", false};
    BooleanProperty m_useEtaDepententMinHitTrtWithOutliers
      {this, "useEtaDepententMinHitTrtWithOutliers", false};

    ToolHandle<Trk::ITrackSummaryTool> m_trackSumTool
      {this, "TrackSummaryTool", "Trk::TrackSummaryTool"};
    ToolHandle<Trk::ITrackParticleCreatorTool> m_particleCreator
      {this, "TrackParticleCreatorTool", ""};
    ToolHandle<Trk::IExtrapolator> m_extrapolator
      {this, "Extrapolator", "Trk::Extrapolator"};
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey
      {this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    ToolHandle<ITrtDriftCircleCutTool> m_trtDCTool
      {this, "TrtDCCutTool", "InDet::InDetTrtDriftCircleCutTool",
       "Tool to get eta dependent cut on number of TRT hits"};

    ToolHandle< InDet::IInDetTestPixelLayerTool > m_inDetTestPixelLayerTool
      {this, "InDetTestPixelLayerTool", "",
       "Tool to test if the track crosses a dead module on the b-layer"};
    // Read handle for conditions object to get the field cache
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey
      {this, "AtlasFieldCacheCondObj", "fieldCondObj",
       "Name of the Magnetic Field conditions object key"};

    bool m_trackSumToolAvailable = true;
    bool m_partCreatorToolAvailable = false;
            
    // changes for the pt-dependent sct cut
    BooleanProperty m_usePtDependentCuts{this, "UsePtDependentCuts", false};
      
    FloatArrayProperty m_ptBenchmarks{this, "PtBenchmarks", {}};

    IntegerArrayProperty m_nSCTValues{this, "SCTCutValues", {}};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key
      {this, "EventInfo", "EventInfo", "Input event information"};
    BooleanProperty m_useEventInfoBs
      {this, "UseEventInfoBS", false, "Access beamspot via the EvenInfo object"};
    Trk::Vertex* getBeamSpot(const EventContext&) const;

  }; //end of class definitions

} //end of namespace definitions

#endif //TrkMultipleVertexSeedFinders_PVFindingTrackSelectoTool_H
