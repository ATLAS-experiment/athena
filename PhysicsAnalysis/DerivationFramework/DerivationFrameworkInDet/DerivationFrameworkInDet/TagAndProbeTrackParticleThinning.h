/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TagAndProbeTrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

/*
  The tag & probe track thinning tool for SCT DxAOD.
*/

#ifndef DERIVATIONFRAMEWORK_TAGANDPROBETRACKPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_TAGANDPROBETRACKPARTICLETHINNING_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODMuon/MuonContainer.h"

#include <string>
#include <atomic>
#include <vector>
#include <mutex>

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

  class TagAndProbeTrackParticleThinning : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool> {
  public: 
    TagAndProbeTrackParticleThinning(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~TagAndProbeTrackParticleThinning() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode doThinning() const override;

  private:

    StringProperty m_selectionString {
      this, "SelectionString", "", "track selections"};

    //Counters and keys for xAOD::TrackParticle container

    mutable std::atomic<unsigned int> m_ntot{0}, m_npass{0};

    StringProperty m_streamName {
      this, "StreamName", "", "Name of the stream being thinned" };
    SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_inDetParticlesKey {
      this, "InDetTrackParticlesKey", "InDetTrackParticles", ""};
    SG::ReadHandleKey< xAOD::VertexContainer > m_vertexContainerKey{
      this, "VertexContainerKey", "PrimaryVertices"}; 
    SG::ReadHandleKey< xAOD::MuonContainer > m_muonContainerKey{
      this, "MuonContainerKey", "Muons"}; 


    DoubleProperty m_d0SignifCut {
      this, "d0SignifCut", 5.};
    DoubleProperty m_z0Cut {
      this, "z0Cut", 10.};
    DoubleProperty m_massCut {
      this, "MassCut", 30.e3};

    //logic
    mutable std::mutex m_mutex;

  }; 
}

#endif // DERIVATIONFRAMEWORK_TAGANDPROBETRACKPARTICLETHINNING_H
