/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TFCSGANEtaSlice.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// class header include
#include "ISF_FastCaloSimEvent/TFCSGANEtaSlice.h"

#include "CLHEP/Random/RandGauss.h"

#include "TFitResult.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TMath.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>


TFCSGANEtaSlice::TFCSGANEtaSlice() {}

TFCSGANEtaSlice::TFCSGANEtaSlice(int pid, int etaMin, int etaMax,
                                 const TFCSGANXMLParameters &param)
  : m_pid (pid),
    m_etaMin (etaMin),
    m_etaMax (etaMax),
    m_param (param)
{
}

TFCSGANEtaSlice::~TFCSGANEtaSlice() {
  if (m_gan_all != nullptr) {
    delete m_gan_all;
  }
  if (m_gan_low != nullptr) {
    delete m_gan_low;
  }
  if (m_gan_high != nullptr) {
    delete m_gan_high;
  }
}

bool TFCSGANEtaSlice::IsGanCorrectlyLoaded() const {
  if (m_pid == 211 || m_pid == 2212) {
    if (m_gan_all == nullptr) {
      return false;
    }
  } else {
    if (m_gan_high == nullptr || m_gan_low == nullptr) {
      return false;
    }
  }
  return true;
}

bool TFCSGANEtaSlice::LoadGAN() {
  std::string inputFileName;

  CalculateMeanPointFromDistributionOfR();
  ExtractExtrapolatorMeansFromInputs();

  if (m_pid == 211) {
    inputFileName = m_param.GetInputFolder() + "/neural_net_" +
                    std::to_string(m_pid) + "_eta_" + std::to_string(m_etaMin) +
                    "_" + std::to_string(m_etaMax) + "_All.json";
    ATH_MSG_DEBUG("Gan input file name " << inputFileName);
    m_gan_all = new TFCSGANLWTNNHandler();
    return m_gan_all->LoadGAN(inputFileName);
  } else if (m_pid == 2212) {
    inputFileName = m_param.GetInputFolder() + "/neural_net_" +
                    std::to_string(m_pid) + "_eta_" + std::to_string(m_etaMin) +
                    "_" + std::to_string(m_etaMax) + "_High10.json";
    ATH_MSG_DEBUG("Gan input file name " << inputFileName);
    m_gan_all = new TFCSGANLWTNNHandler();
    return m_gan_all->LoadGAN(inputFileName);
  } else {
    bool returnValue;
    inputFileName = m_param.GetInputFolder() + "/neural_net_" +
                    std::to_string(m_pid) + "_eta_" + std::to_string(m_etaMin) +
                    "_" + std::to_string(m_etaMax) + "_High12.json";
    m_gan_high = new TFCSGANLWTNNHandler();
    returnValue = m_gan_high->LoadGAN(inputFileName);
    if (!returnValue) {
      return returnValue;
    }

    inputFileName = m_param.GetInputFolder() + "/neural_net_" +
                    std::to_string(m_pid) + "_eta_" + std::to_string(m_etaMin) +
                    "_" + std::to_string(m_etaMax) + "_UltraLow12.json";
    m_gan_low = new TFCSGANLWTNNHandler();
    return m_gan_low->LoadGAN(inputFileName);
    return true;
  }
}

void TFCSGANEtaSlice::CalculateMeanPointFromDistributionOfR() {
  std::string rootFileName = m_param.GetInputFolder() + "/rootFiles/pid" +
                             std::to_string(m_pid) + "_E1048576_eta_" +
                             std::to_string(m_etaMin) + "_" +
                             std::to_string(m_etaMin + 5) + ".root";
  ATH_MSG_DEBUG("Opening file " << rootFileName);
  TFile *file = TFile::Open(rootFileName.c_str(), "read");
  for (int layer : m_param.GetRelevantLayers()) {
    ATH_MSG_DEBUG("Layer " << layer);
    TFCSGANXMLParameters::Binning binsInLayers = m_param.GetBinning();
    TH2D *h2 = &binsInLayers[layer];

    std::string histoName = "r" + std::to_string(layer) + "w";
    TH1D *h1 = (TH1D *)file->Get(histoName.c_str());
    if (TMath::IsNaN(h1->Integral())) {
      histoName = "r" + std::to_string(layer);
      h1 = (TH1D *)file->Get(histoName.c_str());
    }

    TAxis *x = (TAxis *)h2->GetXaxis();
    for (int ix = 1; ix <= h2->GetNbinsX(); ++ix) {
      ATH_MSG_DEBUG(ix);
      h1->GetXaxis()->SetRangeUser(x->GetBinLowEdge(ix), x->GetBinUpEdge(ix));

      double result = 0;
      if (h1->Integral() > 0 && h1->GetNbinsX() > 2) {
        TFitResultPtr res(0);

        res = h1->Fit("expo", "SQ");
        if (res >= 0 && !std::isnan(res->Parameter(0))) {
          result = res->Parameter(1);
        }
      }
      m_allFitResults[layer].push_back(result);
    }
  }
  ATH_MSG_DEBUG("Done initialisaing fits");
}

void TFCSGANEtaSlice::ExtractExtrapolatorMeansFromInputs() {
  std::string rootFileName = m_param.GetInputFolder() + "/rootFiles/pid" +
                             std::to_string(m_pid) + "_E65536_eta_" +
                             std::to_string(m_etaMin) + "_" +
                             std::to_string(m_etaMin + 5) + "_validation.root";
  ATH_MSG_DEBUG("Opening file " << rootFileName);
  TFile *file = TFile::Open(rootFileName.c_str(), "read");
  for (int layer : m_param.GetRelevantLayers()) {
    std::string branchName = "extrapWeight_" + std::to_string(layer);
    TH1D *h = new TH1D("h", "h", 100, 0.01, 1);
    TTree *tree = (TTree *)file->Get("rootTree");
    std::string command = branchName + ">>h";
    tree->Draw(command.c_str());
    m_extrapolatorWeights[layer] = h->GetMean();
    ATH_MSG_DEBUG("Extrapolation: layer " << layer << " mean "
                                          << m_extrapolatorWeights[layer]);
  }
}

TFCSGANEtaSlice::NetworkOutputs
TFCSGANEtaSlice::GetNetworkOutputs(const TFCSTruthState *truth,
                                   const TFCSExtrapolationState *extrapol,
                                   TFCSSimulationState simulstate) const {
  double randUniformZ = 0.;
  NetworkInputs inputs;

  int maxExp = 0, minExp = 0;
  if (m_pid == 22 || fabs(m_pid) == 11) {
    if (truth->P() >
        4096) { // This is the momentum, not the energy, because the split is
                // based on the samples which are produced with the momentum
      maxExp = 22;
      minExp = 12;
    } else {
      maxExp = 12;
      minExp = 6;
    }
  } else if (fabs(m_pid) == 211) {
    maxExp = 22;
    minExp = 8;
  } else if (fabs(m_pid) == 2212) {
    maxExp = 22;
    minExp = 10;
  }

  int p_min = std::pow(2, minExp);
  int p_max = std::pow(2, maxExp);
  // Keep min and max without mass offset as we do not train on antiparticles
  double Ekin_min =
      std::sqrt(std::pow(p_min, 2) + std::pow(truth->M(), 2)) - truth->M();
  double Ekin_max =
      std::sqrt(std::pow(p_max, 2) + std::pow(truth->M(), 2)) - truth->M();

  for (int i = 0; i < m_param.GetLatentSpaceSize(); i++) {
    randUniformZ = CLHEP::RandGauss::shoot(simulstate.randomEngine(), 0.5, 0.5);
    inputs["node_0"].insert(std::pair<std::string, double>(
        "variable_" + std::to_string(i), randUniformZ));
  }

  // double e = log(truth->Ekin()/Ekin_min)/log(Ekin_max/Ekin_min) ;
  // Could be uncommented , but would need the line above too
  // ATH_MSG_DEBUG( "Check label: " << e <<" Ekin:" << truth->Ekin() <<" p:" <<
  //                truth->P() <<" mass:" << truth->M() <<" Ekin_off:" <<
  //                truth->Ekin_off() << " Ekin_min:"<<Ekin_min<<"
  //                Ekin_max:"<<Ekin_max);
  // inputs["node_1"].insert ( std::pair<std::string,double>("variable_0",
  // truth->Ekin()/(std::pow(2,maxExp))) ); //Old conditioning using linear
  // interpolation, now use logaritminc interpolation
  inputs["node_1"].insert(std::pair<std::string, double>(
      "variable_0", log(truth->Ekin() / Ekin_min) / log(Ekin_max / Ekin_min)));

  if (m_param.GetGANVersion() >= 2) {
    if (false) { // conditioning on eta, should only be needed in transition
                 // regions and added only to the GANs that use it, for now all
                 // GANs have 3 conditioning inputs so filling zeros
      inputs["node_1"].insert(std::pair<std::string, double>(
          "variable_1", fabs(extrapol->IDCaloBoundary_eta())));
    } else {
      inputs["node_1"].insert(std::pair<std::string, double>("variable_1", 0));
    }
  }

  if (m_param.GetGANVersion() == 1 || m_pid == 211 || m_pid == 2212) {
    return m_gan_all->GetGraph()->compute(inputs);
  } else {
    if (truth->P() >
        4096) { // This is the momentum, not the energy, because the split is
                // based on the samples which are produced with the momentum
      ATH_MSG_DEBUG("Computing outputs given inputs for high");
      return m_gan_high->GetGraph()->compute(inputs);
    } else {
      return m_gan_low->GetGraph()->compute(inputs);
    }
  }
}

void TFCSGANEtaSlice::Print() const {
  ATH_MSG_INFO("LWTNN Handler parameters");
  ATH_MSG_INFO("  pid: " << m_pid);
  ATH_MSG_INFO("  etaMin:" << m_etaMin);
  ATH_MSG_INFO("  etaMax: " << m_etaMax);
  m_param.Print();
}
