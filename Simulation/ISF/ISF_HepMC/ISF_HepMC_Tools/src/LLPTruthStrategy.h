/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_TOOLS_LLPTRUTHSTRATEGY_H
#define ISF_TOOLS_LLPTRUTHSTRATEGY_H 1

// stl includes
#include <set>
#include <vector>

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AtlasDetDescr/AtlasRegion.h"

// ISF includes
#include "ISF_HepMC_Interfaces/ITruthStrategy.h"

namespace ISF {

  /** @class LLPTruthStrategy

      An ISF truth strategy for recording long lived particles to
      the MC truth.

      @author Elmar.Ritsch -at- cern.ch
  */
  class LLPTruthStrategy final : public extends<AthAlgTool, ITruthStrategy> {

  public:
    /** Constructor with parameters */
    LLPTruthStrategy( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~LLPTruthStrategy() = default;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    /** True if the ITruthStrategy implementationapplies to the given ITruthIncident */
    virtual bool pass( ITruthIncident& incident) const override final;

    virtual bool appliesToRegion(unsigned short geoID) const override final;

  private:
    /** The process code range (low-high) and the category of processes that
     *  should pass this strategy */
    Gaudi::Property<int> m_passProcessCodeRangeLow{this, "PassProcessCodeRangeLow", 200};
    Gaudi::Property<int> m_passProcessCodeRangeHigh{this, "PassProcessCodeRangeHigh", 299};
    Gaudi::Property<int> m_passProcessCategory{this, "PassProcessCategory", 9};

    IntegerArrayProperty m_regionListProperty{this, "Regions", {}};
  };

}


#endif //> !ISF_TOOLS_LLPTRUTHSTRATEGY_H
