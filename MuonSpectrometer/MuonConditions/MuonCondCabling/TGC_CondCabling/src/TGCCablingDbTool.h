/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDTOOL_TGCCABLINGDBTOOL_H
#define MUONCONDTOOL_TGCCABLINGDBTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonCondInterface/ITGCCablingDbTool.h"
#include "GaudiKernel/IChronoStatSvc.h"

class IIOVSvc;
class IChronoStatSvc;

class TGCCablingDbTool: public extends<AthAlgTool, ITGCCablingDbTool>
{
 public:    
  /** Constructor */
  TGCCablingDbTool(const std::string& type, 
		   const std::string& name, 
		   const IInterface* parent); 
  
  /** Initilize */
  virtual StatusCode initialize() override;
  /** Finalize */
  virtual StatusCode finalize() override;
  /** Method to provide database */
  virtual std::vector<std::string>* giveASD2PP_DIFF_12() override;
  /** Get the folder name */
  virtual std::string getFolderName() const override;
  /** Load parameters from text database */
  virtual StatusCode readASD2PP_DIFF_12FromText() override;

 private: 

  /** Data location */
  std::string m_DataLocation;
  /** Folder name */
  std::string m_Folder;

  /** Database as strings */
  std::vector<std::string>* m_ASD2PP_DIFF_12;

  /** Flag for readASD2PP_DIFF_12FromText() */
  bool m_readASD2PP_DIFF_12FromText;
  /** File name of the text database */ 
  std::string m_filename;
};

#endif // MUONCONDTOOL_TGCCABLINGDBTOOL_H
