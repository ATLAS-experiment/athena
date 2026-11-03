//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#include "AlpakaExampleAlg.h"

// Alpaka include(s).
#include <alpaka/alpaka.hpp>
#include <alpaka/example/ExampleDefaultAcc.hpp>

// System include(s).
#include <cstddef>
#include <cstdint>

namespace {

//! Example Alpaka kernel: compute y[i] = 2 * x[i] + 1
struct VectorTransformKernel {
  template < typename TAcc, typename TElem, typename TIdx >
  ALPAKA_FN_ACC auto operator()(
      TAcc const& acc,
      TElem const* const in,
      TElem* const out,
      TIdx const numElements ) const -> void {
    for( auto i : alpaka::uniformElements( acc, numElements ) ) {
      out[i] = in[i] * 2 + 1;
    }
  }
};

}  // namespace

StatusCode AlpakaExampleAlg::initialize() {
  using Dim = alpaka::DimInt< 1u >;
  using Idx = std::size_t;
  using Acc = alpaka::ExampleDefaultAcc< Dim, Idx >;

  ATH_MSG_INFO( "Using Alpaka accelerator: " << alpaka::getAccName< Acc >() );
  return StatusCode::SUCCESS;
}

StatusCode AlpakaExampleAlg::execute( const EventContext& /*ctx*/ ) const {
  using Dim = alpaka::DimInt< 1u >;
  using Idx = std::size_t;
  using Acc = alpaka::ExampleDefaultAcc< Dim, Idx >;
  using DevAcc = alpaka::Dev< Acc >;
  using QueueAcc = alpaka::Queue< Acc, alpaka::Blocking >;
  using Data = std::uint32_t;

  const Idx numElements = m_nElements;
  const alpaka::Vec< Dim, Idx > extent( numElements );

  // Select device and queue.
  auto const platform = alpaka::Platform< Acc >{};
  auto const devAcc = alpaka::getDevByIdx( platform, 0 );
  QueueAcc queue( devAcc );

  // Select host device.
  using DevHost = alpaka::DevCpu;
  auto const platformHost = alpaka::PlatformCpu{};
  auto const devHost = alpaka::getDevByIdx( platformHost, 0 );

  // Allocate host buffers.
  using BufHost = alpaka::Buf< DevHost, Data, Dim, Idx >;
  BufHost bufHostIn( alpaka::allocBuf< Data, Idx >( devHost, extent ) );
  BufHost bufHostOut( alpaka::allocBuf< Data, Idx >( devHost, extent ) );

  // Initialize host input.
  for( Idx i = 0; i < numElements; ++i ) {
    bufHostIn[i] = static_cast< Data >( i );
    bufHostOut[i] = 0;
  }

  // Allocate device buffers.
  using BufAcc = alpaka::Buf< DevAcc, Data, Dim, Idx >;
  BufAcc bufAccIn( alpaka::allocBuf< Data, Idx >( devAcc, extent ) );
  BufAcc bufAccOut( alpaka::allocBuf< Data, Idx >( devAcc, extent ) );

  // Copy Host -> Device.
  alpaka::memcpy( queue, bufAccIn, bufHostIn );
  alpaka::memcpy( queue, bufAccOut, bufHostOut );

  // Run kernel on queue.
  VectorTransformKernel kernel;
  const Idx elementsPerThread = 1u;
  alpaka::KernelCfg< Acc > const kernelCfg = { extent, elementsPerThread };
  auto const workDiv = alpaka::getValidWorkDiv(
      kernelCfg, devAcc, kernel,
      alpaka::getPtrNative( bufAccIn ),
      alpaka::getPtrNative( bufAccOut ),
      numElements );

  auto const taskKernel = alpaka::createTaskKernel< Acc >(
      workDiv, kernel,
      std::data( bufAccIn ),
      std::data( bufAccOut ),
      numElements );

  alpaka::enqueue( queue, taskKernel );

  // Copy Device -> Host.
  alpaka::memcpy( queue, bufHostOut, bufAccOut );
  alpaka::wait( queue );

  // Validate results.
  bool success = true;
  ATH_MSG_INFO( "Alpaka kernel execution with " << numElements << " elements:" );
  for( Idx i = 0; i < numElements; ++i ) {
    Data const expected = static_cast< Data >( i * 2 + 1 );
    Data const actual = bufHostOut[i];
    ATH_MSG_INFO( "    in[" << i << "] = " << bufHostIn[i]
                  << " -> out[" << i << "] = " << actual
                  << " (expected " << expected << ")" );
    if( actual != expected ) {
      success = false;
    }
  }

  ATH_MSG_INFO( "Result " << ( success ? "VALIDATED" : "FAILED" ) );
  return success ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

