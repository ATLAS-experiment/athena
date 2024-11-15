/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TGCCablingDbTool.h"

#include "PathResolver/PathResolver.h" // needed for readASD2PP_DIFF_12FromText()
#include <fstream> // needed for readASD2PP_DIFF_12FromText()

//**********************************************************
//* Author Monica Verducci monica.verducci@cern.ch
//* Author Susumu Oda Susumu.Oda@cern.ch
//*
//* Tool to retrieve the TGC Cabling Map from COOL DB
//* one callback aganist the  class
//* retrieving of tables from DB
//*********************************************************

TGCCablingDbTool::TGCCablingDbTool(const std::string& type,
				   const std::string& name,
				   const IInterface* parent)
  : base_class(type, name, parent),
    m_DataLocation ("keyTGC"),
    m_ASD2PP_DIFF_12(nullptr)
{
  declareProperty("Folder", m_Folder="/TGC/CABLING/MAP_SCHEMA");

  // ASD2PP_diff_12.db is the text database for the TGCcabling12 package
  declareProperty("filename_ASD2PP_DIFF_12", m_filename="ASD2PP_diff_12_ONL.db");
  declareProperty("readASD2PP_DIFF_12FromText", m_readASD2PP_DIFF_12FromText=true);
}

StatusCode TGCCablingDbTool::initialize() { 
  ATH_MSG_INFO("initialize");  
  return StatusCode::SUCCESS;
}

StatusCode TGCCablingDbTool::finalize() {
  ATH_MSG_INFO("finalize");

  // Database is deleted if exists
  delete m_ASD2PP_DIFF_12; 
  m_ASD2PP_DIFF_12 = nullptr;

  return StatusCode::SUCCESS;
}

std::string TGCCablingDbTool::getFolderName() const {
  return m_Folder;
}

std::vector<std::string>* TGCCablingDbTool::giveASD2PP_DIFF_12() {
  ATH_MSG_INFO("giveASD2PP_DIFF_12 (m_ASD2PP_DIFF_12 is " << (!m_ASD2PP_DIFF_12 ? "NULL" : "not NULL") << ")");

  // If no database found, null pointer is returned. 
  if(!m_ASD2PP_DIFF_12) {
    return nullptr;
  }

  // Copy the database
  return new std::vector<std::string> (*m_ASD2PP_DIFF_12);
}

StatusCode TGCCablingDbTool::readASD2PP_DIFF_12FromText() {
  ATH_MSG_INFO("readTGCMap from text");

  // PathResolver finds the full path of the file (default file name is ASD2PP_diff_12.db) 
  std::string location = PathResolver::find_file(m_filename, "DATAPATH");
  if(location=="") {
    ATH_MSG_FATAL("Could not find " << m_filename.c_str());
    return StatusCode::FAILURE; 
  }
  
  // ASD2PP_diff_12.db is opened as inASDToPP
  std::ifstream inASDToPP;
  inASDToPP.open(location.c_str());
  if(inASDToPP.bad()) { 
    ATH_MSG_FATAL("Could not open file " << location.c_str());
    return StatusCode::FAILURE; 
  } 
  
  ATH_MSG_INFO("readTGCMap found file " << location.c_str());
  
  // Old database is deleted if exists
  delete m_ASD2PP_DIFF_12;
  // New database is created
  m_ASD2PP_DIFF_12 = new std::vector<std::string>;

  unsigned int nLines = 0;
  std::string buf; 
  // Copy database into m_ASD2PP_DIFF_12
  while(getline(inASDToPP, buf)){ 
    m_ASD2PP_DIFF_12->push_back(buf);
    ATH_MSG_DEBUG(m_filename.c_str() << " L" << ++nLines << ": " << buf.c_str());
  }

  inASDToPP.close(); 
  return StatusCode::SUCCESS;
}
