/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGJETDECORATORS_JET_SCALAR_CAST_ALG_H
#define FLAVORTAGJETDECORATORS_JET_SCALAR_CAST_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODJet/JetContainer.h"

namespace FlavorTagJetDecorators {

  /// Read a single float jet decoration and write a bf16-truncated copy.
  ///
  /// bfloat16 truncation uses mantissa bit-masking:
  ///   bits = reinterpret_cast<uint32>(value)
  ///   bits &= ~((1 << (23 - man_bits)) - 1)
  ///   return reinterpret_cast<float>(bits)
  ///
  /// This preserves the float32 exponent range (exp_bits must equal 8).
  /// Truncation is round-toward-zero (same as existing VECBF16 C++ path).
  ///
  /// Primary use-case: cast EnergyPerSampling scalar components
  /// (produced by VectorExploderAlg) to bf16 for DAOD storage.
  class JetScalarCastAlg : public AthReentrantAlgorithm {
  public:
    JetScalarCastAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    /// Input jet collection.
    SG::ReadHandleKey<xAOD::JetContainer> m_collectionKey{
      this, "Collection", "AntiKt4EMPFlowJets",
      "Input jet collection"};

    /// Name of the input scalar float decoration (bare name, no container prefix).
    Gaudi::Property<std::string> m_inputDecor{
      this, "InputDecor", "",
      "Input scalar decoration name (e.g. 'e_PreSamplerB')"};

    /// Name of the output decoration (bare name, no container prefix).
    Gaudi::Property<std::string> m_outputDecor{
      this, "OutputDecor", "",
      "Output decoration name (e.g. 'ft1l_jet_e_PreSamplerB_bf16')"};

    /// Number of exponent bits in the target float format.
    /// Must equal 8 (float32 exponent size); validated in initialize().
    Gaudi::Property<int> m_expBits{
      this, "ExpBits", 8,
      "Number of exponent bits (must be 8 for float32 exponent range)"};

    /// Number of mantissa bits to KEEP in the truncated value.
    /// Default 7 gives bfloat16 (8 exp + 7 man + 1 sign = 16 bits total).
    Gaudi::Property<int> m_manBits{
      this, "ManBits", 7,
      "Number of mantissa bits to keep (7 = bf16)"};

    /// Initialized read/write decor handle keys.
    SG::ReadDecorHandleKey<xAOD::JetContainer>  m_inputKey{};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_outputKey{};
  };

}  // namespace FlavorTagJetDecorators

#endif  // FLAVORTAGJETDECORATORS_JET_SCALAR_CAST_ALG_H
