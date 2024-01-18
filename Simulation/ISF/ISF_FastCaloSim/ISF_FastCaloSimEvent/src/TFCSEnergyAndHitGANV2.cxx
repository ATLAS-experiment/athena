/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ISF_FastCaloSimEvent/TFCSEnergyAndHitGANV2.h"
#include "ISF_FastCaloSimEvent/TFCSLateralShapeParametrizationHitBase.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include "ISF_FastCaloSimEvent/TFCSCenterPositionCalculation.h"

#include "TFile.h"
#include "TF1.h"
#include "TH2D.h"

#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"

#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandFlat.h"

#if defined(__FastCaloSimStandAlone__)
#include "CLHEP/Random/TRandomEngine.h"
#else
#include <CLHEP/Random/RanluxEngine.h>
#endif

#include <iostream>
#include <fstream>
#include <limits>

//=============================================
//======= TFCSEnergyAndHitGANV2 =========
//=============================================

TFCSEnergyAndHitGANV2::TFCSEnergyAndHitGANV2(const char *name,
                                             const char *title)
    : TFCSParametrizationBinnedChain(name, title) {
  set_GANfreemem();
}

TFCSEnergyAndHitGANV2::~TFCSEnergyAndHitGANV2() {
  if (m_slice != nullptr) {
    delete m_slice;
  }
}

bool TFCSEnergyAndHitGANV2::is_match_calosample(int calosample) const {
  if (get_Binning().find(calosample) == get_Binning().cend())
    return false;
  if (get_Binning().at(calosample).GetNbinsX() == 1)
    return false;
  return true;
}

unsigned int TFCSEnergyAndHitGANV2::get_nr_of_init(unsigned int bin) const {
  if (bin >= m_bin_ninit.size())
    return 0;
  return m_bin_ninit[bin];
}

void TFCSEnergyAndHitGANV2::set_nr_of_init(unsigned int bin,
                                           unsigned int ninit) {
  if (bin >= m_bin_ninit.size()) {
    m_bin_ninit.resize(bin + 1, 0);
    m_bin_ninit.shrink_to_fit();
  }
  m_bin_ninit[bin] = ninit;
}

// initialize lwtnn network
bool TFCSEnergyAndHitGANV2::initializeNetwork(
    int pid, int etaMin, const std::string &FastCaloGANInputFolderName) {

  // initialize all necessary constants
  // FIXME eventually all these could be stored in the .json file

  ATH_MSG_INFO(
      "Using FastCaloGANInputFolderName: " << FastCaloGANInputFolderName);
  // get neural net JSON file as an std::istream object
  int etaMax = etaMin + 5;

  reset_match_all_pdgid();
  set_pdgid(pid);
  if (pid == 11)
    add_pdgid(-pid);
  if (pid == 211)
    add_pdgid(-pid);
  set_eta_min(etaMin / 100.0);
  set_eta_max(etaMax / 100.0);
  set_eta_nominal((etaMin + etaMax) / 200.0);

  int pidForXml = pid;
  if (pid != 22 && pid != 11) {
    pidForXml = 211;
  }

  int etaMid = (etaMin + etaMax) / 2;
  m_param.InitialiseFromXML(pidForXml, etaMid, FastCaloGANInputFolderName);
  m_param.Print();
  m_slice = new TFCSGANEtaSlice(pid, etaMin, etaMax, m_param);
  m_slice->Print();
  return m_slice->LoadGAN();
}

const std::string
TFCSEnergyAndHitGANV2::get_variable_text(TFCSSimulationState &simulstate,
                                         const TFCSTruthState *,
                                         const TFCSExtrapolationState *) const {
  return std::string(
      Form("layer=%d", simulstate.getAuxInfo<int>("GANlayer"_FCShash)));
}

bool TFCSEnergyAndHitGANV2::fillEnergy(
    TFCSSimulationState &simulstate, const TFCSTruthState *truth,
    const TFCSExtrapolationState *extrapol) const {
  if (!truth) {
    ATH_MSG_ERROR("Invalid truth pointer");
    return false;
  }

  if (!extrapol) {
    ATH_MSG_ERROR("Invalid extrapolation pointer");
    return false;
  }

  const int pdgId = truth->pdgid();
  const float charge = HepPDT::ParticleID(pdgId).charge();

  float Einit;
  const float Ekin = truth->Ekin();
  if (OnlyScaleEnergy())
    Einit = simulstate.E();
  else
    Einit = Ekin;

  ATH_MSG_VERBOSE("Momentum " << truth->P() << " pdgId " << truth->pdgid());
  // check that the network exists
  if (!m_slice->IsGanCorrectlyLoaded()) {
    ATH_MSG_WARNING("GAN not loaded correctly.");
    return false;
  }

  const TFCSGANEtaSlice::NetworkOutputs &outputs =
      m_slice->GetNetworkOutputs(truth, extrapol, simulstate);
  ATH_MSG_VERBOSE("network outputs size: " << outputs.size());

  const TFCSGANXMLParameters::Binning &binsInLayers = m_param.GetBinning();
  const auto ganVersion = m_param.GetGANVersion();
  const TFCSGANEtaSlice::FitResultsPerLayer &fitResults =
      m_slice->GetFitResults(); // used only if GAN version > 1

  unsigned int energyBins = outputs.size();
  ATH_MSG_VERBOSE("energy voxels size = " << energyBins);

  double totalEnergy = 0;
  for (unsigned int i = 0; i < energyBins; ++i){
    totalEnergy += outputs.at("out_" + std::to_string(i));
  }
  if (totalEnergy < 0){
    ATH_MSG_WARNING("Energy from GAN is negative, skipping particle");
    return false;   
  }

  ATH_MSG_VERBOSE("Get binning");

  simulstate.set_E(0);

  int vox = 0;
  for (const auto &element : binsInLayers) {
    int layer = element.first;
    const TH2D *h = &element.second;

    int xBinNum = h->GetNbinsX();
    int yBinNum = h->GetNbinsY();
    const TAxis *x = h->GetXaxis();

    // If only one bin in r means layer is empty, no value should be added
    if (xBinNum == 1) {
      ATH_MSG_VERBOSE(" Layer "
                      << layer
                      << " has only one bin in r, this means is it not used, "
                         "skipping (this is needed to keep correct "
                         "syncronisation of voxel and layers)");
      // delete h;
      continue;
    }

    ATH_MSG_VERBOSE(" Getting energy for Layer " << layer);

    // First fill energies
    for (int ix = 1; ix <= xBinNum; ++ix) {
      double binsInAlphaInRBin = GetAlphaBinsForRBin(x, ix, yBinNum);
      for (int iy = 1; iy <= binsInAlphaInRBin; ++iy) {
        double energyInVoxel = outputs.at("out_" + std::to_string(vox));
        ATH_MSG_VERBOSE(" Vox " << vox << " energy " << energyInVoxel
                                << " binx " << ix << " biny " << iy);

        if (energyInVoxel <= 0) {
          vox++;
          continue;
        }

        simulstate.add_E(layer, Einit * energyInVoxel);
        vox++;
      }
    }
  }

  for (unsigned int ichain = m_bin_start.back(); ichain < size(); ++ichain) {
    ATH_MSG_DEBUG("now run for all bins: " << chain()[ichain]->GetName());
    if (simulate_and_retry(chain()[ichain], simulstate, truth, extrapol) !=
        FCSSuccess) {
      return FCSFatal;
    }
  }

  vox = 0;
  for (const auto &element : binsInLayers) {
    int layer = element.first;
    const TH2D *h = &element.second;
    int xBinNum = h->GetNbinsX();
    int yBinNum = h->GetNbinsY();
    const TAxis *x = h->GetXaxis();
    const TAxis *y = h->GetYaxis();

    simulstate.setAuxInfo<int>("GANlayer"_FCShash, layer);
    TFCSLateralShapeParametrizationHitBase::Hit hit;

    // If only one bin in r means layer is empty, no value should be added
    if (xBinNum == 1) {
      ATH_MSG_VERBOSE(" Layer "
                      << layer
                      << " has only one bin in r, this means is it not used, "
                         "skipping (this is needed to keep correct "
                         "syncronisation of voxel and layers)");
      // delete h;
      continue;
    }

    if (get_number_of_bins() > 0) {
      const int bin = get_bin(simulstate, truth, extrapol);
      if (bin >= 0 && bin < (int)get_number_of_bins()) {
        for (unsigned int ichain = m_bin_start[bin];
             ichain < TMath::Min(m_bin_start[bin] + get_nr_of_init(bin),
                                 m_bin_start[bin + 1]);
             ++ichain) {
          ATH_MSG_DEBUG("for " << get_variable_text(simulstate, truth, extrapol)
                               << " run init " << get_bin_text(bin) << ": "
                               << chain()[ichain]->GetName());
          if (chain()[ichain]->InheritsFrom(
                  TFCSLateralShapeParametrizationHitBase::Class())) {
            TFCSLateralShapeParametrizationHitBase *sim =
                (TFCSLateralShapeParametrizationHitBase *)(chain()[ichain]);
            if (sim->simulate_hit(hit, simulstate, truth, extrapol) !=
                FCSSuccess) {
              ATH_MSG_ERROR("error for "
                            << get_variable_text(simulstate, truth, extrapol)
                            << " run init " << get_bin_text(bin) << ": "
                            << chain()[ichain]->GetName());
              return false;
            }
          } else {
            ATH_MSG_ERROR("for "
                          << get_variable_text(simulstate, truth, extrapol)
                          << " run init " << get_bin_text(bin) << ": "
                          << chain()[ichain]->GetName()
                          << " does not inherit from "
                             "TFCSLateralShapeParametrizationHitBase");
            return false;
          }
        }
      } else {
        ATH_MSG_WARNING("nothing to init for "
                        << get_variable_text(simulstate, truth, extrapol)
                        << ": " << get_bin_text(bin));
      }
    }

    int binResolution = 5;
    if (layer == 1 || layer == 5) {
      binResolution = 1;
    }

    const double center_eta = hit.center_eta();
    const double center_phi = hit.center_phi();
    const double center_r = hit.center_r();
    const double center_z = hit.center_z();

    ATH_MSG_VERBOSE(" Layer " << layer << " Extrap eta " << center_eta
                              << " phi " << center_phi << " R " << center_r);

    const float dist000 =
        TMath::Sqrt(center_r * center_r + center_z * center_z);
    const float eta_jakobi = TMath::Abs(2.0 * TMath::Exp(-center_eta) /
                                        (1.0 + TMath::Exp(-2 * center_eta)));

    int nHitsAlpha;
    int nHitsR;

    // Now create hits
    for (int ix = 1; ix <= xBinNum; ++ix) {
      int binsInAlphaInRBin = GetAlphaBinsForRBin(x, ix, yBinNum);

      // Horrible work around for variable # of bins along alpha direction
      int binsToMerge = yBinNum == 32 ? 32 / binsInAlphaInRBin : 1;
      for (int iy = 1; iy <= binsInAlphaInRBin; ++iy) {
        double energyInVoxel = outputs.at("out_" + std::to_string(vox));
        int lowEdgeIndex = (iy - 1) * binsToMerge + 1;

        ATH_MSG_VERBOSE(" Vox " << vox << " energy " << energyInVoxel
                                << " binx " << ix << " biny " << iy);

        if (energyInVoxel <= 0) {
          vox++;
          continue;
        }

        if (fabs(pdgId) == 22 || fabs(pdgId) == 11) {
          // maximum 10 MeV per hit, equaly distributed in alpha and r
          int maxHitsInVoxel = energyInVoxel * truth->Ekin() / 10;
          if (maxHitsInVoxel < 1)
            maxHitsInVoxel = 1;
          nHitsAlpha = sqrt(maxHitsInVoxel);
          nHitsR = sqrt(maxHitsInVoxel);
        } else {
          // One hit per mm along r
          nHitsR = x->GetBinUpEdge(ix) - x->GetBinLowEdge(ix);
          if (yBinNum == 1) {
            // nbins in alpha depend on circumference lenght
            double r = x->GetBinUpEdge(ix);
            nHitsAlpha = ceil(2 * TMath::Pi() * r / binResolution);
          } else {
            // d = 2*r*sin (a/2r) this distance at the upper r must be 1mm for
            // layer 1 or 5, 5mm otherwise.
            const TAxis *y = h->GetYaxis();
            double angle = y->GetBinUpEdge(iy) - y->GetBinLowEdge(iy);
            double r = x->GetBinUpEdge(ix);
            double d = 2 * r * sin(angle / 2 * r);
            nHitsAlpha = ceil(d / binResolution);
          }

          if (layer != 1 && layer != 5) {
            // For layers that are not EMB1 or EMEC1 use a maximum of 10 hits
            // per direction, a higher granularity is needed for the other
            // layers
            int maxNhits = 10;
            nHitsAlpha = std::min(maxNhits, std::max(1, nHitsAlpha));
            nHitsR = std::min(maxNhits, std::max(1, nHitsR));
          }
        }

        for (int ir = 0; ir < nHitsR; ++ir) {
          double r =
              x->GetBinLowEdge(ix) + x->GetBinWidth(ix) / (nHitsR + 1) * ir;

          for (int ialpha = 1; ialpha <= nHitsAlpha; ++ialpha) {
            if (ganVersion > 1) {
              if (fitResults.at(layer)[ix - 1] != 0) {
                int tries = 0;
                double a = CLHEP::RandFlat::shoot(simulstate.randomEngine(),
                                                  x->GetBinLowEdge(ix),
                                                  x->GetBinUpEdge(ix));
                double rand_r =
                    log((a - x->GetBinLowEdge(ix)) / (x->GetBinWidth(ix))) /
                    fitResults.at(layer)[ix - 1];
                while ((rand_r < x->GetBinLowEdge(ix) ||
                        rand_r > x->GetBinUpEdge(ix)) &&
                       tries < 100) {
                  a = CLHEP::RandFlat::shoot(simulstate.randomEngine(),
                                             x->GetBinLowEdge(ix),
                                             x->GetBinUpEdge(ix));
                  rand_r =
                      log((a - x->GetBinLowEdge(ix)) / (x->GetBinWidth(ix))) /
                      fitResults.at(layer)[ix - 1];
                  tries++;
                }
                if (tries >= 100) {
                  ATH_MSG_VERBOSE(" Too many tries for bin ["
                                  << x->GetBinLowEdge(ix) << "-"
                                  << x->GetBinUpEdge(ix) << "] having slope "
                                  << fitResults.at(layer)[ix - 1]
                                  << " will use grid (old method)");
                } else {
                  r = rand_r;
                }
              }
            }

            double alpha;
            if (binsInAlphaInRBin == 1) {
              alpha = CLHEP::RandFlat::shoot(simulstate.randomEngine(),
                                             -TMath::Pi(), TMath::Pi());
            } else {
              alpha =
                  y->GetBinLowEdge(lowEdgeIndex) +
                  y->GetBinWidth(iy) * binsToMerge / (nHitsAlpha + 1) * ialpha;

              if (m_param.IsSymmetrisedAlpha()) {
                if (CLHEP::RandFlat::shoot(simulstate.randomEngine(),
                                           -TMath::Pi(), TMath::Pi()) < 0) {
                  alpha = -alpha;
                }
              }
            }

            hit.reset();
            hit.E() = Einit * energyInVoxel / (nHitsAlpha * nHitsR);

            if (layer <= 20) {
              float delta_eta_mm = r * cos(alpha);
              float delta_phi_mm = r * sin(alpha);

              ATH_MSG_VERBOSE("delta_eta_mm " << delta_eta_mm
                                              << " delta_phi_mm "
                                              << delta_phi_mm);

              // Particles with negative eta are expected to have the same shape
              // as those with positive eta after transformation: delta_eta -->
              // -delta_eta
              if (center_eta < 0.)
                delta_eta_mm = -delta_eta_mm;
              // We derive the shower shapes for electrons and positively charged hadrons.
              // Particle with the opposite charge are expected to have the same shower shape
              // after the transformation: delta_phi --> -delta_phi
              if ((charge < 0. && pdgId!=11) || pdgId==-11)
                delta_phi_mm = -delta_phi_mm;

              const float delta_eta = delta_eta_mm / eta_jakobi / dist000;
              const float delta_phi = delta_phi_mm / center_r;

              hit.eta() = center_eta + delta_eta;
              hit.phi() = TVector2::Phi_mpi_pi(center_phi + delta_phi);

              ATH_MSG_VERBOSE(" Hit eta " << hit.eta() << " phi " << hit.phi()
                                          << " layer " << layer);
            } else { // FCAL is in (x,y,z)
              const float hit_r = r * cos(alpha) + center_r;
              float delta_phi = r * sin(alpha) / center_r;
              // We derive the shower shapes for electrons and positively charged hadrons.
              // Particle with the opposite charge are expected to have the same shower shape
              // after the transformation: delta_phi --> -delta_phi
              if ((charge < 0. && pdgId!=11) || pdgId==-11)
                delta_phi = -delta_phi;
              const float hit_phi =
                  TVector2::Phi_mpi_pi(center_phi + delta_phi);
              hit.x() = hit_r * cos(hit_phi);
              hit.y() = hit_r * sin(hit_phi);
              hit.z() = center_z;
              ATH_MSG_VERBOSE(" Hit x " << hit.x() << " y " << hit.y()
                                        << " layer " << layer);
            }

            if (get_number_of_bins() > 0) {
              const int bin = get_bin(simulstate, truth, extrapol);
              if (bin >= 0 && bin < (int)get_number_of_bins()) {
                for (unsigned int ichain =
                         m_bin_start[bin] + get_nr_of_init(bin);
                     ichain < m_bin_start[bin + 1]; ++ichain) {
                  ATH_MSG_DEBUG(
                      "for " << get_variable_text(simulstate, truth, extrapol)
                             << " run " << get_bin_text(bin) << ": "
                             << chain()[ichain]->GetName());
                  if (chain()[ichain]->InheritsFrom(
                          TFCSLateralShapeParametrizationHitBase::Class())) {
                    TFCSLateralShapeParametrizationHitBase *sim =
                        (TFCSLateralShapeParametrizationHitBase
                             *)(chain()[ichain]);
                    if (sim->simulate_hit(hit, simulstate, truth, extrapol) !=
                        FCSSuccess) {
                      ATH_MSG_ERROR(
                          "error for "
                          << get_variable_text(simulstate, truth, extrapol)
                          << " run init " << get_bin_text(bin) << ": "
                          << chain()[ichain]->GetName());
                      return false;
                    }
                  } else {
                    ATH_MSG_ERROR(
                        "for " << get_variable_text(simulstate, truth, extrapol)
                               << " run init " << get_bin_text(bin) << ": "
                               << chain()[ichain]->GetName()
                               << " does not inherit from "
                                  "TFCSLateralShapeParametrizationHitBase");
                    return false;
                  }
                }
              } else {
                ATH_MSG_WARNING(
                    "nothing to do for "
                    << get_variable_text(simulstate, truth, extrapol) << ": "
                    << get_bin_text(bin));
              }
            } else {
              ATH_MSG_WARNING("no bins defined, is this intended?");
            }
          }
        }
        vox++;
      }
    }

    ATH_MSG_VERBOSE("Number of voxels " << vox);
    ATH_MSG_VERBOSE("Done layer " << layer);
  }

  if (simulstate.E() > std::numeric_limits<double>::epsilon()) {
    for (int ilayer = 0; ilayer < CaloCell_ID_FCS::MaxSample; ++ilayer) {
      simulstate.set_Efrac(ilayer, simulstate.E(ilayer) / simulstate.E());
    }
  }

  ATH_MSG_VERBOSE("Done particle");
  return true;
}

FCSReturnCode
TFCSEnergyAndHitGANV2::simulate(TFCSSimulationState &simulstate,
                                const TFCSTruthState *truth,
                                const TFCSExtrapolationState *extrapol) const {
  for (unsigned int ichain = 0; ichain < m_bin_start[0]; ++ichain) {
    ATH_MSG_DEBUG("now run for all bins: " << chain()[ichain]->GetName());
    if (simulate_and_retry(chain()[ichain], simulstate, truth, extrapol) !=
        FCSSuccess) {
      return FCSFatal;
    }
  }

  ATH_MSG_VERBOSE("Fill Energies");
  if (!fillEnergy(simulstate, truth, extrapol)) {
    ATH_MSG_WARNING("Could not fill energies ");
    // bail out but do not stop the job
    return FCSSuccess;
  }

  return FCSSuccess;
}

void TFCSEnergyAndHitGANV2::Print(Option_t *option) const {
  TFCSParametrization::Print(option);
  TString opt(option);
  bool shortprint = opt.Index("short") >= 0;
  bool longprint = msgLvl(MSG::DEBUG) || (msgLvl(MSG::INFO) && !shortprint);
  TString optprint = opt;
  optprint.ReplaceAll("short", "");

  TString prefix = "- ";
  for (unsigned int ichain = 0; ichain < size(); ++ichain) {
    if (ichain == 0 && ichain != m_bin_start.front()) {
      prefix = "> ";
      if (longprint)
        ATH_MSG_INFO(optprint << prefix << "Run for all bins");
    }
    for (unsigned int ibin = 0; ibin < get_number_of_bins(); ++ibin) {
      if (ichain == m_bin_start[ibin]) {
        if (ibin < get_number_of_bins() - 1)
          if (ichain == m_bin_start[ibin + 1])
            continue;
        prefix = Form("%-2d", ibin);
        if (longprint)
          ATH_MSG_INFO(optprint << prefix << "Run for " << get_bin_text(ibin));
      }
    }
    if (ichain == m_bin_start.back()) {
      prefix = "< ";
      if (longprint)
        ATH_MSG_INFO(optprint << prefix << "Run for all bins");
    }
    chain()[ichain]->Print(opt + prefix);
  }
}

void TFCSEnergyAndHitGANV2::unit_test(TFCSSimulationState *simulstate,
                                      const TFCSTruthState *truth,
                                      const TFCSExtrapolationState *extrapol) {
  ISF_FCS::MLogging logger;
  if (!simulstate) {
    simulstate = new TFCSSimulationState();
#if defined(__FastCaloSimStandAlone__)
    simulstate->setRandomEngine(new CLHEP::TRandomEngine());
#else
    simulstate->setRandomEngine(new CLHEP::RanluxEngine());
#endif
  }
  if (!truth) {
    ATH_MSG_NOCLASS(logger, "New particle");
    TFCSTruthState *t = new TFCSTruthState();
    t->SetPtEtaPhiM(65536, 0, 0, 139.6);
    t->set_pdgid(211);
    truth = t;
  }
  if (!extrapol) {
    TFCSExtrapolationState *e = new TFCSExtrapolationState();
    e->set_IDCaloBoundary_eta(truth->Eta());
    for (int i = 0; i < 24; ++i) {
      e->set_eta(i, TFCSExtrapolationState::SUBPOS_ENT, truth->Eta());
      e->set_eta(i, TFCSExtrapolationState::SUBPOS_EXT, truth->Eta());
      e->set_eta(i, TFCSExtrapolationState::SUBPOS_MID, truth->Eta());
      e->set_phi(i, TFCSExtrapolationState::SUBPOS_ENT, 0);
      e->set_phi(i, TFCSExtrapolationState::SUBPOS_EXT, 0);
      e->set_phi(i, TFCSExtrapolationState::SUBPOS_MID, 0);
      e->set_r(i, TFCSExtrapolationState::SUBPOS_ENT, 1500 + i * 10);
      e->set_r(i, TFCSExtrapolationState::SUBPOS_EXT, 1510 + i * 10);
      e->set_r(i, TFCSExtrapolationState::SUBPOS_MID, 1505 + i * 10);
      e->set_z(i, TFCSExtrapolationState::SUBPOS_ENT, 3500 + i * 10);
      e->set_z(i, TFCSExtrapolationState::SUBPOS_EXT, 3510 + i * 10);
      e->set_z(i, TFCSExtrapolationState::SUBPOS_MID, 3505 + i * 10);
    }
    extrapol = e;
  }

  TFCSEnergyAndHitGANV2 GAN("GAN", "GAN");
  GAN.setLevel(MSG::VERBOSE);
  int pid = 211;
  int etaMin = 20;
  int etaMax = etaMin + 5;
  ATH_MSG_NOCLASS(logger, "Initialize Networks");
  GAN.initializeNetwork(pid, etaMin,
                        "/eos/atlas/atlascerngroupdisk/proj-simul/AF3_Run3/"
                        "InputsToBigParamFiles/FastCaloGANWeightsVer02");
  for (int i = 0; i < 24; ++i)
    if (GAN.is_match_calosample(i)) {
      TFCSCenterPositionCalculation *c = new TFCSCenterPositionCalculation(
          Form("center%d", i), Form("center layer %d", i));
      c->set_calosample(i);
      c->setExtrapWeight(0.5);
      c->setLevel(MSG::VERBOSE);
      c->set_pdgid(pid);
      if (pid == 11)
        c->add_pdgid(-pid);
      if (pid == 211)
        c->add_pdgid(-pid);
      c->set_eta_min(etaMin / 100.0);
      c->set_eta_max(etaMax / 100.0);
      c->set_eta_nominal((etaMin + etaMax) / 200.0);

      GAN.push_back_in_bin(c, i);
      GAN.set_nr_of_init(i, 1);
    }

  GAN.Print();

  ATH_MSG_NOCLASS(logger, "Writing GAN to FCSGANtest.root");
  TFile *fGAN = TFile::Open("FCSGANtest.root", "recreate");
  GAN.Write();
  fGAN->ls();
  fGAN->Close();

  ATH_MSG_NOCLASS(logger, "Open FCSGANtest.root");
  fGAN = TFile::Open("FCSGANtest.root");
  TFCSEnergyAndHitGANV2 *GAN2 = (TFCSEnergyAndHitGANV2 *)(fGAN->Get("GAN"));
  GAN2->Print();

  GAN2->setLevel(MSG::DEBUG);
  ATH_MSG_NOCLASS(logger, "Before running GAN2->simulate()");
  GAN2->simulate(*simulstate, truth, extrapol);
  simulstate->Print();
}

int TFCSEnergyAndHitGANV2::GetBinsInFours(double bins) {
  if (bins < 4)
    return 4;
  else if (bins < 8)
    return 8;
  else if (bins < 16)
    return 16;
  else
    return 32;
}

int TFCSEnergyAndHitGANV2::GetAlphaBinsForRBin(const TAxis *x, int ix,
                                               int yBinNum) const {
  double binsInAlphaInRBin = yBinNum;
  if (yBinNum == 32) {
    double widthX = x->GetBinWidth(ix);
    double radious = x->GetBinCenter(ix);
    double circumference = radious * 2 * TMath::Pi();
    if (m_param.IsSymmetrisedAlpha()) {
      circumference = radious * TMath::Pi();
    }

    double bins = circumference / widthX;
    binsInAlphaInRBin = GetBinsInFours(bins);
    ATH_MSG_DEBUG("Bin in alpha: " << binsInAlphaInRBin << " for r bin: " << ix
                                   << " (" << x->GetBinLowEdge(ix) << "-"
                                   << x->GetBinUpEdge(ix) << ")");
  }
  return binsInAlphaInRBin;
}
