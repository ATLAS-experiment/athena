/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENEVENTSSELECTORTOOL_H
#define EVENEVENTSSELECTORTOOL_H

/** @file EvenEventsSelectorTool.h
 *  @brief This file contains the class definition for the EvenEventsSelectorTool class.
 **/

#include "AthenaKernel/IAthenaSelectorTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>

/** @class EvenEventsSelectorTool
 *  @brief This class provides an example for reading with a ISelectorTool to veto events on AttributeList.
 **/
class EvenEventsSelectorTool : public extends<AthAlgTool, IAthenaSelectorTool> {
public: // Constructor and Destructor
   /// Standard Tool Constructor
   using base_class::base_class;
   /// Destructor
   virtual ~EvenEventsSelectorTool();

public:
   /// IAthenaSelectorTool Interface method implementations:
   virtual StatusCode postInitialize() override { return StatusCode::SUCCESS; }
   virtual StatusCode preNext() const override { return StatusCode::SUCCESS; }
   virtual StatusCode postNext() const override;
   virtual StatusCode preFinalize() override { return StatusCode::SUCCESS; }
};

#endif
