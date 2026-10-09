/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef OUTPUTSTREAMATHENAPOOL_SORTEDEVENTTAGWRITER_H
#define OUTPUTSTREAMATHENAPOOL_SORTEDEVENTTAGWRITER_H

#include <AthenaBaseComps/AthAlgorithm.h>
#include <AthenaPoolCnvSvc/IAthenaPoolCnvSvc.h>
#include <GaudiKernel/ServiceHandle.h>
#include <PersistentDataModel/AthenaAttributeList.h>
#include <StoreGate/ReadHandleKey.h>

#include <memory>
#include <string>
#include <vector>

class SortedEventTagWriter : public AthAlgorithm
{
public:
  using AthAlgorithm::AthAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) override;
  StatusCode finalize() override;

private:
  SG::ReadHandleKey<AthenaAttributeList> m_inputAttList{
    this, "InputList", "Input", "Input Athena attribute list ReadHandleKey"};
  Gaudi::Property<std::string> m_outputFile{
    this, "OutputFile", "event-tags.pool.root", "POOL file for sorted event tags"};
  Gaudi::Property<std::string> m_sortAttribute{
    this, "SortAttribute", "LumiBlockN",
    "Numeric attribute of the input list used to sort the event tags"};
  Gaudi::Property<std::string> m_tokenAttribute{
    this, "TokenAttribute", "Token",
    "Name of the token attribute in the input list, and of the output token column"};
  Gaudi::Property<std::vector<std::string>> m_skipAttributes{
    this, "SkipAttributes", {"eventRef"},
    "Input attributes that are not written to the output"};
  ServiceHandle<IAthenaPoolCnvSvc> m_athenaPoolCnvSvc{
    this, "ConversionService", "AthenaPoolCnvSvc", "AthenaPool conversion service"};

  std::vector<std::unique_ptr<AthenaAttributeList>> m_rows;
};

#endif