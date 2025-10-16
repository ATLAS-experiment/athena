/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    TrackParticleCaloCellDecorator(const std::string& t, const std::string& n, const IInterface* p);
    virtual StatusCode initialize() override;
    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    StringProperty m_sgName {
      this, "DecorationPrefix", "", "decoration prefix"};
    SG::ReadHandleKey< xAOD::TrackParticleClusterAssociationContainer > m_trackContainerKey{
      this, "ContainerName", "", "track particle container name"};

    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellEtaKey{
      this, "decCellEtaKey", "_CaloCellEta"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellPhiKey{
      this, "decCellPhiKey", "_CaloCellPhi"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellRKey{
      this, "decCellRKey", "_CaloCellR"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldEtaKey{
      this, "decCelldEtaKey", "_CaloCelldEta"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldPhiKey{
      this, "decCelldPhiKey", "_CaloCelldPhi"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldRKey{
      this, "decCelldRKey", "_CaloCelldR"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellXKey{
      this, "decCellXKey", "_CaloCellX"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellYKey{
      this, "decCellYKey", "_CaloCellY"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellZKey{
      this, "decCellZKey", "_CaloCellZ"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldXKey{
      this, "decCelldXKey", "_CaloCelldX"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldYKey{
      this, "decCelldYKey", "_CaloCelldY"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCelldZKey{
      this, "decCelldZKey", "_CaloCelldZ"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellTKey{
      this, "decCellTKey", "_CaloCellTime"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellEKey{
      this, "decCellEKey", "_CaloCellE"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellIDKey{
      this, "decCellIDKey", "_CaloCellID"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellSamplingKey{
      this, "decCellSamplingKey", "_CaloCellSampling"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellQualityKey{
      this, "decCellQualityKey", "_CaloCellQuality"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellProvenanceKey{
      this, "decCellProvenanceKey", "_CaloCellProvenance"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellGainKey{
      this, "decCellGainKey", "_CaloCellGain"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellEneDiffKey{
      this, "decCellEneDiffKey", "_CaloCellEneDiff"};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_decCellTimeDiffKey{
      this, "decCellTimeDiffKey", "_CaloCellTimeDiff"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRACKPARTICLECALOCELLDECORATOR_H
