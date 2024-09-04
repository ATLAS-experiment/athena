/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetTrackSelectorTool_InDetConversionTrackSelectorTool_H
#define InDetTrackSelectorTool_InDetConversionTrackSelectorTool_H

#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "xAODTracking/TrackParticle.h"
/**
 * A tool to be used for track preselection in conversion
 * vertex finding.
 *
 * Thomas Koffas <Thomas.Koffas@cern.ch>
 * June 2008
 */

namespace Trk {
class Vertex;
class Track;
class TrackParticleBase;
}

namespace InDet {

class InDetConversionTrackSelectorTool
  : virtual public Trk::ITrackSelectorTool
  , public AthAlgTool
{

public:
  virtual StatusCode initialize() override;

  InDetConversionTrackSelectorTool(const std::string& t,
                                   const std::string& n,
                                   const IInterface* p);

  virtual ~InDetConversionTrackSelectorTool() =default;

  /** Select a Trk::Track  */
  virtual bool decision(const Trk::Track& track,
                        const Trk::Vertex* vertex) const override final;

  /** Select a Trk::TrackParticleBase  */
  virtual bool decision(const Trk::TrackParticleBase& track,
                        const Trk::Vertex* vertex) const override final;

  virtual bool decision(const xAOD::TrackParticle&,
                        const xAOD::Vertex*) const override final;

private:
  int getCount(const xAOD::TrackParticle& tp, xAOD::SummaryType type) const
  {
    uint8_t val;
    if (!tp.summaryValue(val, type))
      return 0;
    return val > 0 ? val : 0;
  }

  // Get Eta bin for eta-dependent TRT-track cuts
  unsigned int getEtaBin(const Trk::Perigee& perigee) const;

  Amg::Vector3D getPosOrBeamSpot(const EventContext& ctx,
                                 const xAOD::Vertex*) const;

  ToolHandle<Trk::IExtrapolator> m_extrapolator{ this,
                                                 "Extrapolator",
                                                 "",
                                                 "Extrapolator Tool" };
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
    this,
    "BeamSpotKey",
    "BeamSpotData",
    "SG key for beam spot"
  };
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};
  Gaudi::Property<bool> m_useEventInfoBs{this,"UseEventInfoBS",false};
  Trk::Vertex* getBeamSpot(const EventContext& ctx) const;
  /** Properties for track selection:all cuts are ANDed */
  DoubleProperty m_maxSiD0
    {this, "maxSiD0", 35., "Maximal d0 at (0,0,0) for tracks with Si hits"};
  DoubleProperty m_maxTrtD0
    {this, "maxTrtD0", 100., "Maximal d0 at (0,0,0) for standalone TRT tracks"};
  DoubleProperty m_maxSiZ0{this, "maxSiZ0", 200., "Maximal z0 at (0,0,0)"};
  DoubleProperty m_maxTrtZ0
    {this, "maxTrtZ0", 1200., "Maximal z0 at (0,0,0) for standalone TRT tracks"};
  DoubleProperty m_minPt{this, "minPt", 500., "Minimum Pt of tracks"};
  DoubleProperty m_trRatio1
    {this, "RatioCut1", 0.5, "TR ratio for tracks with 15 or less TRT hits"};
  DoubleProperty m_trRatio2
    {this, "RatioCut2", 0.1, "TR ratio for tracks with 16 to 25 TRT hits"};
  DoubleProperty m_trRatio3
    {this, "RatioCut3", 0.05, "TR ratio for tracks with 26 or more TRT hits"};
  DoubleProperty m_trRatioTRT
    {this, "RatioTRT", 0.1, "TR ratio for all TRT only tracks"};

  DoubleArrayProperty m_TRTTrksEtaBins
    {this, "TRTTrksEtaBins",
     {999., 999., 999., 999., 999., 999., 999., 999., 999., 999.},
     "Eta bins (10 expected) for TRT-only track cuts"};
  DoubleArrayProperty m_TRTTrksBinnedRatioTRT
    {this, "TRTTrksBinnedRatioTRT", {0., 0., 0., 0., 0., 0., 0., 0., 0., 0.},
     "Eta-binned eProbabilityHT for TRT-only track cuts"};

  DoubleProperty m_trRatioV0
    {this, "RatioV0", 1., "TR ratio for pion selection during V0 reconstruction"};
  DoubleProperty m_sD0_Si
    {this, "significanceD0_Si", 2., "Cut on D0 significance of Si tracks"};
  DoubleProperty m_sD0_Trt
    {this, "significanceD0_Trt", 0.5, "Cut on D0 significance of TRT tracks"};
  DoubleProperty m_sZ0_Trt
    {this, "significanceZ0_Trt", 3., "Cut on Z0 significance of TRT tracks"};

  BooleanProperty m_isConv{this, "IsConversion", true, "Conversion flag"};
  BooleanProperty m_PIDonlyForXe
    {this, "PIDonlyForXe", false, "Only check TRT PID if all hits are Xe hits"};

}; // end of class definitions
} // end of namespace definitions

#endif
