/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGJETDECORATORS_VECTOR_EXPLODER_ALG_H
#define FLAVORTAGJETDECORATORS_VECTOR_EXPLODER_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"

#include <map>
#include <string>
#include <vector>
#include <utility>

namespace FlavorTagJetDecorators {

  /// Explode a vector<float> jet decoration into multiple scalar decorations.
  ///
  /// Ported from TDD's VectorExploderAlg (FTagDumper/src/VectorExploderAlg.{h,cxx}).
  /// Primary use-case: split EnergyPerSampling / EnergyPerSamplingCaloBased
  /// (each vector<float> of length 28) into 28 individual float decorations
  /// per jet, persisted directly as float32 jet decorations.
  ///
  /// Differences from TDD version:
  ///   - Namespace FlavorTagJetDecorators:: (TDD uses global namespace).
  ///   - Hardcoded xAOD::JetContainer (TDD templates on IParticleContainer).
  ///   - Default collection "AntiKt4EMPFlowJets" (TDD uses "").

  class VectorExploderAlg : public AthReentrantAlgorithm {
  public:
    VectorExploderAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    /// Input jet collection.
    SG::ReadHandleKey<xAOD::JetContainer> m_collectionKey{
      this, "Collection", "AntiKt4EMPFlowJets",
      "Input jet collection to read decorations from"};

    /// Name of the input vector<float> decoration to explode.
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_inputVectorKey{
      this, "InputVectorName", m_collectionKey, "",
      "Name of the input vector decoration to explode"};

    /// Mapping from vector index to output scalar decoration name.
    Gaudi::Property<std::map<int, std::string>> m_outputNamesMap{
      this, "OutputNamesMap", {},
      "Mapping between indices and names for the output scalar decorations"};

    /// Initialized output keys (index, WriteDecorHandleKey) pairs.
    /// Reserve before filling to avoid pointer invalidation.
    std::vector<std::pair<int, SG::WriteDecorHandleKey<xAOD::JetContainer>>> m_outputKeys;

    /// Maximum index used in OutputNamesMap (for bounds check).
    int m_maxIndex{-1};
  };

}  // namespace FlavorTagJetDecorators

#endif  // FLAVORTAGJETDECORATORS_VECTOR_EXPLODER_ALG_H
