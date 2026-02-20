/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcUtils/ZDCTriggerSim.h"

#include <iostream>
#include <stdexcept>

// dump stream data
void ZDCTriggerSimBase::dump(std::ostream& strm) const {
  for (auto entry : m_stack) {
    strm << entry->getType() << ": ";
    entry->dump(strm);
    strm << std::endl;
  }
}

// Obtain 3 bit output from 4*2 bit arm trigger decisions
void ZDCTriggerSimCombLUT::doSimStage() {
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 2 || ptr->getNumBits() != 4)
    throw std::logic_error("Invalid input data in ZDCTriggerSimCombLUT");

  unsigned int bitsSideA = ptr->getValueTrunc(0);
  unsigned int bitsSideC = ptr->getValueTrunc(1);

  unsigned int address = (bitsSideC << 4) + bitsSideA;
  unsigned int comLUTvalue = m_combLUT.at(address);

  // ZDCTriggerSim::SimDataPtr uses shared_ptr semantics so cleanup is
  // guaranteed
  //
  ZDCTriggerSim::SimDataPtr lutOut_p(new ZDCTriggerSim::CombLUTOutput());
  static_cast<ZDCTriggerSim::CombLUTOutput*>(lutOut_p.get())
      ->setDatum(comLUTvalue);

  stackPush(lutOut_p);
}

// Obtain 4x2 bit output from arm energy sums
void ZDCTriggerSimAllLUTs::doSimStage() {
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 2 || ptr->getNumBits() != 12)
    throw std::logic_error("Invalid input data in ZDCTriggerSimAllLUTs");
  ;

  unsigned int inputSideA = ptr->getValueTrunc(0);
  unsigned int inputSideC = ptr->getValueTrunc(1);

  unsigned int valueA = m_LUTA.at(inputSideA);
  unsigned int valueC = m_LUTC.at(inputSideC);

  // ZDCTriggerSim::SimDataPtr uses shared_ptr semantics so cleanup is
  // guaranteed
  //
  ZDCTriggerSim::SimDataPtr inputs_p(new ZDCTriggerSim::CombLUTInputsInt);
  static_cast<ZDCTriggerSim::CombLUTInputsInt*>(inputs_p.get())
      ->setData({valueA, valueC});

  stackPush(ZDCTriggerSim::SimDataCPtr(inputs_p));
  ZDCTriggerSimCombLUT::doSimStage();
}

// Obtain arm energy sums from Module by Module Calibrated energies
//
void ZDCTriggerSimModuleAmpls::doSimStage() {
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 8 || ptr->getNumBits() != 12)
    throw std::logic_error("Invalid input data in ZDCTriggerSimModuleAmpls");

  unsigned int sumA = 0;
  for (size_t i = 0; i < 4; i++) {
    sumA += ptr->getValueTrunc(i);
  }

  unsigned int sumC = 0;
  for (size_t i = 4; i < 8; i++) {
    sumC += ptr->getValueTrunc(i);
  }

  // The sums get divided by 4
  //
  sumA /= 4;
  sumC /= 4;

  ZDCTriggerSim::SimDataPtr inputs_p(new ZDCTriggerSim::SideLUTInputsInt);
  static_cast<ZDCTriggerSim::SideLUTInputsInt*>(inputs_p.get())
      ->setData({sumA, sumC});

  stackPush(ZDCTriggerSim::SimDataCPtr(inputs_p));

  ZDCTriggerSimAllLUTs::doSimStage();
}

// Obtain per-module amplitudes from the FADC data. The FADC data should have 24 samples per channel
//   for 8 channels. It was easier to just concatenate them all together to match the TriggerSimData
//   implementation.
//
void ZDCTriggerSimFADC::doSimStage() {
  ZDCTriggerSim::SimDataCPtr ptr = stackTopData();
  if (ptr->getNumData() != 24*8 || ptr->getNumBits() != 12)
    throw std::logic_error("Invalid input data in ZDCTriggerSimModuleAmpls");

  unsigned int sampleIdx = 0;

  std::vector<unsigned int> moduleAmplitudes;
  
  // Loop over the eight ZDC modules
  //
  for (size_t side : {0, 1}) {
    for (size_t module : {0, 1, 2, 3}) {
      std::vector<unsigned int> FADCsamples;

      bool adcOverflow = false;
    
      //
      // Loop over the FADCs for the channel and put them in a vector for easier manipulation
      //   While we're at it, check for overflows since the trigger firmware always sends up
      //   4095 if there is an overflow 
      //
      for (size_t i = 0; i < 24; i++) {
	    unsigned int ADC = ptr->getValueTrunc(sampleIdx);
      if (ADC == 4095) {
        adcOverflow = true;
        sampleIdx += (24 - i);
        break;
      } else {
        sampleIdx++;
      }

	
	FADCsamples.push_back(ADC);
      }

      m_baseline[side][module] = 0;
      m_maxADC[side][module] = 0;
      m_maxNegDeriv2nd[side][module] = 0;
      
      if (adcOverflow) {
	//
	// If the ADC overflows, the firmware returns amplitude of 4095
	//
	moduleAmplitudes.push_back(4095);
      }
      else {
	std::vector<unsigned int> negDeriv2nd = CalculateNeg2ndDerivatives(FADCsamples, 2);
	
	// Now loop over the min-max sample range (within single channel) and see
	//   if the second derivative threshold is exceeded. If so, we pick off the 
	//   maximum amplitude in that same range -- this is what the firmware does
	//
	bool havePulse = false;
	unsigned int maxAmp = 0;
	unsigned int baseline = 0;
	unsigned int maxNeg2ndDeriv = 0;
	
	for (unsigned int sampleTest = m_minSampleAna; sampleTest <= m_maxSampleAna; sampleTest++) {
	  if (negDeriv2nd.at(sampleTest) > maxNeg2ndDeriv) maxNeg2ndDeriv = negDeriv2nd.at(sampleTest);
	  if (negDeriv2nd.at(sampleTest) > m_deriv2ndThresholds[side][module] && !havePulse) {
	    havePulse = true;

	    // The firmware evaluates the baseline a certain number of samples before
	    //   the 2nd derivative threshold is crossed
	    //
	    baseline = FADCsamples.at(sampleTest - m_baselineDelta);
	  }
	  
	  if (FADCsamples.at(sampleTest) > maxAmp) maxAmp = FADCsamples.at(sampleTest);
	}

	m_maxNegDeriv2nd[side][module] = maxNeg2ndDeriv;
	if (havePulse) {
	  moduleAmplitudes.push_back(std::max<int>(int(maxAmp - baseline), 0));
	  m_baseline[side][module] = baseline;
	  m_maxADC[side][module] = maxAmp;
	}
	else {
	  moduleAmplitudes.push_back(0);
	  m_baseline[side][module] = 0;
	  m_maxADC[side][module] = 0;
	}
      }
    }
  }
  
  ZDCTriggerSim::SimDataPtr inputs_p(new ZDCTriggerSim::ModuleAmplInputsInt);
  static_cast<ZDCTriggerSim::ModuleAmplInputsInt*>(inputs_p.get())->setData(moduleAmplitudes);

  stackPush(inputs_p);

  ZDCTriggerSimModuleAmpls::doSimStage();
}


std::vector<unsigned int>
ZDCTriggerSimFADC::CalculateNeg2ndDerivatives(const std::vector<unsigned int>& samples, unsigned int step)
{
  std::vector<unsigned int> der2ndVec(step, 0);

  for (size_t sample = step; sample < samples.size() - step; sample++) {
    int diff2nd = samples[sample + step] + samples[sample - step] - 2*samples[sample];
    if (diff2nd > 0) der2ndVec.push_back(0);
    else der2ndVec.push_back(-diff2nd);
  }

  der2ndVec.insert(der2ndVec.end(), step, 0);

  return der2ndVec;
}
