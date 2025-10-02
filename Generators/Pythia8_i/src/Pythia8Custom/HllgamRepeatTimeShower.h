/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IHLLGAMREPEATTIMESHOWER_H
#define IHLLGAMREPEATTIMESHOWER_H

#include "Pythia8_i/IPythia8Custom.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Pythia8/Pythia.h"


/** Tool that will repeatedly perform  time showering on the intial photons
 * from a Higgs decay until at least one is offshell */
class HllgamRepeatTimeShower: public extends<AthAlgTool, IPythia8Custom> {
  
  public:
  
  /** AlgTool style constructor */
  using base_class::base_class;

  /** Destructor */
  virtual ~HllgamRepeatTimeShower(){};
  
  /** AlgTool initialize method */
  StatusCode initialize()  override;
  /** AlgTool finalize method */
  StatusCode finalize()  override;
  
  /** Update the pythia event*/
  StatusCode ModifyPythiaEvent(Pythia8::Pythia& pythia) const override;

  /** Return how much the cross section is modified.
   *  Should only be called once all events have been processed */
  virtual double CrossSectionScaleFactor() const  override;
  
  private:
  
  mutable unsigned long m_nPass{0};
  mutable unsigned long m_nVetos{0};
  
};

#endif
