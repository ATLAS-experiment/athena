/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBSCINTREC_H
#define TBREC_TBSCINTREC_H
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// This algorithm controls event flow and reconstruction and monitoring      //
// tasks.                                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>
#include <vector>

class TBScintillatorRec : public AthAlgorithm
{
 public:
  TBScintillatorRec(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBScintillatorRec();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;


 private:


  ////////////////
  // Properties //
  ////////////////
  Gaudi::Property<std::string> m_SGkey{this, "SGkey", "ScintillatorRawCont", "SG key of the input TBScintillatorRawCont"};
  Gaudi::Property<std::string> m_SGrecordkey{this, "SGrecordkey", "ScintillatorCont", "SG key of the output TBScintillatorCont"};

  //////////////////
  // Calib consts //
  //////////////////

  // Scintillators
  Gaudi::Property<std::vector<std::string>> m_scint_names{this, "SCINTnames", {}, "Scintillator names"};
  Gaudi::Property<std::vector<float>> m_scint_timecalib{this, "SCINTtimecalib", {}, "Scintillator TDC time calibration"};
  Gaudi::Property<std::vector<float>> m_scint_ped{this, "SCINTtimeped", {}, "Scintillator TDC pedestals"};


};

#endif
