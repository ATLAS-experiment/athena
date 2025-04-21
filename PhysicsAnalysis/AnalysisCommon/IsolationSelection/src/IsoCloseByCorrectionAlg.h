/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IsoCloseByCorrectionAlg_H
#define IsoCloseByCorrectionAlg_H

// Gaudi & Athena basics
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/ConstDataVector.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "IsolationSelection/IIsolationCloseByCorrectionTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
/**
 * @brief The algorithm reads in muons, electrons and photons and 
 *        runs the closeBy isolation correction tool on them. 
 */

namespace CP {

    class IsoCloseByCorrectionAlg : public AthReentrantAlgorithm {
    public:
        IsoCloseByCorrectionAlg(const std::string& name, ISvcLocator* svcLoc);

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;

    private:
        StatusCode applySelection(const EventContext& ctx,
                                  const xAOD::Electron* elec,
                                  const SG::Decorator<char>& isOK) const;
        StatusCode applySelection(const EventContext& ctx,
                                  const xAOD::Photon* phot,
                                  const SG::Decorator<char>& isOK) const;
        StatusCode applySelection(const EventContext& ctx,
                                  const xAOD::Muon* muon,
                                  const SG::Decorator<char>& isOK) const;
        template <class CONT_TYPE>
        StatusCode selectLeptonsAndPhotons(const EventContext& ctx, CONT_TYPE particles, const SG::Decorator<char>& isOK) const;

        /// Input containers to retrieve from the storegate
        SG::ReadHandleKeyArray<xAOD::IParticleContainer>     m_contKeys{this, "ParticleContainerKeys", {} };

        /// For lepton/photon selection, normally one uses either a decorator xxxSelKey, or a tool xxxSelTool, 
        /// but logic allows both to be required:

        /// read decorators for selection of incoming particles
        SG::ReadDecorHandleKey<xAOD::MuonContainer>     m_muonSelKey{this, "MuonSelectionKey", ""};
        SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_elecSelKey{this, "ElecSelectionKey", ""};
        SG::ReadDecorHandleKey<xAOD::PhotonContainer>   m_photSelKey{this, "PhotSelectionKey", ""};
        
        /// tools for selection of incoming particles
        ToolHandle<CP::IMuonSelectionTool>     m_muonSelTool{this, "MuonSelectionTool", ""};
        ToolHandle<IAsgElectronLikelihoodTool> m_elecSelTool{this, "ElectronSelectionTool", ""};
        ToolHandle<IAsgPhotonIsEMSelector>     m_photSelTool{this, "PhotonSelectionTool", ""};

        /// The closeBy isolation correction tool
        ToolHandle<CP::IIsolationCloseByCorrectionTool> m_closeByCorrTool{this, "IsoCloseByCorrectionTool", "",
                                                                          "The isolation close by correction tool."};

        /// Kinematic cuts - if needed
        Gaudi::Property<float> m_minElecPt{this, "MinElecPt", 0, 
                                           "Minimum pt cut that the electron needs to pass in order to be selected"};
        Gaudi::Property<float> m_minMuonPt{this, "MinMuonPt", 0, 
                                           "Minimum pt cut that the muon needs to pass in order to be selected"};
        Gaudi::Property<float> m_minPhotPt{this, "MinPhotPt", 0,
                                           "Minimum pt cut that the photon needs to pass in order to be selected"};

        // Name of the isolation selection decoration
        Gaudi::Property<std::string> m_quality_name{
            this, "SelectionDecorator", "isoSelIsOK",
            "Name of the char auxdata defining whether the particle shall be considered for iso correction"};
    };
}  // namespace CP
#endif
