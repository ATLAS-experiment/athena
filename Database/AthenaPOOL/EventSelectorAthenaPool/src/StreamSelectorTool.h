/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STREAMSELECTORTOOL_H
#define STREAMSELECTORTOOL_H

/** @file StreamSelectorTool.h
 *  @brief This file contains the class definition for the StreamSelectorTool class.
 **/

#include "AthenaKernel/IAthenaSelectorTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>

/** @class StreamSelectorTool
 *  @brief This class provides an example for reading with a ISelectorTool to veto events on AttributeList.
 **/
class StreamSelectorTool : public extends<AthAlgTool, IAthenaSelectorTool> {
public: // Constructor and Destructor
   /// Standard Tool Constructor
   using base_class::base_class;
   /// Destructor
   virtual ~StreamSelectorTool();

public:
   /// IAthenaSelectorTool Interface method implementations:
   virtual StatusCode postInitialize() override { return StatusCode::SUCCESS; }
   virtual StatusCode preNext() const override { return StatusCode::SUCCESS; }
   virtual StatusCode postNext() const override;
   virtual StatusCode preFinalize() override { return StatusCode::SUCCESS; }
private:
   StringProperty m_attrListKey{this,"AttributeListKey","Input","Key for attribute list input to be used"};
   StringProperty m_streamName{this,"AcceptStreams","","Name of stream to be used as a ACCEPT"};
};

#endif
