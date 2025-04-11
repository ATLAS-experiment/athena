/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// --------------------------------------------------
//
// File:  GeneratorFilters/xAODHeavyFlavorHadronFilter.h

// AuthorList:
// Marjorie Shapiro
//
// Based on SoftLeptonInJetFilter written by:
// A. Kaczmarska, F. Derue


#ifndef GENERATORFILTERSxAODHEAVYFLAVORHADRONFILTER_H
#define GENERATORFILTERSxAODHEAVYFLAVORHADRONFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

class xAODHeavyFlavorHadronFilter : public GenFilter {
public:
        using GenFilter::GenFilter;

        virtual StatusCode filterInitialize() override final;
        virtual StatusCode filterFinalize() override final;
        virtual StatusCode filterEvent() override final;

private:
        // Setable Properties:-

        Gaudi::Property<double> m_charmPtMin{this,"CharmPtMin",2000.,"Min Pt of charm"};
        Gaudi::Property<double> m_bottomPtMin{this,"BottomPtMin",5000.,"Min Pt of bottom"};
        Gaudi::Property<double> m_charmEtaMax{this,"CharmEtaMax",3.0,"Max eta of charm"};
        Gaudi::Property<double> m_bottomEtaMax{this,"BottomEtaMax",3.0,"Max eta of bottom"};
        Gaudi::Property<double> m_cPtMin{this,"cPtMin",0.0,"Min Pt of c-quark"};
        Gaudi::Property<double> m_bPtMin{this,"bPtMin",0.0,"Min Pt of b-quark"};
        Gaudi::Property<double> m_cEtaMax{this,"cEtaMax",5.0,"Max eta of c-quark"};
        Gaudi::Property<double> m_bEtaMax{this,"bEtaMax",5.0,"Max eta of b-quark"};
        Gaudi::Property<double> m_jetPtMin{this,"JetPtMin",20000.,"Min Pt of jets"};
        Gaudi::Property<double> m_jetEtaMax{this,"JetEtaMax",2.5,"Max eta of jets"};
        Gaudi::Property<double> m_PDGPtMin{this,"PDGPtMin",2000.,"Min Pt of PDG identified particle"};
        Gaudi::Property<double> m_PDGEtaMax{this,"PDGEtaMax",2.5,"Max eta of PDG identified particle"};
        Gaudi::Property<int> m_PDGID{this,"PDGID",0,"PDG ID of a given particle"};
        Gaudi::Property<bool> m_PDGAntiParticleToo{this,"PDGAntiParticleToo",true,"Request PDG antiparticle"};
        Gaudi::Property<bool> m_RequestCharm{this,"RequestCharm",true,"Request a charm"};
        Gaudi::Property<bool> m_RequestBottom{this,"RequestBottom",true,"Request a bottom"};
        Gaudi::Property<bool> m_Request_cQuark{this,"Request_cQuark",true,"Request a c-quark"};
        Gaudi::Property<bool> m_Request_bQuark{this,"Request_bQuark",true,"Request a b-quark"};
        Gaudi::Property<bool> m_RequestSpecificPDGID{this,"RequestSpecificPDGID",false,"Request a specific PDG ID"};
        Gaudi::Property<bool> m_RequireTruthJet{this,"RequireTruthJet",false,"Require truth jets"};
        Gaudi::Property<double> m_deltaRFromTruth{this,"DeltaRFromTruth",0.4,"Delta R from truth jet"};
        SG::ReadHandleKey<xAOD::JetContainer> m_TruthJetContainerName{this, "TruthJetContainer", "AntiKt4TruthJets"}; // Name of the truth jet container
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

        // Local Member Data:-
        int    m_NPass{};
        int    m_Nevt{};
        int    m_NbPass{};
        int    m_NcPass{};
        int    m_NBHadronPass{};
        int    m_NDHadronPass{};
        int    m_NPDGIDPass{};
};

#endif
