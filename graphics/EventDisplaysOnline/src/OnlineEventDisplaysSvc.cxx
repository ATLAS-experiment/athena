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
#include <cstdlib>  // For std::rand() and std::srand()
#include "Python.h"

OnlineEventDisplaysSvc::OnlineEventDisplaysSvc( const std::string& name, 
			  ISvcLocator* pSvcLocator ) : 
  AthService(name, pSvcLocator){}

OnlineEventDisplaysSvc::~OnlineEventDisplaysSvc(){}

void OnlineEventDisplaysSvc::beginEvent(){

  SG::ReadHandle<xAOD::EventInfo> evt (m_evt);
  if (!evt.isValid()) {
    ATH_MSG_FATAL("Could not find event info");
  }
  std::vector<std::string> streams;

  ATH_MSG_INFO("You have requested to only output JiveXML and ESD files when a trigger in the following streams was fired: ");
  for (std::string stream : m_streamsWanted){
    ATH_MSG_INFO(stream);
  }
  m_eventNumber = std::to_string(evt->eventNumber());
  m_runNumber = std::to_string(evt->runNumber());

  //Check what trigger streams were fired, if in list of desired
  //streams to be reconstructed pick one randomly 
  for (const xAOD::EventInfo::StreamTag& tag : evt->streamTags()){
    ATH_MSG_INFO ("A trigger in stream " << tag.type() << "_" << tag.name() << " was fired in this event.");
    std::string stream_fullname = tag.type() + "_" + tag.name();

    if (m_streamsWanted.empty()) {
      ATH_MSG_INFO ("You have not requested any specific streams, going to allow all streams");
      streams.emplace_back(stream_fullname);
    }
    
    else{
      //If the stream is in the list of streams requested, add it
      if(std::find(m_streamsWanted.begin(), m_streamsWanted.end(), tag.name()) != m_streamsWanted.end()){
	streams.emplace_back(stream_fullname);
      }

      bool isPublicStream = std::find(m_publicStreams.begin(), m_publicStreams.end(), tag.name()) != m_publicStreams.end();
      if(m_sendToPublicStream && isPublicStream){
	streams.emplace_back("Public");
      }      
    }
  }
  for (std::string stream : streams){
    ATH_MSG_INFO("streams where a trigger fired and in your desired streams list: " << stream);
  }
  std::random_shuffle(streams.begin(), streams.end());
  //Pick the first stream as the output directory
  if(!streams.empty()){
    m_outputStreamDir = streams[0];
  }
  else{
    ATH_MSG_WARNING("Cannot find a stream adding to .Unknown directory");
    m_outputStreamDir = ".Unkown";
  }
  gid_t zpgid = setOwnershipToZpGrpOrDefault();
  m_entireOutputStr = m_outputDirectory + "/" + m_outputStreamDir;
  createWriteableDir(m_outputDirectory, zpgid);
  createWriteableDir(m_entireOutputStr, zpgid);
  
  std::string FileNamePrefix = m_entireOutputStr + "/JiveXML";
  m_FileNamePrefix = FileNamePrefix;
  ATH_MSG_INFO("in begin: " << m_entireOutputStr);
}

void OnlineEventDisplaysSvc::endEvent(){
  RootUtils::PyGILStateEnsure ensure;
  if(m_BeamSplash){
    m_CheckPair = false;
  }
  PyObject* pCheckPair = PyBool_FromLong(m_CheckPair); // Use 0 for False
  PyObject* pBeamSplash = PyBool_FromLong(m_BeamSplash);
  PyObject* pMaxEvents = PyLong_FromLong(m_maxEvents);
  const char* cString = m_entireOutputStr.c_str();
  PyObject* pDirectory = PyUnicode_FromString(cString);
  PyObject* pArgs = PyTuple_Pack(4, pDirectory, pMaxEvents, pCheckPair,pBeamSplash);
  PyObject* pModule = PyImport_ImportModule("EventDisplaysOnline.EventUtils");
  if ( pModule ) {
    PyObject* cleanDirectory = PyObject_GetAttrString(pModule, "cleanDirectory");
    if ( cleanDirectory ) {
      PyObject_CallObject(cleanDirectory, pArgs);
    }
    else {
      ATH_MSG_WARNING("Could not import EventDisplaysOnline.EventUtils.cleanDirectory");
    }
    Py_DECREF(cleanDirectory);
    if(m_BeamSplash){
      std::string JiveXMLFileName ="JiveXML_"+ m_runNumber+"_"+m_eventNumber+".xml";
      const char* JiveXMLFileName_cString = JiveXMLFileName.c_str();
      PyObject* pJiveXMLFileName = PyUnicode_FromString(JiveXMLFileName_cString);
      PyObject* pArgs_zip = PyTuple_Pack(2, pDirectory, pJiveXMLFileName);
      PyObject* zipXMLFile = PyObject_GetAttrString(pModule, "zipXMLFile");
      if ( zipXMLFile ) {
	PyObject_CallObject(zipXMLFile, pArgs_zip);
      }
      else {
	ATH_MSG_WARNING("Could not import EventDisplaysOnline.EventUtils.zipXMLFile");
      }
      Py_DECREF(pJiveXMLFileName);
      Py_DECREF(zipXMLFile);
      Py_DECREF(pArgs_zip);
      }
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
void OnlineEventDisplaysSvc::createWriteableDir(std::string directory, gid_t zpgid){

  const char* char_dir = directory.c_str();
  
  if (access(char_dir, F_OK) == 0) {
    struct stat directoryStat;
    if (stat(char_dir, &directoryStat) == 0 && S_ISDIR(directoryStat.st_mode) &&
	access(char_dir, W_OK) == 0) {
      ATH_MSG_INFO("Going to write file to existing directory: " << directory);
      if (directoryStat.st_gid != zpgid) {
	ATH_MSG_INFO("Setting group to 'zp' for directory: " << directory);
	chown(char_dir, -1, zpgid);
      }
    } else {
      ATH_MSG_INFO("Directory '" << directory << "' is not usable, trying next alternative");
    }
  } else {
    try {
      mkdir(char_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      chown(char_dir, -1, zpgid);
      ATH_MSG_INFO("Created output directory " << directory);
    } catch (const std::system_error& err) {
      std::cerr << "Failed to create output directory " << directory
		<< err.what() << std::endl;
    }
  }
}

gid_t OnlineEventDisplaysSvc::setOwnershipToZpGrpOrDefault(){
  gid_t zpgid;
  struct group* zp_group = getgrnam("zp");
  if (zp_group != nullptr) {
    zpgid = zp_group->gr_gid;
  } else {
    ATH_MSG_INFO("If running on private machine, zp group might not exist. Just set to the likely value 1307.");
    zpgid = 1307;
  }
  return zpgid;
}

StatusCode OnlineEventDisplaysSvc::initialize(){

  ATH_MSG_INFO("Initializing " << name());
  IIncidentSvc* incSvc = nullptr;
  ATH_CHECK( service("IncidentSvc",incSvc) );

  incSvc->addListener( this, "BeginEvent");
  incSvc->addListener( this, "StoreCleared");

  ATH_CHECK( m_evt.initialize() );
  
  return StatusCode::SUCCESS;
}

StatusCode OnlineEventDisplaysSvc::finalize(){

  ATH_MSG_INFO("Finalizing " << name());
  return StatusCode::SUCCESS;
}

void OnlineEventDisplaysSvc::handle( const Incident& incident ){
  ATH_MSG_INFO("Received incident " << incident.type() << " from " << incident.source() );
  if ( incident.type() == IncidentType::BeginEvent && incident.source() == "BeginIncFiringAlg" ){
    beginEvent();
    
  }
  if ( incident.type() == "StoreCleared" && incident.source() == "StoreGateSvc" ){
    endEvent();
  }
}

StatusCode OnlineEventDisplaysSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( IOnlineEventDisplaysSvc::interfaceID().versionMatch(riid) ) {
    *ppvInterface = dynamic_cast<IOnlineEventDisplaysSvc*>(this);
  } else {
    // Interface is not directly available : try out a base class
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}
