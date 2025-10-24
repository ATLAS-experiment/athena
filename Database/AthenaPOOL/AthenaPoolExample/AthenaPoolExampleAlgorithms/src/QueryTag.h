/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_QUERYTAG_H
#define ATHENAPOOLEXAMPLEALGORITHMS_QUERYTAG_H

/** @file QueryTag.h
 *  @brief This file contains the class definition for the QueryTag class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthenaKernel/IAthenaSelectorTool.h"

#include <string>

namespace AthPoolEx {

/** @class AthPoolEx::QueryTag
 *  @brief This class provides an example for reading with a ISelectorTool to veto events on AttributeList.
 **/
class QueryTag : public extends<AthAlgTool, IAthenaSelectorTool> {
public:
   using base_class::base_class;
   virtual ~QueryTag() = default;

public:
   /// IAthenaSelectorTool Interface method implementations:
   virtual StatusCode initialize() override final;
   virtual StatusCode postInitialize() override final;
   virtual StatusCode preNext() const override final;
   virtual StatusCode postNext() const override final;
   virtual StatusCode preFinalize() override final;
   virtual StatusCode finalize() override final;

private:
   SG::ReadHandleKey<AthenaAttributeList> m_attrListKey;
};

} // end AthPoolEx namespace

#endif
