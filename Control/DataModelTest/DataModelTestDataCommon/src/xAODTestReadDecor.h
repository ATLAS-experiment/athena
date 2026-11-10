// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
// $Id$
/**
 * @file DataModelTestDataCommon/src/xAODTestReadDecor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2017
 * @brief Algorithm to test reading decorations using handles.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTREADDECOR_H
#define DATAMODELTESTDATACOMMON_XAODTESTREADDECOR_H


#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CVec.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "Gaudi/Property.h"


namespace DMTest {


/**
 * @brief Algorithm to test reading decorations using handles.
 */
class xAODTestReadDecor
  : public AthReentrantAlgorithm
{
public:
  /**
   * @brief Constructor.
   * @param name The algorithm name.
   * @param svc The service locator.
   */
  xAODTestReadDecor (const std::string &name, ISvcLocator *pSvcLocator);
  

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

  /// Parameter: Root names of the objects.
  Gaudi::Property<std::string> m_cvecName{this, "CVecName", "cvec", "Root name of the CVec object"};
  Gaudi::Property<std::string> m_cinfoName{this, "CInfoName", "cinfo", "Root name of the CInfo object"};

  /// Handles for reading decorations.
  SG::ReadDecorHandleKey<CVec> m_cvecDecorKey{this, "CVecDecorKey", "", "CVec decoration key"};
  SG::ReadDecorHandleKey<C>    m_cinfoDecorKey{this, "CInfoDecorKey", "", "CInfo decoration key"};
  SG::ReadDecorHandleKey<C>    m_cinfoBaseDecorKey{this, "CInfoBaseDecorKey", "", "CInfo base decoration key"};
  SG::ReadDecorHandleKey<SG::AuxElement> m_objDecorKey{this, "ObjDecorKey", "", "Object decoration key"};
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTREADDECOR_H
