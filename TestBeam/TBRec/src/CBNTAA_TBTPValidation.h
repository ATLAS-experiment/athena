/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_CBNTAA_TBTPVALIDATION_H
#define TBREC_CBNTAA_TBTPVALIDATION_H

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "CBNT_TBRecBase.h"
#include <fstream>
#include <string>
#include <vector>


/** @class CBNTAA_TBTPValidation CBNTAA_TBTPValidation.h "TBRec/CBNTAA_TBTPValidation.h"
ntpl-dumper for all persistified TB classes.
@author Iftach Sadeh
*/

class CBNTAA_TBTPValidation : public CBNT_TBRecBase
{
 public:
  CBNTAA_TBTPValidation(const std::string & name, ISvcLocator * pSvcLocator);

  ~CBNTAA_TBTPValidation();

  //standart algorithm methods
  virtual StatusCode CBNT_initialize();
  virtual StatusCode CBNT_execute();
  virtual StatusCode CBNT_finalize();

 private:

  Gaudi::Property<bool> m_neverReturnFailure{this, "NeverReturnFailure", false, "Never return FAILURE from execute()"};   // controls when StatusCode::FAILURE can be issued in execute()

  // TBADCRawCont
	std::vector<unsigned int> *m_adc{nullptr};
  std::vector< std::string > *m_tbDetectorName_TBADCRawCont{nullptr};
  std::vector< bool > *m_overflow_TBADCRawCont{nullptr};
  Gaudi::Property<std::string> m_containerKey1{this, "ContainerKey1", "ADCRawCont", "SG key of TBADCRawCont"};
  // TBTDCRawCont
  std::vector<unsigned int> *m_tdc_raw{nullptr};
	std::vector<bool> *m_underThreshold_raw{nullptr};
  std::vector< std::string > *m_tbDetectorName_TBTDCRawCont{nullptr};
  std::vector< bool > *m_overflow_TBTDCRawCont{nullptr};
	Gaudi::Property<std::string> m_containerKey2{this, "ContainerKey2", "TDCRawCont", "SG key of TBTDCRawCont"};
  // TBPhase
  std::vector<float> *m_phase{nullptr};
  std::vector<short> *m_phaseind{nullptr};
  std::vector<float> *m_dTtoWAC{nullptr};
	Gaudi::Property<std::string> m_containerKey3{this, "ContainerKey3", "TBPhase", "SG key of TBPhase"};
  // TBEventInfo
  std::vector<int> *m_ev_number{nullptr};
  std::vector<int> *m_ev_clock{nullptr};
  std::vector<int> *m_ev_type{nullptr};
  std::vector<unsigned int> *m_run_num{nullptr};
  std::vector<float> *m_beam_moment{nullptr};
  std::vector<std::string> *m_beam_part{nullptr};
  std::vector<float> *m_cryoX{nullptr};
  std::vector<float> *m_cryoAngle{nullptr};
  std::vector<float> *m_tableY{nullptr};
	Gaudi::Property<std::string> m_containerKey4{this, "m_containerKey4", "TBEventInfo", "SG key of TBEventInfo"};
  // TBBPCCont
  typedef double signal_type_TBBPCCont;
  std::vector< signal_type_TBBPCCont > *m_xPos{nullptr},*m_yPos{nullptr};
  std::vector< signal_type_TBBPCCont > *m_xErr{nullptr},*m_yErr{nullptr};
  std::vector< signal_type_TBBPCCont > *m_xPulse{nullptr}, *m_yPulse{nullptr};
  std::vector< short > *m_hitnumber{nullptr};
  std::vector< bool > *m_xPosOverflow{nullptr},   *m_yPosOverflow{nullptr};
  std::vector< bool > *m_xPulseOverflow{nullptr}, *m_yPulseOverflow{nullptr};
  std::vector< bool > *m_overflowSetFlag{nullptr};
  std::vector< std::string > *m_tbDetectorName_TBBPCCont{nullptr};
  std::vector< bool > *m_overflow_TBBPCCont{nullptr};
	Gaudi::Property<std::string> m_containerKey5{this, "m_containerKey5", "BPCCont", "SG key of TBBPCCont"};
  // TBLArDigitContainer
  std::vector<unsigned int> *m_channelID{nullptr};
  std::vector<unsigned char> *m_gain{nullptr};
  std::vector<unsigned short> *m_nSamples{nullptr};
  std::vector<unsigned short> *m_samples{nullptr};
	Gaudi::Property<std::string> m_containerKey6{this, "m_containerKey6", "FREE", "SG key of TBLArDigitContainer"};
  // TBScintillatorCont
  std::vector< float > *m_signal_scint{nullptr}, *m_time_signal_scint{nullptr};
  std::vector< bool > *m_signal_overflow_scint{nullptr}, *m_time_overflow_scint{nullptr};
  std::vector< std::string > *m_tbDetectorName_TBScintillatorCont{nullptr};
  std::vector< bool > *m_overflow_TBScintillatorCont{nullptr};
	Gaudi::Property<std::string> m_containerKey7{this, "m_containerKey7", "ScintillatorCont", "SG key of TBScintillatorCont"};
  // TBTailCatcher
  std::vector< double > *m_signals{nullptr};  
  std::vector< float > *m_signal_tCatch{nullptr}, *m_time_signal_tCatch{nullptr};
  std::vector< bool > *m_signal_overflow_tCatch{nullptr}, *m_time_overflow_tCatch{nullptr}; 
  std::vector< std::string > *m_tbDetectorName_TBTailCatcher{nullptr};
  std::vector< bool > *m_overflow_TBTailCatcher{nullptr};
	Gaudi::Property<std::string> m_containerKey8{this, "m_containerKey8", "TailCatcher", "SG key of TBTailCatcher"};
  // TBTDC
  std::vector< int > *m_tdc_TBTDC{nullptr};
  std::vector< int > *m_tdcmin_TBTDC{nullptr};
  std::vector< float > *m_scale_TBTDC{nullptr};
  std::vector< int > *m_phase_TBTDC{nullptr};
	Gaudi::Property<std::string> m_containerKey9{this, "m_containerKey9", "TBTDC", "SG key of TBTDC"};
  // TBTrack
  std::vector< int > *m_hitNumberU{nullptr}, *m_hitNumberV{nullptr};
  std::vector<double > *m_residualu{nullptr}, *m_residualv{nullptr};
  std::vector<double > *m_chi2{nullptr}, *m_chi2u{nullptr}, *m_chi2v{nullptr};
  std::vector<double > *m_angle{nullptr};
  std::vector<double > *m_uslope{nullptr},*m_vslope{nullptr};
  std::vector<double > *m_uintercept{nullptr}, *m_vintercept{nullptr};
  std::vector< double > *m_cryou{nullptr},*m_cryov{nullptr},*m_cryow{nullptr};    
	Gaudi::Property<std::string> m_containerKey10{this, "m_containerKey10", "Track", "SG key of TBTrack"};
  // TBTriggerPatternUnit
  std::vector< unsigned int > *m_triggerWord{nullptr};
  std::vector<bool> *m_triggers{nullptr};
	Gaudi::Property<std::string> m_containerKey11{this, "m_containerKey11", "TBTrigPat", "SG key of TBTriggerPatternUnit"};
  // TBMWPCCont
  std::vector < std::vector<float> > *m_cPos{nullptr};
  std::vector < std::vector<float> > *m_cErr{nullptr};
  std::vector < bool > *m_isX{nullptr};
  std::vector < std::vector<float> > *m_clusterSize_c{nullptr}; 
  std::vector< std::string > *m_tbDetectorName_TBMWPCCont{nullptr};
  std::vector< bool > *m_overflow_TBMWPCCont{nullptr};
	Gaudi::Property<std::string> m_containerKey12{this, "m_containerKey12", "MWPCCont", "SG key of TBMWPCCont"};
  // {vector of vector of unsigned} instead of {vector of vector of bool}
  std::vector < std::vector<unsigned> > *m_cPosOverflow{nullptr};


  //Private function to build names for ntuple entries. Concatinates strings
  //but omitts characters that are problematic for root like -+:*/
  std::string add_name(const char* base, const std::string& extension);

  StatusCode CBNT_clear();
};

#endif
