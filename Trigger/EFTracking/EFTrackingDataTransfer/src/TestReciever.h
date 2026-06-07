/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_TESTRECIEVER_H
#define EFTRACKINGDATATRANSFER_TESTRECIEVER_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// STL includes
#include <string>
#include "OffloadToken.h"
/**
 * @class TestReciever
 * @brief
 **/
class TestReciever : public AthReentrantAlgorithm {
 public:
  TestReciever(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TestReciever() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode finalize() override;

 private:
  SG::ReadHandleKey<OffloadToken> m_inputKey{
      this, "InputKey", {}, "Name of the consumed offlaod token"};
};

#endif  // EFTRACKINGDATATRANSFER_TESTRECIEVER_H
