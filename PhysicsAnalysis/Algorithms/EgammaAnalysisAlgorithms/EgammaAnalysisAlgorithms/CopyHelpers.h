/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EGAMMA_ANALYSIS_ALGORITHMS__COPY_HELPERS_H
#define EGAMMA_ANALYSIS_ALGORITHMS__COPY_HELPERS_H

#include <SystematicsHandles/CopyHelpers.h>

#include <CxxUtils/checker_macros.h>
#include <xAODEgamma/EgammaContainer.h>

namespace CP
{
  namespace detail
  {
    template<>
    struct ShallowCopy<xAOD::EgammaContainer>
    {
      static StatusCode
      getCopy (MsgStream& msgStream, const EventContext& ctx,
               xAOD::EgammaContainer*& object,
               const xAOD::EgammaContainer *inputObject,
               const std::string& outputName)
      {
        // Set up a lambda for providing a msg(...) function.
        // Suppress thread-checker warning because this provides just a wrapper to MsgStream.
        const auto msg = [&] ATLAS_NOT_THREAD_SAFE (MSG::Level lvl) -> MsgStream& {
          msgStream << lvl;
          return msgStream;
        };

        xAOD::IParticleContainer *subobject = nullptr;
        if (!ShallowCopy<xAOD::IParticleContainer>::getCopy (msgStream, ctx, subobject, inputObject, outputName).isSuccess())
          return StatusCode::FAILURE;
        if (!(object = dynamic_cast<xAOD::EgammaContainer*>(subobject)))
        {
          ANA_MSG_ERROR ("copy of EgammaContainer is not an EgammaContainer");
          ANA_MSG_ERROR ("check logic in CopyHelpers");
          return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
      }
    };
  }
}

#endif
