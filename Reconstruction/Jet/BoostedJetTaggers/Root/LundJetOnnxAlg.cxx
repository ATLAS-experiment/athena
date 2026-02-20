/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAOD_ANALYSIS
#include "BoostedJetTaggers/LundJetOnnxAlg.h"

// xAOD and StoreGate
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandle.h"
#include "xAODPFlow/FlowElement.h"
#include "AthOnnxUtils/OnnxUtils.h"
// AthOnnx
#include "AthOnnxInterfaces/IAthInferenceTool.h"

// ONNX Runtime
#include <onnxruntime_cxx_api.h>
#include "PathResolver/PathResolver.h"
#include "StoreGate/WriteDecorHandle.h"

#include <sstream>
#include <cmath>

//static const char* k_input_names[]  = {"x","edge_index","batch","Ntrk","counts"};
//static const char* k_output_names[] = {"output"};
StatusCode LundJetOnnxAlg::initialize() {

  ATH_MSG_INFO("Initializing LundJetOnnxAlg");

  ATH_MSG_INFO("Loading ONNX model from: " << m_modelPath);
  // -------------------------
  // Create ONNX Runtime session
  // -------------------------
  Ort::SessionOptions sessionOptions;
  sessionOptions.SetIntraOpNumThreads(1);
  sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

  // Recommended in Athena to reduce memory usage
  sessionOptions.DisableCpuMemArena();

  m_env = std::make_unique<Ort::Env>(
    ORT_LOGGING_LEVEL_WARNING,
    "LundNetGNN"
  );
  m_resolvedModelPath =
    PathResolver::find_file(m_modelPath, "DATAPATH");

  if (m_resolvedModelPath.empty()) {
    ATH_MSG_ERROR("Could not resolve ONNX model path: " << m_modelPath);
    return StatusCode::FAILURE;
  }
  
  try {
    m_session = std::make_unique<Ort::Session>(
					       *m_env,
					       m_resolvedModelPath.c_str(),   // ← ahora es estable
					       sessionOptions
					       );
  } catch (const Ort::Exception& e) {
    ATH_MSG_ERROR("Failed to create ONNX Runtime session: " << e.what());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("ONNX Runtime session successfully created");

  // -------------------------
  // Decorations
  // -------------------------
  ATH_MSG_INFO("InputJetContainer: " << m_inputJetContainer);
  ATH_MSG_INFO("Prefix: '" << m_prefix << "'");
  ATH_MSG_INFO("kT selection: " << m_kTSelection);
  std::string decorFull =
    m_inputJetContainer.value() + "." +
    m_prefix.value() +
    m_scoreName.value();
  std::string validDecorFull =
    m_inputJetContainer.value() + "." +
    m_prefix.value() +
    "LundNetValid";
  
  m_scoreDecorKey = decorFull;
  m_validDecorKey = validDecorFull;
  ATH_CHECK(m_scoreDecorKey.initialize());
  ATH_CHECK(m_validDecorKey.initialize());
  ATH_MSG_INFO("Will write decoration: " << decorFull);

  return StatusCode::SUCCESS;
}

StatusCode LundJetOnnxAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::JetContainer> jets(m_inputJetContainer, ctx);
  if (!jets.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve JetContainer: " << m_inputJetContainer);
    return StatusCode::FAILURE;
  }

  SG::WriteDecorHandle<xAOD::JetContainer, float> scoreDecor(m_scoreDecorKey, ctx);
  SG::WriteDecorHandle<xAOD::JetContainer, char> validDecor(m_validDecorKey, ctx);
  // ONNX helpers
  Ort::AllocatorWithDefaultOptions allocator;
  auto memory_info =
    Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  for (const xAOD::Jet* jptr : *jets) {

    const xAOD::Jet& jet = *jptr;

    validDecor(*jptr) = 0;
    // ---------------------------
    // Build inputs
    // ---------------------------
    std::vector<float>   x_flat;              // [num_nodes * 3]
    std::vector<int64_t> edge_index_flat;     // [2 * num_edges]
    std::vector<int64_t> batch_vec;            // [num_nodes]
    std::vector<int64_t> counts_vec;           // unused but kept
    std::vector<float>   Ntrk_vec;             // [1]

    bool ok = buildOnnxInputs(
			      jet, x_flat, edge_index_flat, batch_vec, counts_vec, Ntrk_vec
			      );

    if (!ok) {
      ATH_MSG_DEBUG("Skipping jet: couldn't build inputs");
      continue;
    }

    const int64_t num_nodes = batch_vec.size();

    if (num_nodes == 0 || x_flat.size() != static_cast<size_t>(num_nodes * 3)) {
      ATH_MSG_WARNING("Inconsistent node inputs");
      continue;
    }

    // ---------------------------
    // Create ONNX tensors
    // ---------------------------
    const int64_t min_nodes = 2;

    if (num_nodes < min_nodes) {
      ATH_MSG_DEBUG("Skipping jet: num_nodes < 2");
      continue;
    }
    
    const int64_t num_edges = edge_index_flat.size() / 2;
    if (num_edges < 1) {
      ATH_MSG_DEBUG("Skipping jet: no edges in graph");
      continue;
    }
    
    // x : [num_nodes, 3]
    std::vector<int64_t> x_shape = { num_nodes, 3 };
    Ort::Value x_tensor =
      Ort::Value::CreateTensor<float>(
				      memory_info,
				      x_flat.data(),
				      x_flat.size(),
				      x_shape.data(),
				      x_shape.size()
				      );

    // edge_index : [2, num_edges]
    std::vector<int64_t> ei_shape = {
      2,
      static_cast<int64_t>(edge_index_flat.size() / 2)
    };
    Ort::Value edge_tensor =
      Ort::Value::CreateTensor<int64_t>(
					memory_info,
					edge_index_flat.data(),
					edge_index_flat.size(),
					ei_shape.data(),
					ei_shape.size()
					);

    // batch : [num_nodes]
    std::vector<int64_t> batch_shape = { num_nodes };
    Ort::Value batch_tensor =
      Ort::Value::CreateTensor<int64_t>(
					memory_info,
					batch_vec.data(),
					batch_vec.size(),
					batch_shape.data(),
					batch_shape.size()
					);

    // Ntrk : [batch_size]
    std::vector<int64_t> ntrk_shape = {static_cast<int64_t>(Ntrk_vec.size())};
    Ort::Value ntrk_tensor =
      Ort::Value::CreateTensor<float>(
				      memory_info,
				      Ntrk_vec.data(),
				      Ntrk_vec.size(),
				      ntrk_shape.data(),
				      ntrk_shape.size()
				      );
    std::array<Ort::Value, 4> input_tensors = {
      std::move(x_tensor),
      std::move(edge_tensor),
      std::move(batch_tensor),
      std::move(ntrk_tensor)
    };

    std::array<const char*, 4> input_names = {
      "x",
      "edge_index",
      "batch",
      "Ntrk"
    };

    std::array<const char*, 1> output_names = {
      "output"
    };

    // ---------------------------
    // Run inference
    // ---------------------------
    std::vector<Ort::Value> output_tensors;
    try {
      output_tensors = m_session->Run(
				      Ort::RunOptions{nullptr},
				      input_names.data(),
				      input_tensors.data(),
				      input_tensors.size(),
				      output_names.data(),
				      output_names.size()
				      );
    }
    catch (const Ort::Exception& e) {
      ATH_MSG_ERROR("ONNX Runtime exception: " << e.what());
      continue;
    }

    // ---------------------------
    // Read output
    // ---------------------------
    if (output_tensors.empty() || !output_tensors.front().IsTensor()) {
      ATH_MSG_WARNING("Invalid output tensor");
      //scoreDecor(*jptr) = -999.f;
      continue;
    }

    float* out_data =
      output_tensors.front().GetTensorMutableData<float>();

    float score = out_data[0]; // shape [-1,1], batch=1

    scoreDecor(*jptr) = score;
    validDecor(*jptr) = 1;
    
    ATH_MSG_DEBUG("Jet decorated with LundNet score = " << score);
  }

  return StatusCode::SUCCESS;
}
bool LundJetOnnxAlg::buildOnnxInputs(const xAOD::Jet& jet,
                                     std::vector<float>& out_x_float,
                                     std::vector<int64_t>& out_edge_index_int64,
                                     std::vector<int64_t>& out_batch_int64,
                                     std::vector<int64_t>& out_counts_int64,
                                     std::vector<float>& out_Ntrk_float) const {
  // Clear outputs
  out_x_float.clear();
  out_edge_index_int64.clear();
  out_batch_int64.clear();
  out_counts_int64.clear();
  out_Ntrk_float.clear();

  auto tryGetVectorFloat = [&](const std::string& baseName, std::vector<float>& out)->bool {
    std::string withPref = m_prefix.value() + baseName;
    if (jet.getAttribute(withPref, out)) {
      ATH_MSG_DEBUG("Found attribute: " << withPref << " (used)");
      return true;
    }
    if (jet.getAttribute(baseName, out)) {
      ATH_MSG_DEBUG("Found attribute: " << baseName << " (used)");
      return true;
    }
    ATH_MSG_DEBUG("Attribute not found: " << withPref << " nor " << baseName);
    return false;
  };

  auto tryGetVectorInt = [&](const std::string& baseName, std::vector<int>& out)->bool {
    std::string withPref = m_prefix.value() + baseName;
    if (jet.getAttribute(withPref, out)) {
      ATH_MSG_DEBUG("Found attribute: " << withPref << " (used)");
      return true;
    }
    if (jet.getAttribute(baseName, out)) {
      ATH_MSG_DEBUG("Found attribute: " << baseName << " (used)");
      return true;
    }
    ATH_MSG_DEBUG("Attribute not found: " << withPref << " nor " << baseName);
    return false;
  };

  auto tryGetInt = [&](const std::string& baseName, int& out)->bool {
    std::string withPref = m_prefix.value() + baseName;
    if (jet.getAttribute(withPref, out)) {
      ATH_MSG_DEBUG("Found attribute: " << withPref << " (used)");
      return true;
    }
    if (jet.getAttribute(baseName, out)) {
      ATH_MSG_DEBUG("Found attribute: " << baseName << " (used)");
      return true;
    }
    ATH_MSG_DEBUG("Attribute not found: " << withPref << " nor " << baseName);
    return false;
  };

  // Read Lund decorations (try prefixed and unprefixed names)
  std::vector<float> lnR, lnkT, z;
  std::vector<int> idp1, idp2;
  int nSplits = 0;

  bool ok_lnR  = tryGetVectorFloat("LundAllLnR", lnR);
  bool ok_lnkT = tryGetVectorFloat("LundAllLnKT", lnkT);
  bool ok_z    = tryGetVectorFloat("LundAllZ", z);
  bool ok_idp1 = tryGetVectorInt("LundAllIDP1", idp1);
  bool ok_idp2 = tryGetVectorInt("LundAllIDP2", idp2);
  bool ok_nsp  = tryGetInt("nSplits", nSplits);

  if (!(ok_lnR && ok_lnkT && ok_z && ok_idp1 && ok_idp2 && ok_nsp)) {
    ATH_MSG_DEBUG("Missing one or more Lund decorations (lnR/lnkT/z/idp1/idp2/nSplits). Aborting build.");
    return false;
  }

  size_t n_nodes = lnR.size();
  if (n_nodes == 0) {
    ATH_MSG_DEBUG("Lund decorations present but zero-length vectors.");
    return false;
  }

  // Determine Ntrk: try various attribute names (try prefixed first, then unprefixed)
  float ntrk_f = 0.f;
  int tmp_ntrk_i = 0;
  bool gotNtrk = false;
  // Candidate names commonly used
  std::vector<std::string> ntrkCandidates = { "LRJ_Nconst_Charged", "nTrk", "Ntrk", "NTracks" };
  for (auto &cand : ntrkCandidates) {
    std::string withPref = m_prefix.value() + cand;
    if (jet.getAttribute(withPref, tmp_ntrk_i)) { ntrk_f = static_cast<float>(tmp_ntrk_i); gotNtrk = true; ATH_MSG_DEBUG("Using Ntrk attr: " << withPref); break; }
    if (jet.getAttribute(cand, tmp_ntrk_i)) { ntrk_f = static_cast<float>(tmp_ntrk_i); gotNtrk = true; ATH_MSG_DEBUG("Using Ntrk attr: " << cand); break; }
  }
  if (!gotNtrk) {
    const auto & links = jet.constituentLinks();
    for (size_t i = 0; i < jet.numConstituents(); ++i) {
      const xAOD::IParticle* p = *links[i];
      const xAOD::FlowElement* fe = dynamic_cast<const xAOD::FlowElement*>(p);
      if (fe && fe->isCharged()) ntrk_f += 1.f;
    }
    ATH_MSG_DEBUG("Computed Ntrk from constituents: " << ntrk_f);
  }

  // Build node mask based on kTSelection.
  // m_kTSelection is a Gaudi Property<float> — get the value for comparison.
  float kTsel_val = m_kTSelection;
  float ln_kTcut = (kTsel_val > 0.f) ? std::log(std::max(1e-12f, kTsel_val)) : -1e9f;

  std::vector<char> mask(n_nodes, 0);
  size_t n_selected = 0;
  for (size_t i = 0; i < n_nodes; ++i) {
    float lnk = lnkT[i];
    if (lnk > ln_kTcut) { mask[i] = 1; ++n_selected; }
    else mask[i] = 0;
  }

  if (n_selected < 1) {
    ATH_MSG_DEBUG("No nodes passed kT selection (n_selected=" << n_selected << ").");
    return false;
  }

  // Build feature matrix x: [ln(1/dR), ln(kt), ln(1/z)] then standardize
  out_x_float.reserve(n_selected * 3);
  for (size_t i = 0; i < n_nodes; ++i) {
    if (!mask[i]) continue;
    float f_ln1overdR = lnR[i];
    float f_lnkT = lnkT[i];
    float zval = std::max(1e-6f, z[i]);
    float f_ln1overz = -std::log(zval);

    float z_std = (f_ln1overz - m_mean_z) / m_std_z;
    float kt_std = (f_lnkT - m_mean_kt) / m_std_kt;
    float dr_std = (f_ln1overdR - m_mean_dr) / m_std_dr;

    out_x_float.push_back(dr_std);
    out_x_float.push_back(kt_std);
    out_x_float.push_back(z_std);
  }

  // Reindex old->new for masked nodes
  std::vector<int> old2new(n_nodes, -1);
  int new_idx = 0;
  for (size_t i = 0; i < n_nodes; ++i) {
    if (mask[i]) old2new[i] = new_idx++;
  }

  // Build edges (only include edges where both endpoints survive)
  for (size_t child = 0; child < n_nodes; ++child) {
    if (!mask[child]) continue;
    int p1 = (child < idp1.size()) ? idp1[child] : -1;
    int p2 = (child < idp2.size()) ? idp2[child] : -1;
    if (p1 >= 0 && p1 < static_cast<int>(n_nodes) && old2new[p1] >= 0) {
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[p1]));
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[child]));
      // optionally also add reverse if model expects undirected edges (you did both)
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[child]));
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[p1]));
    }
    if (p2 >= 0 && p2 < static_cast<int>(n_nodes) && old2new[p2] >= 0) {
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[p2]));
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[child]));
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[child]));
      out_edge_index_int64.push_back(static_cast<int64_t>(old2new[p2]));
    }
  }

  // batch vector: all nodes belong to graph 0
  for (int64_t i = 0; i < new_idx; ++i) out_batch_int64.push_back(0);

  // counts: [num_nodes_selected]
  out_counts_int64.push_back(static_cast<int64_t>(new_idx));

  // Ntrk: normalize as in Python
  float ntrk_std = (ntrk_f - m_mean_ntrk) / m_std_ntrk;
  //out_Ntrk_float.push_back(ntrk_std);
  const int64_t batch_size = m_expectedBatchSize; // Gaudi::Property
  out_Ntrk_float.reserve(batch_size);
  for (int64_t i = 0; i < batch_size; ++i) {
    out_Ntrk_float.push_back(ntrk_std);
  }
  // Final sanity checks
  if (out_x_float.empty() || out_counts_int64.empty()) {
    ATH_MSG_DEBUG("After masking, nothing to evaluate.");
    return false;
  }

  ATH_MSG_DEBUG("Built ONNX input: nodes=" << new_idx
                << " edges=" << (out_edge_index_int64.size()/2)
                << " ntrk=" << ntrk_f);

  return true;
}
#endif //ATHENA-ONLY
