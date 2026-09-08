/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_PVZ_RELATIVE_TO_BEAMSPOT_ALG_HH
#define TRUTH_PVZ_RELATIVE_TO_BEAMSPOT_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODBase/IParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"

// Read and write handle keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace FlavorTagDiscriminants {

  class TruthPVzRelativeToBeamspotAlg: public AthReentrantAlgorithm {
    /** @name TruthPVzRelativeToBeamspotAlg
     *  @brief Re-express the truth primary-vertex z of each jet in beamspot
     *         coordinates, so that it can be compared directly with hit
     *         positions from a HitDecoratorAlg configured with an EventInfo
     *         key. A non-finite input is passed through as NaN.
     */

    public:
      TruthPVzRelativeToBeamspotAlg(const std::string& name,
                                    ISvcLocator* pSvcLocator);

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

    private:
      SG::ReadHandleKey<xAOD::IParticleContainer> m_jetCollectionKey {
        this, "jetContainer", "AntiKt4EMPFlowJets", "Key for the jet collection"};

      SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_truthPVzKey {
        this, "truthPVzDecor", m_jetCollectionKey, "TruthJetPVz",
        "Jet decoration holding the truth primary-vertex z, in detector coordinates"};

      SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_outputKey {
        this, "truthPVzRelToBeamspotDecor", m_jetCollectionKey, "TruthJetPVzRelToBeamspot",
        "Key for the output truth primary-vertex z, relative to the beamspot"};

      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
        this, "eventInfo", "EventInfo", "Key for EventInfo, providing the beamspot position"};
  };
}

#endif
