/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TEST_INITGAUDI_H
# define TEST_INITGAUDI_H
/** @file initGaudi.h
 * @brief  minimal gaudi initialization for AthenaServices unit testing
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov> -ATLAS Collaboration
 **/

#include <string>
#include "GaudiKernel/SmartIF.h"

#undef NDEBUG

class ISvcLocator;


namespace Athena_test {
  /**
   *  Minimal Gaudi initialization for unit testing without job options.
   *
   *  @param pSvcLoc returns a pointer to the Gaudi ServiceLocator
   *  @return true on success, false on failure
   */
  bool initGaudi(ISvcLocator*& pSvcLoc);

  /**
   *  Minimal Gaudi initialization for unit testing.
   *
   *  @param jobOptsFile job options file name
   *  @param pSvcLoc returns a pointer to the Gaudi ServiceLocator
   *  @return true on success, false on failure
   */
  bool initGaudi(const std::string& jobOptsFile, ISvcLocator*& pSvcLoc);


  /**
   * Generic test fixture to setup Gaudi.
   *
   * Example usage with boost test:
   *
   * @code
   * BOOST_FIXTURE_TEST_SUITE( MyTestSuite,
   *    * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("jo.txt")) )
   * @endcode
   */
  class InitGaudi {
  public:
    /**
     * @brief Create Gaudi test fixture.
     * @param jobOptsPath Path to job options
     */
    InitGaudi(const std::string& jobOptsFile);

    /**
     * @brief Create Gaudi test fixture without job options.
     */
    InitGaudi() : InitGaudi("") {}

    /**
     * @brief Finalize Gaudi.
     */
    ~InitGaudi();

    SmartIF<ISvcLocator> svcLoc;
  };

}
#endif // TEST_INITGAUDI_H
