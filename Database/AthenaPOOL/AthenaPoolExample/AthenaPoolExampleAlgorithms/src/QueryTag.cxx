/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file QueryTag.cxx
 *  @brief This file contains the implementation for the QueryTag class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "QueryTag.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "StoreGate/ReadHandle.h"

using namespace AthPoolEx;

//___________________________________________________________________________
StatusCode QueryTag::initialize() {
   ATH_MSG_INFO("in initialize()");
   const IService* parentSvc = dynamic_cast<const IService*>(this->parent());
   if (parentSvc != nullptr) {
      const IProperty* propertyServer = dynamic_cast<const IProperty*>(parentSvc);
      if (propertyServer != nullptr) {
         StringProperty attrKeyProperty("AttributeListKey", "");
         StatusCode status = propertyServer->getProperty(&attrKeyProperty);
         if (status.isSuccess()) {
            m_attrListKey = attrKeyProperty.value();
         }
      }
   }

   ATH_CHECK( m_attrListKey.initialize() );
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode QueryTag::postInitialize() {
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode QueryTag::preNext() const {
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode QueryTag::postNext() const {
   SG::ReadHandle<AthenaAttributeList> attrList (m_attrListKey);
   ATH_CHECK( attrList.isValid() );

   try {
      unsigned int eventNumber = (*attrList)["EventNumber"].data<unsigned int>();
      unsigned int runNumber = (*attrList)["RunNumber"].data<unsigned int>();
      unsigned int magicNumber = (*attrList)["MagicNumber"].data<unsigned int>();

      ATH_MSG_DEBUG("EventNumber = " << eventNumber);
      ATH_MSG_DEBUG("RunNumber = " << runNumber);
      ATH_MSG_DEBUG("MagicNumber = " << magicNumber);
      if (eventNumber < 10 && magicNumber > 17) {
         return StatusCode::RECOVERABLE;
      }
   } catch (...) {
      ATH_MSG_DEBUG("Can't apply attributeList preselection");
      return StatusCode::SUCCESS;
   }
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode QueryTag::preFinalize() {
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode QueryTag::finalize() {
   ATH_MSG_INFO("in finalize()");
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
