/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGDFEMULATOR_FILEREADERWRITER_H_
#define TRIGDFEMULATOR_FILEREADERWRITER_H_

#include <memory>
#include <vector>
#include <exception>

#include <boost/property_tree/ptree.hpp>

#include "EventStorage/DataReader.h"
#include "EventStorage/pickDataReader.h"
#include "EventStorage/DataWriter.h"

#include "ers/ers.h"

namespace DFEF {

class NoMoreEventsInFile: public std::exception { };

class ProblemReadingFromFile: public std::exception { };

class BadConfiguration: public std::exception { };

/** @class FileReaderWriter 
 *  @brief Class that handles reading/writing to ATLAS Event files 
 **/
class FileReaderWriter {
 public:
  FileReaderWriter(const boost::property_tree::ptree & tree);
  ~FileReaderWriter() {};

  //! Print configuration of the FileReaderWriter
  void print();

  //! Get serialized next event and put the FullEventFragment in the memory location
  std::unique_ptr<uint32_t[]> getNextEvent();

  //! Write event to file
  void writeEvent(const unsigned int & size_in_bytes, const uint32_t *event);

 private:
  //! Read the current Event from File
  std::unique_ptr<uint32_t[]> getEventFromFile();

  // Reader methods and variables
  bool nextFile();
  void skipEvents(uint num);
  std::vector<std::string> m_fileNames;
  std::unique_ptr<EventStorage::DataReader> m_currReader;
  int m_nMaxEvents;  // Maximum events to read
  int m_nEvents;  // Current number of event that is read
  uint m_nSkip;  // Number of events to skip
  bool m_loopFiles;
  int m_stride;
  int m_currFile;
  int m_currEventInFile;

  // Writer methods and variables
  std::unique_ptr<EventStorage::DataWriter> m_writer;
  std::string m_outFileName;
};
}  // namespace DFEF
#endif

