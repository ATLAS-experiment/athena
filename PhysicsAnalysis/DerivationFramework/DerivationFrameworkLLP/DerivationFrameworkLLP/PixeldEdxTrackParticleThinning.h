/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

/*
  The low pT track thinning tool for the dE/dx calibration.
*/

#ifndef DERIVATIONFRAMEWORK_PIXELDEDXTRACKPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_PIXELDEDXTRACKPARTICLETHINNING_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "GaudiKernel/ToolHandle.h"

#include <string>
#include <atomic>
#include <vector>
#include <mutex>

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

  class PixeldEdxTrackParticleThinning : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool> {
  public: 
    PixeldEdxTrackParticleThinning(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~PixeldEdxTrackParticleThinning() = default;
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
      this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };
    SG::ReadHandleKey< xAOD::VertexContainer > m_vertexContainerKey{
      this, "VertexContainerKey", "PrimaryVertices"}; 


    DoubleProperty m_unprescalePtCut {
      this, "UnprescalePtCut", 10.e3};
    DoubleProperty m_globalScale {
      this, "GlobalScale", 1.};
    DoubleProperty m_d0SignifCut {
      this, "d0SignifCut", 5.};
    DoubleProperty m_z0Cut {
      this, "z0Cut", 3.};
    DoubleProperty m_etaCut {
      this, "EtaCut", 2.};

    std::vector<double> m_pTbins;
    static const std::vector<double> m_preScales;
    
    //logic
    mutable std::vector<unsigned long long> m_counter ATLAS_THREAD_SAFE;
    mutable std::vector<unsigned long long> m_counter_picked ATLAS_THREAD_SAFE;
    mutable std::mutex m_mutex;

  }; 
}

#endif // DERIVATIONFRAMEWORK_PIXELDEDXTRACKPARTICLETHINNING_H
