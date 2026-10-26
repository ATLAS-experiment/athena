/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERSISTENCY_PARAMETERS_H
#define PERSISTENCY_PARAMETERS_H

#include "PersTestBase.h"

#include <map>
#include <string>


class TestDriver : public PersTestBase {
  public:
    TestDriver(const std::string& filename, const std::string& catname);

    int execute();

    void write();
    void read();

  private:
    std::string           m_fileName;
    std::map< std::string, std::string > m_parameters;
};

#endif
