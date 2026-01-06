/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <boost/property_tree/ptree.hpp>
#include "EFInterfaceEmulator.h"
#include "eformat_utils.h"
#include <cstdlib>

extern "C" std::unique_ptr<daq::df_ef_interface::EventHandler> createEventHandler(const boost::property_tree::ptree &conf){
  std::unique_ptr<daq::df_ef_interface::EventHandler> s(new DFEF::EFInterfaceEmulator(conf));
  return s;
}

DFEF::EFInterfaceEmulator::EFInterfaceEmulator(const boost::property_tree::ptree &cargs)
{
  ERS_DEBUG(2, "Initializing EFInterfaceEmulator");
  try {
    m_name = cargs.get<std::string>("name");
  } 
  catch (const boost::property_tree::ptree_error &e) {
    throw std::runtime_error("Failed to get 'name' from configuration: " + std::string(e.what()));
  }
  {
    std::lock_guard<std::mutex> lock(m_RWMutex);
    m_file_rw = std::make_unique<FileReaderWriter>(cargs);
  }
  // TODO: read ptree to get the compression type and level
  m_comp = eformat::UNCOMPRESSED;
  m_compLevel = 0;
}

void DFEF::EFInterfaceEmulator::open(){
  //Starting input and output threads
    ERS_DEBUG(2, "Starting input and output threads");
    m_inputThread = std::make_unique<HLT::LoopThread>([this]{return inputThreadCallback();},-1);
    m_outputThread = std::make_unique<HLT::LoopThread>([this]{return outputThreadCallback();}, -1);
    m_inputThread->start();
    m_outputThread->start();
    ERS_DEBUG(2, "Input and output threads started");
}

void DFEF::EFInterfaceEmulator::close(){
  ERS_DEBUG(2, "Closing EFInterfaceEmulator");
  m_inputThread->stop();
  m_outputThread->stop();
  ERS_DEBUG(2, "Threads stopped, waiting for them to finish"); 
  m_inputThread->wait();
  m_outputThread->wait();
  ERS_DEBUG(2, "EFInterfaceEmulator closed and threads stopped");
  {
    std::lock_guard<std::mutex> lock(m_RWMutex);
    m_file_rw.reset();
  }
}

std::future<std::unique_ptr<uint32_t[]>> DFEF::EFInterfaceEmulator::getNext() {
  ERS_DEBUG(2,  "EFInterfaceEmulator getNext called");
  std::promise<std::unique_ptr<uint32_t[]>> p;
  auto fut = p.get_future();
  {
    std::lock_guard<std::mutex> lock(m_iQMutex);
    m_inputQueue.push(std::move(p));
  }
  ERS_DEBUG(2,  "Notifying input thread");
  m_inputThread->cond().notify_one(); 
  return fut;
}

std::future<void> DFEF::EFInterfaceEmulator::accept(uint64_t l0id, std::unique_ptr<uint32_t[]> hltr) {
  // TODO: here I have to keep track of the events that I read. When event done is received, I have to assemble the full event and write it to the output.
  std::promise<void> p;
  // Create a pair, push the pair in the inputThreadQueue, and notify the input thread
  std::pair<uint64_t, std::unique_ptr<uint32_t[]>> eventPair(l0id, std::move(hltr));
  {
    std::lock_guard<std::mutex> lock(m_oQMutex);
    m_outputQueue.push(std::move(eventPair));
  } 
  m_outputThread->cond().notify_one();
  p.set_value();
  return p.get_future();
}

std::future<void> DFEF::EFInterfaceEmulator::reject(uint64_t l0id){
    ERS_DEBUG(2, "Rejecting event with L1ID: " << l0id);
    ERS_DEBUG(2, "m_events size before erase: " << m_events.size());
    {
      std::lock_guard<std::mutex> lock(m_mMutex);
      m_events.erase(l0id);
    }
    std::promise<void> p;
    p.set_value();
    return p.get_future(); 
}

DFEF::EFInterfaceEmulator::~EFInterfaceEmulator() {}

void DFEF::EFInterfaceEmulator::inputThreadCallback() {
  ERS_DEBUG(2, "Input thread started");
  while (true) {
    std::promise<std::unique_ptr<uint32_t[]>> p;
    {
      std::lock_guard<std::mutex> lock(m_iQMutex);
      ERS_DEBUG(2, "m_inputQueue size: " << m_inputQueue.size());
      if (m_inputQueue.empty()) {
        ERS_DEBUG(2, "Input queue is empty, breaking...");
        break; 
      }
      p = std::move(m_inputQueue.front());
      m_inputQueue.pop();
    }
    // Get the event from file, create a copy of it to store in the map, and return the original pointer to the promise
    ERS_DEBUG(2, "Reading next event from file");
    std::unique_ptr<uint32_t[]> event = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_RWMutex);
      try {
        //FIXME: Sleep for random duration, only for debugging
        //auto sleepDuration = (std::rand() % 100) * 10;
        //ERS_DEBUG(2, "Sleeping for " << sleepDuration << " milliseconds");
        //std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
        event = std::move(m_file_rw->getNextEvent());
      } catch (const DFEF::NoMoreEventsInFile &ex) {
        ERS_DEBUG(2, "No more events in file");  
        p.set_exception(std::make_exception_ptr(daq::df_ef_interface::NoMoreEvents("No more events in file")));
        continue;
      } catch (DFEF::ProblemReadingFromFile &ex) {
        ERS_DEBUG(2, "Problem reading from file: " << ex.what());
        p.set_exception(std::make_exception_ptr(daq::df_ef_interface::CommunicationError("Problem reading from file")));
        continue;
      }
    }
    ERS_DEBUG(2, "Event read");
    eformat::read::FullEventFragment fullEvent(event.get());
    uint32_t nwords = event[1];
    ERS_DEBUG(2, "Copying full event with L0ID: " << fullEvent.lvl1_id() << ", size: " << nwords * sizeof(uint32_t) << " bytes");
    std::unique_ptr<uint32_t[]> eventCopy(new uint32_t[nwords]);
    std::memcpy(eventCopy.get(), event.get(), nwords * sizeof(uint32_t));
    {
      std::lock_guard<std::mutex> lock(m_mMutex);
      m_events.emplace(fullEvent.lvl1_id(), std::move(eventCopy)); 
    }
    p.set_value(std::move(event));
  }
}

void DFEF::EFInterfaceEmulator::outputThreadCallback() {
  ERS_DEBUG(2, "Output thread started");
  while (true) {
    std::pair<uint64_t, std::unique_ptr<uint32_t[]>> eventPair;
    {
      std::lock_guard<std::mutex> lock(m_oQMutex);
      ERS_DEBUG(2, "m_outputQueue size: " << m_outputQueue.size());
      if (m_outputQueue.empty())
      {
        ERS_DEBUG(2, "Output queue is empty, breaking...");
        break; 
      }
      eventPair = std::move(m_outputQueue.front());
      m_outputQueue.pop();
    }
    std::unique_ptr<uint32_t[]> fullEvent = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_mMutex);
      auto it = m_events.find(eventPair.first);
      if (it != m_events.end()) {
        fullEvent = std::move(it->second); 
        m_events.erase(it);
      } else {
       throw std::runtime_error("Missing the FullEvent copy for event accepted with L0ID: " + std::to_string(eventPair.first)); 
      }
    }
    //FIXME Remove this, just for debugging
    std::unique_ptr<uint32_t[]> hltResult = std::move(eventPair.second);
    uint32_t nwords = hltResult[1];
    eformat::read::FullEventFragment fullEventFragment(fullEvent.get());
    ERS_DEBUG(2, "Processing HLTResult with L0ID: " << fullEventFragment.lvl1_id() << ", size: " << nwords * 4 << " bytes");
    // Merge HLTResult
    auto finalEvent = DFEF::merge_hltresult_with_input_event(hltResult.get(), fullEvent.get(), m_comp, m_compLevel);
    auto finalSize = finalEvent.size();
    auto sizeInBytes = finalSize*4; 
    {
      ERS_DEBUG(2, "Writing fullEventFragment with L0ID: " << eventPair.first << " of size: " << sizeInBytes << " bytes");
      std::lock_guard<std::mutex> lock(m_RWMutex);
      m_file_rw->writeEvent(sizeInBytes, finalEvent.data());
    }
    ERS_DEBUG(2, "Event with L0ID: " << eventPair.first << " written to file");
    ERS_DEBUG(2, "m_events size after erase: " << m_events.size());
  }
  ERS_DEBUG(2, "Output thread finished processing.");
}
