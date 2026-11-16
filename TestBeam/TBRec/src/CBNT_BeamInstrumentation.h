/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_CBNT_BEAM_H
#define TBREC_CBNT_BEAM_H

#include "CBNT_TBRecBase.h"
#include <fstream>
#include <string>

class CBNT_BeamInstrumentation : public CBNT_TBRecBase
{
 public:
  CBNT_BeamInstrumentation(const std::string & name, ISvcLocator * pSvcLocator);

  virtual ~CBNT_BeamInstrumentation();

  //standart algorithm methods
  virtual StatusCode CBNT_initialize() override;
  virtual StatusCode CBNT_execute() override;
  virtual StatusCode CBNT_finalize() override;

  virtual StatusCode CBNT_clear() override;
 private:

  Gaudi::Property<bool> m_neverReturnFailure{this, "NeverReturnFailure", false, "Never return FAILURE from execute()"};   // controls when StatusCode::FAILURE can be issued in execute()

  Gaudi::Property<std::string> m_containerKey1{this, "ContainerKey1", "ScintillatorCont", "SG key of TBScintillatorCont"};
  Gaudi::Property<std::string> m_containerKey2{this, "ContainerKey2", "BPCCont", "SG key of TBBPCCont"};
  Gaudi::Property<std::string> m_containerKey3{this, "ContainerKey3", "BPCRawCont", "SG key of TBBPCRawCont"};
  Gaudi::Property<std::string> m_containerKey4{this, "ContainerKey4", "TBPhase", "SG key of TBPhase"};
  Gaudi::Property<std::string> m_containerKey5{this, "ContainerKey5", "TBTrigPat", "SG key of TBTriggerPatternUnit"};
  Gaudi::Property<std::vector<std::string> > m_scint_names{this, "Scintnames", {}, "Names of scintillators"};
  Gaudi::Property<std::vector<std::string> > m_bpc_names{this, "BPCnames", {}, "Names of BPCs"};

  float *m_scint_adc{nullptr}, *m_scint_tdc{nullptr}, *m_bpc_x{nullptr}, *m_bpc_y{nullptr};
  int m_trig{0};
  float m_clock{0};

  // BPC raw data, set to False by default
  Gaudi::Property<bool> m_bpc_raw{this, "BPCRAW", false, "Store BPC raw data"};
  float *m_bpc_up{nullptr}, *m_bpc_down{nullptr}, *m_bpc_left{nullptr}, *m_bpc_right{nullptr}, *m_bpc_adc_hor{nullptr}, *m_bpc_adc_ver{nullptr};

  // Trigger Array, set to False by default 
  Gaudi::Property<bool> m_trig_array{this, "TrigArray", false, "Store trigger array"};
  Gaudi::Property<std::vector<unsigned int> > m_triggflag{this, "TrigPattern", {}, "Trigger pattern bit masks"};

  int m_trig_id{0};
  std::vector<long> *m_trig_sel{nullptr};

  //Private function to build names for ntuple entries. Concatinates strings
  //but omitts characters that are problematic for root like -+:*/
  std::string add_name(const char* base, const std::string& extension);
};

#endif
