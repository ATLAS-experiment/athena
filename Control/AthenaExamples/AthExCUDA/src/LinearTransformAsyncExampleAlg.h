// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHEXCUDA_LINEARTRANSFORMASYNCEXAMPLEALG_H
#define ATHEXCUDA_LINEARTRANSFORMASYNCEXAMPLEALG_H

// Framework include(s).
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAsynchronousAlgorithm.h"

// STL
#include <vector>

namespace AthCUDAExamples {

   /// Example algorithm demonstrating the use of CUDA in an AthAsynchronousAlgorithm
   ///
   /// Modified from a previous implementation using AthCUDA and a Blocking algorithm by
   /// Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// @author Beojan Stanislaus <beojan.stanislaus@cern.ch>
   class LinearTransformAsyncExampleAlg : public AthAsynchronousAlgorithm {

   public:
      // Inherit the base class's constructor(s).
      using AthAsynchronousAlgorithm::AthAsynchronousAlgorithm;

      /// Function executing the algorithm
      virtual StatusCode execute( const EventContext& ctx ) const override;

   private:
         /// GPU kernel launcher
         StatusCode linearTransform(std::vector<float>& arr, float multiplier) const;

   }; // class LinearTransformAsyncExampleAlg

} // namespace AthCUDAExamples

#endif // ATHEXCUDA_LINEARTRANSFORMASYNCEXAMPLEALG_H
