/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrigServicesHelper.cxx
 * @brief Python bindings for calling HltEventLoopMgr methods directly from Python
 *
 * This provides a way to call prepareForStart() and other ITrigEventLoopMgr
 * methods from Python (e.g., athenaEF.py) without going through PSC/HLTMPPU.
 */

#include <sstream>
#include <string>
#include <memory>
#include <unistd.h>  // for getpid()

#include <boost/python.hpp>
#include <boost/property_tree/ptree.hpp>

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Bootstrap.h"
#include "Gaudi/Interfaces/IOptionsSvc.h"

#include "TrigKernel/ITrigEventLoopMgr.h"

namespace {

/**
 * Build the prepareForStart ptree directly from individual parameters
 */
boost::property_tree::ptree buildPrepareForStartPtree(
    uint32_t run_number,
    const std::string& det_mask,
    const std::string& sor_time,
    uint32_t lb_number,
    uint32_t beam_type,
    uint32_t beam_energy,
    float toroids_current,
    float solenoid_current,
    const std::string& run_type,
    uint32_t trigger_type,
    bool recording_enabled)
{
  boost::property_tree::ptree pt;
  
  // RunParams section
  boost::property_tree::ptree& runParams = pt.add_child("RunParams", boost::property_tree::ptree());
  runParams.put("run_number", run_number);
  runParams.put("max_events", 0);
  runParams.put("recording_enabled", recording_enabled ? 1 : 0);
  runParams.put("trigger_type", trigger_type);
  runParams.put("run_type", run_type);
  runParams.put("det_mask", det_mask);
  runParams.put("beam_type", beam_type);
  runParams.put("beam_energy", beam_energy);
  runParams.put("stream", "");
  runParams.put("lumiblock", lb_number);
  runParams.put("filename_tag", "");
  runParams.put("T0_project_tag", "");
  runParams.put("timeSOR", sor_time);
  runParams.put("timeEOR", "1/1/70 01:00:00");
  runParams.put("totalTime", 0);
  
  // Magnets section
  boost::property_tree::ptree& magnets = pt.add_child("Magnets", boost::property_tree::ptree());
  
  boost::property_tree::ptree& toroidsCurrent = magnets.add_child("ToroidsCurrent", boost::property_tree::ptree());
  toroidsCurrent.put("value", toroids_current);
  toroidsCurrent.put("ts", sor_time);
  
  boost::property_tree::ptree& solenoidCurrent = magnets.add_child("SolenoidCurrent", boost::property_tree::ptree());
  solenoidCurrent.put("value", solenoid_current);
  solenoidCurrent.put("ts", sor_time);
  
  return pt;
}

} // anonymous namespace


/**
 * @class TrigServicesHelper
 * @brief Helper class to call ITrigEventLoopMgr methods from Python
 */
class TrigServicesHelper {
public:
  TrigServicesHelper() = default;
  ~TrigServicesHelper() = default;

  /**
   * Call prepareForStart on the HltEventLoopMgr
   * 
   * @param run_number Run number
   * @param det_mask Detector mask (32 hex chars)
   * @param sor_time Start of run time string (format: "dd/mm/yy HH:MM:SS.ffffff")
   * @param lb_number Lumi block number
   * @param beam_type Beam type
   * @param beam_energy Beam energy
   * @param toroids_current Toroids current (default 20400)
   * @param solenoid_current Solenoid current (default 7730)
   * @param event_loop_name Name of the EventLoopMgr service
   * @return true on success, false on failure
   */
  bool prepareForStart(
      uint32_t run_number,
      const std::string& det_mask,
      const std::string& sor_time,
      uint32_t lb_number = 0,
      uint32_t beam_type = 0,
      uint32_t beam_energy = 0,
      float toroids_current = 20400.0f,
      float solenoid_current = 7730.0f,
      const std::string& run_type = "Physics",
      uint32_t trigger_type = 0,
      bool recording_enabled = false,
      const std::string& event_loop_name = "HltEventLoopMgr")
  {
    // Build the ptree directly
    boost::property_tree::ptree pt = buildPrepareForStartPtree(
        run_number, det_mask, sor_time, lb_number,
        beam_type, beam_energy, toroids_current, solenoid_current,
        run_type, trigger_type, recording_enabled);
    
    // Get the service locator
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    if (!svcLoc) {
      std::cerr << "TrigServicesHelper: Failed to get ISvcLocator" << std::endl;
      return false;
    }
    
    // Get the EventLoopMgr service (use template version that returns SmartIF directly)
    auto evtLoopMgr = svcLoc->service<ITrigEventLoopMgr>(event_loop_name);
    if (!evtLoopMgr) {
      std::cerr << "TrigServicesHelper: Failed to retrieve ITrigEventLoopMgr '" 
                << event_loop_name << "'" << std::endl;
      return false;
    }
    
    // Call prepareForStart
    std::cout << "TrigServicesHelper: Calling prepareForStart on " << event_loop_name << std::endl;
    StatusCode sc = evtLoopMgr->prepareForStart(pt);
    
    if (sc.isFailure()) {
      std::cerr << "TrigServicesHelper: prepareForStart failed" << std::endl;
      return false;
    }
    
    std::cout << "TrigServicesHelper: prepareForStart completed successfully" << std::endl;
    return true;
  }

  /**
   * Call prepareForRun on the HltEventLoopMgr
   * 
   * This initializes the COOL folder helper and fires the BeginRun incident.
   * It must be called after start() and before hltUpdateAfterFork().
   * 
   * @param event_loop_name Name of the EventLoopMgr service
   * @return true on success, false on failure
   */
  bool prepareForRun(const std::string& event_loop_name = "HltEventLoopMgr")
  {
    // Get the service locator
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    if (!svcLoc) {
      std::cerr << "TrigServicesHelper: Failed to get ISvcLocator" << std::endl;
      return false;
    }

    // Get the EventLoopMgr service
    auto evtLoopMgr = svcLoc->service<ITrigEventLoopMgr>(event_loop_name);
    if (!evtLoopMgr) {
      std::cerr << "TrigServicesHelper: Failed to retrieve ITrigEventLoopMgr '" 
                << event_loop_name << "'" << std::endl;
      return false;
    }

    // Build an empty ptree (prepareForRun doesn't use it currently)
    boost::property_tree::ptree pt;

    // Call prepareForRun
    std::cout << "TrigServicesHelper: Calling prepareForRun on " << event_loop_name << std::endl;
    StatusCode sc ATLAS_THREAD_SAFE = evtLoopMgr->prepareForRun(pt);

    if (sc.isFailure()) {
      std::cerr << "TrigServicesHelper: prepareForRun failed" << std::endl;
      return false;
    }

    std::cout << "TrigServicesHelper: prepareForRun completed successfully" << std::endl;
    return true;
  }

  /**
   * Call hltUpdateAfterFork on the HltEventLoopMgr
   * 
   * This initializes the scheduler and other components that need to be 
   * set up after forking. In athenaEF (without PSC), this should
   * be called after prepareForRun() and before running the event loop.
   * 
   * @param worker_id Worker ID (use 1 for single-process mode without forking)
   * @param worker_pid Worker PID (defaults to current process PID)
   * @param event_loop_name Name of the EventLoopMgr service
   * @return true on success, false on failure
   */
  bool hltUpdateAfterFork(int worker_id = 1, 
                          int worker_pid = -1,
                          const std::string& event_loop_name = "HltEventLoopMgr")
  {
    // Use current PID if not specified
    if (worker_pid < 0) {
      worker_pid = getpid();
    }
    
    // Get the service locator
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    if (!svcLoc) {
      std::cerr << "TrigServicesHelper: Failed to get ISvcLocator" << std::endl;
      return false;
    }
    
    // Set DataFlow properties via JobOptionsSvc so updateDFProps() can read them
    // HltEventLoopMgr::updateDFProps() reads from "DataFlowConfig.DF_*" properties
    auto jobOptSvc = svcLoc->service<Gaudi::Interfaces::IOptionsSvc>("JobOptionsSvc");
    if (!jobOptSvc) {
      std::cerr << "TrigServicesHelper: Failed to get JobOptionsSvc" << std::endl;
      return false;
    }
    
    // Set worker ID > 0 to avoid mother process behavior in stop()
    jobOptSvc->set("DataFlowConfig.DF_WorkerId", std::to_string(worker_id));
    jobOptSvc->set("DataFlowConfig.DF_Pid", std::to_string(worker_pid));
    jobOptSvc->set("DataFlowConfig.DF_ApplicationName", "athenaEF");
    
    std::cout << "TrigServicesHelper: Set DataFlowConfig properties (WorkerId=" 
              << worker_id << ", Pid=" << worker_pid << ")" << std::endl;
    
    // Build an empty ptree - the actual values are read from JobOptionsSvc
    boost::property_tree::ptree pt;
    
    // Get the EventLoopMgr service
    auto evtLoopMgr = svcLoc->service<ITrigEventLoopMgr>(event_loop_name);
    if (!evtLoopMgr) {
      std::cerr << "TrigServicesHelper: Failed to retrieve ITrigEventLoopMgr '" 
                << event_loop_name << "'" << std::endl;
      return false;
    }
    
    // Call hltUpdateAfterFork
    std::cout << "TrigServicesHelper: Calling hltUpdateAfterFork on " << event_loop_name << std::endl;
    StatusCode sc = evtLoopMgr->hltUpdateAfterFork(pt);
    
    if (sc.isFailure()) {
      std::cerr << "TrigServicesHelper: hltUpdateAfterFork failed" << std::endl;
      return false;
    }
    
    std::cout << "TrigServicesHelper: hltUpdateAfterFork completed successfully" << std::endl;
    return true;
  }
};


// Boost.Python module definition
BOOST_PYTHON_MODULE(libTrigServicesHelper)
{
  using namespace boost::python;
  
  class_<TrigServicesHelper>("TrigServicesHelper")
    .def("prepareForStart", &TrigServicesHelper::prepareForStart,
         (arg("run_number"),
          arg("det_mask"),
          arg("sor_time"),
          arg("lb_number") = 0,
          arg("beam_type") = 0,
          arg("beam_energy") = 0,
          arg("toroids_current") = 20400.0f,
          arg("solenoid_current") = 7730.0f,
          arg("run_type") = "Physics",
          arg("trigger_type") = 0,
          arg("recording_enabled") = false,
          arg("event_loop_name") = "HltEventLoopMgr"),
         "Call prepareForStart on HltEventLoopMgr with individual parameters")
    .def("prepareForRun", &TrigServicesHelper::prepareForRun,
         (arg("event_loop_name") = "HltEventLoopMgr"),
         "Call prepareForRun on HltEventLoopMgr to initialize COOL folder helper and fire BeginRun incident. "
         "Must be called after start() and before hltUpdateAfterFork().")
    .def("hltUpdateAfterFork", &TrigServicesHelper::hltUpdateAfterFork,
         (arg("worker_id") = 1,
          arg("worker_pid") = -1,
          arg("event_loop_name") = "HltEventLoopMgr"),
         "Call hltUpdateAfterFork on HltEventLoopMgr to initialize the scheduler. "
         "worker_id must be > 0 to avoid mother process behavior during stop().");
}
