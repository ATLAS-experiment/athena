/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InDetTrackBiasingTool.h"
#include "xAODEventInfo/EventInfo.h"
#include <math.h>

#include "PathResolver/PathResolver.h"

#include <TH2.h>
#include <TFile.h>

namespace {
  using TrackCorrTool_t = CP::CorrectionTool< xAOD::TrackParticleContainer >;
}

namespace InDet {

  static const CP::SystematicSet BiasSystematics = 
    {
      InDet::TrackSystematicMap.at(TRK_BIAS_D0_WM),
      InDet::TrackSystematicMap.at(TRK_BIAS_Z0_WM),
      InDet::TrackSystematicMap.at(TRK_BIAS_QOVERP_SAGITTA_WM),
    };

  InDetTrackBiasingTool::InDetTrackBiasingTool(const std::string& name) :
    InDetTrackSystematicsTool(name)
  {

#ifndef XAOD_STANDALONE
    declareInterface<IInDetTrackBiasingTool>(this);
#endif

    declareProperty("biasD0", m_biasD0);
    declareProperty("biasZ0", m_biasZ0);
    declareProperty("biasQoverPsagitta", m_biasQoverPsagitta);
    declareProperty("runNumber", m_runNumber);
    declareProperty("isData", m_isData);
    declareProperty("isSimulation", m_isSimulation);

    declareProperty("calibFileData15", m_calibFileData15 = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2015.root");
    declareProperty("calibFileData16_1stPart", m_calibFileData16_1stPart = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2016_1stPart.root");
    declareProperty("calibFileData16_2ndPart", m_calibFileData16_2ndPart = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2016_2ndPart.root");
    declareProperty("calibFileData17_1stPart", m_calibFileData17_1stPart = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2017_1stPart.root");
    declareProperty("calibFileData17_2ndPart", m_calibFileData17_2ndPart = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2017_2ndPart.root");
    declareProperty("calibFileData18_1stPart", m_calibFileData18_1stPart = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2018_1stPart.root");
    declareProperty("calibFileData18_2stPart", m_calibFileData18_2ndPart = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/REL22_REPRO_2018_2ndPart.root");


  }

  StatusCode InDetTrackBiasingTool::initialize()
  {

    if (m_isData && m_isSimulation) {
      ATH_MSG_ERROR( "Cannot manually set for both data and simulation!" );
      return StatusCode::FAILURE;
    }

    if (m_biasD0 != 0.) {
      ATH_MSG_INFO( "overall d0 bias added = " << m_biasD0
        << " mm (not part of an official recommendation)" );
    }
    if (m_biasZ0 != 0.) {
      ATH_MSG_INFO( "overall z0 bias added = " << m_biasZ0
        << " mm (not part of an official recommendation)" );
    }
    if (m_biasQoverPsagitta != 0.) {
      ATH_MSG_INFO( "overall QoverP sagitta bias added = " << m_biasQoverPsagitta
        << " TeV^-1 (not part of an official recommendation)" );
    }

    if (m_runNumber > 0) {
      ATH_MSG_WARNING( "Using manually-set run number (" << m_runNumber << ") to determine which calibration file to use." );
    }

    ATH_CHECK( initHistograms() );

    ATH_CHECK( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }

  InDetTrackBiasingTool::~InDetTrackBiasingTool() {
    m_runNumber = -1;
  }

  CP::CorrectionCode InDetTrackBiasingTool::applyCorrection(xAOD::TrackParticle& track) {

    [[maybe_unused]] static const bool firstTime = [&]() {
      if ( ! firstCall().isSuccess() ) { // this will check data vs. MC and run number.
        throw std::runtime_error("Error calling InDetTrackBiasingTool::firstCall");
      }
      return false;
    }();

    // specific histograms to be used based on the run number
    TH2* biasD0Histogram = nullptr;
    TH2* biasZ0Histogram = nullptr;
    TH2* biasQoverPsagittaHistogram = nullptr;
    TH2* biasD0HistError = nullptr;
    TH2* biasZ0HistError = nullptr;
    TH2* biasQoverPsagittaHistError = nullptr;

    // determine which run number to use
    const xAOD::EventInfo* eventInfo = evtStore()->retrieve<const xAOD::EventInfo>("EventInfo");
    if (!eventInfo) {
      ATH_MSG_ERROR("Could not retrieve EventInfo object!");
      return CP::CorrectionCode::Error;
    }
    auto runNumber = eventInfo->runNumber(); // start with run number stored in event info
    static const SG::AuxElement::Accessor<unsigned int> randomRunNumber("RandomRunNumber");
    if (m_runNumber > 0) { // if manually-set run number is provided, use it
      runNumber = m_runNumber;
    } else if (m_isSimulation && randomRunNumber.isAvailable(*eventInfo)) { // use RandomRunNumber for simulation if available
      runNumber = randomRunNumber(*(eventInfo));
    }

    // figure out which "IOV" the run number corresponds to
    // TODO: replace StatusCodes with CP::CorrectionCodes
    if (runNumber <= 0) {
      ATH_MSG_WARNING( "Run number not set." );
    }
    if (runNumber >= 286282 && runNumber <= 287931) {
      ATH_MSG_INFO( "Calibrating for 2015 HI and 5 TeV pp runs (286282 to 287931)." );
      ATH_MSG_ERROR( "The 5 TeV and heavy ion runs do not have biasing maps for release 22. "
         "Contact the tracking CP group to discuss the derivation of these maps." );
      return CP::CorrectionCode::Error;
    } else if (runNumber <= 364485) {
      if (runNumber < 296939) { // data15 (before 296939)
        biasD0Histogram = m_data15_biasD0Histogram.get();
        biasZ0Histogram = m_data15_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data15_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data15_biasD0HistError.get();
        biasZ0HistError = m_data15_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data15_biasQoverPsagittaHistError.get();
      } else if (runNumber <= 301912) { // data16 part 1/2 (296939 to 301912)
        biasD0Histogram = m_data16_1stPart_biasD0Histogram.get();
        biasZ0Histogram = m_data16_1stPart_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data16_1stPart_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data16_1stPart_biasD0HistError.get();
        biasZ0HistError = m_data16_1stPart_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data16_1stPart_biasQoverPsagittaHistError.get();
      } else if (runNumber <= 312649) { // data16 part 2/2 (301912 to 312649)
        biasD0Histogram = m_data16_2ndPart_biasD0Histogram.get();
        biasZ0Histogram = m_data16_2ndPart_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data16_2ndPart_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data16_2ndPart_biasD0HistError.get();
        biasZ0HistError = m_data16_2ndPart_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data16_2ndPart_biasQoverPsagittaHistError.get();
      } else if (runNumber <= 334842) { // data17 part 1/2 (324320 to 334842)
        biasD0Histogram = m_data17_1stPart_biasD0Histogram.get();
        biasZ0Histogram = m_data17_1stPart_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data17_1stPart_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data17_1stPart_biasD0HistError.get();
        biasZ0HistError = m_data17_1stPart_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data17_1stPart_biasQoverPsagittaHistError.get();
      } else if (runNumber <= 348197) { // data17 (part 2/2 (334842 to 348197)
        biasD0Histogram = m_data17_2ndPart_biasD0Histogram.get();
        biasZ0Histogram = m_data17_2ndPart_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data17_2ndPart_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data17_2ndPart_biasD0HistError.get();
        biasZ0HistError = m_data17_2ndPart_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data17_2ndPart_biasQoverPsagittaHistError.get();
      } else if (runNumber <= 353000) { // data18 (part 1/2 (348197 to 353000)
        biasD0Histogram = m_data18_1stPart_biasD0Histogram.get();
        biasZ0Histogram = m_data18_1stPart_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data18_1stPart_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data18_1stPart_biasD0HistError.get();
        biasZ0HistError = m_data18_1stPart_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data18_1stPart_biasQoverPsagittaHistError.get();
      } else { // data18 (part 2/2 (353000 to 364485)
        biasD0Histogram = m_data18_2ndPart_biasD0Histogram.get();
        biasZ0Histogram = m_data18_2ndPart_biasZ0Histogram.get();
        biasQoverPsagittaHistogram = m_data18_2ndPart_biasQoverPsagittaHistogram.get();
        biasD0HistError = m_data18_2ndPart_biasD0HistError.get();
        biasZ0HistError = m_data18_2ndPart_biasZ0HistError.get();
        biasQoverPsagittaHistError = m_data18_2ndPart_biasQoverPsagittaHistError.get();
      }
    } else {
      ATH_MSG_ERROR( "Run number = " << runNumber << " not in recognized range (< 364485)." );
      return CP::CorrectionCode::Error;
    }

    // don't do the biasing if the histograms are null
    m_doD0Bias = biasD0Histogram != nullptr;
    m_doZ0Bias = biasZ0Histogram != nullptr;
    m_doQoverPBias = biasQoverPsagittaHistogram != nullptr;

    if (!m_doD0Bias) ATH_MSG_WARNING( "Will not perform d0 bias." );
    if (!m_doZ0Bias) ATH_MSG_WARNING( "Will not perform z0 bias." );
    if (!m_doQoverPBias) ATH_MSG_WARNING( "Will not perform q/p sagitta bias." );

    // declare static accessors to avoid repeating string lookups
    static const SG::AuxElement::Accessor< float > accD0( "d0" );
    static const SG::AuxElement::Accessor< float > accZ0( "z0" );
    static const SG::AuxElement::Accessor< float > accQOverP( "qOverP" );

    const float phi = track.phi0();
    const float eta = track.eta();

    // do the biasing
    if ( m_doD0Bias ) {
      bool d0WmActive = isActive( TRK_BIAS_D0_WM );
      if ( m_isData || d0WmActive ) {
        accD0( track ) += readHistogram(m_biasD0, biasD0Histogram, phi, eta);
        if ( m_isData && d0WmActive ) {
          accD0( track ) += readHistogram(0., biasD0HistError, phi, eta);
        }
      }
    }
    if ( m_doZ0Bias ) {
      bool z0WmActive = isActive( TRK_BIAS_Z0_WM );
      if ( m_isData || z0WmActive ) {
        accZ0( track ) += readHistogram(m_biasZ0, biasZ0Histogram, phi, eta);
        if ( m_isData && z0WmActive ) {
          accZ0( track ) += readHistogram(0., biasZ0HistError, phi, eta);
        }
      }
    }
    if ( m_doQoverPBias ) {
      bool qOverPWmActive = isActive( TRK_BIAS_QOVERP_SAGITTA_WM );
      if ( m_isData || qOverPWmActive ) {
        auto sinTheta = 1.0/cosh(eta);
        // readHistogram flips the sign of the correction if m_isSimulation is true
        accQOverP( track ) += 1.e-6*sinTheta*readHistogram(m_biasQoverPsagitta, biasQoverPsagittaHistogram, phi, eta);
        if ( m_isData && qOverPWmActive ) {
          accQOverP( track ) += 1.e-6*sinTheta*readHistogram(0., biasQoverPsagittaHistError, phi, eta);
        }
      }
    }

    return CP::CorrectionCode::Ok;
  }

  StatusCode InDetTrackBiasingTool::initHistograms()
  {

    ATH_MSG_INFO( "Using for data15 (before 296939) the calibration file " << PathResolverFindCalibFile(m_calibFileData15) );
    ATH_CHECK ( initObject<TH2>(m_data15_biasD0Histogram, m_calibFileData15, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data15_biasZ0Histogram, m_calibFileData15, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data15_biasQoverPsagittaHistogram, m_calibFileData15, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data15_biasD0HistError, m_calibFileData15, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data15_biasZ0HistError, m_calibFileData15, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data15_biasQoverPsagittaHistError, m_calibFileData15, m_sagitta_uncertainty_histName) );

    ATH_MSG_INFO( "Using for data16 part 1/2 (296939 to 301912) the calibration file " << PathResolverFindCalibFile(m_calibFileData16_1stPart) );
    ATH_CHECK ( initObject<TH2>(m_data16_1stPart_biasD0Histogram, m_calibFileData16_1stPart, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_1stPart_biasZ0Histogram, m_calibFileData16_1stPart, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_1stPart_biasQoverPsagittaHistogram, m_calibFileData16_1stPart, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_1stPart_biasD0HistError, m_calibFileData16_1stPart, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_1stPart_biasZ0HistError, m_calibFileData16_1stPart, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_1stPart_biasQoverPsagittaHistError, m_calibFileData16_1stPart, m_sagitta_uncertainty_histName) );

    ATH_MSG_INFO( "Using for data16 part 2/2 (301912 to 312649) the calibration file " << PathResolverFindCalibFile(m_calibFileData16_2ndPart) );
    ATH_CHECK ( initObject<TH2>(m_data16_2ndPart_biasD0Histogram, m_calibFileData16_2ndPart, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_2ndPart_biasZ0Histogram, m_calibFileData16_2ndPart, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_2ndPart_biasQoverPsagittaHistogram, m_calibFileData16_2ndPart, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_2ndPart_biasD0HistError, m_calibFileData16_2ndPart, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_2ndPart_biasZ0HistError, m_calibFileData16_2ndPart, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data16_2ndPart_biasQoverPsagittaHistError, m_calibFileData16_2ndPart, m_sagitta_uncertainty_histName) );

    ATH_MSG_INFO( "Using for data17 part 1/2 (324320 to 334842) the calibration file " << PathResolverFindCalibFile(m_calibFileData17_1stPart) );
    ATH_CHECK ( initObject<TH2>(m_data17_1stPart_biasD0Histogram, m_calibFileData17_1stPart, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_1stPart_biasZ0Histogram, m_calibFileData17_1stPart, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_1stPart_biasQoverPsagittaHistogram, m_calibFileData17_1stPart, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_1stPart_biasD0HistError, m_calibFileData17_1stPart, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_1stPart_biasZ0HistError, m_calibFileData17_1stPart, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_1stPart_biasQoverPsagittaHistError, m_calibFileData17_1stPart, m_sagitta_uncertainty_histName) );

    ATH_MSG_INFO( "Using for data17 (part 2/2 (334842 to 348197) the calibration file " << PathResolverFindCalibFile(m_calibFileData17_2ndPart) );
    ATH_CHECK ( initObject<TH2>(m_data17_2ndPart_biasD0Histogram, m_calibFileData17_2ndPart, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_2ndPart_biasZ0Histogram, m_calibFileData17_2ndPart, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_2ndPart_biasQoverPsagittaHistogram, m_calibFileData17_2ndPart, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_2ndPart_biasD0HistError, m_calibFileData17_2ndPart, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_2ndPart_biasZ0HistError, m_calibFileData17_2ndPart, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data17_2ndPart_biasQoverPsagittaHistError, m_calibFileData17_2ndPart, m_sagitta_uncertainty_histName) );

    ATH_MSG_INFO( "Using for data18 (part 1/2 (348197 to 353000) the calibration file " << PathResolverFindCalibFile(m_calibFileData18_1stPart) );
    ATH_CHECK ( initObject<TH2>(m_data18_1stPart_biasD0Histogram, m_calibFileData18_1stPart, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_1stPart_biasZ0Histogram, m_calibFileData18_1stPart, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_1stPart_biasQoverPsagittaHistogram, m_calibFileData18_1stPart, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_1stPart_biasD0HistError, m_calibFileData18_1stPart, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_1stPart_biasZ0HistError, m_calibFileData18_1stPart, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_1stPart_biasQoverPsagittaHistError, m_calibFileData18_1stPart, m_sagitta_uncertainty_histName) );

    ATH_MSG_INFO( "Using for data18 (part 2/2 (353000 to 364485) the calibration file " << PathResolverFindCalibFile(m_calibFileData18_2ndPart) );
    ATH_CHECK ( initObject<TH2>(m_data18_2ndPart_biasD0Histogram, m_calibFileData18_2ndPart, m_d0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_2ndPart_biasZ0Histogram, m_calibFileData18_2ndPart, m_z0_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_2ndPart_biasQoverPsagittaHistogram, m_calibFileData18_2ndPart, m_sagitta_nominal_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_2ndPart_biasD0HistError, m_calibFileData18_2ndPart, m_d0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_2ndPart_biasZ0HistError, m_calibFileData18_2ndPart, m_z0_uncertainty_histName) );
    ATH_CHECK ( initObject<TH2>(m_data18_2ndPart_biasQoverPsagittaHistError, m_calibFileData18_2ndPart, m_sagitta_uncertainty_histName) );

    return StatusCode::SUCCESS;
  }

  StatusCode InDetTrackBiasingTool::firstCall()
  {
    assert( ! (m_isData && m_isSimulation) );

    const xAOD::EventInfo* ei = nullptr;
    auto sc = evtStore()->retrieve( ei, "EventInfo" );
    if ( ! sc.isSuccess() ) {
      if (m_runNumber <= 0 || !(m_isData||m_isSimulation)) {
        ATH_MSG_ERROR( "Unable to retrieve from event store. Manually set data/simulation and/or run number." );
        return StatusCode::FAILURE;
      }
    }
    bool isSim = ei->eventType( xAOD::EventInfo::IS_SIMULATION );
    if (isSim) {
      if ( m_isData ) {
        ATH_MSG_WARNING( "Manually set to data setting, but the type is detected as simulation." );
        ATH_MSG_WARNING( "Ensure that this behaviour is desired." );
      } else {
        m_isSimulation = true;
      }
    } else {
      if ( m_isSimulation ) {
        ATH_MSG_WARNING( "Manually set to simulation setting, but the type is detected as data." );
        ATH_MSG_WARNING( "Ensure that this behaviour is desired." );
      } else {
        m_isData = true;
      }
    }
    assert( m_isData != m_isSimulation ); // one must be true and the other false
    if (m_isData) ATH_MSG_INFO( "Set to data. Will apply biases to correct those observed in data." );
    if (m_isSimulation) ATH_MSG_INFO( "Set to simulation. Will apply biases in direction that is observed in data." );

    // warn if set to simulation but RandomRunNumber not found and no run number provided (will use run number set in event info)
    static const SG::AuxElement::Accessor<unsigned int> randomRunNumber("RandomRunNumber");
    if (m_isSimulation && !randomRunNumber.isAvailable(*ei) && m_runNumber <= 0) {
      ATH_MSG_WARNING("Set to simulation with no run number provided, but RandomRunNumber not available. Will use default run number from EventInfo, "
        "but biasing won't accurately reflect intervals of validity throughout the year. Run PileupReweightingTool first to pick up RandomRunNumber decorations.");
    }
    return StatusCode::SUCCESS;
  }

  float InDetTrackBiasingTool::readHistogram(float fDefault, TH2* histogram, float phi, float eta) const {
    if (histogram == nullptr) {
      ATH_MSG_ERROR( "Configuration histogram is invalid. Check the run number and systematic configuration combination.");
      throw std::runtime_error( "invalid configuration" );
    }

    // safety measure:
    if( eta>2.499 )  eta= 2.499;
    if( eta<-2.499 ) eta=-2.499;

    // the sign assumes that we apply a correction opposite to what the maps give
    float f = histogram->GetBinContent(histogram->FindBin(eta, phi));
    if (m_isSimulation) f = -f;
    f += fDefault;   // should be zero unless a manual override is provided

    return f;
  }

  CP::CorrectionCode InDetTrackBiasingTool::correctedCopy( const xAOD::TrackParticle& in,
							    xAOD::TrackParticle*& out )
  {
    return TrackCorrTool_t::correctedCopy(in, out);
  }
  
  CP::CorrectionCode InDetTrackBiasingTool::applyContainerCorrection( xAOD::TrackParticleContainer& cont )
  {
    return TrackCorrTool_t::applyContainerCorrection(cont);
  }

  bool InDetTrackBiasingTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet InDetTrackBiasingTool::affectingSystematics() const
  {
    return BiasSystematics;
  }

  CP::SystematicSet InDetTrackBiasingTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode InDetTrackBiasingTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }


}
