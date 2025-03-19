/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This algorithm reads the aux-task outputs from a jet object and decorates the associated tracks with the same outputs.
  The mapping between the aux-task names for jets and tracks needs to be supplied in the job options.
*/

#ifndef GNN_AUXTASK_DECORATOR_ALG_H
#define GNN_AUXTASK_DECORATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"

#include "Gaudi/Property.h"

#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include <vector>

namespace FlavorTagDiscriminants {
  class GNNAuxTaskDecoratorAlg : public AthReentrantAlgorithm
  {
  public:
    GNNAuxTaskDecoratorAlg(const std::string& name, ISvcLocator* svcloc);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& cxt) const override;
    virtual StatusCode finalize() override;

  private:

    SG::ReadHandleKey<xAOD::JetContainer>  m_jetContainerKey {
      this, "jet_container", "AntiKt4EMPFlow", "Jet container name to read the aux-task outputs from"
    };
    SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_trackContainerKey {
      this, "track_container", "InDetTrackParticles", "Track container name to decorate with the aux-task outputs"
    };
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>  m_trackLinksKey {
      this, "track_links", m_jetContainerKey, "GN2v01_TrackLinks", "TrackLinks name associated with the jet object"
    };
    Gaudi::Property<std::map<std::string,std::string>>  m_trackAuxTasks {
      this, "track_aux_tasks", {}, "Map between aux-task decorations for jets and decorations for tracks"
    };

    SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_trackAuxTasksDecorKeys{this, "TrackDecorations", {}};
    SG::ReadDecorHandleKeyArray<xAOD::JetContainer> m_readDecorKeys{this, "JetDecorations", {}};
  };
}

#endif
