/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef EVGENPRODTOOLS_GENWEIGHTDECLARATION_H
#define EVGENPRODTOOLS_GENWEIGHTDECLARATION_H

#include "GaudiKernel/INamedInterface.h"
#include "AthenaKernel/ICutFlowSvc.h"
#include "GenInterfaces/IHepMCWeightSvc.h"
#include "GeneratorModules/GenBase.h"
#include "GaudiKernel/ServiceHandle.h"

class CutFlowSvc;

/**
 * @class GenWeightDeclaration
 * @brief Declare the number of generator weights to the CutFlowSvc.
 *
 * Some generators such as Pythia construct the weight variations
 * at run time after the first event has been generated.
 * The CutFlowSvc needs to know the number of weights in advance,
 * in order to create the correct number of CutBookkeepers for each filter.
 * This algorithm declares the number of weights after generation and FixHepMC 
 * but before filters and before CutFlowSvc counts any event.
 */
class GenWeightDeclaration : public GenBase {
public:
  GenWeightDeclaration(const std::string& name, ISvcLocator* svcLoc);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

private:
  ServiceHandle<ICutFlowSvc> m_cutFlowSvc{
    this, "CutFlowSvc", "CutFlowSvc/CutFlowSvc",
    "Cut-flow service"};

  // The CutFlowSvc implementation, used to call the 
  // setNumberOfWeightVariations method
  CutFlowSvc* m_cutFlowSvcImpl{nullptr};

  ServiceHandle<IHepMCWeightSvc> m_hepMCWeightSvc{
    this, "HepMCWeightSvc", "HepMCWeightSvc/HepMCWeightSvc",
    "Service receiving the generator weight names"};

  // Indicates whether the weights have been declared to the CutFlowSvc
  bool m_weightsDeclared{false};
};

#endif

#endif
