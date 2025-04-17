/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTupleAuxDynWriter.h"

#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"

#include "TClass.h"

namespace RootAuxDynIO {

/// Simple Constructor
RNTupleAuxDynWriter::RNTupleAuxDynWriter(TClass& tc)
   : AthMessaging(std::string("AuxDynAttrTool")),
     AuxDynAttrAccess(tc)
{
}

   
/// Collect AuxDyn data information
std::vector<attrDataTuple>
RNTupleAuxDynWriter::collectAuxAttributes( const std::string& base_name, void* object )
{
   SG::IAuxStoreIO* store = castIOStore(object);
   std::vector<attrDataTuple> result;
   if( !store ) return result;
   const SG::auxid_set_t selection = store->getSelectedAuxIDs();
   ATH_MSG_DEBUG("Writing " << base_name << " with " << selection.size()
                 << " Dynamic attributes");
   for (SG::auxid_t id : selection) {
      const std::string attr_type = SG::normalizedTypeinfoName(*store->getIOType(id));
      const std::string attr_name = SG::AuxTypeRegistry::instance().getName(id);
      const std::string field_name = RootAuxDynIO::auxFieldName(attr_name, base_name);
      void* attr_data ATLAS_THREAD_SAFE = const_cast<void*>(store->getIOData(id));

      result.emplace_back(field_name, attr_type, attr_data);
   }
   return result;
}

}  // namespace RootAuxDynIO
