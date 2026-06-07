/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_TESTSENDER_H
#define EFTRACKINGDATATRANSFER_TESTSENDER_H

// Framework includes
#include <grpcpp/grpcpp.h>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "OffloadToken.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

#include "TestMessages.grpc.pb.h"

// STL includes
#include <string>
/**
 * @class TestSender
 * @brief
 **/

class TestSender : public AthReentrantAlgorithm {
 public:
  TestSender(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TestSender() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode finalize() override;

 private:
  Gaudi::Property<int32_t> m_valueToSend{
      this, "ValueToSend", 0,
      "Numbers which will be used to send the response to"};
  Gaudi::Property<int32_t> m_sizeToSend{this, "SizeToSend", 0,
                                        "Size of of numbers array"};
  SG::WriteHandleKey<OffloadToken> m_outputKey{
      this, "OutputKey", "Data", "Name of the produced offlaod token"};

  // this code should be moved to an athena service
  std::unique_ptr<TestService::Stub> m_stub;
};

#endif  // EFTRACKINGDATATRANSFER_TESTSENDER_H
