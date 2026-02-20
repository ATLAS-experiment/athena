/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleMap.h"

namespace MuonTGC_Cabling {

TGCModuleMap::~TGCModuleMap() = default;

std::unique_ptr<TGCModuleId> TGCModuleMap::popModule(const int port) {
    auto itr = m_moduleMap.find(port);
    if (itr != m_moduleMap.end()) {
        std::unique_ptr<TGCModuleId> returnMe = std::move(itr->second);
        m_moduleMap.erase(itr);
        return returnMe;
    }
    return nullptr;
}
TGCModuleMap::Store_t::const_iterator TGCModuleMap::begin() const {
    return m_moduleMap.begin();
}
TGCModuleMap::Store_t::const_iterator TGCModuleMap::end() const {
    return m_moduleMap.end();
}
TGCModuleMap::Store_t::iterator TGCModuleMap::begin() {
    return m_moduleMap.begin();
}
TGCModuleMap::Store_t::iterator TGCModuleMap::end() {
    return m_moduleMap.end();
}

void TGCModuleMap::insert(int connector,
                          std::unique_ptr<TGCModuleId> moduleId) {
    m_moduleMap.emplace(connector, std::move(moduleId));
}

bool TGCModuleMap::empty() const {
    return m_moduleMap.empty();
}
std::size_t TGCModuleMap::size() const {
    return m_moduleMap.size();
}

void TGCModuleMap::clear() {
    m_moduleMap.clear();
}

}  // namespace MuonTGC_Cabling
