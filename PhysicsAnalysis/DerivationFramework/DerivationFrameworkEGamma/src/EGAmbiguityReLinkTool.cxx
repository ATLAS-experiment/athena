/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGAmbiguityReLinkTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
//

#include "DerivationFrameworkEGamma/EGAmbiguityReLinkTool.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

namespace DerivationFramework {

EGAmbiguityReLinkTool::EGAmbiguityReLinkTool(const std::string& t,
                                               const std::string& n,
                                               const IInterface* p)
    : AthAlgTool(t, n, p)
    , m_decorName("")
{
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    declareProperty("DecoratorName", m_decorName);
}

StatusCode EGAmbiguityReLinkTool::initialize()
{
    if (m_decorName.empty()) {
        ATH_MSG_ERROR("No DecoratorName provided for the output of EGAmbiguityReLinkTool!");
        return StatusCode::FAILURE;
    }

    ATH_CHECK(m_srcContainerName.initialize());
    ATH_CHECK(m_dstContainerName.initialize());
    m_decoratorHandle = m_srcContainerName.key() + "." + m_decorName;
    ATH_CHECK(m_decoratorHandle.initialize());
    return StatusCode::SUCCESS;
}

StatusCode EGAmbiguityReLinkTool::addBranches() const
{

    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::EgammaContainer> srcParticles{m_srcContainerName, ctx};
    SG::ReadHandle<xAOD::EgammaContainer> dstParticles{m_dstContainerName, ctx};
    SG::WriteDecorHandle<xAOD::EgammaContainer, ElementLink<xAOD::EgammaContainer>> decoratorHandle(m_decoratorHandle, ctx);

    static const SG::AuxElement::Accessor<std::vector<ElementLink<xAOD::CaloClusterContainer>>> caloClusterLinks("constituentClusterLinks");

    ElementLink<xAOD::EgammaContainer> dummylink;
    for (const xAOD::Egamma* src : *srcParticles ) {
        decoratorHandle(*src) = dummylink;
        if (src->author() != xAOD::EgammaParameters::AuthorAmbiguous) {
            continue;
        }

        for (
            size_t dstIndex = 0;
            dstIndex < dstParticles->size();
            ++dstIndex
        ) {
            const xAOD::Egamma* dst = dstParticles->at(dstIndex);
            if (dst->author() != xAOD::EgammaParameters::AuthorAmbiguous) {
                continue;
            }

            if (caloClusterLinks(*(dst->caloCluster())).at(0) ==
                caloClusterLinks(*(src->caloCluster())).at(0)) {
                ElementLink<xAOD::EgammaContainer> link(*dstParticles, dstIndex, ctx);
                decoratorHandle(*src) = link;
                break;
            }
        }
    }

    return StatusCode::SUCCESS;
}

} // namespace DerivationFramework

