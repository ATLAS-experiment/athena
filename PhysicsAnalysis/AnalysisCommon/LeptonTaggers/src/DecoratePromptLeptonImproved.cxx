/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local
#include "LeptonTaggers/DecoratePromptLeptonImproved.h"
#include "LeptonTaggers/PromptUtils.h"

// ROOT
#include "TMVA/Config.h"
#include "TH1.h"

// C/C++
#include <cmath>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

//======================================================================================================
Prompt::DecoratePromptLeptonImproved::DecoratePromptLeptonImproved(const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm(name, pSvcLocator)
{}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonImproved::initialize()
{
  ATH_MSG_DEBUG("Initializing DecoratePromptLeptonImproved...");
  ATH_MSG_DEBUG("m_leptonsName = " << m_leptonsName);
  if (m_printTime)
  {
    //
    // Reset timers
    //
    m_timerAll .Reset();
    m_timerExec.Reset();
    m_timerMuon.Reset();
    m_timerElec.Reset();

    //
    // Start full timer
    //
    m_timerAll.Start();
  }

  ATH_CHECK(m_trackJetsKey.initialize());
  ATH_CHECK(m_primaryVertexKey.initialize());
  ATH_CHECK(m_clusterContainerKey.initialize());

  ATH_MSG_DEBUG("Initializing " << m_electronsKey);

  ATH_CHECK(m_electronsKey.initialize());
  ATH_CHECK(m_muonsKey.initialize());

  ATH_MSG_DEBUG("Number of int vars to read: " << m_stringIntVars.size());
  ATH_MSG_DEBUG("Number of float vars to read: " << m_stringFloatVars.size());

  // Setup variable holder
  m_vars = std::make_unique<Prompt::VarHolder>();

  //
  // Read vector<string> vars into vector<Var> and append
  //
  m_intVars   = m_vars->readVectorVars(m_stringIntVars);
  m_floatVars = m_vars->readVectorVars(m_stringFloatVars);

  m_allVars.insert(m_allVars.end(), m_intVars  .begin(), m_intVars  .end());
  m_allVars.insert(m_allVars.end(), m_floatVars.begin(), m_floatVars.end());

  m_varTMVA.resize(m_allVars.size());

  //
  // Fill decorator maps
  //
  ATH_CHECK(initializeDecorators());

  //
  // Initialize const accessors
  //
  initializeConstAccessors();

  ATH_MSG_DEBUG("Initialized DecoratePromptLeptonImproved.");

  return StatusCode::SUCCESS;

  //
  // Initialize TMVA Reader
  //
  // TODO: need to implement
}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonImproved::finalize()
{
  if(m_printTime) {
    //
    // Print full time stopwatch
    //
    m_timerAll.Stop();

    ATH_MSG_INFO("Real time: " << m_timerAll.RealTime() << "\t CPU time: " << m_timerAll.CpuTime());

    ATH_MSG_INFO("Execute time: " << PrintResetStopWatch(m_timerExec));
    ATH_MSG_INFO("Muon    time: " << PrintResetStopWatch(m_timerMuon));
    ATH_MSG_INFO("Elec    time: " << PrintResetStopWatch(m_timerElec));
  }

  return StatusCode::SUCCESS;
}


//=============================================================================
StatusCode Prompt::DecoratePromptLeptonImproved::execute()
{

  //
  // Start execute timer
  //
  TimerScopeHelper timer(m_timerExec);

  //
  // Retrieve containers from evtStore
  //
  SG::ReadHandle<xAOD::JetContainer> trackJets(m_trackJetsKey);
  SG::ReadHandle<xAOD::VertexContainer> vertices(m_primaryVertexKey);
  SG::ReadHandle<xAOD::CaloClusterContainer> clusters(m_clusterContainerKey);

  ATH_MSG_DEBUG("======================================="
    << "\n\t\t\t  Size of vertex    container: " << vertices ->size()
    << "\n\t\t\t  Size of track jet container: " << trackJets->size()
    << "\n-----------------------------------------------------------------");

  //
  // Find default Primary Vertex
  //
  const xAOD::Vertex *primaryVertex = nullptr;

  for(const xAOD::Vertex *vertex: *vertices) {
    if(vertex->vertexType() == xAOD::VxType::PriVtx) {
      primaryVertex = vertex;
      break;
    }
  }

  if(m_leptonsName == "Electrons") {
    //
    // Process electrons
    //
    ATH_MSG_DEBUG("Reading " << m_electronsKey);
    SG::ReadHandle<xAOD::ElectronContainer> electrons(m_electronsKey);

    for(const xAOD::Electron *elec: *electrons) {
      decorateElec(*elec, *trackJets, *clusters, primaryVertex);
    }
  } else if(m_leptonsName == "Muons") {
    //
    // Process muons
    //
    ATH_MSG_DEBUG("Reading " << m_muonsKey);
    SG::ReadHandle<xAOD::MuonContainer> muons(m_muonsKey);

    for(const xAOD::Muon *muon: *muons) {
      decorateMuon(*muon, *trackJets, primaryVertex);
      ATH_MSG_DEBUG("Muon decorated");
    }
  } else {
    ATH_MSG_ERROR("Must specify Electrons or Muons");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode Prompt::DecoratePromptLeptonImproved::initializeDecorators()
{
  //
  // Fill short variable map
  //
  for(Prompt::Def::Var &var: m_intVars) {
    SG::AuxElement::Decorator<short> shortDecorator(m_inputVarDecoratePrefix + m_vars->asStr(var));

    if(!m_shortMap.insert(shortDecoratorMap::value_type(var, shortDecorator)).second) {
      ATH_MSG_ERROR("Instantiation of Decorator class failed for short decorator map for var: " << m_vars->asStr(var));
      return StatusCode::FAILURE;
    }
  }

  //
  // Fill float variable map
  //
  for(Prompt::Def::Var &var: m_floatVars) {
    SG::AuxElement::Decorator<float> floatDecorator(m_inputVarDecoratePrefix + m_vars->asStr(var));

    if(!m_floatMap.insert(floatDecoratorMap::value_type(var, floatDecorator)).second) {
      ATH_MSG_ERROR("Instantiation of Decorator class failed for float decorator map for var: " << m_vars->asStr(var));
      return StatusCode::FAILURE;
    }
  }

  //
  // Fill additional variables
  //

  for(const std::string &evar: m_extraDecoratorFloatVars) {
    const Def::Var ekey = m_vars->registerDynamicVar(evar);

    if(ekey == Def::NONE) {
      ATH_MSG_ERROR("Failed to create key for variable name=" << evar);
      return StatusCode::FAILURE;
    }

    if(m_floatMap.find(ekey) != m_floatMap.end()) {
      ATH_MSG_DEBUG("Ignore duplicate variable name=" << evar);
      continue;
    }

    if(!m_floatMap.insert(floatDecoratorMap::value_type(ekey, SG::AuxElement::Decorator<float>(m_inputVarDecoratePrefix + evar))).second) {
      ATH_MSG_ERROR("Failed to add variable: \"" << evar << "\"");
      return StatusCode::FAILURE;
    }
  }

  for(const std::string &evar: m_extraDecoratorShortVars) {
    const Def::Var ekey = m_vars->registerDynamicVar(evar);

    if(ekey == Def::NONE) {
      ATH_MSG_ERROR("Failed to create key for variable name=" << evar);
      return StatusCode::FAILURE;
    }

    if(m_shortMap.find(ekey) != m_shortMap.end()) {
      ATH_MSG_DEBUG("Ignore duplicate variable name=" << evar);
      continue;
    }

    if(!m_shortMap.insert(shortDecoratorMap::value_type(ekey, SG::AuxElement::Decorator<short>(m_inputVarDecoratePrefix + evar))).second) {
      ATH_MSG_ERROR("Failed to add variable: \"" << evar << "\"");
      return StatusCode::FAILURE;
    }
  }

  //
  // Veto the decorators of the variables in the veto-list if exist
  //
  for(const std::string &vvar: m_vetoDecoratorFloatVars) {
    const Def::Var vkey = m_vars->registerDynamicVar(vvar);

    if(vkey == Def::NONE) {
      ATH_MSG_ERROR("Failed to create key for variable name=" << vvar);
      return StatusCode::FAILURE;
    }

    floatDecoratorMap::iterator iter = m_floatMap.find(vkey);

    if(iter != m_floatMap.end()) {
      ATH_MSG_DEBUG("Remove the variable from the veto-list, name=" << vvar);
      m_floatMap.erase(iter);
    }
  }

  for(const std::string &vvar: m_vetoDecoratorShortVars) {
    const Def::Var vkey = m_vars->registerDynamicVar(vvar);

    if(vkey == Def::NONE) {
      ATH_MSG_ERROR("Failed to create key for variable name=" << vvar);
      return StatusCode::FAILURE;
    }

    shortDecoratorMap::iterator iter = m_shortMap.find(vkey);

    if(iter != m_shortMap.end()) {
      ATH_MSG_DEBUG("Remove the variable from the veto-list, name=" << vvar);
      m_shortMap.erase(iter);
    }
  }

  // Print decorator counts
  ATH_MSG_DEBUG("Added " << m_shortMap.size() << " short decorators");
  ATH_MSG_DEBUG("Added " << m_floatMap.size() << " float decorators");

  // Instantiate MVA X bin
  if(m_leptonPtBinsVector.size() < 2) {
    ATH_MSG_ERROR("Invalid PtBins size=" << m_leptonPtBinsVector.size());
    return StatusCode::FAILURE;
  }

  std::unique_ptr<double []> PtBins = std::make_unique<double []>(m_leptonPtBinsVector.size());

  for(unsigned i = 0; i < m_leptonPtBinsVector.size(); i++) {
    PtBins[i] = m_leptonPtBinsVector[i];
  }

  m_leptonPtBinHist = std::make_unique<TH1D>("PtBin", "PtBin", m_leptonPtBinsVector.size() - 1, PtBins.get());

  return StatusCode::SUCCESS;
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::initializeConstAccessors()
{
  //
  // Instantiate isolation accessors
  //
  m_accessCalIsolation30       = std::make_unique<AccessFloat> ("topoetcone30");
  m_accessTrackIsolation30     = std::make_unique<AccessFloat> ("ptvarcone30");
  m_accessTrackIsolation30TTVA = std::make_unique<AccessFloat> ("ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500");

  m_accessDeepSecondaryVertex  = std::make_unique<AccessVertex>(m_vertexLinkName);

  //
  // Instantiate accessors for the muon specific variables
  //
  m_accessMuonCalE            = std::make_unique<AccessFloat> ("calE");
  m_accessMuonParamEnergyLoss = std::make_unique<AccessFloat> ("ParamEnergyLoss");

  //
  // Instantiate accessors for RNN variables
  //
  for(const std::string &name: m_accessorRNNVars) {
    const Def::Var akey = m_vars->registerDynamicVar(name);

    if(m_accessRNNMap.insert(floatAccessorMap::value_type(akey, AccessFloat(name))).second) {
      ATH_MSG_DEBUG("Add float RNN accessor: " << name);
    }
    else {
      ATH_MSG_WARNING("Skip duplicate float accessor: " << name);
    }
  }
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::decorateElec(
  const xAOD::Electron &electron,
  const xAOD::JetContainer &trackJets,
  const xAOD::CaloClusterContainer &clusters,
  const xAOD::Vertex *primaryVertex
)
{
  //
  // Find nearest track jet to electron
  //
  TimerScopeHelper timer(m_timerElec);

  Prompt::VarHolder vars;

  const xAOD::Jet *track_jet = findTrackJet(electron, trackJets);

  if(track_jet) {
    //
    // Get muon calorimeter energy variable, RNN and secondary vertex variables
    //
    getElectronAnpVariables(electron, clusters, vars, primaryVertex);

    //
    // Get mutual variables, passing track as argument
    //
    getMutualVariables(electron, *track_jet, electron.trackParticle(), vars);

    //
    // Pass variables to TMVA
    //
    // TODO: setup TMVA decoration
  }
  else {
    //
    // Decorate electron with default values
    //
    fillVarDefault(vars);

    ATH_MSG_DEBUG("No track jet found near to electron");
  }

  //
  // Decorate electron with input vars and BDT weight
  //
  decorateAuxLepton(electron, vars);
}


//=============================================================================
void Prompt::DecoratePromptLeptonImproved::decorateMuon(
  const xAOD::Muon         &muon,
  const xAOD::JetContainer &trackJets,
  const xAOD::Vertex *primaryVertex
)
{
  //
  // Find nearest track jet to muon
  //
  TimerScopeHelper timer(m_timerMuon);

  Prompt::VarHolder vars;

  const xAOD::Jet *track_jet = findTrackJet(muon, trackJets);

  if(track_jet) {
    //
    // Get muon calorimeter energy variable, RNN and secondary vertex variables
    //
    getMuonAnpVariables(muon, vars, primaryVertex);

    //
    // Get mutual variables, passing track as argument
    //
    getMutualVariables(muon, *track_jet, muon.primaryTrackParticle(), vars);

    //
    // Add variables to TMVA Reader
    //
    // TODO: setup TMVA decoration
  }
  else {
    //
    // Decorate muon with default values
    //
    fillVarDefault(vars);

    ATH_MSG_DEBUG("No track jet found near to muon");
  }

  //
  // Decorate muon with input vars and BDT weight
  //
  decorateAuxLepton(muon, vars);
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::getElectronAnpVariables(
  const xAOD::Electron             &elec,
  const xAOD::CaloClusterContainer &clusters,
  Prompt::VarHolder                &vars,
  const xAOD::Vertex               *primaryVertex
)
{
  //
  // Get Muon variables - calorimeter
  //
  double sumCoreEt_large = 0.0, CaloClusterSumEtRel = 0.0;

  if(elec.caloCluster()) {
    const double elec_calEta = elec.caloCluster()->eta();
    const double elec_calPhi = elec.caloCluster()->phi();

    for(const xAOD::CaloCluster *cluster: clusters) {
      const double deta =                      elec_calEta - cluster->eta();
      const double dphi = TVector2::Phi_mpi_pi(elec_calPhi - cluster->phi());
      const double dr   = std::sqrt(deta*deta + dphi*dphi);

      if(dr < m_elecMinCalErelConeSize) {
        sumCoreEt_large += cluster->pt();
      }
    }
  }

  if(elec.pt() > 0.0) CaloClusterSumEtRel = sumCoreEt_large/elec.pt();

  vars.addVar(Prompt::Def::CaloClusterSumEtRel, CaloClusterSumEtRel);

  //
  // Get lepton isolation variables
  //
  const double Topoetcone30rel = accessIsolation(*m_accessCalIsolation30, elec);
  const double Ptvarcone30rel  = accessIsolation(*m_accessTrackIsolation30, elec);

  vars.addVar(Prompt::Def::Topoetcone30rel, Topoetcone30rel);
  vars.addVar(Prompt::Def::Ptvarcone30rel,  Ptvarcone30rel);

  //
  // Get secondary vertex variable
  //
  std::vector<double> goodVertexNdistLong;

  if(m_accessDeepSecondaryVertex->isAvailable(elec)) {
    std::vector<ElementLink<xAOD::VertexContainer> > vtxLinks = (*m_accessDeepSecondaryVertex)(elec);

    for(ElementLink<xAOD::VertexContainer> &vtxLink: vtxLinks) {
      if(!vtxLink.isValid()) {
        ATH_MSG_WARNING("VertexContainer : invalid link");
        continue;
      }

      const xAOD::Vertex *vtx = *vtxLink;

      const double fitProb = Prompt::getVertexFitProb(vtx);

      if(fitProb < m_vertexMinChiSquaredProb) {
        continue;
      }

      const double theta = std::acos(getVertexCosThetaWithLepDir(elec, vtx, primaryVertex));

      if      (theta < m_vertexMinThetaBarrElec && std::fabs(elec.eta()) <= m_vertexBarrEcapAbsEtaAt) continue;
      else if (theta < m_vertexMinThetaEcapElec && std::fabs(elec.eta()) >  m_vertexBarrEcapAbsEtaAt) continue;

      const double vertex_ndist_long = getVertexLongitudinalNormDist(elec, vtx, primaryVertex);

      goodVertexNdistLong.push_back(vertex_ndist_long);
    }
  }
  else {
    ATH_MSG_WARNING("VertexContainer : " << m_vertexLinkName << " not found for the electron");
  }

  double best_vertex_ndist_long = 0.0;

  if(goodVertexNdistLong.size() > 0) {
    std::sort(goodVertexNdistLong.begin(), goodVertexNdistLong.end());
    best_vertex_ndist_long = goodVertexNdistLong.back();
  }

  vars.addVar(Prompt::Def::CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx, best_vertex_ndist_long);
  vars.addVar(Prompt::Def::CandVertex_NPassVtx,                         goodVertexNdistLong.size());
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::getMuonAnpVariables(
  const xAOD::Muon   &muon,
  Prompt::VarHolder                &vars,
  const xAOD::Vertex *primaryVertex
)
{
  //
  // Get Muon variables - calorimeter
  //
  double calE = -99.0, peloss = -99.0, caloClusterERel = -99.0;

  if(muon.clusterLink().isValid()) {
    const xAOD::CaloCluster* cluster = *(muon.clusterLink());

    if(m_accessMuonCalE->isAvailable(*cluster) && m_accessMuonParamEnergyLoss->isAvailable(muon)) {
      calE   = (*m_accessMuonCalE)(*cluster);
      peloss = (*m_accessMuonParamEnergyLoss)(muon);

      caloClusterERel = calE/peloss;
    }
    else {
      ATH_MSG_WARNING("Muon calE or ParamEnergyLoss not found in auxiliary store");
    }
  }

  vars.addVar(Prompt::Def::CaloClusterERel, caloClusterERel);

  //
  // Get lepton isolation variables
  //
  const double Topoetcone30rel = accessIsolation(*m_accessCalIsolation30, muon);
  const double ptvarcone30TightTTVAPt500rel = accessIsolation(*m_accessTrackIsolation30TTVA, muon);

  vars.addVar(Prompt::Def::Topoetcone30rel,              Topoetcone30rel);
  vars.addVar(Prompt::Def::Ptvarcone30_TightTTVA_pt500rel, ptvarcone30TightTTVAPt500rel);

  //
  // Get Muon Secondary Vertex variable
  //
  std::vector<double> goodVertexNdistLong;

  if(m_accessDeepSecondaryVertex->isAvailable(muon)) {
    std::vector<ElementLink<xAOD::VertexContainer> > vtxLinks = (*m_accessDeepSecondaryVertex)(muon);
    goodVertexNdistLong.reserve(vtxLinks.size());

    for(ElementLink<xAOD::VertexContainer> &vtxLink: vtxLinks) {
      if(!vtxLink.isValid()) {
        ATH_MSG_WARNING("VertexContainer : invalid link");
        continue;
      }

      const xAOD::Vertex *vtx = *vtxLink;

      const double fitProb = Prompt::getVertexFitProb(vtx);

      if(fitProb > m_vertexMinChiSquaredProb) {
        const double vertex_ndist_long = getVertexLongitudinalNormDist(muon, vtx, primaryVertex);

        goodVertexNdistLong.push_back(vertex_ndist_long);
      }
    }
  }
  else {
    ATH_MSG_WARNING("VertexContainer : " << m_vertexLinkName << " not found for the muon");
  }

  double best_vertex_ndist_long = 0.0;

  if(goodVertexNdistLong.size() > 0) {
    std::sort(goodVertexNdistLong.begin(), goodVertexNdistLong.end());
    best_vertex_ndist_long = goodVertexNdistLong.back();
  }

  vars.addVar(Prompt::Def::CandVertex_normDistToPriVtxLongitudinalBest, best_vertex_ndist_long);
  vars.addVar(Prompt::Def::CandVertex_NPassVtx,                         goodVertexNdistLong.size());
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::getMutualVariables(
  const xAOD::IParticle     &particle,
  const xAOD::Jet           &track_jet,
  const xAOD::TrackParticle *track,
  Prompt::VarHolder         &vars
)
{
  //
  // Add lepton - jet variables to VarHolder
  //
  double PtFrac = -99.;
  double PtRel  = -99.;

  if(particle.pt() > 0.0 && track_jet.pt() > 0.0) {

    if(track) {
      PtFrac = track->pt()/track_jet.pt();
    }

    const double angle = particle.p4().Vect().Angle(track_jet.p4().Vect());

    PtRel = particle.pt() * std::sin(angle);
  }

  //
  // Add vars to VarHolder
  //
  vars.addVar(Prompt::Def::PtFrac,         PtFrac);
  vars.addVar(Prompt::Def::PtRel,          PtRel);
  vars.addVar(Prompt::Def::DRlj,           track_jet.p4().DeltaR(particle.p4()));
  vars.addVar(Prompt::Def::TrackJetNTrack, track_jet.getConstituents().size());

  //
  // Get RNN variables
  //
  for(floatAccessorMap::value_type &acc: m_accessRNNMap) {
    if(acc.second.isAvailable(particle)) {
      vars.addVar(acc.first, acc.second(particle));
    }
    else {
      ATH_MSG_WARNING("LeptonTagger RNN not found in auxiliary store for variable=" << vars.asStr(acc.first));
    }
  }

  //
  // Get lepton variables - pT X bin
  //
  const double lepPt = particle.pt();

  const double xmax = m_leptonPtBinHist->GetXaxis()->GetXmax();
  const double xmin = m_leptonPtBinHist->GetXaxis()->GetXmin();

  int curr_bin = 0;

  if(xmin < lepPt && lepPt < xmax) {
    curr_bin = m_leptonPtBinHist->FindBin(lepPt);
  }
  else if (!(lepPt < xmax)) {
    curr_bin = m_leptonPtBinHist->GetNbinsX();
  }
  else if (!(lepPt > xmin)) {
    curr_bin = 1;
  }

  vars.addVar(Prompt::Def::MVAXBin, curr_bin);
  vars.addVar(Prompt::Def::RawPt,   lepPt);

  ATH_MSG_DEBUG("getMutualVariables - lepPt = " << lepPt << ", MVAXBin = " << curr_bin);
}

//=============================================================================
float Prompt::DecoratePromptLeptonImproved::accessIsolation(
  AccessFloat           &isoAccessor,
  const xAOD::IParticle &particle
) {
  double isolation = -99., isolationrel = -99.;

  if(isoAccessor.isAvailable(particle)) {
    isolation = isoAccessor(particle);
  }
  else {
    ATH_MSG_WARNING("Lepton isolation not found in auxiliary store");
  }

  if(particle.pt() > 0.0) {
    isolationrel = isolation / particle.pt();
  }

  return isolationrel;
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::fillVarDefault(Prompt::VarHolder &vars) const
{
  //
  // Add default values to VarHolder
  //
  for(const floatDecoratorMap::value_type &dec: m_floatMap) {
    vars.addVar(dec.first, -99.0);
  }

  for(const shortDecoratorMap::value_type &dec: m_shortMap) {
    vars.addVar(dec.first, -99.0);
  }
}

//=============================================================================
void Prompt::DecoratePromptLeptonImproved::decorateAuxLepton(
  const xAOD::IParticle &particle,
  Prompt::VarHolder &vars
)
{
  //
  // Decorate lepton with input short variables
  //
  for(shortDecoratorMap::value_type &dec: m_shortMap) {
    double val = 0.0;

    if(vars.getVar(dec.first, val)) {
      dec.second(particle) = static_cast<short>(val);

      ATH_MSG_DEBUG("Short variable: " << vars.asStr(dec.first) << " = " << val);
    }
    else {
      ATH_MSG_WARNING("Short variable " << vars.asStr(dec.first) << " not decorated to lepton");
    }
  }

  //
  // Decorate lepton with input float variables
  //
  for(floatDecoratorMap::value_type &dec: m_floatMap) {
    double val = 0.0;

    if(vars.getVar(dec.first, val)) {
      dec.second(particle) = val;

      ATH_MSG_DEBUG("Float variable: " << vars.asStr(dec.first) << " = " << val);
    }
    else {
      ATH_MSG_WARNING("Float variable " << vars.asStr(dec.first) << " not decorated to lepton");
    }
  }
}

//=============================================================================
template<class T> const xAOD::Jet* Prompt::DecoratePromptLeptonImproved::findTrackJet(const T &part,
                          const xAOD::JetContainer &jets)
{
  //
  // Find nearest track jet and a return a pair of dR and xAOD::Jet*
  //
  const xAOD::Jet *minjet = 0;
  double           mindr  = 10.0;

  for(const xAOD::Jet* jet: jets) {
    const double dr = part.p4().DeltaR(jet->p4());

    if(!minjet || dr < mindr) {
      mindr  = dr;
      minjet = jet;
    }
  }


  if(minjet && mindr < m_maxLepTrackJetDR) {
    return minjet;
  }

  return 0;
}

//=============================================================================
double Prompt::DecoratePromptLeptonImproved::getVertexLongitudinalNormDist(
  const xAOD::IParticle &lepton,
  const xAOD::Vertex    *secondaryVertex,
  const xAOD::Vertex    *primaryVertex
)
{
  //
  // get the Longitudinal nomalized distance between the secondary vertex and primary vertex
  //
  if(!secondaryVertex || !primaryVertex) {
    ATH_MSG_WARNING("getVertexLongitudinalNormDist - invalid pointer of lepton/secondaryVertex/primaryVertex");
    return 0.0;
  }


  float normDist_SVPV  = 0.0;

  if(!Prompt::GetAuxVar(*secondaryVertex, normDist_SVPV,  "normDistToPriVtx")) {
    ATH_MSG_WARNING("getVertexLongitudinalNormDist - missing \"normDistToPriVtx\"");
  }

  double cos_theta = getVertexCosThetaWithLepDir(lepton, secondaryVertex, primaryVertex);

  return normDist_SVPV*cos_theta;
}

//=============================================================================
double Prompt::DecoratePromptLeptonImproved::getVertexCosThetaWithLepDir(const xAOD::IParticle &lepton,
                                                                         const xAOD::Vertex    *secondaryVertex,
                                                                         const xAOD::Vertex    *primaryVertex)
{
  //
  // get the Longitudinal nomalized distance between the secondary vertex and primary vertex
  //
  if(!secondaryVertex || !primaryVertex) {
    ATH_MSG_WARNING("GetVertexThetaWithLepDir - invalid pointer of lepton/secondaryVertex/primaryVertex");
    return 0.0;
  }

  const Amg::Vector3D sv_to_pv_v3 = secondaryVertex->position() - primaryVertex->position();

  const TVector3 sv_to_pv_t3 = TVector3(sv_to_pv_v3.x(), sv_to_pv_v3.y(), sv_to_pv_v3.z());
  const TVector3 lepton_dirt = lepton.p4().Vect();

  const double cos_theta = sv_to_pv_t3.Unit()*lepton_dirt.Unit();

  return cos_theta;
}

