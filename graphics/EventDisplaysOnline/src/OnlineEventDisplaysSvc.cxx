/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "EventDisplaysOnline/OnlineEventDisplaysSvc.h"
#include "RootUtils/PyAthenaGILStateEnsure.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "xAODEventInfo/EventInfo.h"

#include <iostream>
#include <sys/types.h>
#include <grp.h>

#include <cstdlib>  // For std::rand() and std::srand()
#include <sys/stat.h>  //mkdir
#include <unistd.h>  //chown
#include <stdexcept>
#include "Python.h"

namespace{
  template<typename ...Ptr>
  bool
  anyNullPtr(Ptr&&...p){
    return ((p==nullptr) or ...);
  }
}

OnlineEventDisplaysSvc::OnlineEventDisplaysSvc( const std::string& name,
                          ISvcLocator* pSvcLocator ) :
  base_class(name, pSvcLocator){}

void OnlineEventDisplaysSvc::beginEvent(){

  SG::ReadHandle<xAOD::EventInfo> evt (m_evt);
  if (!evt.isValid()) {
    ATH_MSG_FATAL("Could not find event info");
  }
  std::vector<std::string> streams;

  m_eventNumber = evt->eventNumber();
  m_runNumber = evt->runNumber();

  //Check what trigger streams were fired, if in list of desired
  //streams to be reconstructed pick one randomly
  for (const xAOD::EventInfo::StreamTag& tag : evt->streamTags()){
    ATH_MSG_DEBUG("A trigger in stream " << tag.type() << "_" << tag.name() << " was fired in this event.");
    std::string stream_fullname = tag.type() + "_" + tag.name();

    if (m_streamsWanted.empty()) {
      ATH_MSG_WARNING("You have not requested any specific streams, going to allow all streams");
      streams.emplace_back(stream_fullname);
    }

    else{
      //If the stream is in the list of streams requested, add it
      if(std::find(m_streamsWanted.begin(), m_streamsWanted.end(), tag.name()) != m_streamsWanted.end()){
        streams.emplace_back(stream_fullname);
      }
      bool isPublic = false;
      //if the stream is not in the list of public streams wanted, continue
      if(std::find(m_publicStreams.begin(), m_publicStreams.end(), tag.name()) == m_publicStreams.end()) continue;
      // Acquire the Global Interpreter Lock (GIL) to ensure thread safety when interacting with Python objects
      RootUtils::PyGILStateEnsure ensure;
      // Convert the project tag string to a Python Unicode object
      std::string tag = m_projectTag;
      PyObject* pProjectTag = PyUnicode_FromString(tag.c_str());
      if (!pProjectTag) {
        // Error handling: Print Python exception if conversion fails
        PyErr_Print();
        ATH_MSG_WARNING("Failed to create Python Unicode object from project tag");
      } else {
        // Import the Python module
        PyObject* pHelper = PyImport_ImportModule("EventDisplaysOnline.EventDisplaysOnlineHelpers");
        if (!pHelper) {
          // Error handling: Print Python exception if import fails
          PyErr_Print();
          ATH_MSG_WARNING("Failed to import EventDisplaysOnline.EventDisplaysOnlineHelpers module");
        } else {
          // Get the "EventCanBeSeenByPublic" function from the module
          PyObject* EventCanBeSeenByPublic = PyObject_GetAttrString(pHelper, "EventCanBeSeenByPublic");
          if (!EventCanBeSeenByPublic || !PyCallable_Check(EventCanBeSeenByPublic)) {
            // Error handling: Print warning if function not found or not callable
            ATH_MSG_WARNING("Could not find or call EventCanBeSeenByPublic function in EventDisplaysOnline.EventDisplaysOnlineHelpers module");
          } else {
            // Call the "EventCanBeSeenByPublic" function with the project tag as argument
            PyObject* result = PyObject_CallFunctionObjArgs(EventCanBeSeenByPublic, pProjectTag, NULL);
            if (!result) {
              PyErr_Print();
              ATH_MSG_WARNING("Failed to call EventCanBeSeenByPublic function");
            } else {
              // Convert the result to a boolean value
              isPublic = PyObject_IsTrue(result);
              Py_DECREF(result); // Decrement reference count of the result object
            }
          }
          Py_XDECREF(EventCanBeSeenByPublic);
          Py_DECREF(pHelper);
        }
        Py_DECREF(pProjectTag);
      }
      if(isPublic){
        streams.emplace_back("Public");
        ATH_MSG_DEBUG("Can send event to public stream");
      }
    }
  }
  for (const std::string& stream : streams){
    ATH_MSG_DEBUG("streams where a trigger fired and in your desired streams list: " << stream);
  }
  std::random_shuffle(streams.begin(), streams.end());
  //Pick the first stream as the output directory
  if(!streams.empty()){
    m_outputStreamDir = streams[0];
  }
  else{
    ATH_MSG_WARNING("Cannot find a stream adding to .Unknown directory");
    m_outputStreamDir = ".Unknown";
  }

  m_entireOutputStr = m_outputDirectory + "/" + m_outputStreamDir;
  m_FileNamePrefix = m_entireOutputStr + "/JiveXML";;

  gid_t zpgid = setOwnershipToZpGrpOrDefault();
  createWriteableDir(m_outputDirectory, zpgid);
  createWriteableDir(m_entireOutputStr, zpgid);
}

void OnlineEventDisplaysSvc::endEvent(){
  RootUtils::PyGILStateEnsure ensure;
  PyObject* pCheckPair = PyBool_FromLong(m_CheckPair);
  PyObject* pBeamSplash = PyBool_FromLong(m_BeamSplash);
  PyObject* pMaxEvents = PyLong_FromLong(m_maxEvents);
  const char* cString = m_entireOutputStr.c_str();
  PyObject* pDirectory = PyUnicode_FromString(cString);
  PyObject* pArgs = PyTuple_Pack(4, pDirectory, pMaxEvents, pCheckPair,pBeamSplash);
  PyObject* pModule = PyImport_ImportModule(const_cast< char* >("EventDisplaysOnline.EventUtils"));
  if(!pCheckPair || !pBeamSplash || !pMaxEvents || !pDirectory || !pArgs){
    PyErr_Print();
    ATH_MSG_WARNING("Failed to create Python Unicode object");}
  if (!pModule) {
    PyErr_Print();
    ATH_MSG_WARNING("Failed to import EventDisplaysOnline.EventUtils module");
  } else {
    ATH_MSG_DEBUG("Successfully imported EventDisplaysOnline.EventUtils module");

    // Get the "cleanDirectory" function from the module
    PyObject* cleanDirectory = PyObject_GetAttrString(pModule, "cleanDirectory");
    if (!cleanDirectory || !PyCallable_Check(cleanDirectory)) {
      ATH_MSG_WARNING("Could not find or call cleanDirectory function in EventDisplaysOnline.EventUtils module");
    } else {
      ATH_MSG_DEBUG("Found cleanDirectory function in EventDisplaysOnline.EventUtils module");

      // Call the "cleanDirectory" function with the provided arguments
      PyObject_CallObject(cleanDirectory, pArgs);
      if (PyErr_Occurred()) {
        PyErr_Print();
      }
    }
    if (anyNullPtr(cleanDirectory)){
      throw std::runtime_error("OnlineEventDisplaysSvc::endEvent: Py_DECREF on nullptr argument");
    }
    Py_DECREF(cleanDirectory);
  }

  if(m_BeamSplash){
    std::string JiveXMLFileName ="JiveXML_"+ std::to_string(m_runNumber)+"_"+std::to_string(m_eventNumber)+".xml";
    const char* JiveXMLFileName_cString = JiveXMLFileName.c_str();
    PyObject* pJiveXMLFileName = PyUnicode_FromString(JiveXMLFileName_cString);
    PyObject* pArgs_zip = PyTuple_Pack(2, pDirectory, pJiveXMLFileName);
    PyObject* zipXMLFile = PyObject_GetAttrString(pModule, "zipXMLFile");
    if (!zipXMLFile) {
      PyErr_Print();
      ATH_MSG_WARNING("Failed to import EventDisplaysOnline.EventUtils.zipXMLFile");
    } else {
      PyObject_CallObject(zipXMLFile, pArgs_zip);
    }
    if (anyNullPtr(pJiveXMLFileName, zipXMLFile, pArgs_zip)){
      throw std::runtime_error("OnlineEventDisplaysSvc::endEvent: Py_DECREF on nullptr argument");
    }
    Py_DECREF(pJiveXMLFileName);
    Py_DECREF(zipXMLFile);
    Py_DECREF(pArgs_zip);
  }
  if (anyNullPtr(pModule, pArgs, pCheckPair, pMaxEvents, pDirectory)){
    throw std::runtime_error("OnlineEventDisplaysSvc::endEvent: Py_DECREF on nullptr argument");
  }
  Py_DECREF(pModule);
  Py_DECREF(pArgs);
  Py_DECREF(pCheckPair);
  Py_DECREF(pMaxEvents);
  Py_DECREF(pDirectory);
}

std::string OnlineEventDisplaysSvc::getFileNamePrefix(){
  return m_FileNamePrefix;
}

std::string OnlineEventDisplaysSvc::getEntireOutputStr(){
  return m_entireOutputStr;
}

std::string OnlineEventDisplaysSvc::getStreamName(){
  return m_outputStreamDir;
}
void OnlineEventDisplaysSvc::createWriteableDir(const std::string& directory, gid_t zpgid){

  const char* char_dir = directory.c_str();

  if (access(char_dir, F_OK) == 0) {
    struct stat directoryStat;
    if (stat(char_dir, &directoryStat) == 0 && S_ISDIR(directoryStat.st_mode) &&
        access(char_dir, W_OK) == 0) {
      ATH_MSG_DEBUG("Going to write file to existing directory: " << directory);
      if (directoryStat.st_gid != zpgid) {
        ATH_MSG_DEBUG("Setting group to 'zp' for directory: " << directory);
        chown(char_dir, -1, zpgid);
      }
    } else {
      ATH_MSG_WARNING("Directory '" << directory << "' is not usable, trying next alternative");
    }
  } else {
    try {
      auto rc = mkdir(char_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      if (rc !=0){
        ATH_MSG_ERROR("mkdir failed for directory: " << directory);
      }
      rc = chown(char_dir, -1, zpgid);
      if (rc !=0){
         ATH_MSG_ERROR( "chown failed for directory: " << directory);
      }
      ATH_MSG_DEBUG("Created output directory " << directory);
    } catch (const std::system_error& err) {
      ATH_MSG_ERROR( "Failed to create output directory " << directory << err.what());
    }
  }
}

gid_t OnlineEventDisplaysSvc::setOwnershipToZpGrpOrDefault(){
  gid_t zpgid;
  struct group grp;
  struct group* grp_result;
  char buf[1024];  // sysconf(_SC_GETGR_R_SIZE_MAX)
  (void)getgrnam_r("zp", &grp, buf, sizeof(buf), &grp_result);
  if (grp_result != nullptr) {
    zpgid = grp.gr_gid;
  } else {
    ATH_MSG_DEBUG("If running on private machine, zp group might not exist. Just set to the likely value 1307.");
    zpgid = 1307;
  }
  return zpgid;
}

StatusCode OnlineEventDisplaysSvc::initialize(){

  ATH_MSG_DEBUG("Initializing " << name());
  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
  ATH_CHECK( incSvc.retrieve() );
  ATH_MSG_DEBUG("You have requested to only output JiveXML and ESD files when a trigger in the following streams was fired: ");
  for (const std::string& stream : m_streamsWanted){
    ATH_MSG_DEBUG(stream);
  }
  if(m_BeamSplash){
    m_CheckPair = false;
  }  
  incSvc->addListener( this, "BeginEvent");
  incSvc->addListener( this, "EndEvent");

  ATH_CHECK( m_evt.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode OnlineEventDisplaysSvc::finalize(){

  ATH_MSG_DEBUG("Finalizing " << name());
  return StatusCode::SUCCESS;
}

void OnlineEventDisplaysSvc::handle( const Incident& incident ){
  ATH_MSG_DEBUG("Received incident " << incident.type() << " from " << incident.source() );
  if ( incident.type() == IncidentType::BeginEvent && incident.source() == "BeginIncFiringAlg" ){
    beginEvent();
  }
  if (incident.type() == "EndEvent" && incident.source() == "EndIncFiringAlg"){
    endEvent();
  }
}
