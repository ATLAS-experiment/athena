/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Dual-use tool interface for Rivet routine for classifying MC events according to the Higgs template cross section categories
 * Authors: Jim Lacey (Carleton University)
 * <james.lacey@cern.ch,jlacey@physics.carleton.ca>
 */

#ifndef TRUTHRIVETTOOLS_HIGGSTRUTHCATEGORYTOOL_H
#define TRUTHRIVETTOOLS_HIGGSTRUTHCATEGORYTOOL_H 1

#include "CxxUtils/checker_macros.h"
#include "Rivet/AnalysisHandler.hh"
#include "TruthRivetTools/HiggsTemplateCrossSections.h"

// To avoid coflict of UNUSED macro of
// Control/CxxUtils/CxxUtils/unused.h and Rivet/Tools/Utils.hh
#ifdef UNUSED
#undef UNUSED
#endif // UNUSED

// Base classes
#include "AsgTools/AsgTool.h"
#include "GenInterfaces/IHiggsTruthCategoryTool.h"

// Return type (non-pointer)
// Note: the Template XSec Defs *depends* on having included
//  the TLorentzVector header *before* it is included -- it
//  uses the include guard from TLorentzVector to decide
//  what is available
#include "TLorentzVector.h"
#include "TruthRivetTools/HiggsTemplateCrossSectionsDefs.h"

#include "AtlasHepMC/GenEvent.h"

// System include(s).
#include <memory>

class HiggsTruthCategoryTool
: public asg::AsgTool,
  public virtual IHiggsTruthCategoryTool
{
 public:
   ASG_TOOL_CLASS( HiggsTruthCategoryTool , IHiggsTruthCategoryTool )
   HiggsTruthCategoryTool( const std::string& name );

 public:
   std::unique_ptr<Rivet::AnalysisHandler> m_rivetAnaHandler; //!
   Rivet::HiggsTemplateCrossSections* m_higgsTemplateCrossSections; //!
   virtual StatusCode  initialize() override;
   StatusCode finalize () override;
   HTXS::HiggsClassification* getHiggsTruthCategoryObject(const HepMC::GenEvent& HepMCEvent, const HTXS::HiggsProdMode prodMode) const override;
 private:
   mutable std::atomic_flag m_isInitialized ATLAS_THREAD_SAFE = ATOMIC_FLAG_INIT;
   bool m_outHistos;
};

#endif //> !HIGGSTRUTHCLASSIFIER_HIGGSTRUTHCATEGORYTOOL_H
