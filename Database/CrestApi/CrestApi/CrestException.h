/*
   Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
 */

namespace Crest
{

  class CrestException : public std::runtime_error
  {
  public:
    CrestException(const std::string& what = "") : std::runtime_error(what) {}
  };


  class CrestConflictException : public CrestException
  {
  public:
    CrestConflictException(const std::string& what = "") : CrestException(what) {}
  };

}
