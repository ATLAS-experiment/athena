/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 This assumes that TauDecayModeNNClassifier.cxx has been called before and that therefore the decay modes are decorated to the taus
 That is necessary, because we have three separete regression NNs, one for 1p1n, one or 1pXn and one for 3pXn (and of course none for the 0n decay modes)
 E, eta, and phi are set to 0 for 1p0n and 3p0n decay modes.
*/

// local include(s)
#include "tauRecTools/TauNeutralFourVecNNRegression.h"

// helper function include(s)
#include "PathResolver/PathResolver.h"
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
  // declareProperty("OutputName", m_outputName = "TauPi0FourVec"); // not needed, since we decorate three individual values, instead of one vector, so there's not just 1 Output
  declareProperty("OutputPrefix", m_outputPrefix = "pi0_NN_");
  declareProperty("WeightFile_1p1n", m_weightFile_1p1n = "");
  declareProperty("WeightFile_1pXn", m_weightFile_1pXn = "");
  declareProperty("WeightFile_3pXn", m_weightFile_3pXn = "");
  declareProperty("MaxTauTracks", m_maxTauTracks = 3);
  declareProperty("MaxNeutralPFOs", m_maxNeutralPFOs = 8);
  declareProperty("MaxShotPFOs", m_maxShotPFOs = 6);
  declareProperty("MaxConvTracks", m_maxConvTracks = 4);
  declareProperty("NeutralPFOPtCut", m_neutralPFOPtCut = 1.5);
  declareProperty("DecayModeName", m_decayModeName = "NNDecayMode"); // needs to be same as m_outputName in TauDecayModeNNClassifier.cxx
  // declareProperty("FourVecDimNames", m_fourVecDimNames = {"E", "eta", "phi"});
}

TauNeutralFourVecNNRegression::~TauNeutralFourVecNNRegression()
{
}

StatusCode TauNeutralFourVecNNRegression::initialize()
{
  ATH_MSG_INFO("Initializing TauNeutralFourVecNNRegression");

  // find input JSON files
  std::string weightFile_1p1n = find_file(m_weightFile_1p1n);
  std::string weightFile_1pXn = find_file(m_weightFile_1pXn);
  std::string weightFile_3pXn = find_file(m_weightFile_3pXn);
  if (weightFile_1p1n.empty())
  {
    ATH_MSG_ERROR("Could not find 1p1n network weights: " << m_weightFile_1p1n);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loaded 1p1n network configuration from: " << weightFile_1p1n);
  if (weightFile_1pXn.empty())
  {
    ATH_MSG_ERROR("Could not find 1pXn network weights: " << m_weightFile_1pXn);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loaded 1pXn network configuration from: " << weightFile_1pXn);
  if (weightFile_3pXn.empty())
  {
    ATH_MSG_ERROR("Could not find 3pXn network weights: " << m_weightFile_3pXn);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loaded 3pXn network configuration from: " << weightFile_3pXn);

  // load lwt graph configurations
  std::ifstream inputFile_1p1n(weightFile_1p1n);
  lwt::GraphConfig lwtGraphConfig_1p1n;
  std::ifstream inputFile_1pXn(weightFile_1pXn);
  lwt::GraphConfig lwtGraphConfig_1pXn;
  std::ifstream inputFile_3pXn(weightFile_3pXn);
  lwt::GraphConfig lwtGraphConfig_3pXn;
  try
  {
    lwtGraphConfig_1p1n = lwt::parse_json_graph(inputFile_1p1n);
  }
  catch (const std::logic_error &e)
  {
    ATH_MSG_ERROR("Error parsing 1p1n network config: " << e.what());
    return StatusCode::FAILURE;
  }
  try
  {
    lwtGraphConfig_1pXn = lwt::parse_json_graph(inputFile_1pXn);
  }
  catch (const std::logic_error &e)
  {
    ATH_MSG_ERROR("Error parsing 1pXn network config: " << e.what());
    return StatusCode::FAILURE;
  }
  try
  {
    lwtGraphConfig_3pXn = lwt::parse_json_graph(inputFile_3pXn);
  }
  catch (const std::logic_error &e)
  {
    ATH_MSG_ERROR("Error parsing 3pXn network config: " << e.what());
    return StatusCode::FAILURE;
  }

  // configure neural networks
  try
  {
    m_lwtGraph_1p1n = std::make_unique<lwt::LightweightGraph>(lwtGraphConfig_1p1n, lwtGraphConfig_1p1n.outputs.cbegin()->first);
  }
  catch (const lwt::NNConfigurationException &e)
  {
    ATH_MSG_ERROR("Error configuring 1p1n network: " << e.what());
    return StatusCode::FAILURE;
  }
  try
  {
    m_lwtGraph_1pXn = std::make_unique<lwt::LightweightGraph>(lwtGraphConfig_1pXn, lwtGraphConfig_1pXn.outputs.cbegin()->first);
  }
  catch (const lwt::NNConfigurationException &e)
  {
    ATH_MSG_ERROR("Error configuring 1pXn network: " << e.what());
    return StatusCode::FAILURE;
  }
  try
  {
    m_lwtGraph_3pXn = std::make_unique<lwt::LightweightGraph>(lwtGraphConfig_3pXn, lwtGraphConfig_3pXn.outputs.cbegin()->first);
  }
  catch (const lwt::NNConfigurationException &e)
  {
    ATH_MSG_ERROR("Error configuring 3pXn network: " << e.what());
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode TauNeutralFourVecNNRegression::execute(xAOD::TauJet &xTau) const
{
  // Read the previously classified decay mode of the tau
  // Decay modes are "1p0n", "1p1n", "1pXn", "3p0n", "3pXn",
  // here they are encoded as 0, 1, 2, 3, 4 (as in TauDecayModeNNClassifier.cxx)
  const static SG::Accessor<int> accDecayMode(m_decayModeName); // This can probably also be a ConstAccessor?
  int decayMode = 7; // 7 is the error mode used as initialisation
  if (accDecayMode.isAvailable(xTau))
  {
    decayMode = accDecayMode(xTau);
    // concert to enum DecayMode and throw error if that fails
    // then change if else statements to use this enum "if Mode_1p0n" instead of "if 0" etc.
  }
  else
  {
    ATH_MSG_WARNING("Initializing TauNeutralFourVecNNRegression"); // maybe this should even be an error?
    // decorate zeros.
  }

  // inputs
  // ------
  // m_inputMap will not hold any information,
  // but it is required by the lwtnn API.
  //
  InputMap inputMapDummy;
  InputSequenceMap inputSeqMap;
  
  DMHelper::initMapKeys(inputSeqMap, branches);

  ATH_CHECK(getInputs(xTau, inputSeqMap));

  // output
  // ------
  ValueMap outputs;

  // inference
  // ---------
  if (decayMode == 1) // 1p1n
  {
    try
    {
      outputs = m_lwtGraph_1p1n->compute(inputMapDummy, inputSeqMap);
    }
    catch (const std::exception &e)
    {
      ATH_MSG_ERROR("Error evaluating the network: " << e.what());
      return StatusCode::FAILURE;
    }
  }
  else if (decayMode == 2) // 1pXn
  {
    try
    {
      outputs = m_lwtGraph_1pXn->compute(inputMapDummy, inputSeqMap);
    }
    catch (const std::exception &e)
    {
      ATH_MSG_ERROR("Error evaluating the network: " << e.what());
      return StatusCode::FAILURE;
    }
  }
  else if (decayMode == 4) // 3pXn
  {
    try
    {
      outputs = m_lwtGraph_3pXn->compute(inputMapDummy, inputSeqMap);
    }
    catch (const std::exception &e)
    {
      ATH_MSG_ERROR("Error evaluating the network: " << e.what());
      return StatusCode::FAILURE;
    }
  }
  /*
  * This should also work, but I think it's more convoluted. To fill the outputs map to then read it to the pi0fourVec std:array
  * Instead I'll just initialise the array with 0s in all entries and only fill it with the content of the output map, if the decay mode is not a 0n one.
  */
  // else // 1p0n or 3p0n
  // {
  //   for (int i = 0; i < 3; i++)
  //   {
  //     outputs[m_fourVecDimNames[i]] = 0;
  //   }
  // }


  // Results
  // -------
  /*
  * TO DO
  *  - I think, we're currently predicting p not E, but wanna change that. Gotta make sure, this uses whatever the final version of the network is.
  *  - I'm currently just guessing, that the names of the output are going to be c_E, c_eta, and c_phi. E, eta, and phi make sense to me (though they might be like E_pi0 or something?), the c_ prefix I copy pasted from the DecayModeClassifier tool, because I assume that's a convention for these kind of json files or something. Gonna have to check that with Lukas' code (but that will of course also show up in testing)
  *  - Maybe add mass to output (pion mass for 1p1n, but not trivial for 1pXn and 3pXn). In the end we wanna decorate TauDecayParticle Objects to the tau, not just the individual values, so for this step of development, I don't really need it. But in the end we do still need a decision on what mass to decorate onto the Xn objects.
  */
  // Outputs are E, eta, and phi
  // here they are encoded as 0, 1, 2
  const std::array<std::string, 3> m_fourVecDimNames = {"E", "eta", "phi"}; // ideally this shouldn't be "buried" down here in the code, but with declare properties but that doesn't seem to like getting vectors.
  std::array<float, 3> pi0fourVec = {}; // = {} should initialize all values in the array to be 0 (which we want for deacy modes without neutral pions)
  if (decayMode != 0 && decayMode != 3) // not 1p0n or 1p3n
  {
    // the prefix to match to output name in the json weight file
    std::string prefix = "c_";
    for (std::size_t i = 0; i < pi0fourVec.size(); ++i)
    {
      pi0fourVec[i] = outputs.at(prefix + m_fourVecDimNames[i]);
    }
  }

  for (std::size_t i = 0; i < pi0fourVec.size(); ++i)
  {
    const std::string fourVecDimName = m_outputPrefix + m_fourVecDimNames[i];
    const SG::AuxElement::Accessor<float> accPi0(fourVecDimName);
    accPi0(xTau) = pi0fourVec[i];
  }

  return StatusCode::SUCCESS;
}

StatusCode TauNeutralFourVecNNRegression::getInputs(const xAOD::TauJet &xTau, InputSequenceMap &inputSeqMap) const
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

  DMHelper::sortAndKeep<TrkPtr>(vTauTracks, m_maxTauTracks);
  DMHelper::sortAndKeep<PFOPtr>(vNeutralPFOs, m_maxNeutralPFOs);
  DMHelper::sortAndKeep<PFOPtr>(vShotPFOs, m_maxShotPFOs);
  DMHelper::sortAndKeep<TrkPtr>(vConvTracks, m_maxConvTracks);

  // set variables
  // -------------

  // tau variables
  const TLorentzVector &tau_p4 = xTau.p4(xAOD::TauJetParameters::TauCalibType::IntermediateAxis);

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
  auto setCommonP4Vars = [&tau_p4, &tau_etaTrkECal, &tau_phiTrkECal](VectorMap &in_seq_map, const TLorentzVector &obj_p4) {
    in_seq_map["dphiECal"].push_back(DMVar::deltaPhiECal(obj_p4, tau_phiTrkECal));
    in_seq_map["detaECal"].push_back(DMVar::deltaEtaECal(obj_p4, tau_etaTrkECal));
    in_seq_map["dphi"].push_back(DMVar::deltaPhi(obj_p4, tau_p4));
    in_seq_map["deta"].push_back(DMVar::deltaEta(obj_p4, tau_p4));
    in_seq_map["pt_log"].push_back(DMHelper::Log10Robust(obj_p4.Pt()));
    in_seq_map["jetpt_log"].push_back(DMHelper::Log10Robust(tau_p4.Pt()));
  };

  // a function to set the track impact parameter variables
  auto setTrackIPVars = [](VectorMap &in_seq_map, const TrkPtr &trk) {
    in_seq_map["d0TJVA"].push_back(trk->d0TJVA());
    in_seq_map["d0SigTJVA"].push_back(trk->d0SigTJVA());
    in_seq_map["z0sinthetaTJVA"].push_back(trk->z0sinthetaTJVA());
    in_seq_map["z0sinthetaSigTJVA"].push_back(trk->z0sinthetaSigTJVA());
  };

  // a function to set the neutral pfo variables
  auto setNeutralPFOVars = [](VectorMap &in_seq_map, const PFOPtr &pfo) {
    // get the attributes of a given PFO object
    auto getAttr = std::bind(DMVar::pfoAttr<float>, pfo, std::placeholders::_1);
    auto getAttrInt = std::bind(DMVar::pfoAttr<int>, pfo, std::placeholders::_1);

    in_seq_map["FIRST_ETA"].push_back(getAttr(PFOAttributes::cellBased_FIRST_ETA));
    in_seq_map["SECOND_R_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_SECOND_R), 1e-3f));
    in_seq_map["DELTA_THETA"].push_back(getAttr(PFOAttributes::cellBased_DELTA_THETA));
    in_seq_map["CENTER_LAMBDA_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_CENTER_LAMBDA), 1e-3f));
    in_seq_map["LONGITUDINAL"].push_back(getAttr(PFOAttributes::cellBased_LONGITUDINAL));
    in_seq_map["ENG_FRAC_CORE"].push_back(getAttr(PFOAttributes::cellBased_ENG_FRAC_CORE));
    in_seq_map["SECOND_ENG_DENS_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_SECOND_ENG_DENS), 1e-6f));
    in_seq_map["NPosECells_EM1"].push_back(getAttrInt(PFOAttributes::cellBased_NPosECells_EM1));
    in_seq_map["NPosECells_EM2"].push_back(getAttrInt(PFOAttributes::cellBased_NPosECells_EM2));
    in_seq_map["energy_EM1"].push_back(getAttr(PFOAttributes::cellBased_energy_EM1));
    in_seq_map["energy_EM2"].push_back(getAttr(PFOAttributes::cellBased_energy_EM2));
    in_seq_map["EM1CoreFrac"].push_back(getAttr(PFOAttributes::cellBased_EM1CoreFrac));
    in_seq_map["firstEtaWRTClusterPosition_EM1"].push_back(getAttr(PFOAttributes::cellBased_firstEtaWRTClusterPosition_EM1));
    in_seq_map["firstEtaWRTClusterPosition_EM2"].push_back(getAttr(PFOAttributes::cellBased_firstEtaWRTClusterPosition_EM2));
    in_seq_map["secondEtaWRTClusterPosition_EM1_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM1), 1e-6f));
    in_seq_map["secondEtaWRTClusterPosition_EM2_log"].push_back(DMHelper::Log10Robust(getAttr(PFOAttributes::cellBased_secondEtaWRTClusterPosition_EM2), 1e-6f));
  };

  // set tau tracks variables
  VectorMap &chrg_map = inputSeqMap.at("TauTrack");
  DMHelper::initMapKeys(chrg_map, DMVar::sCommonP4Vars);
  DMHelper::initMapKeys(chrg_map, DMVar::sTrackIPVars);
  for (const auto &trk : vTauTracks)
  {
    setCommonP4Vars(chrg_map, trk->p4());
    setTrackIPVars(chrg_map, trk);
  }

  // set Neutral PFOs variables
  VectorMap &neut_map = inputSeqMap.at("NeutralPFO");
  DMHelper::initMapKeys(neut_map, DMVar::sCommonP4Vars);
  DMHelper::initMapKeys(neut_map, DMVar::sNeutralPFOVars);
  for (const auto &pfo : vNeutralPFOs)
  {
    setCommonP4Vars(neut_map, pfo->p4());
    try
    {
      setNeutralPFOVars(neut_map, pfo);
    }
    catch (const std::exception &e)
    {
      ATH_MSG_ERROR("Error setting neutral PFO variables: " << e.what());
      return StatusCode::FAILURE;
    }
  }

  // set Shot PFOs variables
  VectorMap &shot_map = inputSeqMap.at("ShotPFO");
  DMHelper::initMapKeys(shot_map, DMVar::sCommonP4Vars);
  for (const auto &pfo : vShotPFOs)
  {
    setCommonP4Vars(shot_map, pfo->p4());
  }

  // set Conversion tracks variables
  VectorMap &conv_map = inputSeqMap.at("ConvTrack");
  DMHelper::initMapKeys(conv_map, DMVar::sCommonP4Vars);
  DMHelper::initMapKeys(conv_map, DMVar::sTrackIPVars);
  for (const auto &trk : vConvTracks)
  {
    setCommonP4Vars(conv_map, trk->p4());
    setTrackIPVars(conv_map, trk);
  }

  return StatusCode::SUCCESS;
}