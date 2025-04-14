/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITrackToVertex.h, (c) ATLAS Detector software 2005
///////////////////////////////////////////////////////////////////

#ifndef TRACKTOVERTEX_ITRACKTOVERTEX_H
#define TRACKTOVERTEX_ITRACKTOVERTEX_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// Trk
#include "TrkParameters/TrackParameters.h"
// xAOD
#include "xAODTracking/TrackParticleFwd.h"


namespace Trk {
 class Track;
 class StraightLineSurface;
}

namespace InDet{
 class BeamSpotData;
}

class EventContext;

namespace Reco {

  static const InterfaceID IID_ITrackToVertex("ITrackToVertex", 1, 0);

  /**
   @class ITrackToVertex

   Interface class for the TrackToVertex AlgTool,
   for more detailed information, take a look at the header file of the actual
   implementation file

   @author Andreas.Salzburger@cern.ch, Fredrik.Akesson@cern.ch
   */


  class ITrackToVertex : virtual public IAlgTool {

     public:
       /** Virtual destructor */
       virtual ~ITrackToVertex()=default;

       /** AlgTool interface methods */
       static const InterfaceID& interfaceID() { return IID_ITrackToVertex; };

       /** Use this for MT Coding */
       virtual std::unique_ptr<Trk::StraightLineSurface> GetBeamLine(
           const InDet::BeamSpotData*) const = 0;

       /** Interface method for use with xAOD::Trackparticle and given vertex
        * position */
       virtual std::unique_ptr<Trk::Perigee> perigeeAtVertex(
         const EventContext& ctx,
         const xAOD::TrackParticle& tp,
         const Amg::Vector3D& gp) const = 0;

       /** Interface method for use with TrackParticle and default primary
        * vertex from TrackParticle */
       virtual std::unique_ptr<Trk::Perigee> perigeeAtVertex(
         const EventContext& ctx,
         const xAOD::TrackParticle& tp) const = 0;

       /** Interface method for use with Track and given vertex position*/
       virtual std::unique_ptr<Trk::Perigee> perigeeAtVertex(
         const EventContext& ctx,
         const Trk::Track& trk,
         const Amg::Vector3D& gp) const = 0;

       /** Interface method for use with Track and the beamline*/
       virtual std::unique_ptr<Trk::Perigee> perigeeAtBeamline(
         const EventContext& ctx,
         const Trk::Track& trk,
         const InDet::BeamSpotData* beamSpotData) const = 0;

       /** Interface method for use with TrackParticle and the beamline*/
       virtual std::unique_ptr<Trk::TrackParameters> trackAtBeamline(
         const EventContext& ctx,
         const xAOD::TrackParticle& tp,
         const InDet::BeamSpotData*) const = 0;

       /** Interface method for use with Track and the beamline*/
       virtual std::unique_ptr<Trk::TrackParameters> trackAtBeamline(
         const EventContext& ctx,
         const Trk::Track& trk,
         const Trk::StraightLineSurface*) const = 0;

       /** Interface method for use with Track and the beamline from the
        * BeamSpotSvc - TrackParameters  */
       virtual std::unique_ptr<Trk::TrackParameters> trackAtBeamline(
         const EventContext& ctx,
         const Trk::TrackParameters& tpars,
         const Trk::StraightLineSurface*) const = 0;
  };
}

#endif // TRACKTOVERTEX_ITRACKTOVERTEX_H

