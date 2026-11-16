/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBBPCREC_H
#define TBREC_TBBPCREC_H
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// This algorithm controls event flow and reconstruction and monitoring      //
// tasks.                                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>
#include <vector>
#include "TBEvent/TBBPCCont.h"
#include "TBEvent/TBBPCRawCont.h"


class TBBPCRec : public AthAlgorithm
{



 public:

  TBBPCRec(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBBPCRec();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

  StatusCode getnewcalib();

 private:


  ////////////////
  // Properties //
  ////////////////

  Gaudi::Property<std::string> m_SGkey{this, "SGkey", "BPCRawCont", "input BPC raw container key"};
  Gaudi::Property<std::string> m_SGrecordkey{this, "SGrecordkey", "BPCCont", "output BPC container key"};
  Gaudi::Property<std::string> m_calib_filename{this, "CalibFileName", "H6BPCCalib.txt", "BPC calibration constants file"};

  unsigned int m_runnumber{0};

  //////////////////
  // Calib consts //
  //////////////////  
  
  Gaudi::Property<std::vector<std::string>> m_bpc_names{this, "BPCnames", {}, "BPC names"};

  Gaudi::Property<std::vector<float>> m_bpc_calibX{this, "BPCcalibX", {}, "BPC X calibration"};
  Gaudi::Property<std::vector<float>> m_bpc_calibY{this, "BPCcalibY", {}, "BPC Y calibration"};
  Gaudi::Property<std::vector<float>> m_bpc_leftright{this, "BPCleftright", {}, "BPC left/right coefficient"};
  Gaudi::Property<std::vector<float>> m_bpc_updown{this, "BPCupdown", {}, "BPC up/down coefficient"};
  Gaudi::Property<std::vector<float>> m_bpc_udoffset{this, "BPCudoffset", {}, "BPC up/down offset"};
  Gaudi::Property<std::vector<float>> m_bpc_lroffset{this, "BPClroffset", {}, "BPC left/right offset"};

  Gaudi::Property<std::vector<float>> m_bpc_invX{this, "BPCinvX", {}, "BPC X inversion"};
  Gaudi::Property<std::vector<float>> m_bpc_invY{this, "BPCinvY", {}, "BPC Y inversion"};

  Gaudi::Property<std::vector<float>> m_tdccutmin{this, "TDCmin", {}, "minimum TDC cut"};
  Gaudi::Property<std::vector<float>> m_tdccutmax{this, "TDCmax", {}, "maximum TDC cut"};
  Gaudi::Property<std::vector<float>> m_tdccutlr{this, "TDClrcut", {}, "TDC left+right sum cut"};
  Gaudi::Property<std::vector<float>> m_tdccutud{this, "TDCudcut", {}, "TDC up+down sum cut"};

  Gaudi::Property<std::vector<float>> m_rotation{this, "BPCRotation", {}, "BPC rotation angle"};

};

#endif
