/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// RCJetSubstructureAug.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: G. Albouy (galbouy@lpsc.in2p3.fr)
// This tool computes substructure variables for ReClustered jets
// from LCTopo clusters ghost associated to RC jets 
// by constructing cluster jets 

#ifndef DERIVATIONFRAMEWORK_RCJetSubstructureAug_H
#define DERIVATIONFRAMEWORK_RCJetSubstructureAug_H

#include "fastjet/tools/Filter.hh"
#include "fastjet/contrib/SoftDrop.hh"

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include <string>
#include <optional>

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {
    enum EJetParser { kRCJetSelection , kNumJetParser };
    class RCJetSubstructureAug :  public extends<ExpressionParserUser<AthAlgTool, kNumJetParser>, IAugmentationTool> {
        public: 
        RCJetSubstructureAug(const std::string& t, const std::string& n, const IInterface* p);
        virtual ~RCJetSubstructureAug();
        virtual StatusCode initialize() override;
        virtual StatusCode addBranches(const EventContext& ctx) const override;

        private:
        
        StringProperty m_streamName
            { this, "StreamName", "", "Name of the stream" };
        Gaudi::Property< std::vector<std::string> > m_ghostNames
            { this, "GhostConstitNames", {"GhostLCTopo"}, "Names of the ghost constituents for substructure computation"};
        StringProperty m_selectionString 
            { this, "SelectionString", "", "Selection to apply to jet"};
        StringProperty m_suffix
            { this, "Suffix", "", "Suffix for variables"};
        StringProperty m_grooming
            { this, "Grooming", "", "Name of gromming technic to apply (Trimming or SofDrop)"};
        
        Gaudi::Property<float> m_rclus  
            {this, "RClusTrim", 0.3 , "R for reclustering (0 for none)"};
        Gaudi::Property<float> m_ptfrac      
            {this, "PtFracTrim", 0.03, "pT fraction for retaining subjets"};
        Gaudi::Property<float> m_beta 
            {this, "BetaSoft", 1. , "How much to consider angular dependence"};
        Gaudi::Property<float> m_zcut 
            {this, "ZcutSoft", 1. , "pT fraction for retaining subjets"};
        Gaudi::Property<float> m_R0 
            {this, "R0Soft", 1. , "Normalization of angular distance, usually the characteristic jet radius (default R0 = 1)"};
        
        SG::ReadHandleKey< xAOD::JetContainer > m_jetKey
          { this, "JetContainerKey", ""};

        using WDHK = SG::WriteDecorHandleKey<xAOD::JetContainer>;

        /// Qw decorator
        WDHK m_dec_Qw { this, "dec_Qw", m_jetKey, "Qw_" };

        /// Nsubjetiness decorators
        WDHK m_dec_Tau1 { this, "dec_Tau1", m_jetKey, "Tau1_" };
        WDHK m_dec_Tau2 { this, "dec_Tau2", m_jetKey, "Tau2_" };
        WDHK m_dec_Tau3 { this, "dec_Tau3", m_jetKey, "Tau3_" };
        WDHK m_dec_Tau4 { this, "dec_Tau4", m_jetKey, "Tau4_" };
        WDHK m_dec_Tau21 { this, "dec_Tau21", m_jetKey, "Tau21_" };
        WDHK m_dec_Tau32 { this, "dec_Tau32", m_jetKey, "Tau32_" };

        /// KtSplittingScale decorators
        WDHK m_dec_Split12 { this, "dec_Split12", m_jetKey, "Split12_" };
        WDHK m_dec_Split23 { this, "dec_Split23", m_jetKey, "Split23_" };
        WDHK m_dec_Split34 { this, "dec_Split34", m_jetKey, "Split34_" };

        /// Energy correlation factors decorators
        WDHK m_dec_ECF1 { this, "dec_ECF1", m_jetKey, "ECF1_" };
        WDHK m_dec_ECF2 { this, "dec_ECF2", m_jetKey, "ECF2_" };
        WDHK m_dec_ECF3 { this, "dec_ECF3", m_jetKey, "ECF3_" };
        WDHK m_dec_ECF4 { this, "dec_ECF4", m_jetKey, "ECF4_" };
        WDHK m_dec_C2 { this, "dec_C2", m_jetKey, "C2_" };
        WDHK m_dec_D2 { this, "dec_D2", m_jetKey, "D2_" };

        /// Reclustered jets information decorators 
        WDHK m_dec_pT { this, "dec_pT", m_jetKey, "pT_" };
        WDHK m_dec_m { this, "dec_m", m_jetKey, "m_" };
        WDHK m_dec_NConstits { this, "dec_NConstits", m_jetKey, "NConstits_" };
        WDHK m_dec_eta { this, "dec_eta", m_jetKey, "eta_" };
        WDHK m_dec_phi { this, "dec_phi", m_jetKey, "phi_" };

        /// Timing information 
        WDHK m_dec_timing { this, "dec_timing", m_jetKey, "timing_" };

        // The filter object that will apply the grooming
        std::optional<fastjet::Filter> m_trimmer;
        std::optional<fastjet::contrib::SoftDrop> m_softdropper;
    };

}

#endif // DERIVATIONFRAMEWORK_RCJetSubstructureAug_H
