/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMAALGS_ACTSEGAMMASELECTEDTRACKCOPY_H
#define EGAMMAALGS_ACTSEGAMMASELECTEDTRACKCOPY_H

#include "egammaInterfaces/IEMExtrapolationTools.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"

#include "AthContainers/ConstDataVector.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/TrackParticleFwd.h"


#include "egammaInterfaces/IegammaCaloClusterSelector.h"
#include <Gaudi/Accumulators.h>

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

/**
  @class ActsEgammaSelectedTrackCopy
  ACTS-specific algorithm for selecting tracks to be GSF refitted.

  This is a variant of egammaSelectedTrackCopy with modified selection logic
  for ACTS track extrapolation to the calorimeter.

  - Input container xAOD::CaloClusterContainer: ClusterContainerName
  - Input container xAOD::TrackParticleContainer: TrackParticleContainerName
  - Output container xAOD::TrackParticleContainer: OutputTrkPartContainerName

  */
class ActsEgammaSelectedTrackCopy : public AthReentrantAlgorithm
{
public:
  /** @brief Default constructor. */
  ActsEgammaSelectedTrackCopy(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ActsEgammaSelectedTrackCopy() override = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

  constexpr static double s_calorimeterEtaCoverage = 1.5;
  constexpr static double s_maxExtrapolationRadius = 2250.0;

private:
  /** @brief Track selection method. */
  bool matchWithExtrapolation(const EventContext& ctx,
                              const xAOD::CaloCluster& cluster,
                              const xAOD::TrackParticle& track,
                              const std::shared_ptr<const Acts::Surface>& perigeeSurface) const;

  bool checkBroadCriteria(const xAOD::CaloCluster& cluster,
                          const xAOD::TrackParticle& track) const;

  struct CaloMatch {
    int layer;
    double eta, phi, deltaEta, deltaPhi;
  };

  std::array<std::optional<CaloMatch>, 4> extrapolateToCalo(
    const Acts::BoundTrackParameters &parameters,
    const xAOD::CaloCluster& cluster,
    const EventContext& ctx) const;

 ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

  ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool {
    this,
    "ExtrapolationTool",
    "ExtrapolationTool",
    "Tool to run propagation in an ACTS trackign geometry"
  };

  /** @brief Tool to filter the calo clusters. */
  ToolHandle<IegammaCaloClusterSelector> m_egammaCaloClusterSelector {
    this,
    "egammaCaloClusterSelector",
    "egammaCaloClusterSelector",
    "Tool that makes the cluster selection"
  };

  /** @brief Names of input output collections. */
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterContainerKey {
    this,
    "ClusterContainerName",
    "egammaTopoCluster",
    "Input calo cluster for seeding"
  };

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleContainerKey {
    this,
    "TrackParticleContainerName",
    "InDetTrackParticles",
    "Input TrackParticles to select from"
  };

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_trackParticleTimeDecorKey{
    this,
    "TrackParticleTimeDecoration",
    "",
    "Time assigned to this track"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey {
    this,
    "CaloDetDescrManager",
    "CaloDetDescrManager",
    "SG Key for CaloDetDescrManager in the Condition Store"
  };

  SG::WriteHandleKey<ConstDataVector<xAOD::TrackParticleContainer>> m_OutputTrkPartContainerKey {
    this,
    "OutputTrkPartContainerName",
    "ActsEgammaSelectedTrackParticles",
    "Output selected TrackParticles"
  };

  /** @brief Broad windows. */
  Gaudi::Property<double> m_broadDeltaEta {
    this,
    "broadDeltaEta",
    0.2,
    "Value of broad cut for delta eta"
  };

  Gaudi::Property<double> m_broadDeltaPhi {
    this,
    "broadDeltaPhi",
    0.3,
    "Value of broad cut for delta phi"
  };

  /** @brief Narrow windows. */
  Gaudi::Property<double> m_narrowDeltaEta{
    this,
    "narrowDeltaEta",
    0.05,
    "Value of narrow cut for delta eta"
  };

  Gaudi::Property<double> m_narrowDeltaPhi{
    this,
    "narrowDeltaPhi",
    0.05,
    "Value of narrow cut for delta phi"
  };

  Gaudi::Property<double> m_narrowDeltaPhiBrem{
    this,
    "narrowDeltaPhiBrem",
    0.2,
    "Value of the narrow cut for delta phi in the brem direction"
  };

  Gaudi::Property<double> m_narrowRescale{
    this,
    "narrowDeltaPhiRescale",
    0.05,
    "Value of the narrow cut for delta phi Rescale"
  };

  Gaudi::Property<double> m_narrowRescaleBrem{
    this,
    "narrowDeltaPhiRescaleBrem",
    0.1,
    "Value of the narrow cut for delta phi Rescale Brem"
  };

  std::array<Acts::GeometryIdentifier, 4> m_barrelCaloGeoIds;

  mutable Gaudi::Accumulators::Counter<> m_AllClusters {};
  mutable Gaudi::Accumulators::Counter<> m_SelectedClusters {};
  mutable Gaudi::Accumulators::Counter<> m_AllTracks {};
  mutable Gaudi::Accumulators::Counter<> m_SelectedTracks {};
};
#endif
