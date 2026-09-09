/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Dual-use tool interface for Rivet routine for classifying MC events according to the Higgs template cross section categories
 * Authors: Jim Lacey (Carleton University)
 * <james.lacey@cern.ch,jlacey@physics.carleton.ca>
 */

#ifndef GENINTERFACES_IHIGGSTRUTHCATEGORYTOOL_H
#define GENINTERFACES_IHIGGSTRUTHCATEGORYTOOL_H 1

#include "AsgTools/IAsgTool.h"
#include "AtlasHepMC/GenEvent.h"
#include "TruthRivetTools/HiggsTemplateCrossSectionsDefs.h"
namespace HTXS {
  struct HiggsClassification;
 
}

class IHiggsTruthCategoryTool : public virtual asg::IAsgTool {
 public:
  ASG_TOOL_INTERFACE( IHiggsTruthCategoryTool ) //declares the interface to athena
    virtual ~IHiggsTruthCategoryTool() = default;
 public: 
  virtual HTXS::HiggsClassification* getHiggsTruthCategoryObject(const HepMC::GenEvent& HepMCEvent, const HTXS::HiggsProdMode prodMode, const HTXS::HiggsDecayMode decayMode) const =0;
};

#endif //> !GENINTERFACES_IHIGGSTRUTHCATEGORYTOOL_H
