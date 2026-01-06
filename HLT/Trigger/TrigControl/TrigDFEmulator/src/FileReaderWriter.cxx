/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FileReaderWriter.h"
#include "EventStorage/RawFileName.h"

DFEF::FileReaderWriter::FileReaderWriter(const boost::property_tree::ptree &args) {
  ERS_DEBUG(1, "Configuring FileReaderWriter");
  m_stride=args.get("stride",1);
  m_currFile=args.get("fileOffset",-1);
  m_nMaxEvents=args.get("numEvents",-1);
  m_nSkip=args.get("skipEvents",0);
  m_loopFiles=(args.get("loopOverFiles","false")!="False");
  m_nEvents=0;
  m_currEventInFile=0;
  try{
    auto of=args.get_child("outputFileName");
    m_outFileName=std::string(of.data());
    ERS_LOG("Output file name: \"" << m_outFileName << "\"");
  }catch(boost::property_tree::ptree_bad_path &ex){
    ERS_LOG("Failed to get outputFileName");
  }
  auto r(args.get_child("fileList").equal_range("file"));
  for (auto it = r.first; it != r.second; ++it) {
    m_fileNames.push_back(it->second.get_value<std::string>());
    ERS_LOG(" Adding file "+it->second.get_value<std::string>());
  }
  if(m_fileNames.size()==0){
    ERS_LOG("No input files specified");
    throw BadConfiguration();
  }

  // Get correct run number and detector_mask from prepareForRun ptree
  auto run_params = args.get_child("RunParams");
  std::string detmask = run_params.get("det_mask", "");
  uint64_t detmask_LS(0), detmask_MS(0);
  if (detmask.size() == 32) {  // Detmask is supposed to be a 32 digit hex number. Otherwise ignore
    detmask_LS = std::stoull(detmask.substr(16, 32), 0, 16);
    detmask_MS = std::stoull(detmask.substr(0, 16), 0, 16);
  }
  if(!m_outFileName.empty()){
    std::unique_ptr<EventStorage::run_parameters_record> runParams;
    runParams=std::make_unique<EventStorage::run_parameters_record>();
    runParams->run_number = std::stoul(run_params.get("run_number", "0"));
    runParams->max_events = 0;
    runParams->rec_enable = 0;
    runParams->trigger_type = std::stoul(run_params.get("trigger_type", "0"));
    runParams->detector_mask_LS = detmask_LS;
    runParams->detector_mask_MS = detmask_MS;
    runParams->beam_type = std::stoul(run_params.get("beam_type", "0"));
    runParams->beam_energy = std::stoul(run_params.get("beam_energy", "0"));
    ERS_LOG("Opening outputfile with base name: " << m_outFileName);

    std::string path = ".";  // use work Directory
    std::string project = run_params.get("T0_project_tag", "");
    std::string streamType = "unknown";
    std::string streamName = "SingleStream";
    std::string stream = run_params.get("stream", "");
    unsigned int lumiBlockNumber = run_params.get("lumiblock", 0);;
    std::string applicationName = std::string("DFEF_") + m_outFileName;

    std::vector<std::string> fmdStrings = {};

    ERS_LOG("Going to open raw file");
    auto rawfilename = boost::shared_ptr<daq::RawFileName>
    (new daq::RawFileName(project, runParams->run_number,streamType, streamName, lumiBlockNumber, applicationName));

    //FIXME just for debug
    ERS_LOG("Raw file name: ");
    rawfilename->print();

    m_writer=std::make_unique<EventStorage::DataWriter>(path,
							rawfilename,
							*runParams,
                                                        project,
                                                        streamType,
                                                        streamName,
                                                        stream,
                                                        lumiBlockNumber,
                                                        applicationName,
							fmdStrings);
    m_writer->setMaxFileMB(2000);
  }
  nextFile(); // Open next file

  uint eventsInFile=m_currReader->eventsInFile();
  ERS_LOG("Events in file: " << eventsInFile);
  // Skip events if necessary
  if (m_nSkip >= eventsInFile ) {
      ERS_LOG("Events to skip: " + std::to_string(m_nSkip) +  ", total events in file: " + std::to_string(eventsInFile));
      throw BadConfiguration();
  }
  if (m_nSkip) {
    ERS_LOG("Skipping " << m_nSkip << " events");
    skipEvents(m_nSkip);
  }

  print();
}

void DFEF::FileReaderWriter::print() {
  std::cout << "FileReaderWriter: " << std::endl;
  std::cout << "  Loop?               : " << m_loopFiles           << std::endl;
  std::cout << "  stride              : " << m_stride               << std::endl;
  std::cout << "  currEventInFile     : " << m_currEventInFile      << std::endl;
  std::cout << "  nSkip               : " << m_nSkip      << std::endl;
  std::cout << "  nMaxEvents          : " << m_nMaxEvents      << std::endl;
  for (auto & file : m_fileNames) {
    std::cout << "    file: " << file  << std::endl;
  }
}

void DFEF::FileReaderWriter::skipEvents(uint num) {
  ERS_LOG("Going to skip " << num << " events in file: " << m_currFile);
  for (uint i = 0; i < num; i++) {
    auto blob = getEventFromFile();
  }
  m_currEventInFile += num;
}

std::unique_ptr<uint32_t[]> DFEF::FileReaderWriter::getNextEvent(){
  ERS_DEBUG(2, "FileReaderWriter::Enter, event: " << m_nEvents);
  if((m_nMaxEvents>0) && (m_nEvents>=m_nMaxEvents)){
    std::string message = "Event count reached, max events to read: " + std::to_string(m_nMaxEvents);
    ERS_DEBUG(2, message);
    throw NoMoreEventsInFile();
  }

  uint eventsInFile=m_currReader->eventsInFile();
  std::unique_ptr<uint32_t[]> blob;
  if((uint)(m_currEventInFile+m_stride)<=eventsInFile){
    unsigned int target=m_currEventInFile+m_stride-1;
    ERS_DEBUG(2, "Current event in file: " << m_currEventInFile << ", stride: " << m_stride << ", target: " << target);
    skipEvents(target - m_currEventInFile);
    blob=getEventFromFile();
    m_currEventInFile++;
  }else{
    int newoffset=m_currEventInFile+m_stride-eventsInFile;
    if(nextFile()){
      eventsInFile=m_currReader->eventsInFile();
      if(eventsInFile>=(uint)newoffset){
        if(newoffset>1){
          ERS_DEBUG(2, "Skipping for offset " << newoffset-1 << " events in file: " << m_currFile);
          skipEvents(newoffset-1);
        }
        blob=getEventFromFile();
        m_currEventInFile=newoffset;
      }else{
        throw NoMoreEventsInFile();
      }
    }else{
      throw NoMoreEventsInFile();
    }
  }
  m_nEvents++;
  return blob;
}

std::unique_ptr<uint32_t[]> DFEF::FileReaderWriter::getEventFromFile(){
  ERS_DEBUG(2, "FileReaderWriter::getEventFromFile, event: " << m_nEvents << ", current file: " << m_currFile);
  char *buff = nullptr;
  unsigned int size=0;
  try{
    int error_code=m_currReader->getData(size,&buff);
    while (error_code == DRWAIT) {
      usleep(500000);
      ERS_DEBUG(2, " Waiting for more data.");
      error_code = m_currReader->getData(size, &buff);
    }
    if (error_code == DRNOOK) {
      ERS_DEBUG(2, " Reading of data NOT OK!");
      delete[] buff;
      throw ProblemReadingFromFile();
    }
    if (error_code == DROK) {
      ERS_DEBUG(2, " Event OK");
    }
  } catch(std::exception &ex){
    ERS_LOG("Failed reading event. Caught exception \""<<ex.what()<<"\"");
    std::cerr<<"Reading event failed. Caught exception \""<<ex.what()<<"\""<<std::endl;
  } catch (...) {
    ERS_LOG("Unexpected exception!");
  }
  uint32_t *blob=reinterpret_cast<uint32_t*>(buff);
  return std::unique_ptr<uint32_t[]>(blob);
}

bool DFEF::FileReaderWriter::nextFile(){
  if(m_fileNames.size()==0){
    throw std::runtime_error("No input files defined!");
  }
  ERS_DEBUG(2, "m_loopFiles: " << m_loopFiles);
  if(m_currFile>=(int)m_fileNames.size()){
    if(m_loopFiles){
      m_currFile=-1;
    }else{
      throw NoMoreEventsInFile();
    }
  }
  ERS_DEBUG(2, "Current file: " << m_currFile);
  if(m_currFile<(int)m_fileNames.size()){
    if(m_currReader){
      m_currReader.reset();
    }
    m_currFile++;
    if(m_currFile==(int)m_fileNames.size()){
      if(!m_loopFiles){
        throw NoMoreEventsInFile();
      }else{
        m_currFile=0;
      }
    }
    ERS_LOG("Opening file: " << m_fileNames.at(m_currFile));
    try{
      std::unique_ptr<EventStorage::DataReader> temp (pickDataReader(m_fileNames.at(m_currFile)));
      m_currReader = std::move(temp);
    }catch(ers::Issue &e){
      ERS_LOG("Caught issue "<<e.what());
    }
    if (!m_currReader) {
      ERS_LOG("Failed to open file \""<<m_fileNames.at(m_currFile)<<"\" (DataReader not created)");
      throw std::runtime_error("Can't open file");
    }
    if (!m_currReader->good()) {
      ERS_LOG("Failed to open file \""<<m_fileNames.at(m_currFile)<<"\" (DataReader->good()==false)");
      m_currReader.reset();
      throw std::runtime_error("Can't open file");
    }
    // Reset event index for the newly opened file
    m_currEventInFile = 0;
    ERS_LOG("Opened file \""<<m_fileNames.at(m_currFile)<<"\"");
    return true;
  }
  ERS_DEBUG(1,"m_currFile="<<m_currFile<<", m_fileNames.size()="<<m_fileNames.size());
  throw NoMoreEventsInFile();
}

void DFEF::FileReaderWriter::writeEvent(const unsigned int & size_in_bytes, const uint32_t *event){
  if (!m_outFileName.empty()) {
    ERS_DEBUG(2, "Writing event to disk, " << size_in_bytes << " bytes, " << *event);
    auto wres=m_writer->putData(size_in_bytes, event);
    if (wres) {
      ERS_DEBUG(2, "File writing failed, code: " << wres);
    }
  }
}
