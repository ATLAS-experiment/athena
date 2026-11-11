/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
//
//     Author:
//     Alexander.Paramonov@cern.ch
//
//     Created:
//    05 February 2016
//
//  	 File Name:
//     TileRawChannelBuilderQIEFilter.h
//
//    Description:
//		The code calculates time and energy/charge of QIE pulses
//
//////////////////////////////////////////////////////////////////////

#include "TileEvent/TileRawChannel.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Atlas includes
#include "AthAllocators/DataPool.h"
#include "AthenaKernel/errorcheck.h"

// Tile includes
#include "TileRecUtils/TileRawChannelBuilderQIEFilter.h"
#include "CLHEP/Matrix/Matrix.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileDigits.h"
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileConditions/TileInfo.h"

//using namespace std;
#include <algorithm>

//interface stuff
static const InterfaceID IID_ITileRawChannelBuilderQIEFilter("TileRawChannelBuilderQIEFilter", 1, 0);

const InterfaceID& TileRawChannelBuilderQIEFilter::interfaceID() {
	return IID_ITileRawChannelBuilderQIEFilter;
}

#define TILE_QIEFilterBUILDERVERBOSE false

TileRawChannelBuilderQIEFilter::TileRawChannelBuilderQIEFilter(const std::string& type
							       , const std::string& name
							       , const IInterface *parent):
  TileRawChannelBuilder(type, name, parent)
{
  //declare interfaces
  declareInterface < TileRawChannelBuilder > (this);
  declareInterface < TileRawChannelBuilderQIEFilter > (this);
  
  m_rawChannelContainerKey = "TileRawChannelQIE";
}

TileRawChannelBuilderQIEFilter::~TileRawChannelBuilderQIEFilter() = default;

StatusCode TileRawChannelBuilderQIEFilter::initialize() {

  ATH_MSG_INFO("initialize()");

  m_rChType = TileFragHash::OptFilterOffline; // type for offline Opt Filter

  // init in superclass
  CHECK(TileRawChannelBuilder::initialize());

  m_nSamples = m_tileInfo->NdigitSamples();

  if (m_nSamples != 7) { // && (m_pedestalMode == 71 || m_pedestalMode == 7621)) {
    ATH_MSG_ERROR("Incompatable pedestal mode [" << m_pedestalMode.value() << "] and number of samples [" << m_nSamples << "]");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

TileRawChannel* TileRawChannelBuilderQIEFilter::rawChannel(const TileDigits* digits, const EventContext& /*ctx*/)
{
  ++m_chCounter;

  float pedestal = 10.; // A non-zero default is used to avoid the data quality cuts.
  float energy = 0.;
  float time = 0.;
  float chi2 = 0.;
  m_digits = digits->samples(); //the 7 samples; currently charge is expected. It the future these will be digitized
  const HWIdentifier adcId = digits->adc_HWID();
  int gain = m_tileHWID->adc(adcId); //this is set to 1 for QIE FEB by default

  ATH_MSG_VERBOSE("Building Raw Channel, with QIEFilter, HWID:" << m_tileHWID->to_string(adcId) << " gain=" << gain);

  int ros = m_tileHWID->ros(adcId);
  int drawer = m_tileHWID->drawer(adcId);
  int channel = m_tileHWID->channel(adcId);
  chi2 = filter(ros, drawer, channel, energy, time);

  if (msgLvl(MSG::VERBOSE)) {
    msg(MSG::VERBOSE) << "Creating QIEFilter RawChannel" << " a=" << energy << " t=" << time
		      << " q=" << chi2 << endmsg;

    msg(MSG::VERBOSE) << "digits:";
    for (unsigned int i = 0; i < m_digits.size(); ++i)
      msg(MSG::VERBOSE) << " " << m_digits[i];
    msg(MSG::VERBOSE) << " " << endmsg;
  }

  // return new TileRawChannel
  // TileRawChannel *rawCh = new TileRawChannel(adcId,OptFilterEne,OptFilterTime,OptFilterChi2,OptFilterPed);
  DataPool<TileRawChannel> tileRchPool(m_dataPoollSize);
  TileRawChannel *rawCh = tileRchPool.nextElementPtr();
  rawCh->assign (adcId,
		 energy,
		 time,
		 chi2,
		 pedestal);

  if (TileID::HIGHGAIN == gain) {
    ++m_nChH;
    m_RChSumH += energy;
  } else {
    ++m_nChL;
    m_RChSumL += energy;
  }

  return rawCh;
}

int TileRawChannelBuilderQIEFilter::findMaxDigitPosition() {

  ATH_MSG_VERBOSE("  findMaxDigitPosition()");
  
  int iMaxDigit = 0;
  float maxDigit = 0.;

  for (unsigned int i = 0; i < m_digits.size(); i++) {
    if (maxDigit < m_digits[i]) {
      maxDigit = m_digits[i];
      iMaxDigit = i;
    }
  }

  if (msgLvl(MSG::VERBOSE)) {
    for (unsigned int i = 0; i < m_digits.size(); i++) {
      msg(MSG::VERBOSE) << " " << m_digits[i];
    }

    msg(MSG::VERBOSE) << "; Max: digit[" << iMaxDigit << "]=" << maxDigit << endmsg;

  }

  return iMaxDigit;
}

float TileRawChannelBuilderQIEFilter::filter(int ros, int drawer, int channel, float &amplitude, float &time) {
  
  ATH_MSG_VERBOSE("filter()");

  amplitude = 0.;
  time = 0.;
  float chi2 = -1.;

  auto minMaxDigits = std::minmax_element(m_digits.begin(), m_digits.end());
  float minDigit = *minMaxDigits.first;
  float maxDigit = *minMaxDigits.second;

  if (maxDigit - minDigit < 0.01) { // constant value in all samples // TODO: the cut-off needs to be adjusted for the charge difference

    //pedestal = minDigit;
    chi2 = 0.;
    ATH_MSG_VERBOSE("CASE NO SIGNAL: maxdig-mindig = " << maxDigit << "-" << minDigit << " = " << maxDigit - minDigit);
    
    m_nConst++;

  } else {

    chi2 = compute(ros, drawer, channel, amplitude, time);
    m_nSignal++;

  }

  return chi2;
}

//charge per 25 ns is calculated here
float TileRawChannelBuilderQIEFilter::compute(int ros, int drawer, int channel, float &amplitude, float &time) {

  ATH_MSG_VERBOSE("compute();" << " ros=" << ros << " drawer=" << drawer << " channel=" << channel);
  
  float chi2 = -999.; //the chi2 is not calculated by default
  float fraction_mean = 0;

  amplitude = 0.;
  time = 0.; //TODO: Time needs to be implemented using the QIE TDC

  //TODO: These numbers may be placed in a database
  const float avg_fraction = 0.18;
  const float Q_1pe = 17.; //charge per a photo-electron in fC

  // With pileup suppression
  double prob_cut_off = 0.005;

  if (m_digits[2] > 0) //check if there is pileup in the preceding BC.
    amplitude = m_digits[3] - avg_fraction * m_digits[2] / (1. - avg_fraction);

  if (amplitude < 0)
    amplitude = 0;

  if (m_digits[4] > 0) {
    amplitude += m_digits[4];
    fraction_mean = m_digits[4] / amplitude;
    
    if (fraction_mean > avg_fraction) { // Pileup is suspected in the 2nd integration window
      int n = (int) (amplitude / Q_1pe);
      int k = (int) (m_digits[4] / Q_1pe);

      //Here we calculate the binomial coefficient; n>=k
      // n!/(k! (n-k!))
      double binom_coeff = 1.;
      
      for (int j = n; j > 0; j--) {
	if (j > k)
	  binom_coeff = binom_coeff * (double) j;
	
	if (j < (n - k + 1))
	  binom_coeff = binom_coeff / (double) j;
      }

      //binomial probability
      double prob = binom_coeff * pow(avg_fraction, k) * pow(1. - avg_fraction, n - k);
      
      if (prob < prob_cut_off) { //likely there is too much pileup in the 2nd integration window.
	amplitude -= m_digits[4];
	amplitude *= 1. / (1. - avg_fraction);
      }
    }
    
  }

  bool goodEnergy = (fabs(amplitude) > 1.0e-04);
  if (goodEnergy) {
    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "QIEFilterEne=" << amplitude << endmsg;
      msg(MSG::VERBOSE) << "QIEFilterTimee=" << time << endmsg;
    }
  } else {
    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "QIEFilterEne=" << amplitude << "   ... assuming 0.0" << endmsg;
      msg(MSG::VERBOSE) << "QIEFilterTime=" << time << "   ... assuming 0.0" << endmsg;
    }
  }

  if (fabs(chi2) > 1.0e-04 || goodEnergy) {
    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "QIEFilterTime=" << time << endmsg;
      msg(MSG::VERBOSE) << "QIEFilterChi2=" << chi2 << endmsg;
    }
  } else {
    if (msgLvl(MSG::VERBOSE)) {
      msg(MSG::VERBOSE) << "QIEFilterTime=" << time << endmsg;
      msg(MSG::VERBOSE) << "QIEFilterChi2=" << chi2 << "   ... assuming 0.0" << endmsg;
    }
  }

  return chi2;
}

