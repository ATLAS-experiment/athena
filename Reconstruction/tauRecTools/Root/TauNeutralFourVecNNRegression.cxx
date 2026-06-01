/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 This assumes that TauDecayModeNNClassifier.cxx has been called before and that therefore the decay modes are decorated to the taus.
 That is necessary, because we have three separete regression NNs, one for 1p1n, one for 1pXn and one for 3pXn (and of course none for the 0n decay modes).
 Neutral pT, eta, and phi are set to 0 for 1p0n and 3p0n decay modes.
*/

// local include(s)
#include "tauRecTools/TauNeutralFourVecNNRegression.h"

// helper function include(s)
#include "PathResolver/PathResolver.h"
#include "AthOnnxUtils/OnnxUtils.h"
#include "TVector2.h"
// #include "EvaluateUtils.h"
#include "tauRecTools/TauDecayModeNNClassifier.h" // to get TauDecayModeNNVariable and TauDecayModeNNHelper from the tauRecTools namespace

// standard library include(s)
#include <array>
#include <functional>
#include <algorithm>
#include <fstream>

using PFOPtr = const xAOD::PFO *;
using TrkPtr = const xAOD::TauTrack *;
using PFOAttributes = xAOD::PFODetails::PFOAttributes;
using DMVar = tauRecTools::TauDecayModeNNVariable;
using DMHelper = tauRecTools::TauDecayModeNNHelper;
using ValueMap = std::map<std::string, double>;
using VectorMap = std::map<std::string, std::vector<double>>;
using InputMap = std::map<std::string, ValueMap>;
using InputSequenceMap = std::map<std::string, VectorMap>;

static const std::set<std::string> branches = {"TauTrack", "NeutralPFO", "ShotPFO", "ConvTrack"};

TauNeutralFourVecNNRegression::TauNeutralFourVecNNRegression(const std::string &name)
    : TauRecToolBase(name)
{
}

TauNeutralFourVecNNRegression::~TauNeutralFourVecNNRegression()
{
}

StatusCode TauNeutralFourVecNNRegression::initialize()
{
  ATH_MSG_INFO("Initializing TauNeutralFourVecNNRegression");
  ATH_CHECK ( m_decayModeName.initialize() );
  ATH_CHECK( m_onnxTool_1p1n.retrieve() );
  // ATH_CHECK( m_onnxTool_1pXn.retrieve() );
  // ATH_CHECK( m_onnxTool_3pXn.retrieve() );
  // find input JSON files
  // std::string weightFile_1p1n = find_file(m_weightFile_1p1n);
  // std::string weightFile_1pXn = find_file(m_weightFile_1pXn);
  // std::string weightFile_3pXn = find_file(m_weightFile_3pXn);
  // if (weightFile_1p1n.empty())
  // {
  //   ATH_MSG_ERROR("Could not find 1p1n network weights: " << m_weightFile_1p1n);
  //   return StatusCode::FAILURE;
  // }
  // ATH_MSG_INFO("Loaded 1p1n network configuration from: " << weightFile_1p1n);
  // if (weightFile_1pXn.empty())
  // {
  //   ATH_MSG_ERROR("Could not find 1pXn network weights: " << m_weightFile_1pXn);
  //   return StatusCode::FAILURE;
  // }
  // ATH_MSG_INFO("Loaded 1pXn network configuration from: " << weightFile_1pXn);
  // if (weightFile_3pXn.empty())
  // {
  //   ATH_MSG_ERROR("Could not find 3pXn network weights: " << m_weightFile_3pXn);
  //   return StatusCode::FAILURE;
  // }
  // ATH_MSG_INFO("Loaded 3pXn network configuration from: " << weightFile_3pXn);

  // load lwt graph configurations
  // std::ifstream inputFile_1p1n(weightFile_1p1n);
  // lwt::GraphConfig lwtGraphConfig_1p1n;
  // std::ifstream inputFile_1pXn(weightFile_1pXn);
  // lwt::GraphConfig lwtGraphConfig_1pXn;
  // std::ifstream inputFile_3pXn(weightFile_3pXn);
  // lwt::GraphConfig lwtGraphConfig_3pXn;
  // try
  // {
  //   lwtGraphConfig_1p1n = lwt::parse_json_graph(inputFile_1p1n);
  // }
  // catch (const std::logic_error &e)
  // {
  //   ATH_MSG_ERROR("Error parsing 1p1n network config: " << e.what());
  //   return StatusCode::FAILURE;
  // }
  // try
  // {
  //   lwtGraphConfig_1pXn = lwt::parse_json_graph(inputFile_1pXn);
  // }
  // catch (const std::logic_error &e)
  // {
  //   ATH_MSG_ERROR("Error parsing 1pXn network config: " << e.what());
  //   return StatusCode::FAILURE;
  // }
  // try
  // {
  //   lwtGraphConfig_3pXn = lwt::parse_json_graph(inputFile_3pXn);
  // }
  // catch (const std::logic_error &e)
  // {
  //   ATH_MSG_ERROR("Error parsing 3pXn network config: " << e.what());
  //   return StatusCode::FAILURE;
  // }

  // // configure neural networks
  // try
  // {
  //   m_lwtGraph_1p1n = std::make_unique<lwt::LightweightGraph>(lwtGraphConfig_1p1n, lwtGraphConfig_1p1n.outputs.cbegin()->first);
  // }
  // catch (const lwt::NNConfigurationException &e)
  // {
  //   ATH_MSG_ERROR("Error configuring 1p1n network: " << e.what());
  //   return StatusCode::FAILURE;
  // }
  // try
  // {
  //   m_lwtGraph_1pXn = std::make_unique<lwt::LightweightGraph>(lwtGraphConfig_1pXn, lwtGraphConfig_1pXn.outputs.cbegin()->first);
  // }
  // catch (const lwt::NNConfigurationException &e)
  // {
  //   ATH_MSG_ERROR("Error configuring 1pXn network: " << e.what());
  //   return StatusCode::FAILURE;
  // }
  // try
  // {
  //   m_lwtGraph_3pXn = std::make_unique<lwt::LightweightGraph>(lwtGraphConfig_3pXn, lwtGraphConfig_3pXn.outputs.cbegin()->first);
  // }
  // catch (const lwt::NNConfigurationException &e)
  // {
  //   ATH_MSG_ERROR("Error configuring 3pXn network: " << e.what());
  //   return StatusCode::FAILURE;
  // }

  ATH_MSG_INFO("Successfully initialized TauNeutralFourVecNNRegression");
  return StatusCode::SUCCESS;
}

StatusCode TauNeutralFourVecNNRegression::execute(xAOD::TauJet &xTau) const
{
  // Read the previously classified decay mode of the tau
  // Decay modes are "1p0n", "1p1n", "1pXn", "3p0n", "3pXn",
  // they are encoded as 0, 1, 2, 3, 4
  
  ATH_MSG_INFO("Executing TauNeutralFourVecNNRegression");
  
  SG::ReadDecorHandle<xAOD::TauJetContainer, int> decayModeHandle( m_decayModeName );
  if (!decayModeHandle.isPresent())
  {
      ATH_MSG_ERROR( "TauJet container " << m_decayModeName << " not available!" );
      return StatusCode::FAILURE;  
  }
  if (!decayModeHandle.isAvailable())
  {
      ATH_MSG_ERROR( "TauJet decoration " << m_decayModeName << " not available!" );
      return StatusCode::FAILURE;  
  }
  int decayMode = decayModeHandle(xTau);
  ATH_MSG_INFO("Loaded Decay Mode to be " << decayMode);

  // // inputs
  // // ------
  // // m_inputMap will not hold any information,
  // // but it is required by the lwtnn API.
  // //
  // InputMap inputMapDummy;
  // InputSequenceMap inputSeqMap;
  
  // DMHelper::initMapKeys(inputSeqMap, branches);

  // ATH_CHECK(getInputs(xTau, inputSeqMap));

  // // output
  // // ------
  // ValueMap outputs;

  // // inference
  // // ---------
  // if (decayMode == 1) // 1p1n
  // {
  //   try
  //   {
  //     outputs = m_lwtGraph_1p1n->compute(inputMapDummy, inputSeqMap);
  //   }
  //   catch (const std::exception &e)
  //   {
  //     ATH_MSG_ERROR("Error evaluating the network: " << e.what());
  //     return StatusCode::FAILURE;
  //   }
  // }
  // else if (decayMode == 2) // 1pXn
  // {
  //   try
  //   {
  //     outputs = m_lwtGraph_1pXn->compute(inputMapDummy, inputSeqMap);
  //   }
  //   catch (const std::exception &e)
  //   {
  //     ATH_MSG_ERROR("Error evaluating the network: " << e.what());
  //     return StatusCode::FAILURE;
  //   }
  // }
  // else if (decayMode == 4) // 3pXn
  // {
  //   try
  //   {
  //     outputs = m_lwtGraph_3pXn->compute(inputMapDummy, inputSeqMap);
  //   }
  //   catch (const std::exception &e)
  //   {
  //     ATH_MSG_ERROR("Error evaluating the network: " << e.what());
  //     return StatusCode::FAILURE;
  //   }
  // }
  // /*
  // * This should also work, but I think it's more convoluted. To fill the outputs map to then read it to the neutralFourVec std:array
  // * Instead I'll just initialise the array with 0s in all entries and only fill it with the content of the output map, if the decay mode is not a 0n one.
  // */
  // // else // 1p0n or 3p0n
  // // {
  // //   for (int i = 0; i < 3; i++)
  // //   {
  // //     outputs[m_fourVecDimNames[i]] = 0;
  // //   }
  // // }

  // prepare inputs
  std::vector<float> inputDataVector_chargedPFOs;
  std::vector<float> inputDataVector_neutralPFOs;
  std::vector<float> inputDataVector_conversionTracks;
  std::vector<float> inputDataVector_photonShots;

  ATH_MSG_INFO("Calling getInputs()");
  ATH_CHECK(getInputs(xTau, inputDataVector_chargedPFOs, inputDataVector_neutralPFOs, inputDataVector_conversionTracks, inputDataVector_photonShots));
  ATH_MSG_INFO("Called getInputs()");

  // copied from the onnx athena example, which uses images.
  // in the example the m_input_tensor_values_notFlat is defined inside initialize.
  // TODO: adopt this syntax to actually use the input, ideally using the getInputs function, which set up the inputs for lwtnn.
  // inputDataVector.reserve(m_input_tensor_values_notFlat.size());
  // for (const std::vector<std::vector<float> >& imageData : m_input_tensor_values_notFlat){
  //   std::vector<float> flatten = AthOnnxUtils::flattenNestedVectors(imageData);
  //   inputDataVector.insert(inputDataVector.end(), flatten.begin(), flatten.end());
  // }

  // I'm copy pasting the commands to fill these vectors from the getInputs() function. For efficiency I should probably rewrite getInputs() to then also take these vectors as arguments as it does with xTau, instead of also building them from scratch.
  // just gotta make sure I got all things like the max length still correct.
  std::vector<TrkPtr> vTauTracks;
  std::vector<PFOPtr> vNeutralPFOs;
  std::vector<PFOPtr> vShotPFOs;
  std::vector<TrkPtr> vConvTracks;

  vTauTracks = xTau.tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged);

  for (std::size_t i = 0; i < xTau.nNeutralPFOs(); ++i)
  {
    const auto pfo = xTau.neutralPFO(i);
    // Apply pt threshold
    if (pfo->pt() < m_neutralPFOPtCut * 1e3)
      continue;
    vNeutralPFOs.push_back(pfo);
  }

  for (std::size_t i = 0; i < xTau.nShotPFOs(); ++i)
  {
    const auto pfo = xTau.shotPFO(i);
    // skip PFOs without photons
    int nPhotons{-1};
    try
    {
      nPhotons = DMVar::pfoAttr<int>(pfo, PFOAttributes::tauShots_nPhotons);
    }
    catch (const std::exception &e)
    {
      ATH_MSG_ERROR("Error retrieving tauShots_nPhotons: " << e.what());
      return StatusCode::FAILURE;
    }
    if (nPhotons < 1)
      continue;
    vShotPFOs.push_back(pfo);
  }

  vConvTracks = xTau.tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedConversion);

  DMHelper::sortAndKeep<TrkPtr>(vTauTracks, m_maxTauTracks); // these are also copy pasted from getInputs(), but I don't see m_maxTauTracks defined anywhere in this file. I guess that's inherited from somewhere else in athena and just defined in this namespace or something?
  DMHelper::sortAndKeep<PFOPtr>(vNeutralPFOs, m_maxNeutralPFOs);
  DMHelper::sortAndKeep<PFOPtr>(vShotPFOs, m_maxShotPFOs);
  DMHelper::sortAndKeep<TrkPtr>(vConvTracks, m_maxConvTracks);
  const int n_tracks = vTauTracks.size();
  const int n_nPFOs = vNeutralPFOs.size();
  const int n_shots = vShotPFOs.size();
  const int n_convTracks = vConvTracks.size();
  const int maxTracks = m_maxTauTracks;
  const int maxnPFOs = m_maxNeutralPFOs;
  const int maxShots = m_maxShotPFOs;
  const int maxConvTracks = m_maxConvTracks;
  ATH_MSG_INFO("vTauTracks.size(): " << n_tracks);
  ATH_MSG_INFO("vNeutralPFOs.size(): " << n_nPFOs);
  ATH_MSG_INFO("vShotPFOs.size(): " << n_shots);
  ATH_MSG_INFO("vConvTracks.size(): " << n_convTracks);
  int n_chargedPFOs = std::min(n_tracks, maxTracks); // m_maxTauTracks should be 3
  int n_neutralPFOs = std::min(n_nPFOs, maxnPFOs); // m_maxTauTracks should be 10
  int n_conversionTracks = std::min(n_shots, maxShots); // m_maxTauTracks should be 6
  int n_photonShots = std::min(n_convTracks, maxConvTracks); // m_maxTauTracks should be 4
  ATH_MSG_INFO("n_chargedPFOs: " << n_chargedPFOs);
  ATH_MSG_INFO("n_neutralPFOs: " << n_neutralPFOs);
  ATH_MSG_INFO("n_conversionTracks: " << n_conversionTracks);
  ATH_MSG_INFO("n_photonShots: " << n_photonShots);

  std::vector<int64_t> inputShape_chargedPFOs = {1, n_chargedPFOs, 4};
  std::vector<int64_t> inputShape_neutralPFOs = {1, n_neutralPFOs, 12};
  std::vector<int64_t> inputShape_conversionTracks = {1, n_conversionTracks, 4};
  std::vector<int64_t> inputShape_photonShots = {1, n_photonShots, 4};

  ATH_MSG_INFO("Input Vector Shapes Defined. Creating Input Data Map.");

  AthInfer::InputDataMap inputData;
  inputData["input_1"] = std::make_pair( 
    inputShape_chargedPFOs, std::move(inputDataVector_chargedPFOs)
  );
  inputData["input_2"] = std::make_pair(
    inputShape_neutralPFOs, std::move(inputDataVector_neutralPFOs)
  );
  inputData["input_3"] = std::make_pair(
    inputShape_conversionTracks, std::move(inputDataVector_conversionTracks)
  );
  inputData["input_4"] = std::make_pair(
    inputShape_photonShots, std::move(inputDataVector_photonShots)
  );

  // This doesn't work because val is read only...
  // ATH_MSG_INFO("Filling Input Data Map with arabitray values for testing.");
  // for (const auto& [key, valuePair] : inputData) {
  //   if (std::holds_alternative<std::vector<float>>(valuePair.second)) {
  //     for (const auto& val : std::get<std::vector<float>>(valuePair.second))
  //       val = 0.5;
  //   } else if (std::holds_alternative<std::vector<int64_t>>(valuePair.second)) {
  //     for (const auto& val : std::get<std::vector<int64_t>>(valuePair.second))
  //       val = 1;
  //   } 
  // }

  ATH_MSG_INFO("Input Data Map Created. Creating Output Data Map.");

  AthInfer::OutputDataMap outputData;
  outputData["dense_10"] = std::make_pair(
    std::vector<int64_t>{1, 1}, std::vector<float>{} // pT
  );
  outputData["dense_11"] = std::make_pair(
    std::vector<int64_t>{1, 1}, std::vector<float>{} // eta
  );
  outputData["dense_12"] = std::make_pair(
    std::vector<int64_t>{1, 1}, std::vector<float>{} // phi
  );

  ATH_MSG_INFO("Output Data Map Created. Running Inference.");

  ATH_MSG_INFO("Input Data Map:");
  for (const auto& [key, valuePair] : inputData) {
    ATH_MSG_INFO("Key: " << key);

    std::ostringstream oss1;
    oss1 << "[ ";
    for (const auto& val : valuePair.first) oss1 << val << " ";
    oss1 << "]";
    ATH_MSG_INFO("  Shape: " << oss1.str());

    std::ostringstream oss2;
    oss2 << "[ ";
    // Handle variant type for data
    if (std::holds_alternative<std::vector<float>>(valuePair.second)) {
      for (const auto& val : std::get<std::vector<float>>(valuePair.second))
        oss2 << val << " ";
    } else if (std::holds_alternative<std::vector<int64_t>>(valuePair.second)) {
      for (const auto& val : std::get<std::vector<int64_t>>(valuePair.second))
        oss2 << val << " ";
    } else {
      oss2 << "(unknown type)";
    }
    oss2 << "]";
    ATH_MSG_INFO("  Data:  " << oss2.str());
  }
  ATH_MSG_INFO("Output Data Map:");
  for (const auto& [key, valuePair] : outputData) {
    ATH_MSG_INFO("Key: " << key);

    std::ostringstream oss1;
    oss1 << "[ ";
    for (const auto& val : valuePair.first) oss1 << val << " ";
    oss1 << "]";
    ATH_MSG_INFO("  Shape: " << oss1.str());

    std::ostringstream oss2;
    oss2 << "[ ";
    // Handle variant type for data
    if (std::holds_alternative<std::vector<float>>(valuePair.second)) {
      for (const auto& val : std::get<std::vector<float>>(valuePair.second))
        oss2 << val << " ";
    } else if (std::holds_alternative<std::vector<int64_t>>(valuePair.second)) {
      for (const auto& val : std::get<std::vector<int64_t>>(valuePair.second))
        oss2 << val << " ";
    } else {
      oss2 << "(unknown type)";
    }
    oss2 << "]";
    ATH_MSG_INFO("  Data:  " << oss2.str());
  }

  // ATH_MSG_INFO("Cutting the execute() short for testing reasons");
  // return StatusCode::SUCCESS;

  if (decayMode == 1)
  {
    ATH_CHECK(m_onnxTool_1p1n->inference(inputData, outputData));
    ATH_MSG_INFO("ONNX Inference Successfully Run for Decay Mode 1 (1p1n).");
  }
  else if (decayMode == 2)
  {
    ATH_CHECK(m_onnxTool_1p1n->inference(inputData, outputData));
    // only using 1p1n for initial tests
    // ATH_CHECK(m_onnxTool_1pXn->inference(inputData, outputData));
    ATH_MSG_INFO("ONNX Inference Successfully Run for Decay Mode 2 (1pXn). Used 1p1n Network for testing purposes.");
  }
  else if (decayMode == 4)
  {
    ATH_CHECK(m_onnxTool_1p1n->inference(inputData, outputData));
    // only using 1p1n for initial tests
    // ATH_CHECK(m_onnxTool_3pXn->inference(inputData, outputData));
    ATH_MSG_INFO("ONNX Inference Successfully Run for Decay Mode 4 (3pXn). Used 1p1n Network for testing purposes.");
  }
  else
  {
    // TODO
    // fill outputData with zeros.
    // alternatively initialize it that way?
    // or initialize the neutralFourVec as {0,0,0} and only overwrite that if decayMode is 1, 2 or 4?
    ATH_MSG_INFO("Nothing done, because decay mode isn't 1, 2, or 4 and filling outputData with zeros isn't implemented yet.");
  }

  std::array<float, 3> neutralFourVec = {std::get<std::vector<float>>(outputData["dense_10"].second).front(), std::get<std::vector<float>>(outputData["dense_11"].second).front(), std::get<std::vector<float>>(outputData["dense_12"].second).front()};

  ATH_MSG_INFO("Read Inference Output into neutralFourVec array: " << neutralFourVec);
  // Results
  // -------
  /*
  * TODO
  *  - I think, we're currently predicting pT not E, but wanna change that. Gotta make sure, this uses whatever the final version of the network is.
  *  - I'm currently just guessing, that the names of the output are going to be c_E, c_eta, and c_phi. E, eta, and phi make sense to me (though they might be like E_pi0 or something?), the c_ prefix I copy pasted from the DecayModeClassifier tool, because I assume that's a convention for these kind of json files or something. Gonna have to check that with Lukas' code (but that will of course also show up in testing)
  *  - Maybe add mass to output (pion mass for 1p1n, but not trivial for 1pXn and 3pXn). In the end we wanna decorate TauDecayParticle Objects to the tau, not just the individual values, so for this step of development, I don't really need it. But in the end we do still need a decision on what mass to decorate onto the Xn objects.
  */
  // Outputs are E, eta, and phi
  // here they are encoded as 0, 1, 2
  // std::array<float, 3> neutralFourVec = {}; // = {} should initialize all values in the array to be 0 (which we want for deacy modes without neutral pions)
  // if (decayMode != 0 && decayMode != 3) // not 1p0n or 3p0n
  // {
  //   // the prefix to match to output name in the json weight file
  //   std::string prefix = "c_";
  //   for (std::size_t i = 0; i < neutralFourVec.size(); ++i)
  //   {
  //     neutralFourVec[i] = outputs.at(prefix + m_fourVecDimNames[i]);
  //   }
  // }

  for (std::size_t i = 0; i < neutralFourVec.size(); ++i)
  {
    const std::string fourVecDimName = m_outputPrefix + m_fourVecDimNames[i];
    const SG::AuxElement::Accessor<float> accPi0(fourVecDimName);
    accPi0(xTau) = neutralFourVec[i];
  }

  ATH_MSG_INFO("Successfully Exectued TauNeutralFourVecNNRegression.");
  return StatusCode::SUCCESS;
}

StatusCode TauNeutralFourVecNNRegression::getInputs(const xAOD::TauJet &xTau, std::vector<float> &inputDataVector_chargedPFOs, std::vector<float> &inputDataVector_neutralPFOs, std::vector<float> &inputDataVector_conversionTracks, std::vector<float> &inputDataVector_photonShots) const//, InputSequenceMap &inputSeqMap) const
{
  std::vector<TrkPtr> vTauTracks;
  std::vector<PFOPtr> vNeutralPFOs;
  std::vector<PFOPtr> vShotPFOs;
  std::vector<TrkPtr> vConvTracks;

  // set objects
  // -----------

  // classified tau tracks
  vTauTracks = xTau.tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged);

  // neutral PFOs
  for (std::size_t i = 0; i < xTau.nNeutralPFOs(); ++i)
  {
    const auto pfo = xTau.neutralPFO(i);
    // Apply pt threshold
    if (pfo->pt() < m_neutralPFOPtCut * 1e3)
      continue;
    vNeutralPFOs.push_back(pfo);
  }

  // shot PFOs
  for (std::size_t i = 0; i < xTau.nShotPFOs(); ++i)
  {
    const auto pfo = xTau.shotPFO(i);
    // skip PFOs without photons
    int nPhotons{-1};
    try
    {
      nPhotons = DMVar::pfoAttr<int>(pfo, PFOAttributes::tauShots_nPhotons);
    }
    catch (const std::exception &e)
    {
      ATH_MSG_ERROR("Error retrieving tauShots_nPhotons: " << e.what());
      return StatusCode::FAILURE;
    }
    if (nPhotons < 1)
      continue;
    vShotPFOs.push_back(pfo);
  }

  // classified conversion tracks
  vConvTracks = xTau.tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedConversion);

  ATH_MSG_INFO("vTauTracks.size() in getInputs(): " << vTauTracks.size());
  ATH_MSG_INFO("vNeutralPFOs.size() in getInputs(): " << vNeutralPFOs.size());
  ATH_MSG_INFO("vShotPFOs.size() in getInputs(): " << vShotPFOs.size());
  ATH_MSG_INFO("vConvTracks.size() in getInputs(): " << vConvTracks.size());

  DMHelper::sortAndKeep<TrkPtr>(vTauTracks, m_maxTauTracks);
  DMHelper::sortAndKeep<PFOPtr>(vNeutralPFOs, m_maxNeutralPFOs);
  DMHelper::sortAndKeep<PFOPtr>(vShotPFOs, m_maxShotPFOs);
  DMHelper::sortAndKeep<TrkPtr>(vConvTracks, m_maxConvTracks);

  ATH_MSG_INFO("vTauTracks.size() nach sortAndKeep in getInputs(): " << vTauTracks.size());
  ATH_MSG_INFO("vNeutralPFOs.size() nach sortAndKeep in getInputs(): " << vNeutralPFOs.size());
  ATH_MSG_INFO("vShotPFOs.size() nach sortAndKeep in getInputs(): " << vShotPFOs.size());
  ATH_MSG_INFO("vConvTracks.size() nach sortAndKeep in getInputs(): " << vConvTracks.size());

  std::cout << "vTauTracks ";
  for (TrkPtr i: vTauTracks){
    std::cout << i << ' ';
  }
  std::cout << "vNeutralPFOs ";
  for (PFOPtr i: vNeutralPFOs){
    std::cout << i << ' ';
  }
  std::cout << "vShotPFOs ";
  for (PFOPtr i: vShotPFOs){
    std::cout << i << ' ';
  }
  std::cout << "vConvTracks ";
  for (TrkPtr i: vConvTracks){
    std::cout << i << ' ';
  }

  // set variables
  // -------------

  // tau variables
  const TLorentzVector &tau_p4 = xTau.p4(xAOD::TauJetParameters::TauCalibType::IntermediateAxis);
  const float pt_jet = xTau.pt();
  const float pt_jet_log = DMHelper::Log10Robust(pt_jet, 1e-6); // Is that correct?! Or is the pt_jet_log input variable something else than

  // pair: (1st) the value, (2nd) successfully retrieved
  std::pair<float, bool> tau_etaTrkECal{0., false};
  std::pair<float, bool> tau_phiTrkECal{0., false};
  if (xTau.nTracks() > 0)
  {
    TrkPtr trk = xTau.track(0);
    if (!trk->detail(xAOD::TauJetParameters::CaloSamplingPhiEM, tau_phiTrkECal.first))
    {
      ATH_MSG_WARNING("Failed to retrieve extrapolated track phi in ECal");
    }
    else
    {
      tau_phiTrkECal.second = true;
    }
    if (!trk->detail(xAOD::TauJetParameters::CaloSamplingEtaEM, tau_etaTrkECal.first))
    {
      ATH_MSG_WARNING("Failed to retrieve extrapolated track eta in ECal");
    }
    else
    {
      tau_etaTrkECal.second = true;
    }
  }

  // a function to set the common 4-momentum variables, this is needed for all later
  // auto setCommonP4Vars = [&tau_p4, &tau_etaTrkECal, &tau_phiTrkECal](VectorMap &in_seq_map, const TLorentzVector &obj_p4) {
  //   in_seq_map["dphiECal"].push_back(DMVar::deltaPhiECal(obj_p4, tau_phiTrkECal));
  //   in_seq_map["detaECal"].push_back(DMVar::deltaEtaECal(obj_p4, tau_etaTrkECal));
  //   in_seq_map["dphi"].push_back(DMVar::deltaPhi(obj_p4, tau_p4));
  //   in_seq_map["deta"].push_back(DMVar::deltaEta(obj_p4, tau_p4));
  //   in_seq_map["pt_log"].push_back(DMHelper::Log10Robust(obj_p4.Pt()));
  //   in_seq_map["jetpt_log"].push_back(DMHelper::Log10Robust(tau_p4.Pt()));
  // };

  // // a function to set the track impact parameter variables
  // auto setTrackIPVars = [](VectorMap &in_seq_map, const TrkPtr &trk) {
  //   in_seq_map["d0TJVA"].push_back(trk->d0TJVA());
  //   in_seq_map["d0SigTJVA"].push_back(trk->d0SigTJVA());
  //   in_seq_map["z0sinthetaTJVA"].push_back(trk->z0sinthetaTJVA());
  //   in_seq_map["z0sinthetaSigTJVA"].push_back(trk->z0sinthetaSigTJVA());
  // };

  // a function to set the neutral pfo variables
    // auto setNeutralPFOVars = [](VectorMap &in_seq_map, const PFOPtr &pfo) {
    // get the attributes of a given PFO object
    // auto getAttr = std::bind(DMVar::pfoAttr<float>, pfo, std::placeholders::_1);
    // auto getAttrInt = std::bind(DMVar::pfoAttr<int>, pfo, std::placeholders::_1);
    // ATH_MSG_INFO("getAttr(PFOAttributes::cellBased_FIRST_ETA): " << getAttr(PFOAttributes::cellBased_FIRST_ETA));
    // ATH_MSG_INFO("getAttrInt(PFOAttributes::cellBased_NPosECells_EM1): " << getAttrInt(PFOAttributes::cellBased_NPosECells_EM1));
  //   in_seq_map["FIRST_ETA"].push_back(getAttr(PFOAttributes::cellBased_FIRST_ETA));
  //   in_seq_map["SECOND_R_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_SECOND_R), 1e-3f));
  //   in_seq_map["DELTA_THETA"].push_back(getAttr(PFOAttributes::cellBased_DELTA_THETA));
  //   in_seq_map["CENTER_LAMBDA_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_CENTER_LAMBDA), 1e-3f));
  //   in_seq_map["LONGITUDINAL"].push_back(getAttr(PFOAttributes::cellBased_LONGITUDINAL));
  //   in_seq_map["ENG_FRAC_CORE"].push_back(getAttr(PFOAttributes::cellBased_ENG_FRAC_CORE));
  //   in_seq_map["SECOND_ENG_DENS_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_SECOND_ENG_DENS), 1e-6f));
  //   in_seq_map["NPosECells_EM1"].push_back(getAttrInt(PFOAttributes::cellBased_NPosECells_EM1));
  //   in_seq_map["NPosECells_EM2"].push_back(getAttrInt(PFOAttributes::cellBased_NPosECells_EM2));
  //   in_seq_map["energy_EM1"].push_back(getAttr(PFOAttributes::cellBased_energy_EM1));
  //   in_seq_map["energy_EM2"].push_back(getAttr(PFOAttributes::cellBased_energy_EM2));
  //   in_seq_map["EM1CoreFrac"].push_back(getAttr(PFOAttributes::cellBased_EM1CoreFrac));
  //   in_seq_map["firstEtaWRTClusterPosition_EM1"].push_back(getAttr(PFOAttributes::cellBased_firstEtaWRTClusterPosition_EM1));
  //   in_seq_map["firstEtaWRTClusterPosition_EM2"].push_back(getAttr(PFOAttributes::cellBased_firstEtaWRTClusterPosition_EM2));
  //   in_seq_map["secondEtaWRTClusterPosition_EM1_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM1), 1e-6f));
  //   in_seq_map["secondEtaWRTClusterPosition_EM2_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM2), 1e-6f));
  // };

  // // set tau tracks variables
  // VectorMap &chrg_map = inputSeqMap.at("TauTrack");
  // DMHelper::initMapKeys(chrg_map, DMVar::sCommonP4Vars);
  // DMHelper::initMapKeys(chrg_map, DMVar::sTrackIPVars);
  // for (const auto &trk : vTauTracks)
  // {
  //   setCommonP4Vars(chrg_map, trk->p4());
  //   try
  //   {
  //     setTrackIPVars(chrg_map, trk);
  //   }
  //   catch (const std::exception &e)
  //   {
  //     ATH_MSG_ERROR("Error setting tau track variables: " << e.what());
  //     return StatusCode::FAILURE;
  //   }
  // }
  for (const auto &trk : vTauTracks)
  {
    // std::vector<float> trackVariables;
    const auto &p4 = trk->p4();
    // ATH_MSG_INFO("trk: " << trk);
    // ATH_MSG_INFO("trk->p4(): ");
    // p4.Print();
    const float dEta = trk->eta() - xTau.eta();
    const float dPhi = TVector2::Phi_mpi_pi(trk->phi() - xTau.phi());
    // ATH_MSG_INFO("dEta trk: " << dEta);
    // ATH_MSG_INFO("dPhi trk: " << dPhi);
    const float pT = trk->pt();
    const float log_pT = DMHelper::Log10Robust(pT, 1e-6);
    // ATH_MSG_INFO("pT trk: " << pT);
    // ATH_MSG_INFO("log_10(pT) trk: " << log_pT);
    inputDataVector_chargedPFOs.push_back(dPhi);
    inputDataVector_chargedPFOs.push_back(dEta);
    inputDataVector_chargedPFOs.push_back(log_pT);
    inputDataVector_chargedPFOs.push_back(pt_jet_log);
    // trackVariables.push_back(dPhi);
    // trackVariables.push_back(dEta);
    // trackVariables.push_back(log_pT);
    // trackVariables.push_back(pt_jet_log);
    // inputDataVector_chargedPFOs.push_back(trackVariables);
  }

  // // set Neutral PFOs variables
  // VectorMap &neut_map = inputSeqMap.at("NeutralPFO");
  // DMHelper::initMapKeys(neut_map, DMVar::sCommonP4Vars);
  // DMHelper::initMapKeys(neut_map, DMVar::sNeutralPFOVars);
  // for (const auto &pfo : vNeutralPFOs)
  // {
  //   setCommonP4Vars(neut_map, pfo->p4());
  //   try
  //   {
  //     setNeutralPFOVars(neut_map, pfo);
  //   }
  //   catch (const std::exception &e)
  //   {
  //     ATH_MSG_ERROR("Error setting neutral PFO variables: " << e.what());
  //     return StatusCode::FAILURE;
  //   }
  // }
  for (const auto &npfo : vNeutralPFOs)
  {
    // std::vector<float> nPFOVariables;
    const auto &p4 = npfo->p4();
    // ATH_MSG_INFO("npfo: " << npfo);
    // ATH_MSG_INFO("npfo->p4(): ");
    // p4.Print();

    // ATH_MSG_INFO("DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_FIRST_ETA): " << DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_FIRST_ETA));
    // ATH_MSG_INFO("DMHelper::Log10Robust(DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_CENTER_LAMBDA), 1e-3f): " << DMHelper::Log10Robust(DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_CENTER_LAMBDA), 1e-3f));
    
    const float dEta = npfo->eta() - xTau.eta();
    const float dPhi = TVector2::Phi_mpi_pi(npfo->phi() - xTau.phi());

    const float pT = npfo->pt();
    const float log_pT = DMHelper::Log10Robust(pT, 1e-6);
    
    const float pi0BDTscore = npfo->bdtPi0Score();
    const int NHitsInEM1_int = DMVar::pfoAttr<int>(npfo, PFOAttributes::cellBased_NPosECells_EM1);
    const float NHitsInEM1 = static_cast<float>(NHitsInEM1_int);
    const float SECOND_R = DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_SECOND_R);
    const float secondEtaWRTClusterPosition_EM1 = DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM1);
    const int NPosECells_EM1_int = DMVar::pfoAttr<int>(npfo, PFOAttributes::cellBased_NPosECells_EM1);
    const float NPosECells_EM1 = static_cast<float>(NPosECells_EM1_int);
    const float ENG_FRAC_CORE = DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_ENG_FRAC_CORE);
    const float energyfrac_EM2 = DMVar::pfoAttr<float>(npfo, PFOAttributes::cellBased_energy_EM2); // is this right? I'm just guessing based on which variables I find in the old lwtnn code, but I'm not sure the "frac" bit is actually implied in the PFO attribute?
    const float ptSubRatio = 0.5; // No idea what this one is. Just hardcoding an arbitary number right now for testing.
    
    inputDataVector_neutralPFOs.push_back(dPhi);
    inputDataVector_neutralPFOs.push_back(dEta);
    inputDataVector_neutralPFOs.push_back(log_pT);
    inputDataVector_neutralPFOs.push_back(pt_jet_log);
    inputDataVector_neutralPFOs.push_back(pi0BDTscore);
    inputDataVector_neutralPFOs.push_back(NHitsInEM1); 
    inputDataVector_neutralPFOs.push_back(SECOND_R);
    inputDataVector_neutralPFOs.push_back(secondEtaWRTClusterPosition_EM1);
    inputDataVector_neutralPFOs.push_back(NPosECells_EM1);
    inputDataVector_neutralPFOs.push_back(ENG_FRAC_CORE);
    inputDataVector_neutralPFOs.push_back(energyfrac_EM2);
    inputDataVector_neutralPFOs.push_back(ptSubRatio);
    // nPFOVariables.push_back(dPhi);
    // nPFOVariables.push_back(dEta);
    // nPFOVariables.push_back(log_pT);
    // nPFOVariables.push_back(pt_jet_log);
    // nPFOVariables.push_back(pi0BDTscore);
    // nPFOVariables.push_back(NHitsInEM1); 
    // nPFOVariables.push_back(SECOND_R);
    // nPFOVariables.push_back(secondEtaWRTClusterPosition_EM1);
    // nPFOVariables.push_back(NPosECells_EM1);
    // nPFOVariables.push_back(ENG_FRAC_CORE);
    // nPFOVariables.push_back(energyfrac_EM2);
    // nPFOVariables.push_back(ptSubRatio);
    // inputDataVector_neutralPFOs.push_back(nPFOVariables);
  }

  // // set Shot PFOs variables
  // VectorMap &shot_map = inputSeqMap.at("ShotPFO");
  // DMHelper::initMapKeys(shot_map, DMVar::sCommonP4Vars);
  // for (const auto &pfo : vShotPFOs)
  // {
  //   setCommonP4Vars(shot_map, pfo->p4());
  // }
  for (const auto &shotpfo : vShotPFOs)
  {
    const auto &p4 = shotpfo->p4();
    // ATH_MSG_INFO("shotpfo: " << shotpfo);
    // ATH_MSG_INFO("shotpfo->p4(): ");
    // p4.Print();
    const float dEta = shotpfo->eta() - xTau.eta();
    const float dPhi = TVector2::Phi_mpi_pi(shotpfo->phi() - xTau.phi());

    const float pT = shotpfo->pt();
    const float log_pT = DMHelper::Log10Robust(pT, 1e-6);

    inputDataVector_photonShots.push_back(dPhi);
    inputDataVector_photonShots.push_back(dEta);
    inputDataVector_photonShots.push_back(log_pT);
    inputDataVector_photonShots.push_back(pt_jet_log);
  }

  // // set Conversion tracks variables
  // VectorMap &conv_map = inputSeqMap.at("ConvTrack");
  // DMHelper::initMapKeys(conv_map, DMVar::sCommonP4Vars);
  // DMHelper::initMapKeys(conv_map, DMVar::sTrackIPVars);
  // for (const auto &trk : vConvTracks)
  // {
  //   setCommonP4Vars(conv_map, trk->p4());
  //   try
  //   {
  //     setTrackIPVars(conv_map, trk);
  //   }
  //   catch (const std::exception &e)
  //   {
  //     ATH_MSG_ERROR("Error setting conversion track variables: " << e.what());
  //     return StatusCode::FAILURE;
  //   }
  // }
  for (const auto &convtrk : vConvTracks)
  {
    const auto &p4 = convtrk->p4();
    // ATH_MSG_INFO("convtrk: " << convtrk);
    // ATH_MSG_INFO("convtrk->p4(): ");
    // p4.Print();
    const float dEta = convtrk->eta() - xTau.eta();
    const float dPhi = TVector2::Phi_mpi_pi(convtrk->phi() - xTau.phi());

    const float pT = convtrk->pt();
    const float log_pT = DMHelper::Log10Robust(pT, 1e-6);

    inputDataVector_conversionTracks.push_back(dPhi);
    inputDataVector_conversionTracks.push_back(dEta);
    inputDataVector_conversionTracks.push_back(log_pT);
    inputDataVector_conversionTracks.push_back(pt_jet_log);
  }

  return StatusCode::SUCCESS;
}
