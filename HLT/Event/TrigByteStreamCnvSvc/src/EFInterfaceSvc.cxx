/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFInterfaceSvc.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaInterprocess/Incidents.h"
#include "eformat/eformat.h"
#include <functional>
#include <boost/property_tree/json_parser.hpp>

EFInterfaceSvc::EFInterfaceSvc(const std::string& name, ISvcLocator* svc)
  : base_class(name, svc)
{
}

StatusCode EFInterfaceSvc::initialize ATLAS_NOT_THREAD_SAFE ()
{
  ATH_MSG_DEBUG("EFInterfaceSvc initialized");
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
  ATH_CHECK( incSvc.retrieve() );
  incSvc->addListener(this, AthenaInterprocess::UpdateAfterFork::type());
  return StatusCode::SUCCESS;
}

/**************************************************************************************/

void EFInterfaceSvc::handle(const Incident& incident)
{
  if (incident.type() == AthenaInterprocess::UpdateAfterFork::type()) 
  {
    ATH_MSG_DEBUG("Going to initialize the EFInterface"); 
    std::string full_libname = "lib" + m_interface_library_name + ".so";
    m_efdfinterface_library = boost::dll::shared_library(full_libname, boost::dll::load_mode::type::search_system_folders | boost::dll::load_mode::type::rtld_global);
    using cEventHandler = std::unique_ptr<daq::df_ef_interface::EventHandler> (const boost::property_tree::ptree&);
    std::function<cEventHandler> cs;
    try {
      cs = m_efdfinterface_library.get<cEventHandler>("createEventHandler");
    } catch (std::exception & ex) {
      ATH_MSG_ERROR("Cant load function createEventHandler. Are you trying to use incorrect library? - " << ex.what());
      throw; 
    }
    //Add Tree
    boost::property_tree::ptree configTree = prepInterfacePTree();
    std::ostringstream oss;
    boost::property_tree::write_json(oss, configTree, true);
    ATH_MSG_INFO("EFInterface configuration:\n" << oss.str());
    try {
      m_eventHandler = cs(configTree);
    } catch (std::exception & ex) {
      ATH_MSG_ERROR("Cant create EventHandler from DataSource library. Are you trying to use incorrect configuration? - " << ex.what());
      throw;
    }
    // Going to open the EventHandler connection
    try {
      ATH_MSG_DEBUG("Opening EventHandler");
      m_eventHandler->open();
    } 
    catch (daq::df_ef_interface::CommunicationError & ex) {
      ATH_MSG_ERROR("CommunicationError while opening EventHandler: " << ex.what());
      throw;
    } 
    catch (std::exception & ex) {
      ATH_MSG_ERROR("Exception while opening EventHandler: " << ex.what());
      throw;
    } 
    catch (...) {
      ATH_MSG_ERROR("Unknown exception while opening EventHandler");
      throw;
    }
  }
}

StatusCode EFInterfaceSvc::stop()
{
  ATH_MSG_DEBUG("EFInterfaceSvc stopped");
  // Close the EventHandler connection
  if (!m_eventHandler) {
    ATH_MSG_DEBUG("EventHandler is not initialized, probably I'm in the mother process.");
  }
  else
  {
    try {
      m_eventHandler->close();
    }
    catch (daq::df_ef_interface::CommunicationError & ex) {
      ATH_MSG_ERROR("CommunicationError while closing EventHandler: " << ex.what());
      throw;
    }
    catch (std::exception & ex) {
      ATH_MSG_ERROR("Exception while closing EventHandler: " << ex.what());
      throw;
    }
    catch (...) {
      ATH_MSG_ERROR("Unknown exception while closing EventHandler");
      throw;
    }
  }
  ATH_MSG_INFO("EFInterfaceSvc Stopping. Event processing summary:");
  ATH_MSG_INFO("Processed Events: " << m_processedEvents);
  ATH_MSG_INFO("Accepted Events: " << m_acceptedEvents);
  ATH_MSG_INFO("Rejected Events: " << m_rejectedEvents);
  return StatusCode::SUCCESS;
}

StatusCode EFInterfaceSvc::finalize()
{
  ATH_MSG_DEBUG("EFInterfaceSvc finalized");
  m_eventHandler.reset();
  m_efdfinterface_library.unload();
  return StatusCode::SUCCESS;
}

void EFInterfaceSvc::eventDone(std::unique_ptr<uint32_t[]> rawEventPtr)
{
  eformat::read::FullEventFragment ev(rawEventPtr.get());
  m_processedEvents++;
  bool accepted = ((ev.nstream_tag() > 0)?true:false);
  ATH_MSG_DEBUG("Number of stream tags: " <<  ev.nstream_tag());
  if(accepted){
    ATH_MSG_DEBUG("Event " << ev.lvl1_id() << " accepted");
    auto result = m_eventHandler->accept(ev.lvl1_id(), std::move(rawEventPtr));
    m_acceptedEvents++;
    //TODO: decide if we have to check for the future result
  }else{
    ATH_MSG_DEBUG("Event " << ev.lvl1_id() << " rejected");
    auto result = m_eventHandler->reject(ev.lvl1_id());
    m_rejectedEvents++;
  }
}

EFInterfaceSvc::Status EFInterfaceSvc::getNext(std::unique_ptr<uint32_t[]>& rawEventPtr)
{
  try{
    //check if we have a getNext call already pending
    std::future<std::unique_ptr<uint32_t []>> future;
    {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      if (m_getNextFuture.empty()){
        ATH_MSG_DEBUG("No pending getNext call, creating a new one");
        future = m_eventHandler->getNext();
      } else {
        ATH_MSG_DEBUG("Pending getNext call found, using it");
        future = std::move(m_getNextFuture.front());
        m_getNextFuture.pop();
      }
    }
    // Wait for the future to be ready
    auto status = future.wait_for(std::chrono::milliseconds(m_getNextTimeout));
    if (status == std::future_status::ready) {
      auto result = future.get();
      rawEventPtr = std::move(result);
      return Status::OK; 
    } else {
      // Future is not ready, return NO_EVENT
      ATH_MSG_DEBUG("getNext timed out, returning NO_EVENT");
      rawEventPtr = nullptr;
      {
        //Add the future to the queue for later access
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_getNextFuture.push(std::move(future));
      }
      return Status::NO_EVENT;
    }
  }
  catch (daq::df_ef_interface::NoMoreEvents &ex){
    ATH_MSG_DEBUG("NoMoreEvents, returning");
    return Status::STOP;
  }
  catch (daq::df_ef_interface::CommunicationError &ex){
    ATH_MSG_DEBUG("CommunicationError received from EFInterface, returning NO_EVENT");
    return Status::NO_EVENT;
  }
  catch (std::exception &ex){
    ATH_MSG_ERROR("EFInterface: caught exception: \""<<ex.what()<<"\" throwing!");
    throw; 
  }
  catch(...) {
    ATH_MSG_ERROR("EFInterface: caught very unknown exception");
    throw;
  }
}

boost::property_tree::ptree EFInterfaceSvc::prepInterfacePTree()
{
  boost::property_tree::ptree configTree;
    // Top-level values
  configTree.put("name", "DFEFInterfaceSvc");
  configTree.put("stride", m_stride.value()); // NB: if this is set to 0 it will cause a seg fault! 
  configTree.put("fileOffset", m_fileOffset.value());
  configTree.put("numEvents", m_numEvents.value());
  configTree.put("skipEvents", m_skipEvents.value());
  configTree.put("loopOverFiles", m_loopOverFiles.value() ? "true" : "false");  // as string
  configTree.put("outputFileName", m_outputFileName.value());
  // File list
  boost::property_tree::ptree fileList;
  for (const std::string& fname : m_files.value()) {
    boost::property_tree::ptree fileNode;
    fileNode.put("", fname);
    fileList.push_back(std::make_pair("file", fileNode));
  }
  configTree.add_child("fileList", fileList);
  // Run parameters
  boost::property_tree::ptree runParams;
  runParams.put("run_number", m_runNumber.value());
  runParams.put("trigger_type", m_triggerType.value());
  runParams.put("beam_type", m_beamType.value());
  runParams.put("beam_energy", m_beamEnergy.value());
  runParams.put("det_mask", m_detMask.value());
  runParams.put("T0_project_tag", m_T0_project_tag.value());
  runParams.put("stream", m_stream.value());
  runParams.put("lumiblock", m_lumiblock.value());
  configTree.add_child("RunParams", runParams);

  return configTree;
}    
