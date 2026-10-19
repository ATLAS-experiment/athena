/* emacs: this is -*- c++ -*- */
/**
 **   @file    Utils.cxx        
 **                   
 **   @author  sutt
 **   @date    Sat  1 Aug 2026 13:12:33 BST
 **
 **   $Id: Utils.cxx, v0.0   Sat  1 Aug 2026 13:12:33 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#include "Utils.h"

#include <sys/time.h>
#include <ctime>

/// this is as optimal as it gets because it allows
/// ctime_r to construct the time string in place 
/// and exploits the fixed size ctime_r output
std::string IDTPM2::stime() { 
  time_t t;
  time(&t);
  std::string s(25,'\0');
  ctime_r(&t,&s[0]);
  s.pop_back();
  return s; 
}
