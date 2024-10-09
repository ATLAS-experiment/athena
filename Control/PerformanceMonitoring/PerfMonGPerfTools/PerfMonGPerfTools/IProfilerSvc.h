// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERFMONGPERFTOOLS_IPROFILERSVC_H
#define PERFMONGPERFTOOLS_IPROFILERSVC_H

// STL include(s):
#include <string>

// Gaudi/Athena include(s):
#include "GaudiKernel/IService.h"

namespace GPT {

   /**
    *  @short Simple interface for the profiler service
    *
    *         In order to allow clients to interact directly with the profiler
    *         service, this interface is used. But for basic profiling the
    *         users don't actually have to use it.
    *
    * @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
    */
   class IProfilerSvc : public virtual IService {

   public:
      /// Interface ID definition
      DeclareInterfaceID(IProfilerSvc, 0, 1);

      /// Start GPT profiling
      /**
       * This function can be used to start the profiling, collecting the
       * data into a file with the specified name.
       *
       * @param filename Name of the GPT profile file
       * @returns <code>StatusCode::SUCCESS</code> if the operation was
       *          successful, something else otherwise
       */
      virtual StatusCode startCPUProfiling( const std::string& filename = "gpt.profile" ) = 0;

      /// Stop the GPT profiling
      /**
       * This function should be called to stop the CPU profiling.
       *
       * @returns <code>StatusCode::SUCCESS</code> if the operation was
       *          successful, something else otherwise
       */
      virtual StatusCode stopCPUProfiling() = 0;

      /// Is the GPT profiling running at the moment?
      /**
       * This function can be used to ask the service whether CPU profiling
       * is running at the moment.
       *
       * @returns <code>true</code> if the profiling is running at the moment,
       *          <code>false</code> otherwise
       */
      virtual bool isCPUProfilingRunning() const = 0;

   }; // class IProfilerSvc

} // namespace GPT

#endif // PERFMONGPERFTOOLS_IPROFILERSVC_H
