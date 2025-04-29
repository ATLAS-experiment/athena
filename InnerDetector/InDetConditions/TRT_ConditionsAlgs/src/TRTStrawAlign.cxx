/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file TRTStrawAlign.cxx
 * @brief Algorithm to implement individual straw alignment 
 * @author John Alison <johnda@hep.upenn.edu> and Peter Hansen <phansen@nbi.dk>
 **/

//
#include <fstream>
#include <iostream>
#include <string>
#include "TRTStrawAlign.h"

#include "TRT_ReadoutGeometry/TRT_DetectorManager.h" 
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "InDetIdentifier/TRT_ID.h"

#include "TRT_ConditionsData/StrawDxContainer.h"

TRTStrawAlign::TRTStrawAlign(const std::string& name, ISvcLocator* pSvcLocator)
  :AthAlgorithm   (name, pSvcLocator),
   m_inputModuleAlignmentTextFile(""),
   m_inputStrawAlignmentTextFile(""),
   m_outputModuleAlignmentTextFile(""),
   m_outputStrawAlignmentTextFile(""),
   m_outputPOOLFile(""),
   m_moduleAlignTag("TRTAlign_00"),
   m_stawAlignTag("TrtCalibDx_00"),
   m_runRangeBegin(0),
   m_runRangeEnd(9999999),
   m_eventRangeBegin(0),
   m_eventRangeEnd(9999999)

{
  declareProperty("Write",m_doWriteToPOOL);
  declareProperty("RegisterIOV",m_doRegIOV);
  declareProperty("InputFile",m_inputModuleAlignmentTextFile);
  declareProperty("InputStrawAlignmentFile",m_inputStrawAlignmentTextFile);
  declareProperty("PoolOutputFile",m_outputPOOLFile);
  declareProperty("TextOutputFile",m_outputModuleAlignmentTextFile);
  declareProperty("StrawAlignmentTextOutputFile",m_outputStrawAlignmentTextFile);
  declareProperty("VersionTag",m_moduleAlignTag);
  declareProperty("VersionTagStrawAlign",m_stawAlignTag);
  declareProperty("ValidRun1",m_runRangeBegin);
  declareProperty("ValidRun2",m_runRangeEnd);
  declareProperty("ValidEvent1",m_eventRangeBegin);
  declareProperty("ValidEvent2",m_eventRangeEnd);
  declareProperty("DoStrawAlign",m_doStrawAlign);
  declareProperty("DoModuleAlign",m_doModuleAlign);
}


TRTStrawAlign::~TRTStrawAlign(void)
= default;

StatusCode TRTStrawAlign::initialize() {

  ATH_MSG_DEBUG(" in initialize() ");

  //
  // Get TRT manager and ID helper
  ATH_CHECK(detStore()->retrieve(m_trtman,"TRT"));
  ATH_CHECK(detStore()->retrieve(m_trt,"TRT_ID"));
  ATH_MSG_DEBUG("TRT manager and helper found ");

  //get Database manager tools
  if (m_doStrawAlign) {
    ATH_CHECK(p_caldbtool.retrieve());
    ATH_MSG_DEBUG(" TRTStrawAlignDbTool found ");
    if (m_doWriteToPOOL) {
      ATH_MSG_INFO("Straw alignment will be written to POOL file " << m_outputPOOLFile);
    }
    if (m_doRegIOV) {
      ATH_MSG_INFO("Straw alignment will be registered with IOV");
      ATH_MSG_INFO(" run range: " << m_runRangeBegin << " to " << m_runRangeEnd);
      ATH_MSG_INFO(" version tag: " << m_stawAlignTag);
    }
    if (!m_inputStrawAlignmentTextFile.empty()) {
      ATH_MSG_INFO("Straw Alignment will read from text file " << m_inputStrawAlignmentTextFile);
    }
    if (!m_outputStrawAlignmentTextFile.empty()) {
      ATH_MSG_INFO("Straw Alignment will be written on text file " << m_outputStrawAlignmentTextFile);
    }
  }
  
  if (m_doModuleAlign) {
    ATH_CHECK(p_aligndbtool.retrieve());
    ATH_MSG_DEBUG(" TRTAlignDbTool found ");

    if (m_doWriteToPOOL) {
      ATH_MSG_INFO("Module alignment will be written to POOL file " << m_outputPOOLFile);
    }
    if (m_doRegIOV) {
      ATH_MSG_INFO("Module alignment will be registered with IOV");
      ATH_MSG_INFO(" run range: " << m_runRangeBegin << " to " << m_runRangeEnd);
      ATH_MSG_INFO(" version tag: " << m_moduleAlignTag);
    }
    if (!m_inputModuleAlignmentTextFile.empty()) {
      ATH_MSG_INFO("Module Alignment will read from text file " << m_inputModuleAlignmentTextFile);
    }
    if (!m_outputModuleAlignmentTextFile.empty()) {
      ATH_MSG_INFO("Module Alignment will be written on text file " << m_outputModuleAlignmentTextFile);
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode TRTStrawAlign::execute() {

  StatusCode sc=StatusCode::SUCCESS;
  //
  // at first event:
  if (!m_setup) {
    m_setup=true;

    if (m_doStrawAlign) {
      // read alignment constants from text file
      if (!m_inputStrawAlignmentTextFile.empty()) {
	sc=p_caldbtool->readTextFile(m_inputStrawAlignmentTextFile);
	if(sc!=StatusCode::SUCCESS) {
          ATH_MSG_ERROR(" Could not read input text file ");
          return StatusCode::FAILURE;
	}
      }

      // write alignment constants to text file
      if (!m_outputStrawAlignmentTextFile.empty()) {
	sc=p_caldbtool->writeTextFile(m_outputStrawAlignmentTextFile);
	if(sc!=StatusCode::SUCCESS) {
          ATH_MSG_ERROR(" Could not write output text file ");
          return StatusCode::FAILURE;
	}
      }

      if(m_doWriteToPOOL) {
	if( StatusCode::SUCCESS != p_caldbtool->streamOutObjects()) {
	  ATH_MSG_ERROR(" Could not stream Straw Alignment objects to " << m_outputPOOLFile);
	  return StatusCode::FAILURE;
	}
      }
      if(m_doRegIOV) {
	if( StatusCode::SUCCESS != p_caldbtool->registerObjects(m_stawAlignTag,m_runRangeBegin,m_eventRangeBegin,m_runRangeEnd,m_eventRangeEnd) ) {
	  ATH_MSG_ERROR(" Could not register Straw Alignment objects ");
	  return StatusCode::FAILURE;
	}
      }
    }

    if (m_doModuleAlign) {
      // read alignment constants from text file
      if (!m_inputModuleAlignmentTextFile.empty()) {
	sc=p_aligndbtool->readAlignTextFile(m_inputModuleAlignmentTextFile);
	if(sc!=StatusCode::SUCCESS) {
          ATH_MSG_ERROR(" Could not read input text file ");
          return StatusCode::FAILURE;
	}
      }

      // write alignment constants to text file
      if (!m_outputModuleAlignmentTextFile.empty()) {
	sc=p_aligndbtool->writeAlignTextFile(m_outputModuleAlignmentTextFile);
	if(sc!=StatusCode::SUCCESS) {
          ATH_MSG_ERROR( " Could not write output text file ");
	  return StatusCode::FAILURE;
	}
      }

      if(m_doWriteToPOOL) {
	if( StatusCode::SUCCESS != p_aligndbtool->streamOutAlignObjects()) {
	  ATH_MSG_ERROR(" Could not stream Module Alignment objects to " << m_outputPOOLFile);
	  return StatusCode::FAILURE;
	}
      }
      if(m_doRegIOV) {
	if( StatusCode::SUCCESS != p_aligndbtool->registerAlignObjects(m_moduleAlignTag
								       ,m_runRangeBegin
								       ,m_eventRangeBegin
								       ,m_runRangeEnd
								       ,m_eventRangeEnd) ) {
	  
	  ATH_MSG_ERROR(" Could not register Module Alignment objects ");
	  return StatusCode::FAILURE;
	}
      }
    }

  }

  return StatusCode::SUCCESS;
}


StatusCode TRTStrawAlign::finalize() {

  //
  // check a few straw positions
  // NB: alignment corrections will not be seen because elelemnts are taken from the Det Manager
  for(int strlay=0;strlay<3;strlay++) {
    for(int str=7;str<10;str++) {
      Identifier id=m_trt->layer_id(-1,1,0,strlay);
      const InDetDD::TRT_BaseElement* element = m_trtman->getElement(id);
      float y = element->strawCenter(str).x();
      float x = element->strawCenter(str).y();
      float z = element->strawCenter(str).z();
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)
	    << "bec -1 layer 0 sector 1 plane "
            << strlay << " straw " << str << endmsg;
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)
	    << " x " << x << " y " << y << " z " << z << endmsg;
      id=m_trt->layer_id(1,1,0,strlay);
      element = m_trtman->getElement(id);
      y = element->strawCenter(str).x();
      x = element->strawCenter(str).y();
      z = element->strawCenter(str).z();
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)
	    << "bec 1 layer 0 sector 1 plane "
            << strlay << " straw " << str << endmsg;
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)
	    << " x " << x << " y " << y << " z " << z << endmsg;
    }
  }

  return StatusCode::SUCCESS;
}

