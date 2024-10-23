/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARELECCALIB_ILARDIGITOSCILLATIONCORRTOOL_H
#define LARELECCALIB_ILARDIGITOSCILLATIONCORRTOOL_H

#include "GaudiKernel/IAlgTool.h"
class LArDigitContainer;

class ILArDigitOscillationCorrTool: virtual public IAlgTool {
  /**
   *  @brief  AlgoTool to correct for oscillating noise
   *
   *  @author T. Barillari
   **/

 public:
  DeclareInterfaceID( ILArDigitOscillationCorrTool, 1, 0 );

  virtual ~ILArDigitOscillationCorrTool() {};
  
  virtual StatusCode calculateEventPhase(const LArDigitContainer &theDC) = 0;
  virtual StatusCode correctLArDigits(LArDigitContainer &theDC) = 0;

};

#endif 





