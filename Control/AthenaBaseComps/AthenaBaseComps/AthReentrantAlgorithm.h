///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

// AthReentrantAlgorithm.h 
// Header file for class AthReentrantAlgorithm
// Author: Beojan Stanislaus
// Author: Charles Leggett
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHREENTRANTALGORITHM_H
#define ATHENABASECOMPS_ATHREENTRANTALGORITHM_H 1


// STL includes
#include "AthenaBaseComps/AthCommonReentrantAlgorithm.h"


#include "Gaudi/Algorithm.h"


/**
 * @brief An algorithm that can be simultaneously executed in multiple threads.
 *
 * The default behavior of AthenaMT when an algorithm is to be run on different
 * events in multiple threads is to clone the algorithm object and use
 * a different instance for each thread.  If, however, your algorithm
 * derives from @c AthReentrantAlgorithm, then the same algorithm object may
 * be used in multiple threads without cloning.
 *
 * Rather than @c execute, a reentrant algorithm will call @c execute,
 * which has two differences.  First, @c execute is @c const.  If the
 * same object is being used in multiple threads, it should not have any
 * state which is being changed.  Any attempt to get around this with
 * @c mutable or @c const_cast should be viewed with great suspicion:
 * that may not be thread-safe.
 *
 * Second, the @c execute method takes an explicit event context argument.
 * This may be used to find the proper store for the event being processed
 * by the current thread.
 *
 * The typical usage will be to declare a key object as a property of the
 * algorithm and then construct a transient handle instance during @c execute
 * from the key and the event context.  For example:
 *
 *@code
 *  class MyAlg : public AthReentrantAlgorithm
 *  {
 *  ...
 *    SG::ReadHandleKey<MyObj> m_rhandle;
 *  };
 *
 *  MyAlg::MyAlg (const std::string& name, ISvcLocator* svcLoc)
 *    : AthReentrantAlgorithm (name, svcLoc)
 *  {
 *    declareProperty ("rhandle", m_rhandle);
 *  }
 *
 *  StatusCode MyAlg::initialize()
 *  {
 *    ATH_CHECK( m_rhandle.initialize() );
 *    return StatusCode::SUCCESS;
 *  }
 *
 *  StatusCode MyAlg::execute (const EventContext& ctx) const
 *  {
 *    SG::ReadHandle<MyObj> myobj (m_rhandle, ctx);
 *    const MyObj& p = *myobj;
 *    ...
 *  }
 @endcode
 */
class AthReentrantAlgorithm : public AthCommonReentrantAlgorithm<Gaudi::Algorithm>{
    using AthCommonReentrantAlgorithm<Gaudi::Algorithm>::AthCommonReentrantAlgorithm;
};

#endif //> !ATHENABASECOMPS_ATHREENTRANTALGORITHM_H
