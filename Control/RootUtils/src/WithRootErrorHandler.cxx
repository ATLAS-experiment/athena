/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file RootUtils/src/WithRootErrorHandler.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2021
 * @brief Run a MT piece of code with an alternate root error handler.
 */


#include "RootUtils/WithRootErrorHandler.h"
#include <vector>
#include <cstdlib>
#include <mutex>
#include <atomic>


namespace {


/// Thread-local list of registered error handlers, from oldest to newest.
using Handler_t = RootUtils::WithRootErrorHandler::Handler_t;
thread_local std::vector<Handler_t> rootErrorHandlers;


/// Pointer to the previous handler.
std::atomic<ErrorHandlerFunc_t> origHandler;


/**
 * @brief Global root error handler.
 */
void errorHandler (int level,
                   Bool_t abort,
                   const char* location,
                   const char* msg)
{
  // Execute all the handlers in our thread-local list from newest to oldest.
  // Stop if one returns false.
  for (int i = rootErrorHandlers.size()-1; i >= 0; --i) {
    if (!rootErrorHandlers[i] (level, abort, location, msg)) return;
  }
  // They all returned true.  Call the previous handler.
  origHandler (level, abort, location, msg);
}


} // anonymous namespace


namespace RootUtils {


/**
 * @brief Temporarily install a thread-local root error handler.
 * @param errhand The handler to be installed.
 *
 * The installed handler will only run in the current thread, and it will
 * be removed when this object is deleted.  In addition to the usual
 * arguments for a root error handler, it returns a bool.  If the returned
 * value is true, previous handlers will also be executed; otherwise,
 * no further handlers will be executed.
 */
WithRootErrorHandler::WithRootErrorHandler (Handler_t errhand)
  : m_size (rootErrorHandlers.size()+1)
{
  // Install our handler the first time we're called.
  // We used to do that when the library was loaded, via a global static,
  // but then we ran to issues where the behavior could depend on library
  // loading order, since other libraries (such as Gaudi) also try to install
  // their own handler.
  //
  // By the time we're called, there may be multiple threads running,
  // so it is in principle not safe to call SetErrorHandler.
  // However, there shouldn't be anything else in Athena calling it,
  // so in practice it should be ok.  As an extra check, we abort if the
  // handler we get back isn't what we set, indicating a potential race.
  //
  // See ATLASRECTS-7967.
  static std::once_flag flag;
  std::call_once (flag, []() {
    origHandler = ::SetErrorHandler (errorHandler);
    if (::GetErrorHandler() != errorHandler) {
      std::abort();
    }
  });

  rootErrorHandlers.push_back (errhand);
}


/**
 * @brief Destructor.
 *
 * Remove the error handler.
 */
WithRootErrorHandler::~WithRootErrorHandler()
{
  if (m_size != rootErrorHandlers.size()) std::abort();
  rootErrorHandlers.pop_back();
}


} // namespace RootUtils
