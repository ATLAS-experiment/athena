/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/Decoration.h"


namespace ActsTrk::detail {

template<typename T> 
bool build (const std::type_info* typeInfo, const std::string& name, std::vector<Decoration>& decorations){
  if (*typeInfo == typeid(T)) {
    decorations.emplace_back(
        decoration<T>(name, ActsTrk::detail::constDecorationGetter<T>,
                          ActsTrk::detail::decorationCopier<T>));
    return true;
  }
  return false;
}  

std::vector<Decoration> restoreDecorations(
    const SG::IConstAuxStore* container,
    const std::set<std::string>& staticVariables) {
  std::vector<Decoration> decorations;
  for (auto id : container->getAuxIDs()) {
    const std::string name = SG::AuxTypeRegistry::instance().getName(id);
    const std::type_info* typeInfo =
        SG::AuxTypeRegistry::instance().getType(id);
    if (staticVariables.count(name) == 1) {
      continue;
    }

    // try making decoration accessor of matching type
    // there is a fixed set of supported types (as there is a fixed set
    // available in MutableMTJ) setters are not needed so replaced by a
    if ( build<float>(typeInfo, name, decorations) ||
         build<double>(typeInfo, name, decorations) ||
         build<short>(typeInfo, name, decorations) ||
         build<int>(typeInfo, name, decorations) ||
         build<uint8_t>(typeInfo, name, decorations) ||
         build<uint16_t>(typeInfo, name, decorations) ||
         build<uint32_t>(typeInfo, name, decorations) ||
         build<uint64_t>(typeInfo, name, decorations) ||
         build<int8_t>(typeInfo, name, decorations) ||
         build<int16_t>(typeInfo, name, decorations) ||
         build<int32_t>(typeInfo, name, decorations) ||
         build<int64_t>(typeInfo, name, decorations)
        ) {
      continue;
    } 
    throw std::runtime_error("Can't restore decoration of  " + name +
        " because it is of an unsupported type");
  }
  return decorations;
}
}  // namespace ActsTrk::detail