/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArBadChannel_H
#define LArBadChannel_H

template <class T> class TLArBadChanBitPackingBase;

class  LArBadChannel {
 public:
  typedef unsigned int      PosType;
  typedef unsigned int      BitWord;


  class LArBadChannelEnum {
    public:

    enum ProblemType {
       deadReadoutBit = 0, 
       deadCalibBit = 1, 
       deadPhysBit = 2, 
       almostDeadBit = 3, 
       shortBit = 4, 
       unstableBit = 5,
       distortedBit = 6,
       lowNoiseHGBit = 7,
       highNoiseHGBit = 8,
       unstableNoiseHGBit = 9,
       lowNoiseMGBit = 10,
       highNoiseMGBit = 11,
       unstableNoiseMGBit = 12,
       lowNoiseLGBit = 13,
       highNoiseLGBit = 14,
       unstableNoiseLGBit = 15,
       missingFEBBit = 16,
       peculiarCalibrationLineBit = 17,
       problematicForUnknownReasonBit = 18,
       sporadicBurstNoiseBit = 19,
       deadSCACellBit = 20,
       badFirstSampleBit = 21,
       unflaggedByLADIeSBit = 22,
       reflaggedByLADIeSBit = 23
    };

  };

  class LArBadChannelSCEnum : public LArBadChannelEnum{
    public:

    enum ProblemType {
       maskedOSUMBit = 0, 
       deadReadoutBit = 1, 
       deadCalibBit = 2, 
       deadPhysBit = 3, 
       lowNoiseBit = 4,
       highNoiseBit = 5,
       problematicForUnknownReasonBit = 6,
       sporadicBurstNoiseBit = 7,
       DeformedTailBit = 8,
       DeformedPulseBit = 9,
       NonLinearRampBit = 10,
       ADCJumpBit = 11,
       SCAProblemBit = 12,
       OffOFCsBit=13,
       OffAmplitudeBit=14,
       OffScaleBit=15,
       lowLightFibreBit=16,
       transmissionErrorFibreBit=17,
       unflaggedByLADIeSBit = 30,
       reflaggedByLADIeSBit = 31
    };

  };


  explicit LArBadChannel( BitWord rawStatus=0, bool isSC=false) : m_word(rawStatus), m_isSC(isSC)  {}
	
  /// Returns true if corresponding status bit its set		
  bool statusBad(PosType  pb) const {
    BitWord mask = 1u << pb;
    return ((m_word & mask) != 0);
  }

  /// Returns true if there is no problem in corresponding status bit
  bool statusOK( PosType pb) const {return !statusBad(pb);}
	
  /// Returns true if no problems at all (all bits at zero)
  bool good() const {return m_word == 0;}
	

  //Cases used only for supercells
  bool maskedOSUM() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::maskedOSUMBit); else return false;}
  bool deformedTail() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::DeformedTailBit); else return false;}
  bool deformedPulse() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::DeformedPulseBit); else return false;}
  bool nonLinearRamp() const{ if (m_isSC) return statusBad( LArBadChannelSCEnum::NonLinearRampBit); else return false;}
  bool ADCJump() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::ADCJumpBit); else return false;}
  bool SCAProblem() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::SCAProblemBit); else return false;}
  bool offOFCs() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::OffOFCsBit); else return false;}
  bool offAmplitude() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::OffAmplitudeBit); else return false;}
  bool offScale() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::OffScaleBit); else return false;}
  bool lowLightFibre() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::lowLightFibreBit); else return false;}
  bool transmissionErrorFibre() const {if (m_isSC) return statusBad( LArBadChannelSCEnum::transmissionErrorFibreBit); else return false;}


  //Common cases (regular and supercell)
  bool deadReadout() const { if(m_isSC) return statusBad( LArBadChannelSCEnum::deadReadoutBit); else return statusBad( LArBadChannelEnum::deadReadoutBit);}
  bool deadCalib() const { if(m_isSC) return statusBad( LArBadChannelSCEnum::deadCalibBit); else return statusBad( LArBadChannelEnum::deadCalibBit);} 
  bool deadPhys() const { if(m_isSC) return statusBad( LArBadChannelSCEnum::deadPhysBit); else return statusBad( LArBadChannelEnum::deadPhysBit);} 
  bool almostDead() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::almostDeadBit);} 
  bool shortProblem() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::shortBit);} 
  bool unstable() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::unstableBit);}
  bool distorted() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::distortedBit);}
  bool lowNoiseHG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::lowNoiseHGBit);}
  bool highNoiseHG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::highNoiseHGBit);}
  bool unstableNoiseHG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::unstableNoiseHGBit);}
  bool lowNoiseMG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::lowNoiseMGBit);}
  bool highNoiseMG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::highNoiseMGBit);}
  bool unstableNoiseMG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::unstableNoiseMGBit);}
  bool lowNoiseLG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::lowNoiseLGBit);}
  bool highNoiseLG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::highNoiseLGBit);}
  bool unstableNoiseLG() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::unstableNoiseLGBit);}
  bool missingFEB() const { if(m_isSC) return false; else  return statusBad( LArBadChannelEnum::missingFEBBit);}
  bool peculiarCalibrationLine() const { if(m_isSC) return false; else return statusBad( LArBadChannelEnum::peculiarCalibrationLineBit);}
  bool problematicForUnknownReason() const { if(m_isSC) return statusBad( LArBadChannelSCEnum::problematicForUnknownReasonBit); else return statusBad( LArBadChannelEnum::problematicForUnknownReasonBit);}
  bool sporadicBurstNoise() const {if(m_isSC) return statusBad( LArBadChannelSCEnum::sporadicBurstNoiseBit); else return statusBad( LArBadChannelEnum::sporadicBurstNoiseBit);}
  bool deadSCACell() const {if(m_isSC) return false; else return statusBad( LArBadChannelEnum::deadSCACellBit);}
  bool badFirstSample() const {if(m_isSC) return false; else return statusBad( LArBadChannelEnum::badFirstSampleBit);}
  bool unflaggedByLADIeS() const {if(m_isSC) return statusBad( LArBadChannelSCEnum::unflaggedByLADIeSBit); else return statusBad( LArBadChannelEnum::unflaggedByLADIeSBit);}
  bool reflaggedByLADIeS() const {if(m_isSC) return statusBad( LArBadChannelSCEnum::reflaggedByLADIeSBit); else return statusBad( LArBadChannelEnum::reflaggedByLADIeSBit);}
	
  bool reallyNoisy() const {return (highNoiseHG() || highNoiseMG() || highNoiseLG() ||
				    unstableNoiseHG() || unstableNoiseMG() || unstableNoiseLG());}
  bool noisy() const {return (reallyNoisy() || lowNoiseHG() || lowNoiseMG() || lowNoiseLG());}
				
  bool operator!=(BitWord other) {return m_word != other;}
  bool operator!=(LArBadChannel other) {return m_word != other.packedData();}
  bool operator==(BitWord other) {return m_word == other;}
  bool operator==(LArBadChannel other) {return m_word == other.packedData();}

  LArBadChannel& operator|=(LArBadChannel other) {m_word|=other.m_word; return *this;}

  BitWord packedData() const {return m_word;}

  bool isSC() const {return m_isSC;}
  void setSC() {m_isSC=true;}

 private:

	
  BitWord m_word;
  BitWord& packedDataRef() {return m_word;}	
  bool m_isSC;

  friend class TLArBadChanBitPackingBase<LArBadChannel::LArBadChannelSCEnum>;
  friend class TLArBadChanBitPackingBase<LArBadChannel::LArBadChannelEnum>;
};


#endif
