// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/tools/IObjectManager.h"

namespace xAOD::Details {

IObjectManager::IObjectManager(std::unique_ptr<THolder> holder)
    : m_holder(std::move(holder)) {}

IObjectManager::IObjectManager(const IObjectManager& parent) {

  if (parent.m_holder) {
    m_holder = std::make_unique<THolder>(*parent.m_holder);
  }
}

IObjectManager::IObjectManager(IObjectManager&& parent) noexcept = default;

IObjectManager::~IObjectManager() = default;

IObjectManager& IObjectManager::operator=(const IObjectManager& parent) {

  // Check if we need to do anything.
  if (this == &parent) {
    return *this;
  }

  // Make a copy of the holder if there is one.
  if (parent.m_holder) {
    m_holder = std::make_unique<THolder>(*parent.m_holder);
  } else {
    m_holder.reset();
  }

  // Return this object.
  return *this;
}

IObjectManager& IObjectManager::operator=(IObjectManager&& parent) noexcept =
    default;

const THolder* IObjectManager::holder() const {

  return m_holder.get();
}

THolder* IObjectManager::holder() {

  return m_holder.get();
}

}  // namespace xAOD::Details
