/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBXCRYYTABLEREAD_H
#define TBREC_TBXCRYYTABLEREAD_H
// 
// class TBXCryYTableRead 
//
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"

class IToolSvc;
class TBEventInfo;

class ATLAS_NOT_THREAD_SAFE TBXCryYTableRead : public AthAlgorithm {
 public:    
  
  TBXCryYTableRead(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBXCryYTableRead();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
 private: 
  void clear();
  /** Get Xcryo and Ytable from a text file */
  StatusCode getXcryoYtable(float &x, float &y, float &eBeam);
  
  int m_nEvent{0};                  // counter
  int m_nEventRandomTrigger{0};     // counter
  bool m_first{true};               // First event flag

  int m_nRun{0};            /** Run number */
  float m_beamMom{0};       /** Beam momentum */
  float m_xCryo{0};         /** CryoX */
  float m_yTable{0};        /** TableY */

  Gaudi::Property<std::string> m_txtFileWithXY{this, "FileName", "xcryo_ytable.txt", "name of the file with X,y info"};

  TBEventInfo * m_eventinfo{nullptr};
};

#endif
