/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRecConditions/LArBadChanBitPacking.h"

LArBadChanBitPacking::LArBadChanBitPacking():LArBadChanBitPackingBase()
{
  // Here, specify each problem and its gain dependence.
  addBit( LArBadChannel::LArBadChannelEnum::deadReadoutBit, "deadReadout", independent);
  addBit( LArBadChannel::LArBadChannelEnum::deadCalibBit, "deadCalib", independent);
  addBit( LArBadChannel::LArBadChannelEnum::deadPhysBit, "deadPhys", independent);
  addBit( LArBadChannel::LArBadChannelEnum::almostDeadBit, "almostDead", independent);
  addBit( LArBadChannel::LArBadChannelEnum::shortBit, "short", independent);
  addBit( LArBadChannel::LArBadChannelEnum::unstableBit, "unstable", independent);
  addBit( LArBadChannel::LArBadChannelEnum::distortedBit, "distorted", independent);
  addBit( LArBadChannel::LArBadChannelEnum::lowNoiseHGBit, "lowNoiseHG", high);
  addBit( LArBadChannel::LArBadChannelEnum::highNoiseHGBit, "highNoiseHG", high);
  addBit( LArBadChannel::LArBadChannelEnum::unstableNoiseHGBit, "unstableNoiseHG", high);
  addBit( LArBadChannel::LArBadChannelEnum::lowNoiseMGBit, "lowNoiseMG", medium);
  addBit( LArBadChannel::LArBadChannelEnum::highNoiseMGBit, "highNoiseMG", medium);
  addBit( LArBadChannel::LArBadChannelEnum::unstableNoiseMGBit, "unstableNoiseMG", medium);
  addBit( LArBadChannel::LArBadChannelEnum::lowNoiseLGBit, "lowNoiseLG", low);
  addBit( LArBadChannel::LArBadChannelEnum::highNoiseLGBit, "highNoiseLG", low);
  addBit( LArBadChannel::LArBadChannelEnum::unstableNoiseLGBit, "unstableNoiseLG", low);
  addBit( LArBadChannel::LArBadChannelEnum::missingFEBBit, "missingFEB", independent);
  addBit( LArBadChannel::LArBadChannelEnum::peculiarCalibrationLineBit, "peculiarCalibrationLine", low);
  addBit( LArBadChannel::LArBadChannelEnum::problematicForUnknownReasonBit, "problematicForUnknownReason", independent);
  addBit( LArBadChannel::LArBadChannelEnum::sporadicBurstNoiseBit, "sporadicBurstNoise", independent);
  addBit( LArBadChannel::LArBadChannelEnum::deadSCACellBit, "deadSCACell", independent);
  addBit( LArBadChannel::LArBadChannelEnum::badFirstSampleBit, "badFirstSample", independent);
  addBit( LArBadChannel::LArBadChannelEnum::unflaggedByLADIeSBit, "unflaggedByLADIeS", independent);
  addBit( LArBadChannel::LArBadChannelEnum::reflaggedByLADIeSBit, "reflaggedByLADIeS", independent);
  addBit( LArBadChannel::LArBadChannelEnum::badAutoCorrBit, "badAutoCorr", independent);

  for (unsigned int i=0; i<m_enumVec.size(); i++) {
    m_nameMap[m_nameVec[i]] = m_enumVec[i].first;
    /// Initial assignment of bit positions same as enumerator values.
    /// This may be changed FIXME provide method for changing it
    m_bitPos[m_enumVec[i].first] = m_enumVec[i].first;

    m_index[m_enumVec[i].first] = i;
  }

  initMasks(); // initialize the gain masks
};

LArBadChanSCBitPacking::LArBadChanSCBitPacking():LArBadChanSCBitPackingBase()
{
  // Here, specify each problem and its gain dependence.
  addBit( LArBadChannel::LArBadChannelSCEnum::maskedOSUMBit, "maskedOSUM", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::deadReadoutBit, "deadReadout", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::deadCalibBit, "deadCalib", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::deadPhysBit, "deadPhys", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::lowNoiseBit, "lowNoise", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::highNoiseBit, "highNoise", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::problematicForUnknownReasonBit, "problematicForUnknownReason", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::sporadicBurstNoiseBit, "sporadicBurstNoise", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::DeformedTailBit, "DeformedTail", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::DeformedPulseBit, "DeformedPulse", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::NonLinearRampBit, "NonLinearRamp", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::ADCJumpBit, "ADCJump", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::SCAProblemBit, "SCAProblem", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::OffOFCsBit, "OffOFCs", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::OffAmplitudeBit, "OffAmplitude", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::OffScaleBit, "OffScale", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::lowLightFibreBit, "lowLightFibre", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::transmissionErrorFibreBit, "transmissionErrorFibre", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::pedestalJumpBit, "pedestalJump", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::unflaggedByLADIeSBit, "unflaggedByLADIeS", independent);
  addBit( LArBadChannel::LArBadChannelSCEnum::reflaggedByLADIeSBit, "reflaggedByLADIeS", independent);
  for (unsigned int i=0; i<m_enumVec.size(); i++) {
    m_nameMap[m_nameVec[i]] = m_enumVec[i].first;
    /// Initial assignment of bit positions same as enumerator values.
    /// This may be changed FIXME provide method for changing it
    m_bitPos[m_enumVec[i].first] = m_enumVec[i].first;

    m_index[m_enumVec[i].first] = i;
  }

  initMasks(); // initialize the gain masks
}

