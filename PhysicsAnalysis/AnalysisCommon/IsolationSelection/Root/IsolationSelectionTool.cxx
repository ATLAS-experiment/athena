/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include <IsolationSelection/Interp3D.h>
#include <IsolationSelection/IsolationConditionCombined.h>
#include <IsolationSelection/IsolationConditionFormula.h>
#include <IsolationSelection/IsolationConditionGraph.h>
#include <IsolationSelection/IsolationConditionHist.h>
#include <IsolationSelection/IsolationSelectionTool.h>
#include <TF2.h>
#include <TFile.h>
#include <TH3.h>
#include <TKey.h>
#include <TObjString.h>
#include <TROOT.h>
#include <xAODPrimitives/IsolationType.h>

#include "PathResolver/PathResolver.h"

namespace CP {
IsolationSelectionTool::IsolationSelectionTool(const std::string& name)
    : asg::AsgTool(name) {}
const std::vector<std::unique_ptr<IsolationWP>>&
IsolationSelectionTool::getMuonWPs() const {
  return m_muWPs;
}
const std::vector<std::unique_ptr<IsolationWP>>&
IsolationSelectionTool::getElectronWPs() const {
  return m_elWPs;
}
const std::vector<std::unique_ptr<IsolationWP>>&
IsolationSelectionTool::getPhotonWPs() const {
  return m_phWPs;
}
const std::vector<std::unique_ptr<IsolationWP>>&
IsolationSelectionTool::getObjWPs() const {
  return m_objWPs;
}
IsolationSelectionTool::~IsolationSelectionTool() = default;

StatusCode IsolationSelectionTool::initialize() {
  /// Greet the user:
  ATH_MSG_INFO("Initialising...");

  ATH_MSG_INFO("IsoDecSuffix: " << m_isoDecSuffix);

  if (!m_calibFileName.empty()) {
    std::string filename = PathResolverFindCalibFile(m_calibFileName);

    ATH_MSG_INFO("Reading input file " << m_calibFileName << " from "
                                       << filename);
    m_calibFile = std::make_unique<TFile>(filename.c_str(), "READ");

    TObjString* versionInfo{nullptr};
    m_calibFile->GetObject("VersionInfo", versionInfo);
    if (versionInfo)
      ATH_MSG_INFO("VersionInfo:" << versionInfo->String());
    else
      ATH_MSG_WARNING("VersionInfo of input file (" << filename
                                                    << ") is missing.");
  }

  if (m_doInterpE || m_doInterpM) {
    // special setting for electrons
    // do not apply interpolation in crack vicinity for topoetcone
    std::vector<std::pair<double, double>> rangeEtaNoInt;
    std::pair<double, double> apair(1.26, 1.665);
    rangeEtaNoInt.push_back(apair);
    // do not apply interpolation between Z defined and J/Psi defined cuts (pT <
    // > 15 GeV/c) for both calo and track iso
    std::vector<std::pair<double, double>> rangePtNoInt;
    apair.first = 12.5;
    apair.second = 17.5;
    rangePtNoInt.push_back(apair);
    std::map<std::string, Interp3D::VetoInterp> amap;
    Interp3D::VetoInterp veto;
    veto.xRange = rangePtNoInt;
    veto.yRange = std::vector<std::pair<double, double>>();
    amap.insert(std::make_pair(std::string("el_cutValues_ptvarcone20"), veto));
    veto.yRange = rangeEtaNoInt;
    amap.insert(std::make_pair(std::string("el_cutValues_topoetcone20"), veto));
    m_Interp = std::make_unique<Interp3D>(amap);
    m_Interp->debug(false);
  }

  /// setup working points
  if (m_phWPname != "Undefined")
    ATH_CHECK(addPhotonWP(m_phWPname));
  if (m_elWPname != "Undefined")
    ATH_CHECK(addElectronWP(m_elWPname));
  if (m_muWPname != "Undefined")
    ATH_CHECK(addMuonWP(m_muWPname));
  for (const std::string& c : m_muWPvec)
    ATH_CHECK(addMuonWP(c));
  for (const std::string& c : m_elWPvec)
    ATH_CHECK(addElectronWP(c));
  for (const std::string& c : m_phWPvec)
    ATH_CHECK(addPhotonWP(c));

  m_calibFile.reset();
#ifndef XAOD_STANDALONE
  ATH_CHECK(m_isoDecors.initialize());
#endif
  /// Return gracefully:
  return StatusCode::SUCCESS;
}

void IsolationSelectionTool::addDependencies(const std::string& container,
                                             const IsolationWP& wp) {
  if (container.empty())
    return;
  for (const std::unique_ptr<IsolationCondition>& cond : wp.conditions()) {
    for (unsigned int acc = 0; acc < cond->num_types(); ++acc) {
      m_isoDecors.emplace_back(
          container + "." +
          SG::AuxTypeRegistry::instance().getName(cond->accessor(acc).auxid()));
    }
  }
}

StatusCode IsolationSelectionTool::setIParticleCutsFrom(
    xAOD::Type::ObjectType ObjType) {
  if (ObjType == xAOD::Type::Electron) {
    m_iparAcceptInfo = &m_electronAccept;
    m_iparWPs = &m_elWPs;
  } else if (ObjType == xAOD::Type::Muon) {
    m_iparAcceptInfo = &m_muonAccept;
    m_iparWPs = &m_muWPs;
  } else if (ObjType == xAOD::Type::Photon) {
    m_iparAcceptInfo = &m_photonAccept;
    m_iparWPs = &m_phWPs;
  } else {
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode IsolationSelectionTool::addCutToWP(
    IsolationWP* wp, const std::string& key_in,
    const xAOD::Iso::IsolationType t, const std::string& expression,
    const xAOD::Iso::IsolationType isoCutRemap) {
  if (!m_calibFile) {
    ATH_MSG_ERROR("Calibration File (" << m_calibFileName << ") is missing.");
    return StatusCode::FAILURE;
  }

  std::string varname(xAOD::Iso::toCString(isoCutRemap));
  std::string key = key_in + varname;

  TH3F* calibHisto{nullptr};
  m_calibFile->GetObject(key.c_str(), calibHisto);
  if (!calibHisto) {
    ATH_MSG_FATAL(" Failed to load " << key << " from "
                                     << m_calibFile->GetName());
    return StatusCode::FAILURE;
  }
  calibHisto->SetDirectory(nullptr);
  std::unique_ptr<TH3F> histogram(calibHisto);
  std::unique_ptr<IsolationConditionHist> ich =
      std::make_unique<IsolationConditionHist>(varname, t, expression,
                                               std::move(histogram));
  if ((m_doInterpM && key.find("Muon") != std::string::npos) ||
      (m_doInterpE && key.find("Electron") != std::string::npos))
    ich->setInterp(m_Interp);
  wp->addCut(std::move(ich));

  return StatusCode::SUCCESS;
}

StatusCode IsolationSelectionTool::addCutToWP(IsolationWP* wp,
                                              const std::string& key,
                                              const xAOD::Iso::IsolationType t,
                                              const std::string& expression) {
  return addCutToWP(wp, key, t, expression, t);
}

StatusCode IsolationSelectionTool::addMuonWP(const std::string& muWPname) {
  std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(muWPname);
  if (muWPname == "HighPtTrackOnly") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ptcone20_Tight_1p25",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "1.25E03",
        false, m_isoDecSuffix));  // units are MeV!
  } else if (muWPname == "TightTrackOnly_FixedRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTrackOnly_lowPt",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000,
        "0.06*(x>50e3?1e9:x)", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTrackOnly_highPt",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000,
        "0.06*(x>50e3?x:1e9)", false, m_isoDecSuffix));
  } else if (muWPname == "Tight_FixedRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTight_track_lowPt",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000,
        "0.04*(x>50e3?1e9:x)", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTight_track_highPt",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000,
        "0.04*(x>50e3?x:1e9)", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTight_calo", xAOD::Iso::topoetcone20, "0.15*x",
        false, m_isoDecSuffix));
  } else if (muWPname == "Loose_FixedRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuLoose_track_lowPt",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000,
        "0.15*(x>50e3?1e9:x)", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuLoose_track_highPt",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000,
        "0.15*(x>50e3?x:1e9)", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuLoose_calo", xAOD::Iso::topoetcone20, "0.30*x",
        false, m_isoDecSuffix));
  } else if (muWPname == "TightTrackOnly_VarRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTrackOnly",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.06*x",
        false, m_isoDecSuffix));
  } else if (muWPname == "Tight_VarRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTight_track",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.04*x",
        false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuTight_calo", xAOD::Iso::topoetcone20, "0.15*x",
        false, m_isoDecSuffix));
  } else if (muWPname == "Loose_VarRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuLoose_track",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000, "0.15*x",
        false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "MuonFixedCutHighMuLoose_calo", xAOD::Iso::topoetcone20, "0.30*x",
        false, m_isoDecSuffix));
  } else if (muWPname == "PflowTight_FixedRad") {
    std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt500,
        xAOD::Iso::neflowisol20};
    std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "MuonPFlowTightLowPt", isoTypesLowPt,
        std::make_unique<TF2>("pflowTFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.045*(x>50e3?1e9:x)", m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "MuonPFlowTightHighPt", isoTypesHighPt,
        std::make_unique<TF2>("pflowTFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.045*(x>50e3?x:1e9)", m_isoDecSuffix));
  } else if (muWPname == "PflowTight_VarRad") {
    std::vector<xAOD::Iso::IsolationType> isoTypes{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "MuonPFlowTight", isoTypes,
        std::make_unique<TF2>("pflowTFunction", "fabs(x)+0.4*(y>0?y:0)"),
        "0.045*x", m_isoDecSuffix));
  } else if (muWPname == "PflowLoose_FixedRad") {
    std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt500,
        xAOD::Iso::neflowisol20};
    std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "MuonPFlowLooseLowPt", isoTypesLowPt,
        std::make_unique<TF2>("pflowLFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.16*(x>50e3?1e9:x)", m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "MuonPFlowLooseHighPt", isoTypesHighPt,
        std::make_unique<TF2>("pflowLFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.16*(x>50e3?x:1e9)", m_isoDecSuffix));
  } else if (muWPname == "PflowLoose_VarRad") {
    std::vector<xAOD::Iso::IsolationType> isoTypes{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "MuonPFlowLoose", isoTypes,
        std::make_unique<TF2>("pflowTFunction", "fabs(x)+0.4*(y>0?y:0)"),
        "0.16*x", m_isoDecSuffix));
  } else if (muWPname == "R3PLITasPLIVefficiencyTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {3.75},
        {2.4599999999999946, 0.0002400000000000006},
        {2.6437499999999883, 0.00023250000000000085},
        {5.576250000000033, 2.249999999999814e-05},
        {7.061249999999798, -5.2499999999991085e-05},
        {6.933482142856749, -4.553571428570058e-05},
        {7.271590909090752, -5.5909090909086746e-05},
        {5.105882352941061, -1.1764705882350721e-05},
        {4.4250000000000025, -2.97364147850582e-20},
        {4.425000000000001}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R3PLITasPLIVefficiencyTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R3PLITasPLIVefficiencyVeryTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {4.050000000000001},
        {2.5912499999999903, 0.0002625000000000011},
        {2.8012499999999214, 0.00024750000000000623},
        {5.677499999999787, 4.500000000001207e-05},
        {6.2137499999998145, 2.2500000000008228e-05},
        {7.09151785714283, -1.8749999999999094e-05},
        {8.57727272727282, -6.545454545454794e-05},
        {5.969852941176529, -1.0294117647059968e-05},
        {5.528483606557319, -2.581967213113981e-06},
        {5.324999999999999}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R3PLITasPLIVefficiencyVeryTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R3PLITasPLIVrejectionTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {3.6750000000000007},
        {2.4374999999999987, 0.00022500000000000008},
        {2.572499999999916, 0.00022500000000000666},
        {5.351249999999845, 2.2500000000008773e-05},
        {7.113749999999581, -6.749999999998147e-05},
        {7.213392857142764, -6.964285714285394e-05},
        {7.4778409090906415, -7.977272727272016e-05},
        {4.105790441176434, -1.3419117647058116e-05},
        {3.590163934426209, -4.180327868852198e-06},
        {3.1499999999999986}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R3PLITasPLIVrejectionTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R3PLITasPLIVrejectionVeryTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {3.974999999999998},
        {2.5875000000000035, 0.0002549999999999993},
        {2.8687499999999373, 0.00023250000000000495},
        {5.527499999999797, 4.500000000001146e-05},
        {6.048749999999677, 2.2500000000014293e-05},
        {7.019196428571157, -2.4107142857133378e-05},
        {8.878977272727232, -8.38636363636353e-05},
        {5.708823529411479, -1.76470588235239e-05},
        {5.215573770491751, -7.131147540982918e-06},
        {4.649999999999999}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R3PLITasPLIVrejectionVeryTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R2PLITasPLIVefficiencyTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {3.974999999999998},
        {2.789999999999986, 0.00021000000000000172},
        {2.39249999999993, 0.00025500000000000555},
        {5.737499999999933, 1.5000000000003797e-05},
        {7.413749999999598, -6.749999999998225e-05},
        {7.098214285714245, -5.3571428571427186e-05},
        {6.934090909090853, -4.909090909090761e-05},
        {5.253676470587868, -1.6176470588228218e-05},
        {4.275000000000021, -3.17095862137587e-19},
        {4.349999999999998}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R2PLITasPLIVefficiencyTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R2PLITasPLIVefficiencyVeryTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {4.199999999999999},
        {2.9625000000000026, 0.00022499999999999956},
        {2.7674999999999454, 0.0002550000000000043},
        {5.752499999999801, 4.500000000001117e-05},
        {6.2887499999995, 2.2500000000022112e-05},
        {7.265625000000208, -2.4107142857150163e-05},
        {8.287500000000128, -5.863636363636705e-05},
        {5.99329044117658, -1.194852941176682e-05},
        {5.653893442622939, -5.5327868852457475e-06},
        {5.25}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R2PLITasPLIVefficiencyVeryTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R2PLITasPLIVrejectionTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {3.8999999999999986},
        {2.853749999999994, 0.00018750000000000065},
        {2.321250000000002, 0.0002474999999999998},
        {5.426250000000083, 2.2499999999995095e-05},
        {7.312499999999872, -7.499999999999437e-05},
        {7.552232142857126, -8.303571428571374e-05},
        {7.140340909090713, -7.022727272726753e-05},
        {4.7727941176470186, -2.499999999999921e-05},
        {3.679918032786865, -5.409836065573509e-06},
        {3.0}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R2PLITasPLIVrejectionTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R2PLITasPLIVrejectionVeryTight") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {5500.0,  10000.0, 15000.0,
                                            20000.0, 25000.0, 32000.0,
                                            43000.0, 60000.0, 95000.0};
    const std::vector<std::vector<double>> parameters = {
        {4.199999999999999},
        {2.9887499999999982, 0.0002175000000000001},
        {2.9287500000000426, 0.0002324999999999965},
        {5.602499999999976, 4.500000000000134e-05},
        {6.262499999999722, 1.5000000000012243e-05},
        {7.651339285714248, -4.5535714285712985e-05},
        {8.947159090908924, -8.659090909090467e-05},
        {5.994117647058617, -2.3529411764701903e-05},
        {4.8565573770491985, -2.213114754098622e-06},
        {4.349999999999998}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R2PLITasPLIVrejectionVeryTight", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R3PLITVeryLoose") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {
        15000.0, 20000.0,  25000.0,  30000.0,  40000.0, 50000.0,
        75000.0, 100000.0, 110000.0, 120000.0, 160000.0};
    const std::vector<std::vector<double>> parameters = {
        {-0.3040909, 0.0002809},
        {1.56375, 9.75e-05},
        {1.22625, 9.75e-05},
        {1.4625, 7.5e-05},
        {1.4622506, 6.45e-05},
        {1.6695205, 4.62e-05},
        {1.9120284, 3.31e-05},
        {2.9462838, 1.26e-05},
        {3.375},
        {3.45},
        {3.225},
        {3.3}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R3PLITVeryLoose", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else if (muWPname == "R2PLITVeryLoose") {
    const std::vector<std::string>& isoTypes = {"PLIT_TPLTmu_pmuxpromp",
                                                "PLIT_TPLTmu_pnpxall"};
    const std::vector<double> boundaries = {
        15000.0, 20000.0,  25000.0,  30000.0,  40000.0, 50000.0,
        75000.0, 100000.0, 110000.0, 120000.0, 160000.0};
    const std::vector<std::vector<double>> parameters = {{0.0886364, 0.0002536},
                                                         {1.8651869, 8.55e-05},
                                                         {1.7625, 7.5e-05},
                                                         {1.4625, 7.5e-05},
                                                         {1.5238217, 6.21e-05},
                                                         {1.743375, 4.43e-05},
                                                         {2.1522807, 2.78e-05},
                                                         {2.3787162, 1.74e-05},
                                                         {3.3},
                                                         {3.45},
                                                         {3.15},
                                                         {3.075}};
    const std::string cutFunction =
        createPieceWisePolinomialFunction(boundaries, parameters, true);
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "R2PLITVeryLoose", isoTypes,
        std::make_unique<TF2>("muonPLIT", "TMath::Log(x / y)"), cutFunction,
        m_isoDecSuffix, true));
  } else {
    ATH_MSG_ERROR("Unknown muon isolation WP: " << muWPname);
    return StatusCode::FAILURE;
  }
  m_muonAccept.addCut(wp->name(), wp->name());
#ifndef XAOD_STANDALONE
  addDependencies(m_inMuonContainer, *wp);
#endif
  m_muWPs.push_back(std::move(wp));
  return StatusCode::SUCCESS;
}

StatusCode IsolationSelectionTool::addPhotonWP(const std::string& phWPname) {
  std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(phWPname);
  if (phWPname == "TightCaloOnly") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_calo40", xAOD::Iso::topoetcone40, "0.022*x+2450", false,
        m_isoDecSuffix));
  } else if (phWPname == "FixedCutTight") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_calo40", xAOD::Iso::topoetcone40, "0.022*x+2450", false,
        m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_track20", xAOD::Iso::ptcone20, "0.05*x", false,
        m_isoDecSuffix));
  } else if (phWPname == "FixedCutLoose") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_calo20", xAOD::Iso::topoetcone20, "0.065*x", false,
        m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_track20", xAOD::Iso::ptcone20, "0.05*x", false,
        m_isoDecSuffix));
  } else if (phWPname == "Tight") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_calo40", xAOD::Iso::topoetcone40, "0.022*x+2450", false,
        m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_Tighttrack20",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.05*x", false,
        m_isoDecSuffix));
  } else if (phWPname == "Loose") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_calo20", xAOD::Iso::topoetcone20, "0.065*x", false,
        m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "PhFixedCut_Tighttrack20",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000, "0.05*x", false,
        m_isoDecSuffix));
  } else {
    ATH_MSG_ERROR("Unknown photon isolation WP: " << phWPname);
    return StatusCode::FAILURE;
  }

  m_photonAccept.addCut(wp->name(), wp->name());
#ifndef XAOD_STANDALONE
  addDependencies(m_inPhotContainer, *wp);
#endif
  m_phWPs.push_back(std::move(wp));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode IsolationSelectionTool::addElectronWP(const std::string& elWPname) {
  std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(elWPname);

  if (elWPname == "HighPtCaloOnly") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "FCHighPtCaloOnly_calo", xAOD::Iso::topoetcone20,
        "std::max(0.015*x,3.5E3)", false, m_isoDecSuffix));  // units are MeV!
  } else if (elWPname == "Tight_VarRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecTight_track",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000,
        "0.06*x", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecTight_calo", xAOD::Iso::topoetcone20, "0.06*x", false,
        m_isoDecSuffix));
  } else if (elWPname == "Loose_VarRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecLoose_track",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000,
        "0.15*x", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecLoose_calo", xAOD::Iso::topoetcone20, "0.20*x", false,
        m_isoDecSuffix));
  } else if (elWPname == "TightTrackOnly_VarRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecTightTrackOnly",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000,
        "0.06*x", false, m_isoDecSuffix));
  } else if (elWPname == "TightTrackOnly_FixedRad") {
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecTightTrackOnly_lowPt",
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000,
        "0.06*(x>50e3?1e9:x)", false, m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionFormula>(
        "ElecTightTrackOnly_highPt",
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000,
        "0.06*(x>50e3?x:1e9)", false, m_isoDecSuffix));
  } else if (elWPname == "PflowTight_FixedRad") {
    std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
        xAOD::Iso::neflowisol20};
    std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "ElecPFlowTightLowPt", isoTypesLowPt,
        std::make_unique<TF2>("pflowTFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.045*(x>50e3?1e9:x)", m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "ElecPFlowTightHighPt", isoTypesHighPt,
        std::make_unique<TF2>("pflowTFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.045*(x>50e3?x:1e9)", m_isoDecSuffix));
  } else if (elWPname == "PflowTight") {
    std::vector<xAOD::Iso::IsolationType> isoTypes{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "ElecPFlowTight", isoTypes,
        std::make_unique<TF2>("pflowLFunction", "fabs(x)+0.4*(y>0?y:0)"),
        "0.045*x", m_isoDecSuffix));
  } else if (elWPname == "PflowLoose_FixedRad") {
    std::vector<xAOD::Iso::IsolationType> isoTypesHighPt{
        xAOD::Iso::ptcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
        xAOD::Iso::neflowisol20};
    std::vector<xAOD::Iso::IsolationType> isoTypesLowPt{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "ElecPFlowLooseLowPt", isoTypesLowPt,
        std::make_unique<TF2>("pflowLFunctionLowPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.16*(x>50e3?1e9:x)", m_isoDecSuffix));
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "ElecPFlowLooseHighPt", isoTypesHighPt,
        std::make_unique<TF2>("pflowLFunctionHighPt", "fabs(x)+0.4*(y>0?y:0)"),
        "0.16*(x>50e3?x:1e9)", m_isoDecSuffix));
  } else if (elWPname == "PflowLoose") {
    std::vector<xAOD::Iso::IsolationType> isoTypes{
        xAOD::Iso::ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
        xAOD::Iso::neflowisol20};
    wp->addCut(std::make_unique<IsolationConditionCombined>(
        "ElecPFlowLoose", isoTypes,
        std::make_unique<TF2>("pflowLFunction", "fabs(x)+0.4*(y>0?y:0)"),
        "0.16*x", m_isoDecSuffix));
  } else if (elWPname == "isolPLITVeryTightRun3" ||
             elWPname == "isolPLITTightRun3" ||
             elWPname == "isolPLITVeryLooseRun3" ||
	     elWPname == "isolPLITVeryTightRun2" ||
             elWPname == "isolPLITTightRun2" ||
             elWPname == "isolPLITVeryLooseRun2"     
	     ) {
    // open the file path to read out WP definition file
    // if (!m_filePathName.empty()) { should never be empty --> default value
    std::string filename =
        PathResolverFindCalibFile(m_filePathName + "/" + elWPname + ".root");

    ATH_MSG_INFO("Reading input file " << elWPname << " from " << m_filePathName
                                       << "  " << filename << ": " << (m_filePathName + "/" + elWPname + ".root"));
    m_WPdefinitionFile = std::make_unique<TFile>(filename.c_str(), "READ");

    if (!m_WPdefinitionFile || m_WPdefinitionFile->IsZombie()) {
      ATH_MSG_ERROR("Error opening file " << filename);
      return StatusCode::FAILURE;
    }

    // read out histograms/graphs
    TH1F* binning = dynamic_cast<TH1F*>(m_WPdefinitionFile->Get("binning"));
    if (!binning) {
      ATH_MSG_ERROR(
          "Could not retrieve binning histogram in filename=" << filename);
      return StatusCode::FAILURE;
    }

    std::vector<std::unique_ptr<TGraph>> cutGraphUPtr;

    TIter nextkey(m_WPdefinitionFile->GetListOfKeys());
    TKey* key;

    while ((key = (TKey*)nextkey())) {
      TObject* obj = key->ReadObj();
      if (obj->InheritsFrom(TGraph::Class())) {
	std::unique_ptr<TGraph> graph(static_cast<TGraph*>(obj));
	cutGraphUPtr.push_back(std::move(graph));

        // keep this as hint on how to read out RunNumber validity
        // TObjArray* tokens = Obj_name.Tokenize("_");
        // int n = tokens->GetEntries();
        // double min_runnumber = ((TObjString*)tokens->At(n -
        // 4))->GetString().Atof(); double max_runnumber =
        // ((TObjString*)tokens->At(n - 3))->GetString().Atof();
      }
    }

    static const std::vector<std::string> isoTypes = {"PLIT_PLITel_pelxpromp",
                                                      "PLIT_PLITel_pnpxall"};

    if (elWPname == "isolPLITVeryTightRun3") {
      wp->addCut(std::make_unique<IsolationConditionGraph>(
          "isolPLITVeryTightRun3", isoTypes,
          std::make_unique<TF2>("elePLIT", "TMath::Log(x / y)"), std::move(cutGraphUPtr),
          std::make_unique<TH1F>(*binning), m_isoDecSuffix.value(), false));
    } else if (elWPname == "isolPLITTightRun3") {
      wp->addCut(std::make_unique<IsolationConditionGraph>(
          "isolPLITTightRun3", isoTypes,
          std::make_unique<TF2>("elePLIT", "TMath::Log(x / y)"), std::move(cutGraphUPtr),
          std::make_unique<TH1F>(*binning), m_isoDecSuffix.value(), false));
    } else if (elWPname == "isolPLITVeryLooseRun3") {
      wp->addCut(std::make_unique<IsolationConditionGraph>(
          "isolPLITVeryLooseRun3", isoTypes,
          std::make_unique<TF2>("elePLIT", "TMath::Log(x / y)"), std::move(cutGraphUPtr),
          std::make_unique<TH1F>(*binning), m_isoDecSuffix.value(), false));
    } 
    else if (elWPname == "isolPLITVeryTightRun2") {
      wp->addCut(std::make_unique<IsolationConditionGraph>(
          "isolPLITVeryTightRun2", isoTypes,
          std::make_unique<TF2>("elePLIT", "TMath::Log(x / y)"), std::move(cutGraphUPtr),
          std::make_unique<TH1F>(*binning), m_isoDecSuffix.value(), false));
    } else if (elWPname == "isolPLITTightRun2") {
      wp->addCut(std::make_unique<IsolationConditionGraph>(
          "isolPLITTightRun2", isoTypes,
          std::make_unique<TF2>("elePLIT", "TMath::Log(x / y)"), std::move(cutGraphUPtr),
          std::make_unique<TH1F>(*binning), m_isoDecSuffix.value(), false));
    } else if (elWPname == "isolPLITVeryLooseRun2") {
      wp->addCut(std::make_unique<IsolationConditionGraph>(
          "isolPLITVeryLooseRun2", isoTypes,
          std::make_unique<TF2>("elePLIT", "TMath::Log(x / y)"), std::move(cutGraphUPtr),
          std::make_unique<TH1F>(*binning), m_isoDecSuffix.value(), false));
    } 
 else {
      ATH_MSG_ERROR("Unknown electron isolation WP: " << elWPname);
      return StatusCode::FAILURE;
    }
  } else {
    ATH_MSG_ERROR("Unknown electron isolation WP: " << elWPname);
    return StatusCode::FAILURE;
  }

  m_electronAccept.addCut(wp->name(), wp->name());
#ifndef XAOD_STANDALONE
  addDependencies(m_inElecContainer, *wp);
#endif
  m_elWPs.push_back(std::move(wp));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode IsolationSelectionTool::addUserDefinedWP(
    const std::string& WPname, xAOD::Type::ObjectType ObjType,
    std::vector<std::pair<xAOD::Iso::IsolationType, std::string>>& cuts,
    std::string key, IsoWPType type) {
  std::vector<std::unique_ptr<IsolationWP>>* wps(nullptr);
  asg::AcceptInfo* ac = nullptr;
  if (ObjType == xAOD::Type::Electron) {
    if (key == "")
      key = m_elWPKey;
    wps = &m_elWPs;
    ac = &m_electronAccept;
  } else if (ObjType == xAOD::Type::Muon) {
    if (key == "")
      key = m_muWPKey;
    wps = &m_muWPs;
    ac = &m_muonAccept;
  } else if (ObjType == xAOD::Type::Photon) {
    if (key == "")
      key = m_phWPKey;
    wps = &m_phWPs;
    ac = &m_photonAccept;
  } else if (ObjType == xAOD::Type::Other) {
    if (key == "")
      return StatusCode::FAILURE;
    wps = &m_objWPs;
    ac = &m_objAccept;
  } else {
    return StatusCode::FAILURE;
  }

  std::unique_ptr<IsolationWP> wp = std::make_unique<IsolationWP>(WPname);
  if (type == Efficiency) {
    for (auto& c : cuts)
      ATH_CHECK(addCutToWP(wp.get(), key, c.first, c.second));
  } else if (type == Cut) {
    for (auto& c : cuts)
      wp->addCut(std::make_unique<IsolationConditionFormula>(
          xAOD::Iso::toCString(c.first), c.first, c.second));
  } else {
    ATH_MSG_ERROR("Unknown isolation WP type -- should not happen.");
    return StatusCode::FAILURE;
  }

  ac->addCut(wp->name(), wp->name());
  wps->push_back(std::move(wp));
  return StatusCode::SUCCESS;
}

StatusCode IsolationSelectionTool::addWP(const std::string& WP,
                                         xAOD::Type::ObjectType ObjType) {
  if (ObjType == xAOD::Type::Electron) {
    return addElectronWP(WP);
  } else if (ObjType == xAOD::Type::Muon) {
    return addMuonWP(WP);
  } else if (ObjType == xAOD::Type::Photon) {
    return addPhotonWP(WP);
  }

  return StatusCode::FAILURE;
}
StatusCode IsolationSelectionTool::addWP(std::unique_ptr<IsolationWP> wp,
                                         xAOD::Type::ObjectType ObjType) {
  if (ObjType == xAOD::Type::Electron) {
    m_electronAccept.addCut(wp->name(), wp->name());
    m_elWPs.push_back(std::move(wp));
  } else if (ObjType == xAOD::Type::Muon) {
    m_muonAccept.addCut(wp->name(), wp->name());
    m_muWPs.push_back(std::move(wp));

  } else if (ObjType == xAOD::Type::Photon) {
    m_photonAccept.addCut(wp->name(), wp->name());
    m_phWPs.push_back(std::move(wp));

  } else if (ObjType == xAOD::Type::Other) {
    m_objAccept.addCut(wp->name(), wp->name());
    m_objWPs.push_back(std::move(wp));
  } else {
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
template <typename T>
void IsolationSelectionTool::evaluateWP(
    const T& x, const std::vector<std::unique_ptr<IsolationWP>>& WP,
    asg::AcceptData& accept) const {
  accept.clear();
  for (const std::unique_ptr<IsolationWP>& i : WP) {
    if (i->accept(x))
      accept.setCutResult(i->name(), true);
  }
}
asg::AcceptData IsolationSelectionTool::accept(const xAOD::Photon& x) const {
  asg::AcceptData accept(&m_photonAccept);
  evaluateWP(x, m_phWPs, accept);
  return accept;
}

asg::AcceptData IsolationSelectionTool::accept(const xAOD::Electron& x) const {
  asg::AcceptData accept(&m_electronAccept);
  evaluateWP(x, m_elWPs, accept);
  return accept;
}

asg::AcceptData IsolationSelectionTool::accept(const xAOD::Muon& x) const {
  asg::AcceptData accept(&m_muonAccept);
  evaluateWP(x, m_muWPs, accept);
  return accept;
}

asg::AcceptData IsolationSelectionTool::accept(const xAOD::IParticle& x) const {
  if (x.type() == xAOD::Type::Electron) {
    asg::AcceptData accept(&m_electronAccept);
    evaluateWP(x, m_elWPs, accept);
    return accept;
  } else if (x.type() == xAOD::Type::Muon) {
    asg::AcceptData accept(&m_muonAccept);
    evaluateWP(x, m_muWPs, accept);
    return accept;
  } else if (x.type() == xAOD::Type::Photon) {
    asg::AcceptData accept(&m_photonAccept);
    evaluateWP(x, m_phWPs, accept);
    return accept;
  }

  else if (m_iparAcceptInfo && m_iparWPs) {
    asg::AcceptData accept(m_iparAcceptInfo);
    evaluateWP(x, *m_iparWPs, accept);
    return accept;
  }
  ATH_MSG_ERROR("Someting here makes really no  sense");
  return asg::AcceptData(&m_objAccept);
}

asg::AcceptData IsolationSelectionTool::accept(const strObj& x) const {
  if (x.type == xAOD::Type::Electron) {
    asg::AcceptData accept(&m_electronAccept);
    evaluateWP(x, m_elWPs, accept);
    return accept;
  } else if (x.type == xAOD::Type::Muon) {
    asg::AcceptData accept(&m_muonAccept);
    evaluateWP(x, m_muWPs, accept);
    return accept;
  } else if (x.type == xAOD::Type::Photon) {
    asg::AcceptData accept(&m_photonAccept);
    evaluateWP(x, m_phWPs, accept);
    return accept;
  } else {
    asg::AcceptData accept(&m_objAccept);
    evaluateWP(x, m_objWPs, accept);
    return accept;
  }
  return asg::AcceptData(&m_objAccept);
}

const asg::AcceptInfo& IsolationSelectionTool::getPhotonAcceptInfo() const {
  return m_photonAccept;
}

const asg::AcceptInfo& IsolationSelectionTool::getElectronAcceptInfo() const {
  return m_electronAccept;
}

const asg::AcceptInfo& IsolationSelectionTool::getMuonAcceptInfo() const {
  return m_muonAccept;
}
const asg::AcceptInfo& IsolationSelectionTool::getObjAcceptInfo() const {
  return m_objAccept;
}

std::string IsolationSelectionTool::createPieceWisePolinomialFunction(
    const std::vector<double>& boundaries,
    const std::vector<std::vector<double>>& parameters, bool isOpen) const {

  if (isOpen && boundaries.size() != parameters.size() - 1) {
    ATH_MSG_ERROR(
        "The number of region boundaries must be one less than the number of "
        "parameters for the piecewise polynomial function.");
    return "";
  } else if (!isOpen && boundaries.size() != parameters.size() + 1) {
    ATH_MSG_ERROR(
        "The number of region boundaries must be one more than the number of "
        "parameters for the piecewise polynomial function.");
    return "";
  }

  std::ostringstream oss;
  oss << std::setprecision(16);

  // a lambda for the polynomial expression
  // one could remove the zeroes in the parameters vector
  auto polynomial = [](const std::vector<double>& params) {
    std::ostringstream oss;
    oss << std::setprecision(16);
    oss << "(";
    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        oss << " + ";
      if (i == 0) {
        oss << params[i];  // constant term
      } else if (i == 1) {
        oss << params[i] << " * x";                    // linear term
      } else {                                         // higher order terms
        oss << params[i] << " * pow(x, " << i << ")";  // higher order terms
      }
    }
    oss << ")";
    return oss.str();
  };

  // Start the function definition, using concateation of ternary operators
  // if isOpen==false, just nullify the function before the first and after the
  // last boundary just create a copy of params, and insert 0.0 at the beginning
  // and end if isOpen==false
  std::vector<std::vector<double>> params = parameters;
  if (!isOpen) {
    params.insert(params.begin(), {0.0});  // add a zero vector at the beginning
    params.push_back({0.0});               // add a zero vector at the end
  }

  // now loop over the boundaries and parameters (we can ignore isOpen finally)
  for (size_t i = 0; i < boundaries.size(); ++i) {
    if (i == 0)
      oss << "(";
    oss << "(x < " << boundaries[i] << ") ? " << polynomial(params[i]) << " : ";
  }
  oss << polynomial(params.back()) << ")";  // last segment, no ternary operator

  return oss.str();
}

}  // namespace CP
