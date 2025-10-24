/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_WRITETAG_H
#define ATHENAPOOLEXAMPLEALGORITHMS_WRITETAG_H

/** @file WriteTag.h
 *  @brief This file contains the class definition for the WriteTag class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: WriteTag.h,v 1.1 2008-12-10 21:28:11 gemmeren Exp $
 **/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "PersistentDataModel/AthenaAttributeList.h"
#include "StoreGate/WriteHandleKey.h"

class AthenaAttributeListSpecification;

namespace AthPoolEx {

/** @class AthPoolEx::WriteTag
 *  @brief This class provides an example for writing event data objects to Pool.
 **/
class WriteTag : public AthReentrantAlgorithm {
public:
   WriteTag(const std::string& name, ISvcLocator* pSvcLocator);
   virtual ~WriteTag() override final;

public:
   /// Gaudi Service Interface method implementations:
   virtual StatusCode initialize() override final;
   virtual StatusCode execute (const EventContext& ctx) const override final;
   virtual StatusCode finalize() override final;

private:
   SG::WriteHandleKey<AthenaAttributeList> m_key { this, "Key", "RunEventTag" };
   Gaudi::Property<int> m_magic{this, "Magic", 0};
   /// Specification of the event tag metadata schema
   AthenaAttributeListSpecification* m_attribListSpec;
};

} // end AthPoolEx namespace

#endif
