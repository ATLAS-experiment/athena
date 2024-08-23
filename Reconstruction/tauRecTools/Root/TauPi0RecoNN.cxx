/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 This assumes that TauDecayModeNNClassifier.cxx has been called before and that therefore the decay modes are decorated to the taus
 That is necessary, because we have three separete regression NNs, one for 1p1n, one or 1pXn and one for 3pXn (and none for the 0n decay modes)
*/
/*
* TO DO
* make sure when I do the if statements on the decay mode I include something that catches if that information is missing and produces an error informing the user, that the decay mode is missing, and that it's TauDecayModeNNClassifer.cxx who predicts those and decorates them onto the taus.
*/


/*
* As a start this is a copy paste of TauDecayModeNNClassifier.cxx that I'll modify
*/

/*
* TO DO
* create the TauPi0RecoNN.h File
* I only changed this line
*/
// local include(s)
#include "tauRecTools/TauPi0RecoNN.h"

// helper function include(s)
#include "PathResolver/PathResolver.h"

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

TauPi0RecoNN::TauPi0RecoNN(const std::string &name)
    : TauRecToolBase(name)
{
    /*
    * TO DO
    * adapt to TauPi0Reco, I think this is done, unless I need to also declare the decay mode as a property?
    */
  declareProperty("OutputName", m_outputName = "TauPi0FourVec");
  // declareProperty("ProbPrefix", m_probPrefix = "NNDecayModeProb_");
  declareProperty("WeightFile_1p1n", m_weightFile_1p1n = "");
  declareProperty("WeightFile_1pXn", m_weightFile_1pXn = "");
  declareProperty("WeightFile_3pXn", m_weightFile_3pXn = "");
  declareProperty("MaxTauTracks", m_maxTauTracks = 3);
  declareProperty("MaxNeutralPFOs", m_maxNeutralPFOs = 8);
  declareProperty("MaxShotPFOs", m_maxShotPFOs = 6);
  declareProperty("MaxConvTracks", m_maxConvTracks = 4);
  declareProperty("NeutralPFOPtCut", m_neutralPFOPtCut = 1.5);
  // declareProperty("EnsureTrackConsistency", m_ensureTrackConsistency = true);
  // declareProperty("DecorateProb", m_decorateProb = true);
}

TauPi0RecoNN::~TauPi0RecoNN()
{
}

StatusCode TauPi0RecoNN::initialize()
{
    /*
    * TO DO
    * adapt to TauPi0Reco - DONE(?)
    * read in three weight files, instead of one - DONE(?)
    */
  ATH_MSG_INFO("Initializing TauPi0RecoNN");

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
  std::ifstream inputFile(weightFile_1p1n);
  lwt::GraphConfig lwtGraphConfig_1p1n;
  std::ifstream inputFile(weightFile_1pXn);
  lwt::GraphConfig lwtGraphConfig_1pXn;
  std::ifstream inputFile(weightFile_3pXn);
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

StatusCode TauPi0RecoNN::execute(xAOD::TauJet &xTau) const
{
    /*
    * TO DO
    * adapt to TauPi0Reco
    * that is gonna require a few else if statements to get the decay mode of the tau and thereby use the correct network
    * also a final else that decorates zeros onto 1p0n and 3p0n taus (do I want eta and phi to be 0 or should those be null? Probably 0?)
    * maybe I can just initialize the output with zeros and then overwrite those or 1n and Xn cases?
    * on the other hand it might be good to have a consistency check of only writing in zeros for 0n cases and throw and error if there's a different decay mode than the five expected ones?
    */

  // inputs
  // ------
  // m_inputMap will not hold any information,
  // but it is required by the lwtnn API.
  //
  InputMap inputMapDummy;
  InputSequenceMap inputSeqMap;
  std::set<std::string> branches = {"TauTrack", "NeutralPFO", "ShotPFO", "ConvTrack"};
  DMHelper::initMapKeys(inputSeqMap, branches);

  ATH_CHECK(getInputs(xTau, inputSeqMap));

  // output
  // ------
  ValueMap outputs;

  // inference
  // ---------
  try
  {
    outputs = m_lwtGraph->compute(inputMapDummy, inputSeqMap);
  }
  catch (const std::exception &e)
  {
    ATH_MSG_ERROR("Error evaluating the network: " << e.what());
    return StatusCode::FAILURE;
  }

  // Results
  // -------
  /*
  * TO DO
  * check if that is the case. I'm writing this while Lukas is still working on the networks, so I'm basically just guessing the order of the three variables
  * also, I think initally we were predicting E, not p (though p is of course better, because we don't want the NN to have to learn the pion mass), so I'll need to double check we changed that by the time we merge this.
  */
  // Outputs are p, eta, and phi
  // here they are encoded as 0, 1, 2
  //
  std::array<float, 3> pi0fourVec; // is it fine to hard code the dimension here? I don't see what other variable might ever get added that should be part of this vector instead of possible getting decorated on separately?
  for (std::size_t i = 0; i < pi0fourVec.size(); ++i)
  {
    // pi0fourVec[i] = outputs.at(prefix + DMVar::sModeNames[i]);
    /*
    * TO DO
    * that syntax with the prefix is copy pasted from the decay mode classifier, because that wants to decorate strings onto the objects. I obviously just wanna put the floats themselves there, so I gotta check how to do that. Basically I gotta find out, what form the outputs objects has.
    * Can I just skip this step and use outputs as the thing I decorate onto the tau objects?
    * Right now there is this value map thing that maps numbers onto strings. I think I can fully cut all of that and without any mapping just get the output from the network
    */
  }





    /*
    * TO DO
    * Figure out, how exactly these AuxElement things work to decorate stuff onto the tau object. I just don't know athena well enough to fully understand these lines of code (copy pasted from the classifier), I only understand (I think :D) that this is where they decorate the string onto the tau.
    */
  const SG::AuxElement::Accessor<int> accDecayMode(m_outputName);
  accDecayMode(xTau) = std::distance(probs.cbegin(), itMax);

  if (m_decorateProb)
  {
    for (std::size_t i = 0; i < probs.size(); ++i)
    {
      const std::string probName = m_probPrefix + DMVar::sModeNames[i];
      const SG::AuxElement::Accessor<float> accProb(probName);
      accProb(xTau) = probs[i];
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode TauPi0RecoNN::getInputs(const xAOD::TauJet &xTau, InputSequenceMap &inputSeqMap) const
{
    /*
    * TO DO
    * adapt to TauPi0Reco
    * Which might be nothing? The one input we need that TauDecayModeNNClassifier.cxx doesn't need, which is the output of that, the decay modes decorated to the tau, but those are just used for if statements to use the right (if any) weight file, not as input to those NNs, so I gotta check if I need to add that here to the inputs or if that's gonna be read somewhere else.
    */
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

// Helper functions
namespace tauRecTools
{
    /*
    * TO DO
    * Adapt to TauPi0Reco
    */
  const std::set<std::string> TauDecayModeNNVariable::sCommonP4Vars = {
      "dphiECal", "detaECal", "dphi", "deta", "pt_log", "jetpt_log"};

  const std::set<std::string> TauDecayModeNNVariable::sTrackIPVars = {
      "d0TJVA", "d0SigTJVA", "z0sinthetaTJVA", "z0sinthetaSigTJVA"};

  const std::set<std::string> TauDecayModeNNVariable::sNeutralPFOVars = {
      "FIRST_ETA", "SECOND_R_log", "DELTA_THETA", "CENTER_LAMBDA_log", "LONGITUDINAL", "ENG_FRAC_CORE",
      "SECOND_ENG_DENS_log", "NPosECells_EM1", "NPosECells_EM2", "energy_EM1", "energy_EM2", "EM1CoreFrac", 
      "firstEtaWRTClusterPosition_EM1", "firstEtaWRTClusterPosition_EM2", 
      "secondEtaWRTClusterPosition_EM1_log", "secondEtaWRTClusterPosition_EM2_log"};

  const std::array<std::string, TauDecayModeNNVariable::nClasses> TauDecayModeNNVariable::sModeNames = {
      "1p0n", "1p1n", "1pXn", "3p0n", "3pXn"};

  float TauDecayModeNNVariable::deltaPhi(const TLorentzVector &p4, const TLorentzVector &p4_tau)
  {
    return p4_tau.DeltaPhi(p4);
  }

  float TauDecayModeNNVariable::deltaEta(const TLorentzVector &p4, const TLorentzVector &p4_tau)
  {
    return p4.Eta() - p4_tau.Eta();
  }

  float TauDecayModeNNVariable::deltaPhiECal(const TLorentzVector &p4, const std::pair<float, bool> &tau_phiTrkECal)
  {
    // if not retrieved, then set to 0. (mean value)
    return tau_phiTrkECal.second ? TVector2::Phi_mpi_pi(p4.Phi() - tau_phiTrkECal.first) : 0.0f;
  }

  float TauDecayModeNNVariable::deltaEtaECal(const TLorentzVector &p4, const std::pair<float, bool> &tau_etaTrkECal)
  {
    // if not retrieved, then set to 0. (mean value)
    return tau_etaTrkECal.second ? p4.Eta() - tau_etaTrkECal.first : 0.0f;
  }

  template <typename T>
  T TauDecayModeNNVariable::pfoAttr(const PFOPtr pfo, const PFOAttributes &attr)
  {
    T val{static_cast<T>(0)};
    if (!pfo->attribute(attr, val))
    {
      throw std::runtime_error("Can not retrieve PFO attribute! enum = " + std::to_string(static_cast<unsigned>(attr)));
    }
    return val;
  }

  float TauDecayModeNNVariable::ptSubRatio(const PFOPtr pfo)
  {
    float clus0pt = pfo->cluster(0)->pt();
    return clus0pt > 0.0f ? (clus0pt - pfo->pt()) / clus0pt : 0.0f;
  }

  float TauDecayModeNNVariable::energyFracEM2(const PFOPtr pfo, float energy_em2)
  {
    float clus0e = pfo->cluster(0)->e();
    return clus0e > 0.0f ? energy_em2 / clus0e : 0.0f;
  }

  float TauDecayModeNNHelper::Log10Robust(const float val, const float min_val)
  {
    return TMath::Log10(std::max(val, min_val));
  }

  template <typename T>
  void TauDecayModeNNHelper::sortAndKeep(std::vector<T> &vec, const std::size_t n_obj)
  {
    auto cmp_pt = [](const T lhs, const T rhs) { return lhs->pt() > rhs->pt(); };
    std::sort(vec.begin(), vec.end(), cmp_pt);
    if (vec.size() > n_obj)
    {
      vec.erase(vec.begin() + n_obj, vec.end());
    }
  }

  template <typename T>
  void TauDecayModeNNHelper::initMapKeys(std::map<std::string, T> &empty_map,
                                         const std::set<std::string> &keys)
  {
    // T can be any type
    for (const auto &key : keys)
    {
      empty_map[key];
    }
  }
} // namespace tauRecTools
