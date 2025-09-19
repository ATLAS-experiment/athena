/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_IdDictLabel_H
#define IDDICT_IdDictLabel_H

#include <string>
  
struct IdDictLabel {
    const std::string& name() const;
    int value() const;
    bool valued() const;

    std::string m_name;  
    bool m_valued{};  
    int m_value{};  
};  


inline
const std::string& IdDictLabel::name() const
{
    return m_name;
}


inline
int IdDictLabel::value() const
{
  return m_valued ? m_value : 0;
}


inline
bool IdDictLabel::valued() const
{
  return m_valued;
}


#endif

