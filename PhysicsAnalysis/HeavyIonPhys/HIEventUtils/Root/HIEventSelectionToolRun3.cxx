/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "HIEventUtils/HIEventSelectionToolRun3.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "PathResolver/PathResolver.h"

std::string HI::toString(HI::IonDataType period) {
#define ENUMDEF(_N)         \
  case HI::IonDataType::_N: \
    return #_N;
  switch (period) {
    ENUMDEF(PbPb2015)
    ENUMDEF(PbPb2018)
    ENUMDEF(PbPb2023)
    ENUMDEF(PbPb2024_Shadowing)
    ENUMDEF(PbPb2024_NoShadowing)
    ENUMDEF(OO2025)
    ENUMDEF(NeNe2025)
    ENUMDEF(PbPb2025)
    ENUMDEF(PbPb2026)
    default:
      return std::string("UNKNOWN HI DATA TAKING PERIOD ") +
             std::to_string(static_cast<uint8_t>(period));
  }
#undef ENUMDEF
}

std::string HI::toString(HI::PileupVariation variation) {
#define ENUMDEF(_N)             \
  case HI::PileupVariation::_N: \
    return #_N;

  switch (variation) {
    ENUMDEF(Nominal)
    ENUMDEF(Tight)
    ENUMDEF(Loose)
    default:
      return std::string("UNKNOWN PU VARIATION ") +
             std::to_string(static_cast<uint8_t>(variation));
  }
#undef ENUMDEF
}

std::string HI::toString(SelectionMask m) {
#define ENUMDEF(_N)           \
  case HI::SelectionMask::_N: \
    return #_N;

  switch (m) {
    ENUMDEF(NoEventError)
    ENUMDEF(PUFCalVsNTrackLoose)
    ENUMDEF(PUFCalVsNTrackNominal)
    ENUMDEF(PUFCalVsNTrackTight)
    ENUMDEF(PUFCalVsZDCLoose)
    ENUMDEF(PUFCalVsZDCNominal)
    ENUMDEF(PUFCalVsZDCTight)
    ENUMDEF(PUOOSingleVertexNominal)
    ENUMDEF(PUZDCPresampler)
    default:
      return std::string("UNKNOWN mask bit ") +
             std::to_string(static_cast<unsigned int>(m));
  }
#undef ENUMDEF
}

std::unique_ptr<TH1D> loadHist(const std::string& file) {
  const std::string path =
      PathResolver::find_file(std::string("HIEventUtils/") + file, "DATAPATH");
  std::ifstream i(path);
  if (not i) {
    throw std::runtime_error(path + " does nto exist");
  }

  nlohmann::json j;
  i >> j;
  i.close();

  const size_t nbins = j.at("fNbins").get<size_t>();

  auto h = std::make_unique<TH1D>(j.at("fName").get<std::string>().c_str(),
                                  j.at("fTitle").get<std::string>().c_str(),
                                  nbins, j.at("fXmin").get<double>(),
                                  j.at("fXmax").get<double>());
  const std::vector<double> bins = j.at("fArray").get<std::vector<double>>();
  if (bins.size() != j.at("fNbins").get<size_t>() + 2) {
    throw std::runtime_error(
        "Histogram " + j.at("fName").get<std::string>() +
        " has inconsistent number of bins and fNbins (should +2) value " +
        std::to_string(bins.size()) + " and " + std::to_string(nbins));
  }

  for (size_t bin = 0; bin < nbins + 2; bin++) {
    h->SetBinContent(bin, bins[bin]);
  }
  h->SetDirectory(0);
  return h;
}

HI::HIEventSelectionToolRun3::HIEventSelectionToolRun3(const std::string& name)
    : AsgTool(name) {}

StatusCode HI::HIEventSelectionToolRun3::initialize() {
  ATH_MSG_INFO("Initializing HIEventSelectionToolRun3");
  // add printout of overall selection that is configured
  if (!m_trackSelectionTool.empty())
    ATH_CHECK(m_trackSelectionTool.retrieve());

  //----------------------------------------------------------------------
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  m_ZDCEt_UpperCut_5p5Sigma_OO = loadHist("PUFCalVsZDCNominalOO2025.json");

  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  m_ZDCEt_UpperCut_4p0Sigma_NeNe = loadHist("PUFCalVsZDCNominalNeNe2025.json");
  return StatusCode::SUCCESS;
}

bool HI::HIEventSelectionToolRun3::noDetectorError(
    const xAOD::EventInfo* eventInfo) const {
  if (!eventInfo)
    return false;

  // Standard ATLAS error checks
  if (eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error)
    return false;
  if (eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error)
    return false;
  if (eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error)
    return false;
  if (eventInfo->errorState(xAOD::EventInfo::Pixel) == xAOD::EventInfo::Error)
    return false;

  return true;
}

float HI::HIEventSelectionToolRun3::fcalEt(
    HI::IonDataType, const xAOD::HIEventShapeContainer* es) const {
  float et = 0;
  for (auto slice : *es) {
    const static std::set fcalLayers({21, 22, 23});
    if (fcalLayers.contains(slice->layer()))
      et += slice->et();
  }
  return et * 1e-6;  // we operate in TeV
}

float HI::HIEventSelectionToolRun3::zdcE(
    HI::IonDataType, const xAOD::ZdcModuleContainer* zdcModules) const {
  float e = 0;
  static const SG::ConstAccessor<float> calibEnergyAccessor("CalibEnergy");
  for (auto module : *zdcModules) {
    e += calibEnergyAccessor(*module);
  }
  return e * 1e-3;  // we operate in GeV
}

bool HI::HIEventSelectionToolRun3::puZDCvsFCal(
    HI::IonDataType period, const xAOD::HIEventShapeContainer* es,
    const xAOD::ZdcModuleContainer* zdcModules,
    HI::PileupVariation variation) const {
  return puZDCvsFCal(period, fcalEt(period, es), zdcE(period, zdcModules), variation);
}

bool HI::HIEventSelectionToolRun3::puZDCvsFCal(
    HI::IonDataType period, float fcalEt, float zdcE,
    HI::PileupVariation variation) const {
  const float cut = zdcCutValue(period, fcalEt, variation);
  return zdcE < cut;
}

int HI::HIEventSelectionToolRun3::nTrk(HI::IonDataType,
                 const xAOD::TrackParticleContainer* tracks,
                 const xAOD::VertexContainer* vertices) const {
  const xAOD::Vertex* pv = 0;
  for (const xAOD::Vertex* vx : *vertices) {
    if (vx->vertexType() == xAOD::VxType::PriVtx) {
      pv = vx;
      break;
    }
  }

  int count = 0;
  for (const xAOD::TrackParticle* trk : *tracks) {
    if (m_trackSelectionTool->accept(*trk, pv)) {
      count++;
    }
  }
  return count;
}

bool HI::HIEventSelectionToolRun3::puFCalVsNtracks(
    HI::IonDataType period, const xAOD::HIEventShapeContainer* es,
    const xAOD::TrackParticleContainer* tracks,
    const xAOD::VertexContainer* vertices, PileupVariation variation) const {
  return puFCalVsNtracks(period, fcalEt(period, es), nTrk(period, tracks, vertices), variation);
}

bool HI::HIEventSelectionToolRun3::puFCalVsNtracks(HI::IonDataType period,
                                                   float fcalEt, int ntrk,
                                                   HI::PileupVariation) const {
  ATH_MSG_DEBUG("cutting puFCalVsNtracks: fcalEt " << fcalEt << " ntracks "
                                                   << ntrk);
  // for reference, thes numbers are taken from:
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/#fcal-sumet-ntrk-correlation-cut
  // and more is here:
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  if (period == HI::IonDataType::OO2025) {
    if (ntrk < (-80 + fcalEt * 600))
      return false;
    if (ntrk < (-30 + fcalEt * 400))
      return false;
    if (ntrk > (100 + fcalEt * 1700))
      return false;
    return true;
  } else if (period == HI::IonDataType::NeNe2025) {
    if (ntrk < (-70 + fcalEt * 600))
      return false;
    if (ntrk < (-20 + fcalEt * 350))
      return false;
    if (ntrk > (100 + fcalEt * 1700))
      return false;
    return true;
  }
  return false;  // for unimplemented periods
}

bool HI::HIEventSelectionToolRun3::puZDCPresampler(
    HI::IonDataType period, const xAOD::ZdcModuleContainer* zdcModules,
    HI::PileupVariation variation) const {
  float PreSamplerAmp_A = 0;
  float PreSamplerAmp_C = 0;
  static const SG::ConstAccessor<float> accPreSampleAmp("PreSampleAmp");
  for (const auto module : *zdcModules) {
    if (module->zdcType() != 0)
      continue;
    if (module->zdcSide() > 0)
      PreSamplerAmp_C += accPreSampleAmp(*module);
    if (module->zdcSide() < 0)
      PreSamplerAmp_A += accPreSampleAmp(*module);
  }
  return puZDCPresampler(period, PreSamplerAmp_A, PreSamplerAmp_C, variation);
}

bool HI::HIEventSelectionToolRun3::puZDCPresampler(
    HI::IonDataType period, float presamplerA, float presamplerC,
    HI::PileupVariation variation) const {
  // not sure if fcalEt will be involved i.e. apply this cut only above certain
  // fcalEt

  if (period == HI::IonDataType::PbPb2023) {
    // from ATL-COM-PHYS-2025-033 + priv. communication F.Pauwels
    const float peakPositionA = -56;
    const float peakPositionC = -156;
    const float peakWidthA = 51.8;
    const float peakWidthC = 51.8;
    float sigma = 7;
    if (variation == HI::PileupVariation::Tight) {
      sigma = 8;
    }
    if (variation == HI::PileupVariation::Loose) {
      sigma = 6;
    }
    if (presamplerA > (peakPositionA + sigma * peakWidthA) and
        presamplerC > (peakPositionC + sigma * peakWidthC)) {
      return true;  // it is pileup
    }
  }
  return false;
}

bool HI::HIEventSelectionToolRun3::puOOVertexCuts(
    HI::IonDataType, const xAOD::VertexContainer* vertices) const {

  // This is probably redundant as the vx->vertexType() should not be
  // xAOD::VxType::PriVtx for the dummy vertex
  if (vertices->size() <= 1) {
    ATH_MSG_DEBUG("Only dummy vertex present, returning false");
    return false;
  }

  unsigned int nPrimary = 0;
  unsigned int nSplit = 0;
  unsigned int nOther = 0;
  // count primary vertices with sigma_z^2 < threshold
  // documentation: https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  for (const xAOD::Vertex* vx : *vertices) {
    if (vx->vertexType() == xAOD::VxType::PriVtx) {
      // check Primary vertices to see if there are some of good quality
      AmgSymMatrix(3) vtx_err = vx->covariancePosition();
      const double sigmaZSq = vtx_err(2, 2);
      if (sigmaZSq >= 0.02)  // cut in mm^2
        ++nSplit;
      else
        ++nPrimary;
    }
    // vertices that are not PV, note that dummy vertex probably will get
    // assigned here
    else
      ++nOther;
  }
  ATH_MSG_DEBUG("n primary " << nPrimary << ",   nSplit " << nSplit
                             << ",  nOther " << nOther);

  // If all vertices were classified as split, then we consider one of them to
  // be a real vertex
  if (nSplit > 0 && nPrimary == 0) {
    ATH_MSG_DEBUG("Returning true as all vertices classified as split");
    return true;
  }

  return nPrimary == 1;
}

float HI::HIEventSelectionToolRun3::zdcCutValue(
    HI::IonDataType period, float fcalEt, HI::PileupVariation variation) const {
  if (fcalEt > 100.0)
    throw std::runtime_error(
        std::to_string(fcalEt) +
        " the energy that is given to zdcCutValue is well above 100 TeV?, "
        "likely you call it not converting energy to TeV");

  if (period == HI::IonDataType::PbPb2023) {

    auto cutFunction = [](float et) {
      const static double a = 334.29, b = -20.39,
                          c = -2.38;  // from ATL-COM-PHYS-2025-033
      return a + b * et + c * et * et;
    };

    float cut = cutFunction(fcalEt);
    if (fcalEt <= 1.0)  // below 1 TeV use flat
      cut = cutFunction(1.0);
    if (fcalEt >= 4.0)  // below 1 TeV use flat
      cut = cutFunction(4.0);

    if (variation == HI::PileupVariation::Tight) {
      cut *= 1.02;
    }
    if (variation == HI::PileupVariation::Loose) {
      cut *= 0.98;
    }
    return cut;
  }

  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  if (period == HI::IonDataType::OO2025) {
    //*1e3 below to convert fcalEt from TeV to GeV
    int refbin = m_ZDCEt_UpperCut_5p5Sigma_OO->FindFixBin(fcalEt * 1e3);
    if (refbin < 1)
      refbin = 1;

    //*1e3 below to convert Zdc value in histogram from TeV to GeV
    return m_ZDCEt_UpperCut_5p5Sigma_OO->GetBinContent(refbin) * 1e3;
  }

  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  if (period == HI::IonDataType::NeNe2025) {
    //*1e3 below to convert fcalEt from TeV to GeV
    int refbin = m_ZDCEt_UpperCut_4p0Sigma_NeNe->FindFixBin(fcalEt * 1e3);
    if (refbin < 1)
      refbin = 1;

    //*1e3 below to convert Zdc value in histogram from TeV to GeV
    return m_ZDCEt_UpperCut_4p0Sigma_NeNe->GetBinContent(refbin) * 1e3;
  }

  throw std::runtime_error(std::string("period of id ") + HI::toString(period) +
                           " is not handled (yet)");

  return 0;
}

float HI::HIEventSelectionToolRun3::ntrkCutValue(HI::IonDataType, float,
                                                 HI::PileupVariation) const {
  return 0;
}

HI::IonDataType HI::HIEventSelectionToolRun3::toDataType(
    const xAOD::EventInfo* eventInfo) const {
  return runNumberToDataType(eventInfo->runNumber());
}

HI::IonDataType HI::HIEventSelectionToolRun3::runNumberToDataType(
    uint32_t run) const {
  if (run < 365498)
    throw std::runtime_error(std::to_string(run) +
                             " not handled by selection tool");
  if (run <= 367384)
    return HI::IonDataType::PbPb2018;
  if (run <= 463427)
    return HI::IonDataType::PbPb2023;
  if (run <= 489691)
    return HI::IonDataType::PbPb2024_Shadowing;
  if (run <= 490223)
    return HI::IonDataType::PbPb2024_NoShadowing;
  if (run <= 501969)
    return HI::IonDataType::OO2025;
  if (run <= 502008)
    return HI::IonDataType::NeNe2025;
  if (run <= 512049)
    return HI::IonDataType::PbPb2025;
  // fill it up with 2026
  throw std::runtime_error(std::to_string(run) +
                           " not handled by selection tool");
}

unsigned int HI::HIEventSelectionToolRun3::defaultMaskForPeriod(
    HI::IonDataType period) const {
  if (period == HI::IonDataType::OO2025 or
      period == HI::IonDataType::NeNe2025) {
    return static_cast<unsigned int>(HI::SelectionMask::OODefault);
  }
  // this will likely evolve
  return static_cast<unsigned int>(HI::SelectionMask::PBDefault);
}

// }  // namespace HI
