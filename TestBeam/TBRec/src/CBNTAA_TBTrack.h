/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_CBNTAA_TBTRACK_H
#define TBREC_CBNTAA_TBTRACK_H

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "CBNT_TBRecBase.h"
#include <fstream>
#include <string>


class CBNTAA_TBTrack : public CBNT_TBRecBase
{
 public:
  CBNTAA_TBTrack(const std::string & name, ISvcLocator * pSvcLocator);

  virtual ~CBNTAA_TBTrack() {};

  virtual StatusCode CBNT_initialize() override;
  virtual StatusCode CBNT_execute() override;
  virtual StatusCode CBNT_finalize() override;

 private:
  // by A. Mineanko
  /** Get Xcryo and Ytable from a text file */
  Gaudi::Property<bool> m_readFileforXcryo{this, "ReadFileforXcryo", true, "Get Xcryo and Ytable from a text file"};
  StatusCode getXcryoYtable(float &x, float &y, float &eBeam);
  int m_nRun{0};

  // Beam track parameters
  float m_beam_coor_x{0};
  float m_beam_coor_y{0};
  float m_beam_chi2_x{0};
  float m_beam_chi2_y{0};
  float m_beam_intercept_x{0};
  float m_beam_intercept_y{0};
  float m_beam_slope_x{0};
  float m_beam_slope_y{0};

  // taken from CBNTAA_BeamInstrumentation
  //Private function to build names for ntuple entries. Concatinates strings
  //but omitts characters that are problematic for root like -+:*/
  std::string add_name(const char* base, const std::string& extension);
};

#endif
