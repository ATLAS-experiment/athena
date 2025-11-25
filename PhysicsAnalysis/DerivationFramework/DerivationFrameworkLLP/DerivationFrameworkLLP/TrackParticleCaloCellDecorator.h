/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*  TrackParticleCaloCellDecorator.h  */
/*  Decorates the InDetTrackParticles Container with calorimeter  */
/*  cell information.   */

#ifndef DERIVATIONFRAMEWORK_TRACKPARTICLECALOCELLDECORATOR_H
#define DERIVATIONFRAMEWORK_TRACKPARTICLECALOCELLDECORATOR_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODAssociations/TrackParticleClusterAssociationContainer.h"

namespace DerivationFramework {

  class TrackParticleCaloCellDecorator : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_trackParticleContainerKey{
      this, "ContainerName", "", "track particle container name"};

    SG::ReadHandleKey< xAOD::TrackParticleClusterAssociationContainer > m_trackContainerKey{
      this, "ClusterAssocContainerName", "", "track particle container name"};

    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellEtaKey{
      this, "decCellEtaKey", m_trackParticleContainerKey, "_CaloCellEta"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellPhiKey{
      this, "decCellPhiKey", m_trackParticleContainerKey, "_CaloCellPhi"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellRKey{
      this, "decCellRKey", m_trackParticleContainerKey, "_CaloCellR"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldEtaKey{
      this, "decCelldEtaKey", m_trackParticleContainerKey, "_CaloCelldEta"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldPhiKey{
      this, "decCelldPhiKey", m_trackParticleContainerKey, "_CaloCelldPhi"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldRKey{
      this, "decCelldRKey", m_trackParticleContainerKey, "_CaloCelldR"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellXKey{
      this, "decCellXKey", m_trackParticleContainerKey, "_CaloCellX"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellYKey{
      this, "decCellYKey", m_trackParticleContainerKey, "_CaloCellY"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellZKey{
      this, "decCellZKey", m_trackParticleContainerKey, "_CaloCellZ"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldXKey{
      this, "decCelldXKey", m_trackParticleContainerKey, "_CaloCelldX"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldYKey{
      this, "decCelldYKey", m_trackParticleContainerKey, "_CaloCelldY"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldZKey{
      this, "decCelldZKey", m_trackParticleContainerKey, "_CaloCelldZ"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellTKey{
      this, "decCellTKey", m_trackParticleContainerKey, "_CaloCellTime"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellEKey{
      this, "decCellEKey", m_trackParticleContainerKey, "_CaloCellE"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellIDKey{
      this, "decCellIDKey", m_trackParticleContainerKey, "_CaloCellID"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellSamplingKey{
      this, "decCellSamplingKey", m_trackParticleContainerKey, "_CaloCellSampling"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellQualityKey{
      this, "decCellQualityKey", m_trackParticleContainerKey, "_CaloCellQuality"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellProvenanceKey{
      this, "decCellProvenanceKey", m_trackParticleContainerKey, "_CaloCellProvenance"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellGainKey{
      this, "decCellGainKey", m_trackParticleContainerKey, "_CaloCellGain"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellEneDiffKey{
      this, "decCellEneDiffKey", m_trackParticleContainerKey, "_CaloCellEneDiff"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellTimeDiffKey{
      this, "decCellTimeDiffKey", m_trackParticleContainerKey, "_CaloCellTimeDiff"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRACKPARTICLECALOCELLDECORATOR_H
