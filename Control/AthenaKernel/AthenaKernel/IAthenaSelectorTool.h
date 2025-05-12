/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IATHENASELECTORTOOL_H
#define ATHENAKERNEL_IATHENASELECTORTOOL_H

/** @file IAthenaSelectorTool.h
 *  @brief This file contains the class definition for the IAthenaSelectorTool class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

// Gaudi
#include "GaudiKernel/IAlgTool.h"

/** @class IAthenaSelectorTool
 *  @brief This class provides the interface for AthenaSelectorTool classes used by AthenaEventSelector.
 **/
class IAthenaSelectorTool : public extend_interfaces<IAlgTool> {

public:    
   /// Gaudi interface
   DeclareInterfaceID(IAthenaSelectorTool, 1, 0);

   /// Called at the end of initialize
   virtual StatusCode postInitialize() = 0;
   /// Called at the beginning of next
   virtual StatusCode preNext() const = 0;
   /// Called at the end of next
   virtual StatusCode postNext() const = 0;
   /// Called at the beginning of finalize
   virtual StatusCode preFinalize() = 0;
};

#endif
