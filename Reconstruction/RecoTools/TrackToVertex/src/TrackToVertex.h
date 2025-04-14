/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackToVertex.h, (c) ATLAS Detector software 2005
///////////////////////////////////////////////////////////////////


#ifndef RECOTOOLS_TRACKTOVERTEX_H
#define RECOTOOLS_TRACKTOVERTEX_H

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// Trk
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/Track.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "TrkExInterfaces/IExtrapolator.h"

namespace Trk {
  class StraightLineSurface;
}

  /** @class TrackToVertex

     Standard Tool to extrapolate Track/TrackParticleBase to Vertex
     and BeamSpot/BeamLine.

    @author Andreas.Salzburger@cern.ch

    */

namespace Reco {

  class TrackToVertex : public extends <AthAlgTool, ITrackToVertex>
  {
    public:

      /** AlgTool like constructor */
      TrackToVertex(const std::string&,const std::string&,const IInterface*);

      /**Virtual destructor*/
      virtual ~TrackToVertex() = default;

      /** AlgTool initailize method.*/
      virtual StatusCode initialize() override final;

      /** Use this for MT Coding */
      virtual std::unique_ptr<Trk::StraightLineSurface> GetBeamLine(
        const InDet::BeamSpotData*)
        const override final;

      /** Interface method for use with xAOD::Trackparticle and given vertex
       * position*/
      virtual std::unique_ptr<Trk::Perigee> perigeeAtVertex(
        const EventContext& ctx,
        const xAOD::TrackParticle& tp,
        const Amg::Vector3D& gp) const override final;

      /** Interface method for use with TrackParticle and default primary vertex
       * from TrackParticle  - xAOD */
      virtual std::unique_ptr<Trk::Perigee> perigeeAtVertex(
        const EventContext& ctx,
        const xAOD::TrackParticle& tp) const override final;

      /** Interface method for use with Track and given vertex position - ESD */
      virtual std::unique_ptr<Trk::Perigee> perigeeAtVertex(
        const EventContext& ctx,
        const Trk::Track& trk,
        const Amg::Vector3D& gp) const override final;

      /** Interface method for use with Track and the beamline*/
      virtual std::unique_ptr<Trk::Perigee> perigeeAtBeamline(
        const EventContext& ctx,
        const Trk::Track& trk,
        const InDet::BeamSpotData*) const override final;

      /** Interface method for use with TrackParticle and the beamline from the
       * BeamSpotSvc - xAOD*/
      virtual std::unique_ptr<Trk::TrackParameters> trackAtBeamline(
        const EventContext& ctx,
        const xAOD::TrackParticle& tp,
        const InDet::BeamSpotData*) const override final;

      /** Interface method for use with Track and the beamline from the
       * BeamSpotSvc - ESD */
      virtual std::unique_ptr<Trk::TrackParameters> trackAtBeamline(
        const EventContext& ctx,
        const Trk::Track& trk,
        const Trk::StraightLineSurface* beamline) const override final;

      /** Interface method for use with Track and the beamline from the
       * BeamSpotSvc - TrackParameters  */
      virtual std::unique_ptr<Trk::TrackParameters> trackAtBeamline(
        const EventContext& ctx,
        const Trk::TrackParameters& tpars,
        const Trk::StraightLineSurface* beamline) const override final;

    private:
     inline bool startAtOriginalPerigee(const Trk::Track& track) const {
       return m_startTRTSAAtPerigee.value() &&
              track.info().patternRecognition().test(
                  Trk::TrackInfo::TRTStandalone) &&
              track.perigeeParameters();
     }

     ToolHandle<Trk::IExtrapolator> m_extrapolator{
         this, "Extrapolator",
         "Trk::Extrapolator/AtlasExtrapolator"};  //!< ToolHandle for
                                                  //!< Extrapolator
     Gaudi::Property<bool> m_startTRTSAAtPerigee{
         this, "StartTRTStandaloneTracksAtOriginalPerigee", false,
         "When extrapolating TRT standalone start at their original perigee "
         "which may have a more realistic covariance"};

     const static Amg::Vector3D s_origin;  //!< static origin
  };

} // end of namespace


#endif // RECOTOOLS_TRACKTOVERTEX_H

