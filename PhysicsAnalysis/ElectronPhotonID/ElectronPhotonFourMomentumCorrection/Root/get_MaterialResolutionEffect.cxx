/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ElectronPhotonFourMomentumCorrection/get_MaterialResolutionEffect.h"

#include "PathResolver/PathResolver.h"
#include "TArrayD.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include <format>  // For std::format
#include <ranges>  // For std::views::iota

namespace {
template <typename TargetPtr, typename SourcePtr>
TargetPtr checked_cast(SourcePtr ptr) {
  static_assert(std::is_pointer<TargetPtr>::value,
                "attempt to cast to no ptr object");
  static_assert(std::is_pointer<SourcePtr>::value,
                "attempt to cast from no ptr object");
  if (!ptr) {
    throw std::runtime_error(
        "Attempt to cast from nullptr in get_MaterialResolutionEffect");
  }
  TargetPtr obj = dynamic_cast<TargetPtr>(ptr);
  if (not obj) {
    throw std::runtime_error("failed dynamic cast for " +
                             std::string(ptr->GetName()) +
                             " get_MaterialResolutionEffect");
  }
  return obj;
}
}  // namespace

get_MaterialResolutionEffect::get_MaterialResolutionEffect()
    : asg::AsgMessaging("get_MaterialResolutionEffect") {

  const std::string pType[3] = {"Elec", "Unconv", "Conv"};
  const std::string sType[s_nSys] = {"A", "CD", "EL", "FMX"};

  const std::string filename = PathResolverFindCalibFile(
      "ElectronPhotonFourMomentumCorrection/v8/"
      "histos-systematics-material.root");

  std::unique_ptr<TFile> file0(TFile::Open(filename.c_str(), "READ"));

  for (Int_t isys = 0; isys < s_nSys; isys++) {  // 0=A, 1=CD, 2=EL, 3=FMX
    for (Int_t ieta = 0; ieta < s_nEtaBins; ieta++) {
      for (Int_t iconv = 0; iconv < 3;
           iconv++) {  // 0=electron, 1=unconverted, 2=converted
        char name[31];
        sprintf(name, "syst%s_%s_etaBin_%d", pType[iconv].c_str(),
                sType[isys].c_str(), ieta);
        m_hSystPeak.at(isys).at(ieta).at(iconv).reset(
            checked_cast<TH1*>(file0->Get(name)));
        m_hSystPeak.at(isys).at(ieta).at(iconv)->SetDirectory(nullptr);

        sprintf(name, "syst%s_sigmaG_%s_etaBin_%d", pType[iconv].c_str(),
                sType[isys].c_str(), ieta);
        m_hSystResol.at(isys).at(ieta).at(iconv).reset(
            checked_cast<TH1*>(file0->Get(name)));
        m_hSystResol.at(isys).at(ieta).at(iconv)->SetDirectory(nullptr);
      }
    }
  }

  // IBL+PP0 material systematics stored in 2D histograms
  m_hsyst_IBL_PP0.at(0).reset(
      checked_cast<TH2*>(file0->Get("systElec_IBLPP0")));
  m_hsyst_IBL_PP0.at(0)->SetDirectory(nullptr);

  m_hsyst_IBL_PP0.at(1).reset(
      checked_cast<TH2*>(file0->Get("systUnconv_IBLPP0")));
  m_hsyst_IBL_PP0.at(1)->SetDirectory(nullptr);

  m_hsyst_IBL_PP0.at(2).reset(
      checked_cast<TH2*>(file0->Get("systConv_IBLPP0")));
  m_hsyst_IBL_PP0.at(2)->SetDirectory(nullptr);

  m_etBins = m_hSystResol.at(0).at(0).at(1)->GetXaxis()->GetXbins();
}

//============================================================================
// inputs are particle_type (0=elec, 1=reco unconv photon, 2=reco conv photon,
// 3=true unconv photon)
//            energy in MeV
//            eta
//            response_type (0=gaussian core fit, 1=sigmaeff 80%, 2=sigma eff
//            90%)
//
// returned value is sigmaE/E change in quadrature to resolution
//
double get_MaterialResolutionEffect::getDelta(int particle_type, double energy,
                                              double eta, int response_type,
                                              int isyst) const {

  // cout << " in getDelta " << endl;
  if (particle_type < 0 || particle_type > 2)
    return -999;
  if (response_type < 0 || response_type > 1)
    return -999;

  float aeta = std::fabs(eta);
  double energyGeV = energy * 0.001;
  double et = energyGeV / cosh(eta);

  // IBL+PP0
  if (isyst == 5) {
    double et2 = et;
    if (et < 5.) {
      et2 = 5.1;
    }
    if (et > 2000) {
      et2 = 1999.;
    }
    if (aeta>=2.5) aeta=2.49; 

    int ieta = m_hsyst_IBL_PP0.at(particle_type)->GetXaxis()->FindBin(aeta);
    int iet = m_hsyst_IBL_PP0.at(particle_type)->GetYaxis()->FindBin(et2);
    if (m_interpolate) {
      return 0.01*interpolateTH1(m_hsyst_IBL_PP0_ProjectionY[particle_type][ieta - 1].get(), et2, true);
    }
    else {
      return 0.01*m_hsyst_IBL_PP0.at(particle_type)->GetBinContent(ieta, iet);
    }
  }

  int ieta = 0;
  if (aeta < 0.4) {
    ieta = 0;
  } else if (aeta < 0.8) {
    ieta = 1;
  } else if (aeta < 1.1) {
    ieta = 2;
  } else if (aeta < 1.37) {
    ieta = 3;
  } else if (aeta < 1.52) {
    ieta = 4;
  } else if (aeta < 1.80) {
    ieta = 5;
  } else if (aeta < 2.10) {
    ieta = 6;
  } else {
    ieta = 7;
  }

  int ibinEt = m_etBins->GetSize() - 2;
  for (int i = 1; i < m_etBins->GetSize(); i++) {
    if (et < m_etBins->GetAt(i)) {
      ibinEt = i - 1;
      break;
    }
  }

  const auto& hist = response_type==0 ? m_hSystPeak : m_hSystResol;
  if (m_interpolate) {
    return 0.01*interpolateTH1(hist.at(isyst).at(ieta).at(particle_type).get(), et, true);
  }
  else {
    return 0.01*hist.at(isyst).at(ieta).at(particle_type)->GetBinContent(ibinEt+1);
  }

}
//=========================================================================
void get_MaterialResolutionEffect::store_IBL_PP0_YProjections()
{
  for (size_t i = 0; i < m_hsyst_IBL_PP0.size(); i++) {
    if (m_hsyst_IBL_PP0[i]) {
      int nEtaBins = m_hsyst_IBL_PP0[i]->GetNbinsX();
      // Resize the inner vector to match the size of TH2 x-axis bins
      m_hsyst_IBL_PP0_ProjectionY[i].resize(nEtaBins);
      for (int ieta : std::views::iota(1, nEtaBins + 1)) {
        std::string histName = std::format("h1d_IBL_PP0_ptype{}_EtaBin_{}", i, ieta);
        m_hsyst_IBL_PP0_ProjectionY[i][ieta - 1].reset( dynamic_cast<TH1*>(m_hsyst_IBL_PP0[i]->ProjectionY(histName.c_str(), ieta, ieta)) );
        m_hsyst_IBL_PP0_ProjectionY[i][ieta - 1]->SetDirectory( nullptr );
      }
    }      
  }
}

//=========================================================================
double get_MaterialResolutionEffect::interpolateTH1(TH1 *hist, double x, bool abs_bins) 
{
  if (!hist) return 0.;
  int nbins = hist->GetNbinsX();

  if(x<=hist->GetBinCenter(1)) {
    return abs_bins? std::abs(hist->GetBinContent(1)) : hist->GetBinContent(1);
  } 
  else if(x>=hist->GetBinCenter(nbins)) {
    return abs_bins? std::abs(hist->GetBinContent(nbins)) : hist->GetBinContent(nbins);
  } 
  else {
    int xbin = hist->FindBin(x);
    int xbin0, xbin1;
    if(x<=hist->GetBinCenter(xbin)) {
      xbin0 = xbin - 1;
      xbin1 = xbin;
    } 
    else {
      xbin0 = xbin;
      xbin1 = xbin + 1;
    }
    double y0 = abs_bins? std::abs(hist->GetBinContent(xbin0)) : hist->GetBinContent(xbin0);
    double x0 = hist->GetBinCenter(xbin0);
    double y1 = abs_bins? std::abs(hist->GetBinContent(xbin1)) : hist->GetBinContent(xbin1);
    double x1 = hist->GetBinCenter(xbin1);
    return y0 + (x-x0)*((y1-y0)/(x1-x0));
  }
} 