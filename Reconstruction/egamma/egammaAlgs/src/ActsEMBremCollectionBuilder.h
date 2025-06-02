/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMAALGS_ACTSEMBREMCOLLECTIONBUILDER_H
#define EGAMMAALGS_ACTSEMBREMCOLLECTIONBUILDER_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/TrackParticleFwd.h"

#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
/**
 * @class ActsEMBremCollectionBuilder
 * @brief Algorithm which refits Acts tracks using the ACTS GSF.
 * Input: xAOD::TrackParticleContainer
 * Output: ActsTrk::TrackContainer
 * This algorithm stores Acts tracks. Output xAOD::TrackParticles will be
 * created by TrackToTrackParticleCnvAlg.
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
      const std::vector<const xAOD::TrackParticle*>& input,
      ActsTrk::MutableTrackContainer& trackContainer) const;

  /** @Cut on minimum silicon hits*/
  Gaudi::Property<int> m_MinNoSiHits{this, "minNoSiHits", 4,
                                     "Minimum number of silicon hits on track "
                                     "before it is allowed to be refitted"};

  ToolHandle<ActsTrk::IFitterTool> m_actsFitter{this, "ActsFitter", "",
                                                "Acts Fitter"};

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_actsTrackLinkKey{
      this, "ActsTrackLink", "actsTrack", "Link to Acts track"};

  /** @brief Names of input output collections */
  SG::ReadHandleKey<xAOD::TrackParticleContainer>
      m_selectedTrackParticleContainerKey{
          this, "SelectedTrackParticleContainerName",
          "egammaSelectedTrackParticles", "Input of Selected TrackParticles"};

  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{
      this, "TrackingGeometryTool", ""};

  ActsTrk::MutableTrackContainerHandlesHelper m_refittedTracksBackendHandles;

  SG::WriteHandleKey<ActsTrk::TrackContainer> m_refittedTracksKey{
      this, "RefittedTracksLocation", "",
      "Ambiguity resolved output track collection"};

  mutable std::atomic_uint m_nInputTracks{0};
  mutable std::atomic_uint m_nRefittedTracks{0};
};
#endif  //
