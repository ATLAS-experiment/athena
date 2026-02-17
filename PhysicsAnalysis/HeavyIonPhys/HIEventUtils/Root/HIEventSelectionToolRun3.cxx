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

  //----------------------------------------------------------------------
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  m_ZDCEt_UpperCut_5p5Sigma_OO =
      std::make_unique<TH1D>("ZDCEt_UpperCut_5p5Sigma_OO",
                             "Dist2D_ZDCFcalEt_MB_AfterPup", 250, -0.5, 999.5);
  std::vector<Double_t> ZDCEt_UpperCut_5p5Sigma_OO_vals{30.15244850272141,
                                                        32.42921472513576,
                                                        35.10075439491398,
                                                        37.16778275552729,
                                                        38.69279633216428,
                                                        39.88616727603026,
                                                        40.8495752124775,
                                                        41.64530454190049,
                                                        42.32872335704761,
                                                        42.87881557918834,
                                                        43.33648917411151,
                                                        43.7098285722753,
                                                        44.01270078827469,
                                                        44.23201962096336,
                                                        44.40289249934808,
                                                        44.53012276902044,
                                                        44.57385837052205,
                                                        44.60232634403054,
                                                        44.58401919053701,
                                                        44.52475471226591,
                                                        44.41928179492855,
                                                        44.31352952614813,
                                                        44.12651276141978,
                                                        43.96884607455986,
                                                        43.76287370666087,
                                                        43.55570556176079,
                                                        43.31311776214419,
                                                        43.05359310322093,
                                                        42.78223415077986,
                                                        42.49838317594456,
                                                        42.21514806247096,
                                                        41.90502382871026,
                                                        41.58909938279488,
                                                        41.25411441860449,
                                                        40.93664215487441,
                                                        40.58957601125615,
                                                        40.21247140820017,
                                                        39.87923405564943,
                                                        39.51845305886969,
                                                        39.1404521482561,
                                                        38.76330151604378,
                                                        38.41836690058044,
                                                        38.02688521823783,
                                                        37.64686906817849,
                                                        37.25619955156225,
                                                        36.89096585297114,
                                                        36.50633361546052,
                                                        36.13043229944893,
                                                        35.72600752185447,
                                                        35.33856352439545,
                                                        34.96242676110756,
                                                        34.56046678507732,
                                                        34.19972371843255,
                                                        33.81209508226291,
                                                        33.42091138850252,
                                                        33.04400671056584,
                                                        32.65613795689762,
                                                        32.27833025061835,
                                                        31.91413509787477,
                                                        31.54635345843291,
                                                        31.17048387853134,
                                                        30.82165762050397,
                                                        30.46337921942842,
                                                        30.11836283863624,
                                                        29.80655802203649,
                                                        29.42084405043285,
                                                        29.08526715836496,
                                                        28.76621247272501,
                                                        28.41689235631353,
                                                        28.11873414036081,
                                                        27.76958756655553,
                                                        27.50496516305158,
                                                        27.16955976622453,
                                                        26.89239158208542,
                                                        26.58442003092942,
                                                        26.29055598742806,
                                                        25.98488889949361,
                                                        25.70000790885675,
                                                        25.49905117998899,
                                                        25.0700063179173,
                                                        24.90376155314041,
                                                        24.60929892295523,
                                                        24.38498950910959,
                                                        24.14399949895702,
                                                        23.93378259538233,
                                                        23.64562485158734,
                                                        23.38436915280338,
                                                        23.21273202153337,
                                                        22.88039257363098,
                                                        22.69558535022731,
                                                        22.3008210990427,
                                                        22.16079603117572,
                                                        21.98680445938,
                                                        21.78322029680111,
                                                        21.53335291107893,
                                                        21.05701694841585,
                                                        20.96757888183601,
                                                        20.76260618195638,
                                                        20.45609803536658,
                                                        19.9016998073948,
                                                        19.89914047682212,
                                                        19.53987688263938,
                                                        19.11870470224849,
                                                        18.78879182658746,
                                                        18.4533698996144,
                                                        18.34441333305366,
                                                        17.48506800067927,
                                                        16.65151928928474,
                                                        16.24981254366435,
                                                        16.2512588171928,
                                                        15.10716749354254,
                                                        14.93254439160878,
                                                        14.72904021643629,
                                                        13.28119055677913,
                                                        12.32274778733038,
                                                        10.61152610634797,
                                                        11.19010469643575,
                                                        8.853105443257553,
                                                        7.312651813030243,
                                                        7.276821320965176,
                                                        5.801582480504595,
                                                        5.791995748189779,
                                                        3.656944074233373,
                                                        2.296292543411255,
                                                        0.1,
                                                        0.1,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0,
                                                        0};
  for (Int_t bin = 0; bin < 252; bin++) {
    m_ZDCEt_UpperCut_5p5Sigma_OO->SetBinContent(
        bin, ZDCEt_UpperCut_5p5Sigma_OO_vals[bin]);
  }
  m_ZDCEt_UpperCut_5p5Sigma_OO->SetDirectory(0);
  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  m_ZDCEt_UpperCut_4p0Sigma_NeNe =
      std::make_unique<TH1D>("ZDCEt_UpperCut_4p0Sigma_NeNe",
                             "Dist2D_ZDCFcalEt_MB_AfterPup", 300, -0.5, 1199.5);
  std::vector<Double_t> ZDCEt_UpperCut_4p0Sigma_NeNe_vals{
      26.31106835462769,
      27.61172505387843,
      29.89164957161773,
      31.83341566413143,
      33.28479824584519,
      34.47367004064736,
      35.5169564298671,
      36.39941837437408,
      37.20623145740676,
      37.88987093702708,
      38.5230687930144,
      38.99576756491859,
      39.49265579792175,
      39.83990013145648,
      40.22558698770169,
      40.49133658513034,
      40.7698116757644,
      40.92171529128842,
      41.05298632899531,
      41.23933451242071,
      41.30383481358579,
      41.35349898519756,
      41.37852253388261,
      41.38326138886478,
      41.38905522997479,
      41.30904047193012,
      41.23209457453427,
      41.19289569080686,
      41.07622104064043,
      40.97038299740179,
      40.8300025649615,
      40.65541869474887,
      40.47533914077917,
      40.36118975837425,
      40.18177818416288,
      40.00825152195164,
      39.6978606625801,
      39.50636243639208,
      39.31162688413812,
      39.07809723365309,
      38.90298392809427,
      38.59036423742975,
      38.31886108968602,
      38.03606793454716,
      37.75620122686368,
      37.5469667925788,
      37.25605852932247,
      36.96220114696194,
      36.71159582056651,
      36.39580487559545,
      36.08411769788744,
      35.785555840086,
      35.48846846137438,
      35.18741321974212,
      34.89348412323471,
      34.6234336806107,
      34.19976545909555,
      33.96246134803425,
      33.67454877034458,
      33.34167983031337,
      32.99266763562601,
      32.68963773697467,
      32.36509168039773,
      31.96251412636415,
      31.67751203982699,
      31.34268936368364,
      31.07629296581998,
      30.7927008880902,
      30.45864988608594,
      30.14210298657956,
      29.81264383429119,
      29.43887731117196,
      29.18684713943895,
      28.90178417650861,
      28.57735216643974,
      28.20488815496362,
      27.99183505257518,
      27.64514328997088,
      27.2979844435088,
      26.92988161597971,
      26.73004401458928,
      26.48592862291065,
      26.1416931331709,
      25.98676544059163,
      25.58374900828336,
      25.24182737513986,
      24.96550226643504,
      24.63181340114161,
      24.36438323446853,
      24.07948996144699,
      23.80304087848747,
      23.4415011565485,
      23.21921264023182,
      22.97187427930454,
      22.66889855289926,
      22.3893261540843,
      22.0671561693193,
      21.66340220239666,
      21.39530698498289,
      21.15499797888485,
      20.9622854541349,
      20.5020673801756,
      20.08875293938291,
      19.8525422695647,
      19.4976496329425,
      19.05592955968657,
      19.07447415835337,
      18.42651420224979,
      18.18295358762772,
      17.67701121934634,
      17.16570338840665,
      16.78968549335118,
      16.66313326281432,
      15.91694434747913,
      15.86674883783528,
      15.11306050395896,
      14.40189300630865,
      14.07481249551924,
      13.56311771943757,
      12.85989038658455,
      12.05078808874147,
      11.52989978101229,
      10.80768368563314,
      9.908346006170808,
      9.275027666230132,
      8.696692909857239,
      8.161583074959376,
      6.700686086484087,
      6.003701972961426,
      5.035827674727509,
      4.095890998840332,
      3.665288883097031,
      2.669904629389445,
      1.479300989045037,
      -2.775557561562891e-17,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0};
  for (Int_t bin = 0; bin < 302; bin++) {
    m_ZDCEt_UpperCut_4p0Sigma_NeNe->SetBinContent(
        bin, ZDCEt_UpperCut_4p0Sigma_NeNe_vals[bin]);
  }
  m_ZDCEt_UpperCut_4p0Sigma_NeNe->SetDirectory(0);
  //----------------------------------------------------------------------

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
  return zdcE < cut;
}

bool HI::HIEventSelectionToolRun3::puFCalVsNtracks(
    IonDataType dataType, const xAOD::HIEventShapeContainer* es,
    const xAOD::TrackParticleContainer* tracks,
    const xAOD::VertexContainer* vertices, PileupVariation variation) const {
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
  return puFCalVsNtracks(dataType, calcFcalEt(es), count, variation);
}

bool HI::HIEventSelectionToolRun3::puFCalVsNtracks(HI::IonDataType when,
                                                   float fcalEt, int ntrk,
                                                   HI::PileupVariation) const {
  ATH_MSG_DEBUG("cutting puFCalVsNtracks: fcalEt " << fcalEt << " ntracks "
                                                   << ntrk);
  // for reference, thes numbers are taken from:
  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/#fcal-sumet-ntrk-correlation-cut
  // and more is here:
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
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

bool HI::HIEventSelectionToolRun3::puZDCPresampler(
    HI::IonDataType when, const xAOD::ZdcModuleContainer* zdcModules,
    HI::PileupVariation variation) const {
  float PreSamplerAmp_A = 0;
  float PreSamplerAmp_C = 0;
  static const SG::ConstAccessor<float> accPreSamplerAmpA("");
  static const SG::ConstAccessor<float> accPreSamplerAmpC("");
  for (const auto module : *zdcModules) {
    if (module->zdcType() != 0)
      continue;
    if (module->zdcSide() > 0)
      PreSamplerAmp_C += accPreSamplerAmpC(*module);
    if (module->zdcSide() < 0)
      PreSamplerAmp_A += accPreSamplerAmpA(*module);
  }
  return puZDCPresampler(when, PreSamplerAmp_A,PreSamplerAmp_C,  variation);
}

bool HI::HIEventSelectionToolRun3::puZDCPresampler(
    HI::IonDataType when, float presamplerA, float presamplerC,
    HI::PileupVariation variation) const {
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

  // https://atlas-heavy-ions.docs.cern.ch/analyzes/2025/
  // also See Figure 2.6 of
  // https://cds.cern.ch/record/2930965/files/ATL-COM-PHYS-2025-347.pdf
  if (when == HI::IonDataType::OO2025) {
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
  if (when == HI::IonDataType::NeNe2025) {
    //*1e3 below to convert fcalEt from TeV to GeV
    int refbin = m_ZDCEt_UpperCut_4p0Sigma_NeNe->FindFixBin(fcalEt * 1e3);
    if (refbin < 1)
      refbin = 1;

    //*1e3 below to convert Zdc value in histogram from TeV to GeV
    return m_ZDCEt_UpperCut_4p0Sigma_NeNe->GetBinContent(refbin) * 1e3;
  }

  throw std::runtime_error(std::string("period of id ") + HI::toString(when) +
                           "is not handled");

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
