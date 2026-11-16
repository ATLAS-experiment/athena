/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBTAILCATCHERREC_H
#define TBREC_TBTAILCATCHERREC_H
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// This algorithm controls event flow and reconstruction and monitoring      //
// tasks.                                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>
#include <vector>
#include "TBEvent/TBTailCatcher.h"
#include "TBEvent/TBTailCatcherRaw.h"


class TBTailCatcherRec : public AthAlgorithm
{
 public:

  TBTailCatcherRec(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBTailCatcherRec();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;


 private:
  ////////////////
  // Properties //
  ////////////////

  Gaudi::Property<std::string> m_SGkey{this, "SGkey", "TailCatcherRaw", "SG key of the TBTailCatcherRaw object"};

  //////////////////
  // Calib consts //
  //////////////////

  Gaudi::Property<std::vector<float>> m_tailcatch_calib{this, "TailCatcherCalib", {}, "TailCatcher calibration factors"};
  Gaudi::Property<std::vector<float>> m_tailcatch_ped{this, "TailCatcherPed", {}, "TailCatcher pedestals"};

};

#endif
