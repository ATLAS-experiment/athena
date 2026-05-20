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
    ENUMDEF(NoPUFCalVsNTrackLoose)
    ENUMDEF(NoPUFCalVsNTrackNominal)
    ENUMDEF(NoPUFCalVsNTrackTight)
    ENUMDEF(NoPUFCalVsZDCLoose)
    ENUMDEF(NoPUFCalVsZDCNominal)
    ENUMDEF(NoPUFCalVsZDCTight)
    ENUMDEF(NoPUOOSingleVertexNominal)
    ENUMDEF(NoPUZDCPresampler)
    default:
      return std::string("UNKNOWN mask bit ") +
             std::to_string(static_cast<unsigned int>(m));
  }
#undef ENUMDEF
}

static const double MeV = 1.;
static const double GeV = 1e3;
static const double TeV = 1e6;

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


  //----------------------------------------------------------------------
  // https://atlas-heavy-ions.docs.cern.ch/CorrFluc/centrality/
  // https://cds.cern.ch/record/2921744 ATL-COM-PHYS-2025-033

  m_ZDCEt_UpperCut_5Sigma_PbPb2023 = loadHist("PUFCalVsZDCNominalPbPb2023.json");

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
  return et * MeV / GeV;  // we operate in GeV
}

float HI::HIEventSelectionToolRun3::zdcE(
    HI::IonDataType, const xAOD::ZdcModuleContainer* zdcModules) const {
  float e = 0;
  static const SG::ConstAccessor<float> calibEnergyAccessor("CalibEnergy");
  for (auto module : *zdcModules) {
    e += calibEnergyAccessor(*module);
  }
  return e * MeV / GeV;  // we operate in GeV
}

bool HI::HIEventSelectionToolRun3::noPUZDCvsFCal(
    HI::IonDataType period, const xAOD::HIEventShapeContainer* es,
    const xAOD::ZdcModuleContainer* zdcModules,
    HI::PileupVariation variation) const {
  return noPUZDCvsFCal(period, fcalEt(period, es), zdcE(period, zdcModules),
                       variation);
}

bool HI::HIEventSelectionToolRun3::noPUZDCvsFCal(
    HI::IonDataType period, float fcalEt, float zdcE,
    HI::PileupVariation variation) const {
  const float cut = zdcCutValue(period, fcalEt, variation);
  return zdcE < cut;
}

int HI::HIEventSelectionToolRun3::nTrk(
    HI::IonDataType, const xAOD::TrackParticleContainer* tracks,
    const xAOD::VertexContainer* vertices, const double min_pt_cut) const {
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
      if(trk->pt()<min_pt_cut) continue;
      count++;
    }
  }
  return count;
}

bool HI::HIEventSelectionToolRun3::noPUFCalVsNtracks(
    HI::IonDataType period, const xAOD::HIEventShapeContainer* es,
    const xAOD::TrackParticleContainer* tracks,
    const xAOD::VertexContainer* vertices, PileupVariation variation) const {

  double min_pt_cut=-1;
  if (period == HI::IonDataType::OO2025 || period == HI::IonDataType::NeNe2025) {
     min_pt_cut=500;//https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/#fcal-sumet-ntrk-correlation-cut
  }
  
  return noPUFCalVsNtracks(period, fcalEt(period, es),
                           nTrk(period, tracks, vertices,min_pt_cut), variation);
}

bool HI::HIEventSelectionToolRun3::noPUFCalVsNtracks(
    HI::IonDataType period, float fcalEt, int ntrk, HI::PileupVariation) const {
  ATH_MSG_DEBUG("cutting puFCalVsNtracks: fcalEt " << fcalEt << " ntracks "
                                                   << ntrk);
  // for reference, thes numbers are taken from:
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/#fcal-sumet-ntrk-correlation-cut
  // and more is here:
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  if (period == HI::IonDataType::OO2025) {
    if (ntrk < (-80 + fcalEt * 0.6))
      return false;
    if (ntrk < (-30 + fcalEt * 0.4))
      return false;
    if (ntrk > (100 + fcalEt * 1.7))
      return false;
    return true;
  } else if (period == HI::IonDataType::NeNe2025) {
    if (ntrk < (-70 + fcalEt * 0.6))
      return false;
    if (ntrk < (-20 + fcalEt * 0.35))
      return false;
    if (ntrk > (100 + fcalEt * 1.7))
      return false;
    return true;
  }
  throw std::runtime_error(std::string("puFCalVsNtracks for period of id ") +
                           HI::toString(period) + " is not handled (yet)");

  return false;  // for unimplemented periods
}

std::pair<float, float> HI::HIEventSelectionToolRun3::ZDCPresamplerAmps(
    const xAOD::ZdcModuleContainer* zdcModules) const {
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
  return {PreSamplerAmp_A, PreSamplerAmp_C};
}

bool HI::HIEventSelectionToolRun3::noPUZDCPresampler(
    HI::IonDataType period, const xAOD::ZdcModuleContainer* zdcModules,
    HI::PileupVariation variation) const {
  auto [PreSamplerAmp_A, PreSamplerAmp_C] = ZDCPresamplerAmps(zdcModules);
  return noPUZDCPresampler(period, PreSamplerAmp_A, PreSamplerAmp_C, variation);
}

bool HI::HIEventSelectionToolRun3::noPUZDCPresampler(
    HI::IonDataType period, float presamplerA, float presamplerC,
    HI::PileupVariation variation) const {
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
      return false;  // it is pileup
    }
    return true;
  }
  throw std::runtime_error(std::string("ZDC PreSampleAmp for period of id ") +
                           HI::toString(period) + " is not handled (yet)");
}

bool HI::HIEventSelectionToolRun3::noPUOOVertexCuts(
    HI::IonDataType, const xAOD::VertexContainer* vertices) const {

  // This is probably redundant as the vx->vertexType() should not be
  // xAOD::VxType::PriVtx for the dummy vertex
  if (vertices->size() <= 1) {
    ATH_MSG_DEBUG("Only dummy vertex present, returning false");
    return false;
  }

  unsigned int nPrimary = 0;
  unsigned int nSplit   = 0;
  // count primary vertices with sigma_z^2 < threshold
  // documentation: https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  int num_vtx_tot=vertices->size()-1;//exclude dummy vertex
  int vtx_counter=0;
  for (const xAOD::Vertex* vx : *vertices) {
     vtx_counter++;
     if(vtx_counter>num_vtx_tot) break;

    // check Primary vertices to see if there are some of good quality
    AmgSymMatrix(3) vtx_err = vx->covariancePosition();
    const double sigmaZSq = vtx_err(2, 2);
    if (sigmaZSq >= 0.02)  // cut in mm^2
      ++nSplit;
    else
      ++nPrimary;
  }
  ATH_MSG_DEBUG("nPrimary " << nPrimary << ",   nSplit " << nSplit);

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

  if (period == HI::IonDataType::PbPb2023) {
    // the cut is expressed in TeV for 23 data
    float fcalEtTeV = fcalEt * GeV / TeV;

    // float cut = cutFunction(fcalEtTeV);
    if (fcalEtTeV <= 1.0)  // below 1 TeV use value at 1 TeV
      fcalEtTeV = 1.0;
    if (fcalEtTeV >= 4.0)  // above4 TeV use value at 4 TeV
      fcalEtTeV = 1.0;

    int refbin =  m_ZDCEt_UpperCut_5Sigma_PbPb2023->FindFixBin(fcalEtTeV);
    float cut = m_ZDCEt_UpperCut_5Sigma_PbPb2023->GetBinContent(refbin);

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
    int refbin = m_ZDCEt_UpperCut_5p5Sigma_OO->FindFixBin(fcalEt);
    if (refbin < 1)
      refbin = 1;

    return m_ZDCEt_UpperCut_5p5Sigma_OO->GetBinContent(refbin);
  }

  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  if (period == HI::IonDataType::NeNe2025) {
    int refbin = m_ZDCEt_UpperCut_4p0Sigma_NeNe->FindFixBin(fcalEt);
    if (refbin < 1)
      refbin = 1;

    return m_ZDCEt_UpperCut_4p0Sigma_NeNe->GetBinContent(refbin);
  }

  throw std::runtime_error(
      std::string("zdcCutValue needed in FCal vs ZDC E for period of id ") +
      HI::toString(period) + " is not handled (yet)");

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
