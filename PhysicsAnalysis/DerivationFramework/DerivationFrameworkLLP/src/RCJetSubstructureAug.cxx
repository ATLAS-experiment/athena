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

#include "DerivationFrameworkLLP/RCJetSubstructureAug.h"
#include "xAODJet/JetContainer.h"
#include "xAODBase/IParticle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>

#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "fastjet/tools/Filter.hh"
#include "fastjet/contrib/Nsubjettiness.hh"
#include "JetSubStructureUtils/Qw.h"
#include "JetSubStructureUtils/Nsubjettiness.h"
#include "JetSubStructureUtils/KtSplittingScale.h"
#include "JetSubStructureUtils/EnergyCorrelator.h"

// Constructor
DerivationFramework::RCJetSubstructureAug::RCJetSubstructureAug(const std::string& t, const std::string& n, const IInterface* p) :
base_class(t, n, p) {}

// Destructor
DerivationFramework::RCJetSubstructureAug::~RCJetSubstructureAug() 
{}

// Athena initialize
StatusCode DerivationFramework::RCJetSubstructureAug::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");
    if (m_jetKey.key().empty()) {
        ATH_MSG_FATAL("No jet collection provided for augmentation.");
        return StatusCode::FAILURE;
    }
    ATH_CHECK( m_jetKey.initialize() );

    auto init_dec = [&] ( WDHK& k ) {
      k = k.key() + m_suffix;
      return k.initialize();
    };
    ATH_CHECK( init_dec (m_dec_Qw) );
    ATH_CHECK( init_dec (m_dec_Tau1) );
    ATH_CHECK( init_dec (m_dec_Tau2) );
    ATH_CHECK( init_dec (m_dec_Tau3) );
    ATH_CHECK( init_dec (m_dec_Tau4) );
    ATH_CHECK( init_dec (m_dec_Tau21) );
    ATH_CHECK( init_dec (m_dec_Tau32) );
    ATH_CHECK( init_dec (m_dec_Split12) );
    ATH_CHECK( init_dec (m_dec_Split23) );
    ATH_CHECK( init_dec (m_dec_Split34) );
    ATH_CHECK( init_dec (m_dec_ECF1) );
    ATH_CHECK( init_dec (m_dec_ECF2) );
    ATH_CHECK( init_dec (m_dec_ECF3) );
    ATH_CHECK( init_dec (m_dec_ECF4) );
    ATH_CHECK( init_dec (m_dec_C2) );
    ATH_CHECK( init_dec (m_dec_D2) );
    ATH_CHECK( init_dec (m_dec_pT) );
    ATH_CHECK( init_dec (m_dec_m) );
    ATH_CHECK( init_dec (m_dec_NConstits) );
    ATH_CHECK( init_dec (m_dec_eta) );
    ATH_CHECK( init_dec (m_dec_phi) );
    ATH_CHECK( init_dec (m_dec_timing) );

    // Set up the text-parsing machinery for selectiong the jet directly according to user cuts
    if (!m_selectionString.empty()) {
       ATH_CHECK( initializeParser( m_selectionString ));
    }

    // Define the trimmer
    m_trimmer.emplace(fastjet::JetDefinition(fastjet::kt_algorithm, m_rclus), fastjet::SelectorPtFractionMin(m_ptfrac));
    m_softdropper.emplace(m_beta, m_zcut, m_R0);

    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::RCJetSubstructureAug::addBranches() const
{
    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey,ctx);
    if (!jets.isValid()) {
        ATH_MSG_ERROR("No jet collection with name " << m_jetKey.key() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    unsigned int nJets(jets->size());
    std::vector<const xAOD::Jet*> jetToCheck; jetToCheck.clear();
    
    // Execute the text parser if requested
    if (!m_selectionString.empty()) {
        std::vector<int> entries =  m_parser->evaluateAsVector();
        unsigned int nEntries = entries.size();
        // check the sizes are compatible
        if (nJets != nEntries ) {
        	ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used jets??");
            return StatusCode::FAILURE;
        } else {
        	// identify which jets to keep for the thinning check
        	for (unsigned int i=0; i<nJets; ++i) if (entries[i]==1) jetToCheck.push_back((*jets)[i]);
        }
    } else {
        for (unsigned int i=0; i<nJets; ++i) jetToCheck.push_back((*jets)[i]);
    }

    using WDH = SG::WriteDecorHandle<xAOD::JetContainer, float>;
    WDH dec_Qw (m_dec_Qw, ctx);
    WDH dec_Tau1 (m_dec_Tau1, ctx);
    WDH dec_Tau2 (m_dec_Tau2, ctx);
    WDH dec_Tau3 (m_dec_Tau3, ctx);
    WDH dec_Tau4 (m_dec_Tau4, ctx);
    WDH dec_Tau21 (m_dec_Tau21, ctx);
    WDH dec_Tau32 (m_dec_Tau32, ctx);
    WDH dec_Split12 (m_dec_Split12, ctx);
    WDH dec_Split23 (m_dec_Split23, ctx);
    WDH dec_Split34 (m_dec_Split34, ctx);
    WDH dec_ECF1 (m_dec_ECF1, ctx);
    WDH dec_ECF2 (m_dec_ECF2, ctx);
    WDH dec_ECF3 (m_dec_ECF3, ctx);
    WDH dec_ECF4 (m_dec_ECF4, ctx);
    WDH dec_C2 (m_dec_C2, ctx);
    WDH dec_D2 (m_dec_D2, ctx);
    WDH dec_pT (m_dec_pT, ctx);
    WDH dec_m (m_dec_m, ctx);
    WDH dec_NConstits (m_dec_NConstits, ctx);
    WDH dec_eta (m_dec_eta, ctx);
    WDH dec_phi (m_dec_phi, ctx);
    WDH dec_timing (m_dec_timing, ctx);
    
    std::vector<const xAOD::IParticle*> mergedGhostConstits;
    std::vector<const xAOD::IParticle*> ghosts;
    std::vector<fastjet::PseudoJet> constituents;

    for (const xAOD::Jet* jet : jetToCheck) {
        
        // Get list of ghost constituents
        mergedGhostConstits.clear();
        for (std::string ghostName: m_ghostNames) {
            ghosts.clear();
            ATH_CHECK(jet->getAssociatedObjects(ghostName, ghosts));
            mergedGhostConstits.insert(mergedGhostConstits.end(), ghosts.begin(), ghosts.end());
        }

        // Construct list of constituent PseudoJets from constituents and compute timing information
        constituents.clear();
        double eTot = 0;
        double time = 0;
        for (auto constit : mergedGhostConstits){
            if (constit==nullptr) {
                ATH_MSG_INFO("Invalid constituent link, skipping ...");
                continue;
            }
            // Filter constituents on negative energy
            if (constit->e()<0) {ATH_MSG_INFO("################## Negative energy cluster"); continue;}
            constituents.push_back( fastjet::PseudoJet(constit->p4()) );

            // Timing info of constituent only for calo constituents
            if (constit->type() == xAOD::Type::CaloCluster) {
                auto caloConstit = dynamic_cast<const xAOD::CaloCluster*> (constit);
                double eConstit = caloConstit->e()* caloConstit->e();
                time += caloConstit->time()* eConstit;
                eTot += eConstit;
            }            
        }

        // Fill timing info
        if (eTot==0) { 
            dec_timing(*jet) = 0;
        } else {
            dec_timing(*jet) = time/eTot;
        }

        // Get number of constituents
        int nConstits = constituents.size();
        
        // Run clustering on constituents if not empty
        fastjet::PseudoJet groomed_jet;
        if (nConstits!=0) {
            auto jet_def = fastjet::JetDefinition(fastjet::antikt_algorithm, 1.5);
            fastjet::ClusterSequence cs(constituents, jet_def);
            fastjet::PseudoJet recluster_jet = cs.inclusive_jets(0.0).front();

            // Apply grooming to reclustered jet
            if (m_grooming=="Trimming"){
                groomed_jet = m_trimmer->result(recluster_jet);
            } else if (m_grooming=="SoftDrop"){
                groomed_jet = m_softdropper->result(recluster_jet);
            } else {
                ATH_MSG_DEBUG(" No grooming requested or wrong one, will not apply grooming");
                groomed_jet = recluster_jet;
            }

            // update nConstit
            nConstits = groomed_jet.constituents().size();
        }

        // Fill substructure vars with default values when no constituents in jet
        if (nConstits==0) {
            dec_Qw(*jet) = -999;

            dec_Tau1(*jet) = -999;
            dec_Tau2(*jet) = -999;
            dec_Tau3(*jet) = -999;
            dec_Tau4(*jet) = -999;

            dec_Tau21(*jet) = -999;
            dec_Tau32(*jet) = -999;

            dec_Split12(*jet) = -999;
            dec_Split23(*jet) = -999;
            dec_Split34(*jet) = -999;

            dec_ECF1(*jet) = -999;
            dec_ECF2(*jet) = -999;
            dec_ECF3(*jet) = -999;
            dec_ECF4(*jet) = -999;

            dec_C2(*jet) = -999;
            dec_D2(*jet) = -999;

            dec_pT(*jet) = 0;
            dec_m(*jet) = 0;
            dec_NConstits(*jet) = 0;
            dec_eta(*jet) = -999;
            dec_phi(*jet) = -999;

            return StatusCode::SUCCESS;
        }
        
        // Save reclustered jet info
        dec_pT(*jet) = groomed_jet.pt();
        dec_m(*jet) = groomed_jet.m();
        dec_NConstits(*jet) = nConstits;
        dec_eta(*jet) = groomed_jet.eta();
        dec_phi(*jet) = groomed_jet.phi() - M_PI;

        // Qw
        JetSubStructureUtils::Qw qw;
        dec_Qw(*jet) = qw.result(groomed_jet);
        
        // Nsubjetiness
        fastjet::contrib::WTA_KT_Axes wta_kt_axes;
        fastjet::contrib::NormalizedCutoffMeasure normalized_measure(1.0, 1.0, 1000000);
        JetSubStructureUtils::Nsubjettiness Tau1_wta(1, wta_kt_axes, normalized_measure);
        JetSubStructureUtils::Nsubjettiness Tau2_wta(2, wta_kt_axes, normalized_measure);
        JetSubStructureUtils::Nsubjettiness Tau3_wta(3, wta_kt_axes, normalized_measure);
        JetSubStructureUtils::Nsubjettiness Tau4_wta(4, wta_kt_axes, normalized_measure);
        float tau1_wta = Tau1_wta.result(groomed_jet);
        float tau2_wta = Tau2_wta.result(groomed_jet);
        float tau3_wta = Tau3_wta.result(groomed_jet);
        float tau4_wta = Tau4_wta.result(groomed_jet);
        float tau21_wta = -999;
        float tau32_wta = -999;

        if( tau1_wta > 1e-8 ) {
            tau21_wta = tau2_wta / tau1_wta;
        }
        if( tau2_wta > 1e-8 ) {
            tau32_wta = tau3_wta / tau2_wta;
        }

        dec_Tau1(*jet) = tau1_wta;
        dec_Tau2(*jet) = tau2_wta;
        dec_Tau3(*jet) = tau3_wta;
        dec_Tau4(*jet) = tau4_wta;
        dec_Tau21(*jet) = tau21_wta;
        dec_Tau32(*jet) = tau32_wta;

        // KtSplittingScale
        JetSubStructureUtils::KtSplittingScale Split12(1);
        JetSubStructureUtils::KtSplittingScale Split23(2);
        JetSubStructureUtils::KtSplittingScale Split34(3);
        float split12 = Split12.result(groomed_jet);
        float split23 = Split23.result(groomed_jet);
        float split34 = Split34.result(groomed_jet);

        dec_Split12(*jet) = split12;
        dec_Split23(*jet) = split23;
        dec_Split34(*jet) = split34;

        // EnergyCorrelator
        JetSubStructureUtils::EnergyCorrelator ECF1(1, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        JetSubStructureUtils::EnergyCorrelator ECF2(2, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        JetSubStructureUtils::EnergyCorrelator ECF3(3, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        JetSubStructureUtils::EnergyCorrelator ECF4(4, 1.0, JetSubStructureUtils::EnergyCorrelator::pt_R);
        float ecf1 = ECF1.result(groomed_jet);
        float ecf2 = ECF2.result(groomed_jet);
        float ecf3 = ECF3.result(groomed_jet);
        float ecf4 = ECF4.result(groomed_jet);
        float c2 = -999;
        float d2 = -999;

        if( ecf2 > 1e-8 ) {
            c2 = ecf3 * ecf1 / pow( ecf2, 2.0 );
            d2 = ecf3 * pow( ecf1, 3.0 ) / pow( ecf2, 3.0 );
        }

        dec_ECF1(*jet) = ecf1;
        dec_ECF2(*jet) = ecf2;
        dec_ECF3(*jet) = ecf3;
        dec_ECF4(*jet) = ecf4;
        dec_C2(*jet) = c2;
        dec_D2(*jet) = d2;

    }
    
    return StatusCode::SUCCESS;
}


