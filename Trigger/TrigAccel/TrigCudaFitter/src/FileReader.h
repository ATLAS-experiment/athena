// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef __FILEREADER_H__
#define __FILEREADER_H__

#include <fstream>
#include <vector>

class EventData;

class FileReader {
  public:
    FileReader(){};
    ~FileReader(){};

    int readEvents(std::ifstream&, std::vector<EventData*>&);
};

#endif // __FILEREADER_H__
