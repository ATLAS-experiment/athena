/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SystematicsHandles/AthenaDependencyHelpers.h>

#ifndef XAOD_STANDALONE
#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISystematicsSvc.h>

//
// method implementations
//

namespace CP
{
  namespace detail
  {
    StatusCode addSysDependency (MsgStream& msg, const ISystematicsSvc& svc,
                           const std::function<void(const DataObjID&, Gaudi::DataHandle::Mode)>& addAlgDependency,
                           const CLID clid, const std::string& name, Gaudi::DataHandle::Mode mode,
                           const std::string& decoName, bool decoWrite)
    {
      // Build full key string: "StoreGateSvc+name"
      std::string fullKeyStr = "StoreGateSvc+";

      {
        /// I'm only adding a dependency for nominal here, as there are
        /// currently no use cases in which a downstream user would only
        /// read a systematic, but not the nominal. The concern is that we
        /// might add dozens (or in extreme cases hundreds) of
        /// dependencies, without those adding providing any value.
        std::string temp;
        if (svc.makeSystematicsName (temp, name, CP::SystematicSet{}).isFailure())
        {
          msg << MSG::ERROR << "could not build nominal systematic name for " + name << endmsg;
          return StatusCode::FAILURE;
        }
        fullKeyStr += temp;
      }

      if (!decoName.empty())
      {
        std::string temp;
        if (svc.makeSystematicsName (temp, decoName, CP::SystematicSet{}).isFailure())
        {
          msg << MSG::ERROR << "could not build nominal systematic name for " + decoName << endmsg;
          return StatusCode::FAILURE;
        }

        // Build decoration key string
        fullKeyStr += "." + temp;

        // Decoration mode
        mode = decoWrite ? Gaudi::DataHandle::Writer : Gaudi::DataHandle::Reader;
      }

      addAlgDependency(DataObjID{clid, fullKeyStr}, mode);
      return StatusCode::SUCCESS;
    }
  }
}
#endif
