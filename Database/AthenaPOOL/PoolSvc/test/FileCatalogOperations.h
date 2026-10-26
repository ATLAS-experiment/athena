/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FILECATALOGOPERATIONS_H
#define FILECATALOGOPERATIONS_H

#include <string>
#include "SimpleTestClass.h"
#include "PersTestBase.h"

class Token;

class TestDriver : public PersTestBase {
  public:
    explicit TestDriver( const std::string& catname = "FCO.catatlog.xml" );

    int execute();

    void write();
    void addlfns();
    void read();
  
  private:
    std::string           m_fileName1;
    std::string           m_lfn1;
    std::string           m_fileName2;
    std::string           m_lfn2;
    std::string           m_fileName3;
    std::string           m_lfn3;
    int                   m_events;
};

#endif
