/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   ElectronLRTMergingAlg
//
//   Electron merger algorithm merges the standard and LRT electron containers.
//   It uses the ElectronLRTOverlapRemovalTool to remove overlaps.
//   The output merged collection is decorated with isLRT=0/1 to denote
//   if the electron was from the standard or LRT container.  Merging can
//   be output into a transient view container or copied container written out.
///////////////////////////////////////////////////////////////////

#include "EgammaAnalysisAlgorithms/ElectronLRTMergingAlg.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "AsgTools/AsgToolConfig.h"
#include "xAODBase/IParticleHelpers.h"
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP
{
    StatusCode ElectronLRTMergingAlg::initialize()
    {
        ANA_CHECK(m_promptElectronLocation.initialize());
        ANA_CHECK(m_lrtElectronLocation.initialize());
        ANA_CHECK(m_outElectronLocation.initialize());
        m_outElectronViewLocation = m_outElectronLocation.key();
        ANA_CHECK(m_outElectronViewLocation.initialize(m_createViewCollection.value()));
        ANA_CHECK(m_lrtIsLRTKey.initialize());
        ANA_CHECK(m_promptIsLRTKey.initialize());

        /// if the tool is not user-set, configure the automatic instance via our overlap flag
        if (m_overlapRemovalTool.empty())
        {
            asg::AsgToolConfig config("CP::ElectronLRTOverlapRemovalTool/ElectronLRTOverlapRemovalTool");
            ANA_CHECK(config.setProperty("overlapStrategy", m_ORstrategy.value()));
            ANA_CHECK(config.setProperty("ORThreshold", m_ORThreshold.value()));
            ANA_CHECK(config.setProperty("isDAOD", m_isDAOD.value()));
            ANA_CHECK(config.makePrivateTool(m_overlapRemovalTool));
        }

        // Retrieve the tools
        ANA_CHECK(m_overlapRemovalTool.retrieve());

        // Return gracefully:
        return StatusCode::SUCCESS;
    }


    StatusCode ElectronLRTMergingAlg::execute(const EventContext &ctx) const
    {

        // Retrieve electrons from StoreGate
        SG::ReadHandle<xAOD::ElectronContainer> promptCol(m_promptElectronLocation, ctx);
        SG::ReadHandle<xAOD::ElectronContainer> lrtCol(m_lrtElectronLocation, ctx);
        if (!promptCol.isValid())
        {
            ATH_MSG_FATAL("Unable to retrieve xAOD::ElectronContainer, \"" << m_promptElectronLocation << "\", cannot run the LRT electron merger!");
            return StatusCode::FAILURE;
        }
        if (!lrtCol.isValid())
        {
            ATH_MSG_FATAL("Unable to retrieve xAOD::ElectronContainer, \"" << m_lrtElectronLocation << "\", cannot run the LRT electron merger!");
            return StatusCode::FAILURE;
        }


        std::set<const xAOD::Electron *> ElectronsToRemove;
        m_overlapRemovalTool->checkOverlap(*promptCol, *lrtCol, ElectronsToRemove);

        ATH_MSG_DEBUG("Size of overlapping electrons to remove: " << ElectronsToRemove.size());

        // Decorate the electrons with their track type
        // 0 if prompt, 1 if LRT
        SG::WriteDecorHandle<xAOD::ElectronContainer, char> promptIsLRT(m_promptIsLRTKey, ctx);
        SG::WriteDecorHandle<xAOD::ElectronContainer, char> lrtIsLRT(m_lrtIsLRTKey, ctx);
        for (const xAOD::Electron *el : *promptCol)
            promptIsLRT(*el) = 0;
        for (const xAOD::Electron *el : *lrtCol)
            lrtIsLRT(*el) = 1;

        // merging loop over containers and write, using a view container or a deep copy
        if (m_createViewCollection)
        {
            auto transientContainer = std::make_unique<ConstDataVector<xAOD::ElectronContainer>>(SG::VIEW_ELEMENTS);
            transientContainer->reserve(promptCol->size() + lrtCol->size());

            mergeElectron(*promptCol, transientContainer.get(), ElectronsToRemove);
            mergeElectron(*lrtCol, transientContainer.get(), ElectronsToRemove);

            SG::WriteHandle<ConstDataVector<xAOD::ElectronContainer>> h_write(m_outElectronViewLocation, ctx);
            ATH_CHECK(h_write.record(std::move(transientContainer)));
        }
        else
        {
            auto outputCol = std::make_unique<xAOD::ElectronContainer>();
            auto outputAuxCol = std::make_unique<xAOD::ElectronAuxContainer>();
            outputCol->setStore(outputAuxCol.get());
            outputCol->reserve(promptCol->size() + lrtCol->size());

            mergeElectron(*promptCol, outputCol.get(), ElectronsToRemove);
            mergeElectron(*lrtCol, outputCol.get(), ElectronsToRemove);

            SG::WriteHandle<xAOD::ElectronContainer> h_write(m_outElectronLocation, ctx);
            ATH_CHECK(h_write.record(std::move(outputCol), std::move(outputAuxCol)));
        }

        ATH_MSG_DEBUG("Done !");

        return StatusCode::SUCCESS;
    }

    ///////////////////////////////////////////////////////////////////
    // Merge electron collections and remove duplicates, for copy
    ///////////////////////////////////////////////////////////////////
    
    void ElectronLRTMergingAlg::mergeElectron(const xAOD::ElectronContainer &electronCol,
                                              xAOD::ElectronContainer *outputCol,
                                              const std::set<const xAOD::Electron *> &ElectronsToRemove) const
    {
        // loop over electrons, accept them and add them into association tool
        if (!electronCol.empty())
        {
            ATH_MSG_DEBUG("Size of output electron collection " << electronCol.size());

            static const SG::Decorator<ElementLink<xAOD::ElectronContainer>> originalElectronLink("originalElectronLink");

            // loop over electrons
            for (const auto *const electron : electronCol)
            {
                // add electron into output and check if LRT electron failed overlap check
                if (m_doRemoval && ElectronsToRemove.find(electron) != ElectronsToRemove.end())
                    continue;
                else
                {
                    std::unique_ptr<xAOD::Electron> newElectron = std::make_unique<xAOD::Electron>(*electron);
                    ElementLink<xAOD::ElectronContainer> eLink;
                    eLink.toIndexedElement(electronCol, electron->index());
                    originalElectronLink(*newElectron) = eLink;
                    setOriginalObjectLink(*electron, *newElectron);
                    static const SG::Accessor<char> isLRT("isLRT");
                    isLRT(*newElectron) = isLRT(*electron);
                    outputCol->push_back(std::move(newElectron));


                }
            }
            ATH_MSG_DEBUG("Size of merged output electron collection " << outputCol->size());
        }
    }

    ///////////////////////////////////////////////////////////////////
    // Merge electron collections and remove duplicates, for transient
    ///////////////////////////////////////////////////////////////////
    void ElectronLRTMergingAlg::mergeElectron(const xAOD::ElectronContainer &electronCol,
                                              ConstDataVector<xAOD::ElectronContainer> *outputCol,
                                              const std::set<const xAOD::Electron *> &ElectronsToRemove) const
    {
        // loop over electrons, accept them and add them into association tool
        if (!electronCol.empty())
        {
            ATH_MSG_DEBUG("Size of transient electron collection " << electronCol.size());
            // loop over electrons
            for (const auto *const electron : electronCol)
            {
                // add electron into output and check if LRT electron failed overlap check
                if (m_doRemoval && ElectronsToRemove.find(electron) != ElectronsToRemove.end())
                    continue;
                else
                {
                    outputCol->push_back(electron);
                }
            }
            ATH_MSG_DEBUG("Size of transient merged electron collection " << outputCol->size());
        }
    }

}
