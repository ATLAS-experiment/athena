/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Teng Jian Khoo

#include <unordered_map>
#include <iomanip>

#include "AsgAnalysisAlgorithms/SystObjectLinkerAlg.h"
#include "PATInterfaces/SystematicSet.h"
#include "AthContainers/ConstDataVector.h"

typedef ElementLink<xAOD::IParticleContainer> iplink_t;
static const SG::Decorator< iplink_t  > dec_nominalObject("nominalObjectLink");

namespace CP
{
  SystObjectLinkerAlg ::SystObjectLinkerAlg(const std::string &name,
                                  ISvcLocator *pSvcLocator)
      : EL::AnaReentrantAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode SystObjectLinkerAlg ::initialize()
  {

    // Read syst-aware input handles
    ATH_CHECK (m_inputHandle.initialize(m_systematicsList));

    // Intialise syst-aware output decorators
    ATH_CHECK(m_syst_link_decor.initialize(m_systematicsList, m_inputHandle));

    // Intialise syst list (must come after all syst-aware inputs and outputs)
    ATH_CHECK (m_systematicsList.initialize());

    ATH_MSG_DEBUG("Adding nominal/systematic object links for " << m_inputHandle.getNamePattern());

    return StatusCode::SUCCESS;
  }

  StatusCode SystObjectLinkerAlg ::execute(const EventContext&) const
  {

    // Populate a map of systematics hash to container, so we
    // can iterate safely regardless of the ordering of systs
    // from the SystematicsSvc
    // (mainly avoid assumption that nominal comes first)
    std::unordered_map<std::size_t, const xAOD::IParticleContainer*> systhash_to_container;
    systhash_to_container.reserve(m_systematicsList.systematicsVector().size());
    // Record the hash for the nominal for easier access
    size_t nominal_hash{SIZE_MAX};
    // Loop is over CP::SystematicsSet, but recommended to use auto
    // in case this ever changes...
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
        const xAOD::IParticleContainer* sys_container = nullptr;
        ATH_CHECK( m_inputHandle.retrieve(sys_container, sys) );

        // Record the hash for the nominal
        if(sys.name().empty()) {nominal_hash = sys.hash();}

        // We can't find the original container if the systematics
        // copies are empty. This is a slight vulnerability.
        if (sys_container->empty()) {
            ATH_MSG_DEBUG("Container for systematic variation '" << sys.name() << "' was empty.");
            systhash_to_container.insert({sys.hash(), nullptr});
            continue;
        }

        // Navigate to the full container, as this may be a view container
        // holding a subset of the objects
        // Cast from SG::AuxVectorData
        const xAOD::IParticleContainer* full_container =
          static_cast<const xAOD::IParticleContainer*>(sys_container->front()->container());
        systhash_to_container.insert({sys.hash(), full_container});
        if(full_container == sys_container) {
            ATH_MSG_VERBOSE("The unfiltered container and the input container are the same.");
        } else {
            ATH_MSG_DEBUG("Read in container with " << sys_container->size() << " elements.");
            ATH_MSG_DEBUG("Traced back to unfiltered container with " << full_container->size() << " elements.");
        }
    }

    if(nominal_hash==SIZE_MAX) {
        ATH_MSG_ERROR("The nominal variation was not detected!");
        return StatusCode::FAILURE;
    }

    // Iterate over the nominal container, extract the index-parallel syst
    // Then apply the bidirectional links as decorations
    const xAOD::IParticleContainer* nom_cont = systhash_to_container[nominal_hash];
    if(nom_cont==nullptr) {
        ATH_MSG_DEBUG("Unable to retrieve the nominal container, will have to assume there are no relevant objects");
        return StatusCode::SUCCESS;
    }

    for (const xAOD::IParticle* nom_obj : *nom_cont) {
        for (const auto& sys : m_systematicsList.systematicsVector()) {
            if(sys.hash()==nominal_hash) {continue;}
            const xAOD::IParticleContainer *var_cont = systhash_to_container[sys.hash()];
            if(var_cont==nullptr) {
                ATH_MSG_ERROR("Cannot decorate syst '" << sys.name() << "' for obj " << nom_obj->index());
                ATH_MSG_ERROR("Likely the systematics input container was empty after filtering.");
                //must return here, the var_cont pointer is dereferenced in the next line
                return StatusCode::FAILURE;
            }
            const xAOD::IParticle* var_obj = (*var_cont)[nom_obj->index()];
            dec_nominalObject(*var_obj) = iplink_t(*nom_cont, nom_obj->index());
            ATH_MSG_VERBOSE("Writing decoration " << m_syst_link_decor.getName(sys) << " from object " << nom_obj->index());
            m_syst_link_decor.set(*nom_obj, iplink_t(*var_cont, var_obj->index()), sys);
            ATH_MSG_VERBOSE("Nominal object with pt " << std::setprecision(3) << nom_obj->pt()/1e3 << " GeV linked to");
            ATH_MSG_VERBOSE("  '" << sys.name() << "' varied object with pt " << std::setprecision(3) << var_obj->pt()/1e3 << " GeV.");
        }
    }

    return StatusCode::SUCCESS;
  }
}
