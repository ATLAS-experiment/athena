/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TEST_INITGAUDI_H
# define TEST_INITGAUDI_H
/** @file initGaudi.h
 * @brief  minimal gaudi initialization for AthenaServices unit testing
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov> -ATLAS Collaboration
 **/

#include <string>

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
}
#endif // TEST_INITGAUDI_H
