/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGDFEMULATOR_EFINTERFACEEMULATOR_H
#define TRIGDFEMULATOR_EFINTERFACEEMULATOR_H

#include <mutex>
#include <utility>
#include <queue>
#include <future>
#include <unordered_map>
#include <vector>

#include "ers/ers.h"
#include "eformat/eformat.h"
#include "eformat/index.h"
#include "eformat/write/eformat.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/FullEventFragmentNoTemplates.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include "df_ef_interface/df_ef_interface.h"
#include "TrigKernel/EventLoopUtils.h"
#include "FileReaderWriter.h"

/** @class EFInterfaceEmulator
 *  @brief A dataflow emulator class using the df_ef_interface for Event Filter testing purposes
 *
 * File access dataflow emulator, available as a library dynamically loaded by the EFInterfaceSvc
 * Directly runs inside the Athena process
 * Multithreadead safe, but doesn't support running Athena in multiprocess
 *  
 * Main thread manages interface calls and creates/returns futures 
 * Input and output threads set the promises from the future queues and executes the I/O operations
 **/

namespace DFEF{
  class EFInterfaceEmulator:public daq::df_ef_interface::EventHandler{

  public:

    EFInterfaceEmulator(const boost::property_tree::ptree &args);

    /*! \brief Opens the Session.
     */
    virtual void open() override; 

    /*! \brief Closes the Session.
     */
    virtual void close() override;

    /*! \brief Returns a pointer to the next \ref Event object to be processed.
    */
    virtual std::future<std::unique_ptr<uint32_t[]>> getNext() override;

    /*! \brief Marks the event as accepted by the High-Level Trigger.
     */
    virtual std::future<void> accept(uint64_t l0id, std::unique_ptr<uint32_t[]> hltr) override;

    /*! \brief Marks the event as rejected by the High-Level Trigger.
     */
    virtual std::future<void> reject(uint64_t l0id) override;

    virtual ~EFInterfaceEmulator();


  private:

    std::string m_name;
    std::unique_ptr<FileReaderWriter> m_file_rw;
    eformat::Compression m_comp;  //! Compression type of built event
    unsigned int m_compLevel;  //! Compression level of built event

    /// The method executed by the input handling thread
    void inputThreadCallback();
    /// The method executed by the output handling thread
    void outputThreadCallback();

    /// Input handling thread (triggers reading new events)
    std::unique_ptr<HLT::LoopThread> m_inputThread;
    /// Output handling thread (triggers post-processing of finished events)
    std::unique_ptr<HLT::LoopThread> m_outputThread;

    /// queue of promises for getNext() calls
    std::queue<std::promise<std::unique_ptr<uint32_t[]>>> m_inputQueue; 
    /// queue mutex
    std::mutex m_iQMutex;
  
    /// queue of promises for accept() calls
    std::queue<std::pair<uint64_t, std::unique_ptr<uint32_t[]>>> m_outputQueue; 
    /// queue mutex
    std::mutex m_oQMutex;

    /// Map of events read from the input (stores duplicates per L1ID)
    std::unordered_map<uint64_t, std::vector<std::unique_ptr<uint32_t[]>>> m_events;
    /// Map mutex
    std::mutex m_mMutex;

    /// ReadWrite mutex for the file reader/writer
    std::mutex m_RWMutex; 

  };
}
#endif
