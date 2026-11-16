/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_CBNTAA_TBADCRAW_H
#define TBREC_CBNTAA_TBADCRAW_H

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "CBNT_TBRecBase.h"
#include <fstream>
#include <string>
#include <vector>


/** @class CBNTAA_TBADCRaw CBNTAA_TBADCRaw.h "TBRec/CBNTAA_TBADCRaw.h"
ntpl-dumper for TBEvent/TBADCRaw.h.
@author Iftach Sadeh
*/

class CBNTAA_TBADCRaw : public CBNT_TBRecBase
{
 public:
  CBNTAA_TBADCRaw(const std::string & name, ISvcLocator * pSvcLocator);

  virtual ~CBNTAA_TBADCRaw();

  //standart algorithm methods
  virtual StatusCode CBNT_initialize() override;
  virtual StatusCode CBNT_execute() override;
  virtual StatusCode CBNT_finalize() override;

 private:

  Gaudi::Property<bool> m_neverReturnFailure{this, "NeverReturnFailure", false, "Never return FAILURE from execute()"};   // controls when StatusCode::FAILURE can be issued in execute()

  std::vector<unsigned int> *m_adc{nullptr};
  Gaudi::Property<std::string> m_containerKey1{this, "ContainerKey1", "ADCRawCont", "SG key of TBADCRawCont"};

  //Private function to build names for ntuple entries. Concatinates strings
  //but omits characters that are problematic for root like -+:*/
  std::string add_name(const char* base, const std::string& extension);

  virtual StatusCode CBNT_clear() override;
};

#endif
