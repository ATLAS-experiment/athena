/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTupleAuxDynWriter.h"

#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"

namespace RootAuxDynIO {
/// Simple Constructor
RNTupleAuxDynWriter::RNTupleAuxDynWriter()
    : AthMessaging(std::string("RNTupleAuxDynWriter")) {}

/// Collect Aux data information to be written out
std::vector<attrDataTuple> RNTupleAuxDynWriter::collectAuxAttributes(
    const std::string& base_name, SG::IAuxStoreIO* store) {
  std::vector<attrDataTuple> result;
  const SG::auxid_set_t selection = store->getSelectedAuxIDs();
  ATH_MSG_DEBUG("Writing " << base_name << " with " << selection.size()
                           << " Dynamic attributes");
  for (SG::auxid_t id : selection) {
    const std::string attr_type =
        SG::normalizedTypeinfoName(*store->getIOType(id));
    const std::string attr_name = SG::AuxTypeRegistry::instance().getName(id);
    const std::string field_name =
        RootAuxDynIO::auxFieldName(attr_name, base_name);
    void* attr_data ATLAS_THREAD_SAFE = const_cast<void*>(store->getIOData(id));

    result.emplace_back(field_name, attr_type, attr_data);
  }
  return result;
}

}  // namespace RootAuxDynIO
