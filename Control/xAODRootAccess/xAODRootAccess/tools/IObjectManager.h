// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_TOOLS_IOBJECTMANAGER_H
#define XAODROOTACCESS_TOOLS_IOBJECTMANAGER_H

// Local include(s).
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/TVirtualManager.h"

// System include(s).
#include <memory>

namespace xAOD::Details {

/// @short Manager for EDM objects created by ROOT
///
/// This class is used when an EDM object is meant to be created
/// by ROOT's schema evolution system, behind the scenes. It serves as a base
/// class for the @c TTree and @c RNTuple based object managers.
///
class IObjectManager : public TVirtualManager {

 public:
  /// Constructor
  IObjectManager(std::unique_ptr<THolder> holder);
  /// Copy constructor
  IObjectManager(const IObjectManager& parent);
  /// Move constructor
  IObjectManager(IObjectManager&& parent) noexcept;
  /// Destructor
  ~IObjectManager() override;

  /// Copy assignment operator
  IObjectManager& operator=(const IObjectManager& parent);
  /// Move assignment operator
  IObjectManager& operator=(IObjectManager&& parent) noexcept;

  /// Accessor to the Holder object
  const THolder* holder() const;
  /// Accessor to the Holder object
  THolder* holder();

 private:
  /// Object holding onto an EDM object in memory
  std::unique_ptr<THolder> m_holder;

};  // class IObjectManager

}  // namespace xAOD::Details

#endif  // XAODROOTACCESS_TOOLS_IOBJECTMANAGER_H
