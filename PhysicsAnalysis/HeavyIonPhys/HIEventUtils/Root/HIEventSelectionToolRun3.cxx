/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "HIEventUtils/HIEventSelectionToolRun3.h"

std::string HI::toString(HI::IonDataType when) {
  switch (when) {
    case HI::IonDataType::PbPb2015:
      return "PbPb2015";
    case HI::IonDataType::PbPb2018:
      return "PbPb2018";
    case HI::IonDataType::PbPb2023:
      return "PbPb2023";
    case HI::IonDataType::PbPb2024_Shadowing:
      return "PbPb2024_Shadowing";
    case HI::IonDataType::PbPb2024_NoShadowing:
      return "PbPb2024_NoShadowing";
    case HI::IonDataType::OO2025:
      return "OO2025";
    case HI::IonDataType::NeNe2025:
      return "NeNe2025";
    case HI::IonDataType::PbPb2025:
      return "PbPb2025";
    case HI::IonDataType::PbPb2026:
      return "PbPb2026";
    default:
      return std::string("UNKNOWN HI DATA TAKING PERIOD ") +
             std::to_string(static_cast<uint8_t>(when));
  }
}

std::string HI::toString(HI::PileupVariation variation) {
  switch (variation) {
    case HI::PileupVariation::Nominal:
      return "Nominal";
    case HI::PileupVariation::Tight:
      return "Tight";
    case HI::PileupVariation::Loose:
      return "Loose";
    default:
      return std::string("UNKNOWN PU VARIATION ") +
             std::to_string(static_cast<uint8_t>(variation));
  }
}

HI::HIEventSelectionToolRun3::HIEventSelectionToolRun3(const std::string& name)
    : AsgTool(name) {}

StatusCode HI::HIEventSelectionToolRun3::initialize() {
  ATH_MSG_INFO("Initializing HIEventSelectionToolRun3");
  // add printout of overall selection that is configured
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

bool HI::HIEventSelectionToolRun3::puNtrkvsFCal(
    HI::IonDataType when, float fcalEt, int ntrk,
    HI::PileupVariation variation) const {
  const float cut = ntrkCutValue(when, fcalEt, variation);
  return ntrk < cut;
}

bool HI::HIEventSelectionToolRun3::puZDCPSvsFCal(
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
  if (!vertices)
    return false;

  unsigned int nPrimary = 0;
  for (const xAOD::Vertex* vx : *vertices) {
    if (vx->vertexType() == xAOD::VxType::PriVtx)
      ++nPrimary;
  }

  // Typical HI requirement: exactly one PV
  return (nPrimary == 1);
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
// }  // namespace HI