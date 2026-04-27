/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AnyVertexSkimmingTool.h"
#include "xAODTracking/VertexContainer.h"
#include "Gaudi/Property.h"
namespace DerivationFramework {


AnyVertexSkimmingTool::AnyVertexSkimmingTool(const std::string& t, const std::string& n, const IInterface* p)  : base_class(t,n,p)
{}

AnyVertexSkimmingTool::~AnyVertexSkimmingTool() = default;

StatusCode AnyVertexSkimmingTool::initialize(){
    ATH_CHECK(m_keyArray.initialize());
    return StatusCode::SUCCESS;
}

bool AnyVertexSkimmingTool::eventPassesFilter() const{
       const EventContext& ctx = Gaudi::Hive::currentContext();
       bool pass = false;
       for(auto key : m_keyArray){
          ATH_MSG_DEBUG("Key Checking: " << key.key());
          SG::ReadHandle<xAOD::VertexContainer> read(key, ctx);
          if(!read.isValid()){
            std::string error("AnyVertexSkimmingTool - Failed to retrieve : ");
            error += key.key();
            throw  std::runtime_error(error);
          }
          if(not read->empty()) pass |= true;
       }
       return pass;
}

}