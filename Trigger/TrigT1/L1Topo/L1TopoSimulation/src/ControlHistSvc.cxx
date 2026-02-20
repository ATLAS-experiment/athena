/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 *   */



#include "./ControlHistSvc.h"
#include "GaudiKernel/ITHistSvc.h"

#include "AthenaL1TopoHistSvc.h"
#include "L1TopoInterfaces/IL1TopoHistSvc.h"

using namespace LVL1;


ControlHistSvc::ControlHistSvc(const std::string& type, const std::string& name, 
                                       const IInterface* parent) :
  AthAlgTool(type,name,parent),
   m_histSvc("THistSvc", name){};


ControlHistSvc::~ControlHistSvc()
{}


StatusCode ControlHistSvc::initialize(){

  ATH_MSG_INFO("Initialising ControlHistSvc...");

  CHECK(m_histSvc.retrieve());

  return StatusCode::SUCCESS;

}


//Function for controlling the hist service
// 
StatusCode ControlHistSvc::SetHistSvc(const std::unique_ptr<TCS::TopoSteering> &topoSteering, std::string histBaseDir){


  std::shared_ptr<IL1TopoHistSvc> topoHistSvc = std::shared_ptr<IL1TopoHistSvc>( new AthenaL1TopoHistSvc(m_histSvc) );
    
  topoHistSvc->setBaseDir("/EXPERT/"+ histBaseDir);
  topoSteering->setHistSvc(std::move(topoHistSvc));

  return StatusCode::SUCCESS;
}


