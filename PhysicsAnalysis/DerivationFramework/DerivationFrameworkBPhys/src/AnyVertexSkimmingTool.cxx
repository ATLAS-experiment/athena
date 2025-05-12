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
    if(m_useHandles) for(const auto& str : m_containerNames) m_keyArray.emplace_back(str);
    ATH_CHECK(m_keyArray.initialize(m_useHandles));
    return StatusCode::SUCCESS;
}

bool AnyVertexSkimmingTool::eventPassesFilter() const{

    if(m_useHandles){
       bool pass = false;
       for(auto key : m_keyArray){
          ATH_MSG_DEBUG("Key Checking: " << key.key());
          SG::ReadHandle<xAOD::VertexContainer> read(key);
          if(!read.isValid()){
            std::string error("AnyVertexSkimmingTool - Failed to retrieve : ");
            error += key.key();
            throw  std::runtime_error(error);
          }
          if(not read->empty()) pass |= true;
       }
       return pass;
    }else{
       bool pass = false;
       for(const std::string& name : m_containerNames){
           ATH_MSG_DEBUG("Checking: " << name);
           const xAOD::VertexContainer* container = nullptr;
           if(evtStore()->retrieve(container, name).isFailure()){
               std::string error("AnyVertexSkimmingTool - Failed to retrieve : ");
               error += name;
               throw  std::runtime_error(error);
           }
           if(not container->empty()) pass |= true;
           //Not breaking from loop early to ensure all containers are written - avoids production bugs
       }
       return pass;
    }
}

}