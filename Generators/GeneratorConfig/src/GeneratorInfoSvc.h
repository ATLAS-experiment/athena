// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef GENERATORCONFIG_GENERATORINFOSVC_H
#define GENERATORCONFIG_GENERATORINFOSVC_H

#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Property.h" 
#include <string>
#include <vector>

/**
 * @class GeneratorInfoSvc
 *
 * @brief Service holding generator metadata for event generation.
 *
 * This service stores metadata related to Monte Carlo event generation,
 * such as the list of generators used and the generator tune.
 *
 * Historically, this information was stored in the global object created from
 * @c EvgenConfig.py, which is no
 * longer available in the ComponentAccumulator-based configuration.
 * GeneratorInfoSvc replaces that functionality.
 */
class GeneratorInfoSvc : public AthService {
  public:
    /**
     * @brief Standard service constructor.
     * @param name Name of the service
     * @param svcLoc Pointer to the service locator
     */
    GeneratorInfoSvc(const std::string& name, ISvcLocator* svcLoc);

    /**
     * @brief Initialize the service.
     *
     * Called by the framework during initialization.
     */
    virtual StatusCode initialize() override;

  protected:
    /**
     * @brief List of generators that were run.
     *
     * This property is typically populated by base generator configuration
     * fragments e.g. @c Pythia8Config.py.
     */
    StringArrayProperty m_generators{this, "Generators", {}, "List of generators run", "OrderedSet<std::string>"};
    /**
     * @brief Generator tune used for event generation.
     */
    StringProperty m_tune{this, "Tune", "", "Tune used"};
};

#endif
