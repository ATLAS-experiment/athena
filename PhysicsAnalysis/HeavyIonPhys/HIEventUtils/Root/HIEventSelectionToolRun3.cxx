/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "HIEventUtils/HIEventSelectionToolRun3.h"

std::string HI::toString(HI::IonDataType when) {
#define ENUMDEF(_N)         \
  case HI::IonDataType::_N: \
    return #_N;
  switch (when) {
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
             std::to_string(static_cast<uint8_t>(when));
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
    default:
      return std::string("UNKNOWN mask bit ") +
             std::to_string(static_cast<unsigned int>(m));
  }
#undef ENUMDEF
}

HI::HIEventSelectionToolRun3::HIEventSelectionToolRun3(const std::string& name)
    : AsgTool(name) {}

StatusCode HI::HIEventSelectionToolRun3::initialize() {
  ATH_MSG_INFO("Initializing HIEventSelectionToolRun3");
  // add printout of overall selection that is configured
  if (!m_trackSelectionTool.empty())
    ATH_CHECK(m_trackSelectionTool.retrieve());

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

float calcFcalEt(const xAOD::HIEventShapeContainer* es) {
  float et = 0;
  for (auto slice : *es) {
    const static std::set fcalLayers({21, 22, 23});
    if (fcalLayers.contains(slice->layer()))
      et += slice->et();
  }
  return et * 1e-6;  // we operate in TeV
}
float calcZDCE(const xAOD::ZdcModuleContainer* zdcModules) {
  float e = 0;
  static const SG::ConstAccessor<float> calibEnergyAccessor("CalibEnergy");
  for (auto module : *zdcModules) {
    e += calibEnergyAccessor(*module);
  }
  return e * 1e-3;  // we operate in GeV
}

bool HI::HIEventSelectionToolRun3::puZDCvsFCal(
    HI::IonDataType when, const xAOD::HIEventShapeContainer* es,
    const xAOD::ZdcModuleContainer* zdcModules,
    HI::PileupVariation variation) const {
  return puZDCvsFCal(when, calcFcalEt(es), calcZDCE(zdcModules), variation);
}

bool HI::HIEventSelectionToolRun3::puZDCvsFCal(
    HI::IonDataType when, float fcalEt, float zdcE,
    HI::PileupVariation variation) const {
  const float cut = zdcCutValue(when, fcalEt, variation);
  return zdcE > cut;
}

bool HI::HIEventSelectionToolRun3::puFCalVsNtracks(
    IonDataType dataType, const xAOD::HIEventShapeContainer* es,
    const xAOD::TrackParticleContainer* tracks,
    const xAOD::VertexContainer* vertices, PileupVariation variation) const {
  const xAOD::Vertex* pv = 0;
  for (const xAOD::Vertex* vx : *vertices) {
    if (vx->vertexType() == xAOD::VxType::PriVtx) {
      pv = vx;
    }
  }

  int count = 0;
  for (const xAOD::TrackParticle* trk : *tracks) {
    if (m_trackSelectionTool->accept(*trk, pv)) {
      count++;
    }
  }
  return puFCalVsNtracks(dataType, calcFcalEt(es), count, variation);
}

bool HI::HIEventSelectionToolRun3::puFCalVsNtracks(HI::IonDataType when,
                                                   float fcalEt, int ntrk,
                                                   HI::PileupVariation) const {
  ATH_MSG_DEBUG("cutting puFCalVsNtracks: fcalEt " << fcalEt << " ntracks "
                                                   << ntrk);

  if (when == HI::IonDataType::OO2025) {
    if (ntrk < (-80 + fcalEt * 600))
      return false;
    if (ntrk < (-30 + fcalEt * 400))
      return false;
    if (ntrk > (100 + fcalEt * 1700))
      return false;
    return true;
  } else if (when == HI::IonDataType::NeNe2025) {
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

bool HI::HIEventSelectionToolRun3::puFCalVsZDC(
    HI::IonDataType when, float /*fcalEt*/, float presamplerA,
    float presamplerC, HI::PileupVariation variation) const {
  // not sure if fcalEt will be involved i.e. apply this cut only above certain
  // fcalEt

  if (when == HI::IonDataType::PbPb2023) {
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
      if (sigmaZSq >= 0.02)
        ++nSplit;  // cut in mm^2
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
    HI::IonDataType when, float fcalEt, HI::PileupVariation variation) const {
  if (fcalEt > 100.0)
    throw std::runtime_error(
        std::to_string(fcalEt) +
        " the energy that is given to zdcCutValue is well above 100 TeV?, "
        "likely you call it not converting energy to TeV");

  if (when == HI::IonDataType::PbPb2023) {

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
  throw std::runtime_error(std::string("period of id ") + HI::toString(when) +
                           "is not handled");
  ;
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
  if (run <= 461633)
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
