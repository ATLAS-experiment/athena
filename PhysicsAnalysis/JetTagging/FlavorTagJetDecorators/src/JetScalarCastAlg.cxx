/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
 * @file JetScalarCastAlg.cxx
 * @brief Algorithm to cast a scalar float jet decoration to a
 *        reduced-precision float (e.g. bfloat16) via mantissa
 *        bit-masking, and write the result as a new decoration.
 *
 * The cast is parametric: {ExpBits, ManBits} specify the target
 * format.  ExpBits must equal 8 (float32 exponent; can't reduce
 * exponent range without re-encoding NaN/Inf conventions).
 * Default ManBits=7 gives bfloat16.
 *
 * Truncation is round-toward-zero (same as VECBF16 path in the
 * GNN constituent framework, validated bit-identical in sub-exp 09
 * of Experiment 02 storage_type_comparison).
 ****************************************************************/

#include "JetScalarCastAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include <bit>
#include <cstdint>

namespace FlavorTagJetDecorators {

namespace {

/// Truncate a float32 value by masking the low (23 - man_bits) mantissa bits.
/// Preserves sign and exponent (exponent bits are always 8 for float32).
inline float bf16_cast(float v, int man_bits) {
  uint32_t bits = std::bit_cast<uint32_t>(v);
  uint32_t mask = ~((1u << (23 - man_bits)) - 1u);
  return std::bit_cast<float>(bits & mask);
}

}  // anonymous namespace

JetScalarCastAlg::JetScalarCastAlg(const std::string& name,
                                    ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode JetScalarCastAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  if (m_inputDecor.value().empty()) {
    ATH_MSG_ERROR("InputDecor property must be set (e.g. 'e_PreSamplerB').");
    return StatusCode::FAILURE;
  }
  if (m_outputDecor.value().empty()) {
    ATH_MSG_ERROR("OutputDecor property must be set "
                  "(e.g. 'ft1l_jet_e_PreSamplerB_bf16').");
    return StatusCode::FAILURE;
  }
  if (m_expBits.value() != 8) {
    ATH_MSG_ERROR("ExpBits must equal 8 (float32 has 8 exponent bits; "
                  "reducing the exponent range requires re-encoding NaN/Inf "
                  "conventions which this algorithm does not implement).");
    return StatusCode::FAILURE;
  }
  if (m_manBits.value() < 0 || m_manBits.value() > 23) {
    ATH_MSG_ERROR("ManBits must be in [0, 23]; got " << m_manBits.value());
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_collectionKey.initialize());

  const std::string collKey = m_collectionKey.key();
  m_inputKey  = SG::ReadDecorHandleKey<xAOD::JetContainer>(
                    collKey + "." + m_inputDecor.value());
  m_outputKey = SG::WriteDecorHandleKey<xAOD::JetContainer>(
                    collKey + "." + m_outputDecor.value());

  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  ATH_MSG_INFO("  collection  : " << collKey);
  ATH_MSG_INFO("  input decor : " << m_inputDecor.value());
  ATH_MSG_INFO("  output decor: " << m_outputDecor.value());
  ATH_MSG_INFO("  exp bits    : " << m_expBits.value());
  ATH_MSG_INFO("  man bits    : " << m_manBits.value());

  return StatusCode::SUCCESS;
}

StatusCode JetScalarCastAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << "...");

  SG::ReadHandle<xAOD::JetContainer> jets(m_collectionKey, ctx);
  if (!jets.isValid()) {
    ATH_MSG_ERROR("Could not retrieve jet collection '"
                  << m_collectionKey.key() << "'");
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::JetContainer, float>  inHandle(m_inputKey,  ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, float> outHandle(m_outputKey, ctx);

  if (!inHandle.isAvailable()) {
    ATH_MSG_ERROR("Input decoration '" << m_inputKey.key()
                  << "' not available.");
    return StatusCode::FAILURE;
  }

  const int manBits = m_manBits.value();
  for (const xAOD::Jet* jet : *jets) {
    outHandle(*jet) = bf16_cast(inHandle(*jet), manBits);
  }

  return StatusCode::SUCCESS;
}

}  // namespace FlavorTagJetDecorators
