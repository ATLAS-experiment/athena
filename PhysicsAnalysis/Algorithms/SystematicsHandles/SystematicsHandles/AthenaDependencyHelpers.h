/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__ATHENA_DEPENDENCY_HELPERS_H
#define SYSTEMATICS_HANDLES__ATHENA_DEPENDENCY_HELPERS_H

//
// includes
//

#include <xAODMissingET/MissingETContainer.h>
#include <functional>
#include <type_traits>

#ifndef XAOD_STANDALONE
#include <AthenaKernel/CLASS_DEF.h>
#include <AthenaKernel/TopBase.h>
#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/DataHandle.h>
#endif

//
// method implementations
//

namespace CP
{
  class ISystematicsSvc;

  namespace detail
  {
#ifndef XAOD_STANDALONE
    template<typename ContainerType>
      requires (!std::is_const_v<ContainerType>)
    CLID getClidForDependency (const std::string& typeName, const std::string& decoName, bool decoWrite)
    {
      // MissingETContainer special case: `TopBase` is not defined for
      // it, so we don't use it. Instead we will just use the regular
      // CLID. Should the `TopBase` ever be defined for
      // `xAOD::MissingETContainer`, this special case can be removed,
      // and with it the dependency in `CMakeLists.txt`.
      if constexpr (!std::same_as<ContainerType, xAOD::MissingETContainer>)
      {
        // For reading decorations we have a special case that matches
        // what the @ref SG::ReadDecorHandle does: Essentially it
        // registers with the top-most base, instead of the actual type.
        if (!decoName.empty() && !decoWrite)
        {
          // For decorations being read, use TopBase<ContainerType>
          using topbase_t = typename SG::TopBase<ContainerType>::type;
          return ClassID_traits<topbase_t>::ID();
        }
      }
      if (!typeName.empty())
        return DataObjID(typeName, "").clid();
      else
        return ClassID_traits<ContainerType>::ID();
    }


    StatusCode addSysDependency (MsgStream& msg, const ISystematicsSvc& svc,
                           const std::function<void(const DataObjID&, Gaudi::DataHandle::Mode)>& addAlgDependency,
                           const CLID clid, const std::string& name, Gaudi::DataHandle::Mode mode,
                           const std::string& decoName, bool decoWrite);
#endif
  } // namespace detail
}

#endif