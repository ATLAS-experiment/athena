/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_TOOLS_VALIDATIONTRUTHSTRATEGY_H
#define ISF_TOOLS_VALIDATIONTRUTHSTRATEGY_H 1

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AtlasDetDescr/AtlasRegion.h"

// ISF includes
#include "ISF_HepMC_Interfaces/ITruthStrategy.h"

namespace ISF {
  /** @class ValidationTruthStrategy

      This truth strategy is used to record a high number of interaction
      processes. In the end the MC truth output will be used for the
      validation of various fast simulators.

      @author Elmar.Ritsch -at- cern.ch
  */
  class ValidationTruthStrategy final : public extends<AthAlgTool, ITruthStrategy> {

  public:
    /** Constructor with parameters */
    ValidationTruthStrategy( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~ValidationTruthStrategy() = default;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    /** true if the ITruthStrategy implementation applies to the given ITruthIncident */
    virtual bool pass( ITruthIncident& incident) const override final;

    virtual bool appliesToRegion(unsigned short geoID) const override final;


  private:
    /** cuts on the parent particle */
    double m_minParentP2{0.0};  //!< minimum parent particle momentum ^ 2
    Gaudi::Property<double> m_minParentP{this, "ParentMinP", 0.0};  //!< minimum parent particle momentum
    Gaudi::Property<std::vector<int>> m_regionListProperty{this, "Regions", {}};
  };

}


#endif //> !ISF_TOOLS_VALIDATIONTRUTHSTRATEGY_H
