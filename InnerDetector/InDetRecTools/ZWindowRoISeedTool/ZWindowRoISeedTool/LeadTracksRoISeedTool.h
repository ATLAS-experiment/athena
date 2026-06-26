// -*- C++ -*-

/*
  Copyright (C) 2020-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for Z-window RoI from leading two track middle vertex z-.
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedTool_xk_LeadTracksRoISeedTool_h
#define SiSpacePointsSeedTool_xk_LeadTracksRoISeedTool_h

#include "InDetRecToolInterfaces/IZWindowRoISeedTool.h"
#include "GaudiKernel/EventContext.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/Track.h"
#include "ITrackToVertex/ITrackToVertex.h"

#include <vector>
#include <string>

namespace InDet {

  class LeadTracksRoISeedTool final:
    public extends<AthAlgTool, IZWindowRoISeedTool> 
  {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:

    ///////////////////////////////////////////////////////////////////
    /// @name Standard tool methods
    ///////////////////////////////////////////////////////////////////
    //@{
    LeadTracksRoISeedTool(const std::string&,const std::string&,const IInterface*);
    virtual ~LeadTracksRoISeedTool() = default;
    virtual StatusCode               initialize() override;
    //@}

    /** @brief Compute RoI */
    virtual std::vector<ZWindow> getRoIs(const EventContext& ctx) const override;

  protected:

    /**    @name Disallow default instantiation, copy, assignment **/
    LeadTracksRoISeedTool() = delete;
    LeadTracksRoISeedTool(const LeadTracksRoISeedTool&) = delete;
    LeadTracksRoISeedTool &operator=(const LeadTracksRoISeedTool&) = delete;

    ///////////////////////////////////////////////////////////////////
    /// @name Tool configuration properties
    ///////////////////////////////////////////////////////////////////
    //@{
  
    SG::ReadHandleKey<TrackCollection> m_inputTracksCollectionKey{this, "InputTracksCollection", "ExtendedTracks", "Input Track collection."};
    FloatProperty m_trkLeadingPt{this, "LeadingMinTrackPt", 18000., "min. p_{T} of leading track"};
    FloatProperty m_trkSubLeadingPt{this, "SubleadingMinTrackPt", 12500., "min. p_{T} of sub-leading track"};
    FloatProperty m_trkEtaMax{this, "TracksMaxEta", 2.5, "max |eta| for tracks consideration"};
    FloatProperty m_trkD0Max{this, "TracksMaxD0", 9999., "max |d0| for tracks consideration"};
    FloatProperty m_maxDeltaZ{this, "MaxDeltaZTracksPair", 1.0, "maximum delta z0 between leading tracks pair"};
    FloatProperty m_z0Window{this, "TrackZ0Window", 30.0, "width of z0 window"};    
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    ToolHandle< Reco::ITrackToVertex > m_trackToVertex{this, "TrackToVertexTool", "Reco::TrackToVertex/TrackToVertex", "Track-to-Vertex extrapolation tool"};
    
    /** @} */
	
    static bool tracksPtGreaterThan(const Trk::Track* const &track1, const Trk::Track* const &track2)
      {
	float theta1 = track1->perigeeParameters()->parameters()[Trk::theta];
	float ptinv1 = std::abs(track1->perigeeParameters()->parameters()[Trk::qOverP]) / std::sin(theta1);
	float theta2 = track2->perigeeParameters()->parameters()[Trk::theta];
	float ptinv2 = std::abs(track2->perigeeParameters()->parameters()[Trk::qOverP]) / std::sin(theta2);
	//return less than of inverse 
	return (ptinv1 < ptinv2);
      }

  }; // LeadTracksRoISeedTool
} //InDet namespace

#endif // SiSpacePointsSeedMaker_LeadTracksRoISeedTool

