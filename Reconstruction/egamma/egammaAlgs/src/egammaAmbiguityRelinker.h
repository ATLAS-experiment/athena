/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_EGAMMAAMBIGUITYRELINKER_H
#define EGAMMAALGS_EGAMMAAMBIGUITYRELINKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonFwd.h"
#include "xAODEgamma/PhotonContainer.h"
#include <memory>

/**
 * @class egammaAmbiguityRelinker
 * @brief Algorithm to handle ambiguity relinking for electrons and photons.
 *
 * This class is responsible for reading input electron and photon containers,
 * processing them to in order to link ambiguous candidates, and writing to the
 * output containers. It is used in the context of an AODFix to correct an 
 * ambiguityLink association  bug in the Athena Rel24 up to 24.0.83. 
 *
 * @details
 * - Input electron container: specified by ElectronInputName.
 * - Output electron container: specified by ElectronOutputName.
 * - Input photon container: specified by PhotonInputName.
 * - Output photon container: specified by PhotonOutputName.
 *
 */
class egammaAmbiguityRelinker : public AthReentrantAlgorithm
{
public:

    egammaAmbiguityRelinker(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override final;
    StatusCode execute(const EventContext& ctx) const override final;

private:

    // Read/Write handlers

    /** @brief Name of the electron output collection*/
    SG::WriteHandleKey<xAOD::ElectronContainer> m_electronOutputKey {this,
        "ElectronOutputName", "Electrons",
        "Name of Electron Container to be created"};

    /** @brief Name of the electron input collection */
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronInputKey {this,
        "ElectronInputName", "old_Electrons",
        "Name of Electron container to be read in"};
    
    /** @brief Name of the photon output collection*/
    SG::WriteHandleKey<xAOD::PhotonContainer> m_photonOutputKey {this,
        "PhotonOutputName", "Photons",
        "Name of Photon Container to be created"};

    /** @brief Name of the photon input collection */
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonInputKey {this,
        "PhotonInputName", "old_Photons",
        "Name of Photon container to be read in"};

};

#endif
