/*
   Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef CREST_COND_EXCEPTION_H
#define CREST_COND_EXCEPTION_H
#include <iostream>
#include <stdexcept>

namespace Crest {
  const unsigned int UNKNOWN_TYPE = -1;
  const unsigned int COMMON_TYPE = 0;
  const unsigned int FINALIZE_STRUCTURE_TYPE = 1;
  const unsigned int EXISTS_TYPE = 2;
  class CrestBaseException : public std::exception {
    protected:
      std::string m_text;
      uint32_t m_type;
      CrestBaseException():m_text(""),m_type(UNKNOWN_TYPE){;}
    public:
      char const* what() const throw() { return m_text.c_str(); }
      uint32_t getType() {return m_type;}	
  };

  class FinStructureException : public CrestBaseException {
    public:
      FinStructureException(){ m_text="Error: Can not change structure because data already exists.";m_type=FINALIZE_STRUCTURE_TYPE;}
  };
  class ColumnExistException : public CrestBaseException {
    public:
      ColumnExistException(const char* name){ m_text="Error: The column with name '"+std::string(name)+ "'  already exists.";m_type=EXISTS_TYPE;}
  };
  class CommonCrestException: public CrestBaseException {
    public:
      CommonCrestException(const char* text){m_text="Error: "+std::string(text); m_type=COMMON_TYPE;}
  };

}

#endif
