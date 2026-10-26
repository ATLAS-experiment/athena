/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NOCLASSID_H
#define NOCLASSID_H

#include "PersTestBase.h"

#include <vector>
#include <string>

class Token;


class TestDriver : public PersTestBase {
  public:
    explicit TestDriver(const std::string& filename, const std::string& catname );

    int execute();

    void write();
    void read();

  private:
    std::string           m_fileName;
    int                   m_events;
    std::vector< Token* > m_tokens;
};

#endif
