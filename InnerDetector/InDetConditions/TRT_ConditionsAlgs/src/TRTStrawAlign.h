/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTCONDITIONSALGS_TRTSTRAWALIGN_H
#define TRTCONDITIONSALGS_TRTSTRAWALIGN_H

/** @file TRTStrawAlign.h
 * @brief Algorithm to make individual Straw Alignment
 * @author John Alison <johnda@hep.upenn.edu> and Peter Hansen <phansen@nbi.dk>
 **/

//
#include <vector>
#include <string>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TRT_ConditionsServices/ITRT_StrawAlignDbSvc.h"
#include "TRT_ConditionsServices/ITRT_AlignDbSvc.h"

namespace InDetDD{
  class TRT_DetectorManager;
}
class TRT_ID;

class TRTStrawAlign:public AthAlgorithm {
public:
  TRTStrawAlign(const std::string& name, ISvcLocator* pSvcLocator);
  ~TRTStrawAlign(void);

  virtual StatusCode  initialize(void) override;
  virtual StatusCode  execute(void) override;
  virtual StatusCode  finalize(void) override;

private:
  ServiceHandle<ITRT_StrawAlignDbSvc> p_caldbtool{this,"DbTool","TRT_StrawAlignDbSvc"}; //!< db tool
  ServiceHandle<ITRT_AlignDbSvc> p_aligndbtool{this,"AlignDbTool","TRT_AlignDbSvc"}; //!< db tool
  bool m_doStrawAlign{};
  bool m_doModuleAlign{};
  const InDetDD::TRT_DetectorManager* m_trtman{nullptr}; //!< detector manager
  const TRT_ID* m_trt{nullptr};                          //!< trt id helper
  bool m_setup{false};                                   //!< true for first event

  // algorithm parameters to be set in the jobOptions
  bool m_doWriteToPOOL{false};          //!< stream structures to output 
  bool m_doRegIOV{false};       //!< register structures in the IOV service
  std::string m_inputModuleAlignmentTextFile;   //!< text file to read module alignment data
  std::string m_inputStrawAlignmentTextFile;   //!< text file to read straw alignment data 
  std::string m_outputModuleAlignmentTextFile;     //!< output module level text file
  std::string m_outputStrawAlignmentTextFile;     //!< output module level text file  
  std::string m_outputPOOLFile;  //!< POOL file to write
  std::string m_moduleAlignTag;       //!< IOV version tag for module alignment
  std::string m_stawAlignTag;       //!< IOV version tag for straw alignment
  int m_runRangeBegin;              //!< IOV range
  int m_runRangeEnd;              //!< IOV range
  int m_eventRangeBegin;              //!< IOV range
  int m_eventRangeEnd;              //!< IOV range

};


#endif // TRTCONDITIONSALGS_TRTSTRAWALIGN_H
