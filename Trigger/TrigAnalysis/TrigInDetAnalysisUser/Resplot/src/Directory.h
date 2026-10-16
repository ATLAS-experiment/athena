/* emacs: this is -*- c++ -*- */
/**
 **
 **   @file         Directory.h  
 **
 **                 class to keep a directory for each object 
 **                 in a root sort of way, but needed to keep 
 **                 the root objects out of the actual code.   
 ** 
 **   @author       M.Sutton  
 **
 **   @date         Wed May  4 17:54:25 BST 2005
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **                   
 **                   
 **
 **/


#ifndef RESPLOT_DIRECTORY_H
#define RESPLOT_DIRECTORY_H

#include "TDirectory.h"
#include "TH1.h"

// #include "utils.h"

namespace Resplot {

class Directory {

 public:

  Directory() : m_HAddState(true), m_DAddState(true), m_Pop(NULL), m_Dir(NULL) { }
  Directory(const std::string& n) :m_HAddState(true), m_DAddState(true), m_Pop(gDirectory), m_Dir(gDirectory->mkdir(n.c_str())) {
  } 

  virtual ~Directory() {  }

  void push() { m_Pop = gDirectory; if (m_Dir) m_Dir->cd(); }
  void pop()  { if (m_Pop) m_Pop->cd(); }

  void Write() { push(); m_Dir->Write(); pop(); }

  void  pwd() const { m_Dir->pwd(); }
  TDirectory* cwd() { return m_Dir; }

  void disable() {  }
  void restore() {  }

 protected:
  
  bool        m_HAddState;
  bool        m_DAddState;

  TDirectory* m_Pop;
  TDirectory* m_Dir;

};

}

#endif  /* RESPLOT_DIRECTORY_H */










