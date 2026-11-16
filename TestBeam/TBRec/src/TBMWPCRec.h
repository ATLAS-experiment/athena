/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBMWPCREC
#define TBREC_TBMWPCREC
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// MWPC Reconstruction algorithm. Based on code for 2002 TestBeam            //
// (see LArCnv/LArHECTBCnv/hectb/Alg_mwpc.cxx)                               //
//  author : Pierre-Antoine Delsart                                          //
///////////////////////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>
#include <vector>
#include "TBEvent/TBMWPCCont.h"
#include "TBEvent/TBMWPCRawCont.h"


class TBMWPCRec : public AthAlgorithm
{
 public:

  TBMWPCRec(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBMWPCRec();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;


 private:


  ////////////////
  // Properties //
  ////////////////

  Gaudi::Property<std::string> m_SGkey{this, "SGkey", "MWPCRawCont", "SG key of the input TBMWPCRawCont"};
  Gaudi::Property<std::string> m_SGrecordkey{this, "SGrecordkey", "MWPCCont", "SG key of the output TBMWPCCont"};

  //////////////////
  // Calib consts //
  //////////////////  
  
  std::string m_mwpc_names[8];

  Gaudi::Property<std::vector<float>> m_mwpc_wirestep{this, "MWPCwirestep", {}, "MWPC wire step"};
  Gaudi::Property<std::vector<float>> m_mwpc_halfsize{this, "MWPChalfsize", {}, "MWPC half size"};

  Gaudi::Property<std::vector<int>> m_mwpc_invX{this, "MWPCinvX", {}, "MWPC X inversion"};
  Gaudi::Property<std::vector<int>> m_mwpc_invY{this, "MWPCinvY", {}, "MWPC Y inversion"};

};

#endif
