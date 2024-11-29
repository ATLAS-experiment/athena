/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDDICT_Debugger_H
#define  IDDICT_Debugger_H

class Debugger 
{ 
public:
  static bool get_debug_state()
  {
    if (::getenv ("IDDEBUG") != 0) {
      return true;
    }
    return false; 
  }
  static bool debug () 
  { 
    static const bool debug_state = get_debug_state();
    return debug_state; 
  } 
}; 

#endif

