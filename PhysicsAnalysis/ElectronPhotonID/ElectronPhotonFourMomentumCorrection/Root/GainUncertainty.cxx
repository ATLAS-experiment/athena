/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <ElectronPhotonFourMomentumCorrection/GainUncertainty.h>
#include <TFile.h>
#include <TH1.h>

#include <memory>
#include <cstdio> //sprintf
#include <cmath> //abs()

namespace egGain {

//--------------------------------------

GainUncertainty::GainUncertainty(const std::string& filename, bool splitGainUnc,
                                 const std::string& thisname, bool setInterpolation)
    : asg::AsgMessaging(thisname.c_str()) {

  ATH_MSG_INFO("opening file " << filename);
  std::unique_ptr<TFile> gainFile(TFile::Open(filename.c_str(), "READ"));

  if (not(m_alpha_specialGainRun =
              (TH1*)(gainFile->Get("alpha_specialGainRun"))))
    ATH_MSG_FATAL("cannot open alpha_specialGainRun");
  m_alpha_specialGainRun->SetDirectory(nullptr);
  if (not(m_gain_impact_Zee = (TH1*)(gainFile->Get("gain_impact_Zee"))))
    ATH_MSG_FATAL("cannot open gain_impact_Zee");
  m_gain_impact_Zee->SetDirectory(nullptr);
  for (int i = 0; i < s_nEtaBins; i++) {
    char name[60];
    sprintf(name, "gain_Impact_elec_%d", i);
    if (not(m_gain_Impact_elec[i] = (TH1*)(gainFile->Get(name))))
      ATH_MSG_FATAL("cannot open " << name);
    m_gain_Impact_elec[i]->SetDirectory(nullptr);
    sprintf(name, "gain_Impact_conv_%d", i);
    if (not(m_gain_Impact_conv[i] = (TH1*)(gainFile->Get(name))))
      ATH_MSG_FATAL("cannot open " << name);
    m_gain_Impact_conv[i]->SetDirectory(nullptr);
    sprintf(name, "gain_Impact_unco_%d", i);
    if (not(m_gain_Impact_unco[i] = (TH1*)(gainFile->Get(name))))
      ATH_MSG_FATAL("cannot open " << name);
    m_gain_Impact_unco[i]->SetDirectory(nullptr);

    if (splitGainUnc) {
      sprintf(name, "gain_Impact_elec_%d_medium", i);
      if (not(m_gain_Impact_elec_medium[i] = (TH1*)(gainFile->Get(name))))
        ATH_MSG_FATAL("cannot open " << name);
      m_gain_Impact_elec_medium[i]->SetDirectory(nullptr);

      sprintf(name, "gain_Impact_conv_%d_medium", i);
      if (not(m_gain_Impact_conv_medium[i] = (TH1*)(gainFile->Get(name))))
        ATH_MSG_FATAL("cannot open " << name);
      m_gain_Impact_conv_medium[i]->SetDirectory(nullptr);

      sprintf(name, "gain_Impact_unco_%d_medium", i);
      if (not(m_gain_Impact_unco_medium[i] = (TH1*)(gainFile->Get(name))))
        ATH_MSG_FATAL("cannot open " << name);
      m_gain_Impact_unco_medium[i]->SetDirectory(nullptr);

      sprintf(name, "gain_Impact_elec_%d_low", i);
      if (not(m_gain_Impact_elec_low[i] = (TH1*)(gainFile->Get(name))))
        ATH_MSG_FATAL("cannot open " << name);
      m_gain_Impact_elec_low[i]->SetDirectory(nullptr);

      sprintf(name, "gain_Impact_conv_%d_low", i);
      if (not(m_gain_Impact_conv_low[i] = (TH1*)(gainFile->Get(name))))
        ATH_MSG_FATAL("cannot open " << name);
      m_gain_Impact_conv_low[i]->SetDirectory(nullptr);

      sprintf(name, "gain_Impact_unco_%d_low", i);
      if (not(m_gain_Impact_unco_low[i] = (TH1*)(gainFile->Get(name))))
        ATH_MSG_FATAL("cannot open " << name);
      m_gain_Impact_unco_low[i]->SetDirectory(nullptr);
    }
  }

  m_useInterpolation = setInterpolation;
}

//----------------------------------------------

GainUncertainty::~GainUncertainty() {
  delete m_alpha_specialGainRun;
  delete m_gain_impact_Zee;
  for (int i = 0; i < s_nEtaBins; i++) {
    delete m_gain_Impact_elec[i];
    delete m_gain_Impact_conv[i];
    delete m_gain_Impact_unco[i];

    delete m_gain_Impact_elec_medium[i];
    delete m_gain_Impact_conv_medium[i];
    delete m_gain_Impact_unco_medium[i];

    delete m_gain_Impact_elec_low[i];
    delete m_gain_Impact_conv_low[i];
    delete m_gain_Impact_unco_low[i];
  }
}

//----------------------------------------------

// returns relative uncertainty on energy

double GainUncertainty::getUncertainty(double etaCalo_input, double et_input,
                                       PATCore::ParticleType::Type ptype,
                                       bool useL2GainUncertainty,
                                       GainType gainType) const {
  double aeta = std::fabs(etaCalo_input);
  int ibin = -1;
  if (aeta < 0.8)
    ibin = 0;
  else if (aeta < 1.37)
    ibin = 1;
  else if (aeta < 1.52)
    ibin = 2;
  else if (aeta < 1.80)
    ibin = 3;
  else if (aeta < 2.50)
    ibin = 4;
  if (ibin < 0)
    return 0.;
  ATH_MSG_VERBOSE("GainUncertainty::getUncertainty " << etaCalo_input << " "
                                                     << et_input << " " << ptype
                                                     << " ibin " << ibin);

  // Protection needed as the histograms stops at 1 TeV
  if (et_input > 999999.)
    et_input = 999999.;

  //       std::cout << " --- in  GainUncertainty::getUncertainty " <<
  //       etaCalo_input << " " << et_input << " " << ptype << " ibin " << ibin
  //       << std::endl;

  TH1* hImpact = nullptr;
  // Medium+Low gain effect
  if (gainType == GainType::MEDIUMLOW) {
    if (ptype == PATCore::ParticleType::Electron)
      hImpact = m_gain_Impact_elec[ibin];
    else if (ptype == PATCore::ParticleType::ConvertedPhoton)
      hImpact = m_gain_Impact_conv[ibin];
    else if (ptype == PATCore::ParticleType::UnconvertedPhoton)
      hImpact = m_gain_Impact_unco[ibin];
    else {
      ATH_MSG_WARNING(
          "Trying to get Gain correction of not allowed particle type");
      return 0;
    }
  }

  // Medium gain effect
  else if (gainType == GainType::MEDIUM) {
    if (ptype == PATCore::ParticleType::Electron)
      hImpact = m_gain_Impact_elec_medium[ibin];
    else if (ptype == PATCore::ParticleType::ConvertedPhoton)
      hImpact = m_gain_Impact_conv_medium[ibin];
    else if (ptype == PATCore::ParticleType::UnconvertedPhoton)
      hImpact = m_gain_Impact_unco_medium[ibin];
    else {
      ATH_MSG_WARNING(
          "Trying to get Gain correction of not allowed particle type");
      return 0;
    }
  }
  // Low gain effect
  else if (gainType == GainType::LOW) {
    if (ptype == PATCore::ParticleType::Electron)
      hImpact = m_gain_Impact_elec_low[ibin];
    else if (ptype == PATCore::ParticleType::ConvertedPhoton)
      hImpact = m_gain_Impact_conv_low[ibin];
    else if (ptype == PATCore::ParticleType::UnconvertedPhoton)
      hImpact = m_gain_Impact_unco_low[ibin];
    else {
      ATH_MSG_WARNING(
          "Trying to get Gain correction of not allowed particle type");
      return 0;
    }
  }
  if (!hImpact)[[unlikely]]{
    ATH_MSG_WARNING("hImpact is null");
    return 0;
  }
  double max_et = hImpact->GetXaxis()->GetBinUpEdge(hImpact->GetNbinsX());
  // Protection needed to match maximum Et in the histogram
  if (0.001 * et_input > max_et) {
    et_input = (max_et - 1.) * 1000.;
  }

  double impact = 0;
  if (m_useInterpolation) {
    impact = hImpact->Interpolate(0.001 * et_input);
    ATH_MSG_DEBUG(
        "L2 gain impact without interpolation: "
        << hImpact->GetBinContent(hImpact->FindFixBin(0.001 * et_input)));
    ATH_MSG_DEBUG("L2 gain impact with interpolation: "
                  << hImpact->Interpolate(0.001 * et_input));
  } else {
    impact = hImpact->GetBinContent(hImpact->FindFixBin(0.001 * et_input));
  }

  int ieta = m_alpha_specialGainRun->FindFixBin(aeta);
  if (useL2GainUncertainty)
    ATH_MSG_INFO("Applying 100% uncertainy on l2 gain corrections");

  double alphaG = m_alpha_specialGainRun->GetBinContent(ieta);

  double impactZee =
      m_gain_impact_Zee->GetBinContent(m_gain_impact_Zee->FindFixBin(aeta));

  double_t sigmaE = alphaG * impact / impactZee;

  ATH_MSG_VERBOSE("alpha_specialGainRun, gain_impact_Zee, impact, sigmaE = "
                  << alphaG << " " << impactZee << " " << impact << " "
                  << sigmaE);

  return sigmaE;
}

}  // namespace egGain
