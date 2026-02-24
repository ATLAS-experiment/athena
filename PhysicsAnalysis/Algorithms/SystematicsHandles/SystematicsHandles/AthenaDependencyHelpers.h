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
#include <type_traits>

#ifndef XAOD_STANDALONE
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadDecorHandleKey.h>
#include <StoreGate/UpdateHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include <StoreGate/WriteDecorHandleKey.h>
#endif

//
// method implementations
//

namespace CP
{
  namespace detail
  {
#ifndef XAOD_STANDALONE
    template<typename ContainerType,unsigned mode,typename AlgorithmType>
      requires (mode <= 2 && !std::is_const_v<ContainerType>)
    StatusCode
    addDependency (AlgorithmType& owner, const std::string& name,const std::string& decoName,bool decoWrite)
    {
      // I'm creating a `****HandleKey` for the given name and then take
      // the proper dependency information from that, as that reuses as
      // much as possible from the official AthenaMT dependency
      // machinery.
      std::conditional_t<mode == 0,SG::ReadHandleKey<ContainerType>,
        std::conditional_t<mode == 1,SG::WriteHandleKey<ContainerType>,
          SG::UpdateHandleKey<ContainerType>>> key {name};
      if (!key.initialize().isSuccess())
        return StatusCode::FAILURE;

      if (decoName.empty())
      {
        owner.addDependency (key.fullKey(), key.mode());
        return StatusCode::SUCCESS;
      }

      // This `if` exempts the MissingETContainer, which doesn't seem to
      // like decoration handles. once this is fixed, this `if` should
      // be removed, as well as the link dependency to xAODMissingET in
      // the CMakeLists.txt of this package.
      if constexpr (std::same_as<ContainerType, xAOD::MissingETContainer>)
      {
        owner.msg() << MSG::WARNING << "Can not add decoration dependency " << name << "." << decoName << " as MissingET doesn't support decoration dependencies. This is only problematic if you are running in AthenaMT and rely on this dependency to exist." << endmsg;
        return StatusCode::SUCCESS;
      } else
      {
        if (decoWrite)
        {
          SG::WriteDecorHandleKey<ContainerType> decoKey {key, decoName};
          if (!decoKey.initialize().isSuccess())
            return StatusCode::FAILURE;
          owner.addDependency (decoKey.fullKey(), decoKey.mode());
        } else
        {
          SG::ReadDecorHandleKey<ContainerType> decoKey {key, decoName};
          if (!decoKey.initialize().isSuccess())
            return StatusCode::FAILURE;
          owner.addDependency (decoKey.fullKey(), decoKey.mode());
        }
        return StatusCode::SUCCESS;
      }
    }
#endif
  } // namespace detail
}

#endif