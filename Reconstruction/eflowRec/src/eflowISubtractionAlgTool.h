/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_EFLOWISUBTRACTIONALGTOOL_H
#define EFLOWREC_EFLOWISUBTRACTIONALGTOOL_H

/********************************************************************

NAME:     eflowBaseAlgTool.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  R.ZHANG
CREATED:  22th July, 2015

********************************************************************/

class eflowCaloObjectContainer;
class eflowRecTrackContainer;
class eflowRecClusterContainer;
#include "GaudiKernel/IAlgTool.h"

class eflowISubtractionAlgTool : virtual public IAlgTool {

 public:

  virtual StatusCode intialize() {return StatusCode::SUCCESS;}
  virtual void execute(eflowCaloObjectContainer*, eflowRecTrackContainer*, eflowRecClusterContainer*) = 0;
  virtual StatusCode finalize() {return StatusCode::SUCCESS;}

};

#endif
