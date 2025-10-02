/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_TEST_METADATATOOLSTUB_H
#define ATHENASERVICES_TEST_METADATATOOLSTUB_H

#include "AthenaKernel/IMetaDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"


#include <string>
/**the following is in AthenaKernel/​SourceID.h
namespace SG{
  typedef std::string SourceID;
}
**/
class MetaDataToolStub : public extends<AthAlgTool, IMetaDataTool> {
  public:
  using base_class::base_class;
  virtual StatusCode initialize() override {return StatusCode::SUCCESS;}
  virtual StatusCode metaDataStop() override {return StatusCode::SUCCESS;}
  //
  virtual StatusCode 
  beginInputFile(const SG::SourceID& guid) override {
    if (guid == "badGuid") {
      ATH_MSG_ERROR("Failing");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
  //
  virtual StatusCode 
  endInputFile(const SG::SourceID& guid) override {
    if (guid == "badGuid") {
      ATH_MSG_ERROR("Failing");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
};

#endif
