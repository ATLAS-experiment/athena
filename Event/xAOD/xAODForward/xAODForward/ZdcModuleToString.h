/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCMODULETOSTRING_H
#define ZDCMODULETOSTRING_H

#include "xAODForward/ZdcModule.h" //typedef
#include "xAODForward/ZdcModuleContainer.h" //typedef
#include <string>


std::string ZdcModuleToString(const xAOD::ZdcModule& zm) ;
std::string ZdcModuleToString(const xAOD::ZdcModuleContainer& zc) ;

#endif
