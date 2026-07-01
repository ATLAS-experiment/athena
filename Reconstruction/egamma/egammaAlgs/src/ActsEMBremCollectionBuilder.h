/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMAALGS_ACTSEMBREMCOLLECTIONBUILDER_H
#define EGAMMAALGS_ACTSEMBREMCOLLECTIONBUILDER_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsToolInterfaces/ITrackToTrackParticleCnvTool.h"
/**
 * @class ActsEMBremCollectionBuilder
 * @brief Algorithm which refits Acts tracks using the ACTS GSF.
 * Input: xAOD::TrackParticleContainer
 * Output: ActsTrk::TrackContainer and xAOD::TrackParticleContainer
 * Each output TrackParticle has an originalTrackParticle link back to the
 * input track particle and an actsTrack link to the refitted Acts track.
 *
 * Workflow:
 * 1. Reads TrackParticles from the selected input container
 * 2. Filters tracks that meet the minimum silicon hits criteria (minNoSiHits)
 * 3. For each eligible track particle:
 *    a. Retrieves the associated Acts track via the ElementLink
 *    b. Passes the track to the ActsFitter tool for GSF refitting
 *    c. Stores the refitted track in the output Acts track container
 *
 */
class ActsEMBremCollectionBuilder : public AthReentrantAlgorithm {
 public:
  ActsEMBremCollectionBuilder(const std::string& name,
                              ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

 private:
  StatusCode refitActsTracks(
      const EventContext& ctx,
      const xAOD::TrackParticleContainer& input,
      ActsTrk::MutableTrackContainer& trackContainer,
      std::vector<const xAOD::TrackParticle*>& originals,
      const InDet::BeamSpotData* beamSpotData) const;

  StatusCode convertTracks(
      const EventContext& ctx,
      const ActsTrk::TrackContainer& actsContainer,
      const std::vector<const xAOD::TrackParticle*>& originals,
      const xAOD::TrackParticleContainer& originalTPs,
      xAOD::TrackParticleContainer& outputTPs,
      const InDet::BeamSpotData* beamSpotData) const;

  /** @Cut on minimum silicon hits*/
  Gaudi::Property<int> m_MinNoSiHits{this, "minNoSiHits", 4,
                                     "Minimum number of silicon hits on track "
                                     "before it is allowed to be refitted"};

  ToolHandle<ActsTrk::IFitterTool> m_actsFitter{this, "ActsFitter", "",
                                                "Acts Fitter"};

  ToolHandle<ActsTrk::ITrackToTrackParticleCnvTool> m_cnvTool{
      this, "TrackToTrackParticleCnvTool", "",
      "Tool to convert Acts tracks to xAOD::TrackParticles"};

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_actsTrackLinkKey{
      this, "ActsTrackLink", "actsTrack", "Link to Acts track"};

  /** @brief Names of input output collections */
  SG::ReadHandleKey<xAOD::TrackParticleContainer>
      m_selectedTrackParticleContainerKey{
          this, "SelectedTrackParticleContainerName",
          "egammaSelectedTrackParticles", "Input of Selected TrackParticles"};

  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};

  ActsTrk::MutableTrackContainerHandlesHelper m_refittedTracksBackendHandles{this};

  SG::WriteHandleKey<ActsTrk::TrackContainer> m_refittedTracksKey{
      this, "RefittedTracksLocation", "",
      "Ambiguity resolved output track collection"};

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey{
      this, "TrackParticleContainerName", "InDetTrackParticles",
      "Input TrackParticles for originalTrackParticle link"};

  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outputTrackParticlesKey{
      this, "TrackParticlesOutKey", "GSFTrackParticles",
      "Output xAOD::TrackParticle container"};

  SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_actsTrackOutLinkKey{
      this, "ActsTrackOutLink", "actsTrack",
      "Decoration: link from xAOD particle back to Acts track"};

  SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

  Gaudi::Property<bool> m_doTruth{this, "doTruth", false};
  Gaudi::Property<bool> m_doPix{this, "usePixel", true};
  Gaudi::Property<bool> m_doStrip{this, "useStrip", true};
  Gaudi::Property<bool> m_doHGTD{this, "useHGTD", false};
  
  mutable std::atomic_uint m_nInputTracks{0};
  mutable std::atomic_uint m_nRefittedTracks{0};
};
#endif  //
