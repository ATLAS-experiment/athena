// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file DataModelTestDataCommon/src/xAODTestDecor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Algorithm to test adding decorations to xAOD types.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTDECOR_H
#define DATAMODELTESTDATACOMMON_XAODTESTDECOR_H


#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CVec.h"
#include "xAODEventInfo/EventInfo.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "Gaudi/Property.h"


namespace DMTest {


/**
 * @brief Algorithm to test adding decorations to xAOD data.
 */
class xAODTestDecor
  : public AthReentrantAlgorithm
{
public:
  /**
   * @brief Constructor.
   * @param name The algorithm name.
   * @param svc The service locator.
   */
  xAODTestDecor (const std::string &name, ISvcLocator *pSvcLocator);
  

  /**
   * @brief Algorithm initialization; called at the beginning of the job.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Algorithm event processing.
   */
  virtual StatusCode execute (const EventContext& ctx) const override; 


  /**
   * @brief Algorithm finalization; called at the end of the job.
   */
  virtual StatusCode finalize() override;


private:
  /// Parameter: Prefix for names read from SG.
  Gaudi::Property<std::string> m_readPrefix{this, "ReadPrefix", "", "Prefix for names read from SG"};

  /// Parameter: Name of the decoration.
  Gaudi::Property<std::string> m_decorName{this, "DecorName", "dInt1", "Name of the decoration"};

  /// Parameter: Offset to be applied to decoration.
  Gaudi::Property<int> m_offset{this, "Offset", 0, "Offset to be applied to decoration"};

  /// Flags.
  Gaudi::Property<bool> m_doCVec{this, "DoCVec", true, "Decorate CVec"};
  Gaudi::Property<bool> m_doCInfo{this, "DoCInfo", true, "Decorate CInfo"};
  Gaudi::Property<bool> m_doCTrig{this, "DoCTrig", true, "Decorate CTrig"};

  /// Handles for writing decorations.
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EvtInfo", "EventInfo", "EventInfo name"};
  SG::WriteDecorHandleKey<CVec> m_cvecDecorKey{this, "CVecDecorKey", "", "CVec decoration key"};
  SG::WriteDecorHandleKey<C>    m_cinfoDecorKey{this, "CInfoDecorKey", "", "CInfo decoration key"};
  SG::WriteDecorHandleKey<SG::AuxElement> m_objDecorKey{this, "ObjDecorKey", "", "Object decoration key"};
  SG::WriteDecorHandleKey<CVec> m_ctrigDecorKey{this, "CTrigDecorKey", "", "CTrig decoration key"};
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTDECOR_H
