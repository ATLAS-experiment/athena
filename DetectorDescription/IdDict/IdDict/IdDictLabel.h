/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictLabel_H
#define IDDICT_IdDictLabel_H

#include <string>
  
struct IdDictLabel {
    std::string m_name;  
    bool m_valued{};  
    int m_value{};  
};  

#endif

