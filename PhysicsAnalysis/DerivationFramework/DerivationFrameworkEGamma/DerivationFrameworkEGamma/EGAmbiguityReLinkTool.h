/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AmbiguityReLinkTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EGAMBIGUITYRELINKTOOL_H
#define DERIVATIONFRAMEWORK_EGAMBIGUITYRELINKTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandleKey.h"
//
#include "xAODEgamma/EgammaContainer.h"

namespace DerivationFramework {

    class EGAmbiguityReLinkTool
        : public AthAlgTool
        , public IAugmentationTool
    {
    public:
        EGAmbiguityReLinkTool(const std::string& t,
                               const std::string& n,
                               const IInterface* p);

        StatusCode initialize() override final;
        virtual StatusCode addBranches() const override final;

    private:
        SG::ReadHandleKey<xAOD::EgammaContainer> m_srcContainerName{ this,
                                                                     "SourceEGammaContainer",
                                                                     "",
                                                                     "Input" };
        SG::ReadHandleKey<xAOD::EgammaContainer> m_dstContainerName{ this,
                                                                     "DestEGammaContainer",
                                                                     "",
                                                                     "Input" };

        SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_decoratorHandle{ this, "DecoratorHandle", "", "" };
        std::string m_decorName;
    };
}

#endif // DERIVATIONFRAMEWORK_EGAMBIGUITYRELINKTOOL_H
