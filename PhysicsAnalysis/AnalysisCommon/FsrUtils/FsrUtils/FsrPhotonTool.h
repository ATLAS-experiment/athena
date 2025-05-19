/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FSRUTILS_FsrPhotonTool_H
#define FSRUTILS_FsrPhotonTool_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include <AsgTools/PropertyWrapper.h>


// Local include(s):
#include "FsrUtils/IFsrPhotonTool.h"

namespace CP
{
    class IIsolationSelectionTool;
    class IEgammaCalibrationAndSmearingTool;
}
class IAsgSelectionTool;


namespace FSR {

    /// Implementation for the "FSR" provider tool
    ///
    /// @author Tulay Cuhadar Donszelmann <tcuhadar@cern.ch>
    /// @author RD Schaffer <R.D.Schaffer@cern.ch>
    ///
    class FsrPhotonTool : public virtual IFsrPhotonTool,
                          public asg::AsgTool {

        /// Create a proper constructor for Athena
        ASG_TOOL_CLASS( FsrPhotonTool, FSR::IFsrPhotonTool )

        public:
        /// Create a constructor for standalone usage
        FsrPhotonTool( const std::string& name );

        /// Create a destructor
        ~FsrPhotonTool();

        /// @name Function(s) implementing the asg::IAsgTool interface
        /// @{

        /// Function initialising the tool
        virtual StatusCode initialize();

        /// @}

        /// @name Function(s) implementing the IFsrPhotonTool interface
        /// @{

        /// Get the "FSR candidate" as a return value for a muon (collinar and far FSR)
        virtual CP::CorrectionCode getFsrPhoton(const xAOD::IParticle* part, FsrCandidate& candidate,
                                                xAOD::PhotonContainer* photons,
                                                const xAOD::ElectronContainer* electrons);

        /// Find ALL FSR candidates for a given particle (electron or muon)
        ///   providing newly calibrated photon and electron containers
        virtual std::vector<FsrCandidate>* getFsrCandidateList(const xAOD::IParticle* part,
                                                               xAOD::PhotonContainer* photons,
                                                               const xAOD::ElectronContainer* electrons);

        /// Find and Return ALL FAR FSR candidates
        virtual std::vector<FsrCandidate>* getFarFsrCandidateList(const xAOD::IParticle* part,
                                                                  xAOD::PhotonContainer* photons_cont);

        /// Find and Return ALL NEAR FSR candidates
        virtual std::vector<FsrCandidate>* getNearFsrCandidateList(const xAOD::Muon* part,
                                                                   const xAOD::PhotonContainer* photons_cont,
                                                                   const xAOD::ElectronContainer* electrons_cont);

        /// @}
    private:
        /// Need for the FSR search
        std::vector<FsrCandidate>* sortFsrCandidates( const std::vector< std::pair <const xAOD::IParticle*, double> >& FsrCandList,
                                                      const std::string& option="ET");
        bool isOverlap(const xAOD::Electron_v1* electron, const std::vector< std::pair <const xAOD::IParticle*, double> >& phfsr,
                       unsigned int nofPhFsr);
        double deltaR(float muonEta, float muonPhi, float phEta, float phPhi) const;
        double deltaPhi(float phi1, float phi2) const;
        static bool compareEt(const FsrCandidate& c1, const FsrCandidate& c2) { return (c1.Et > c2.Et); }

        // Note: selecion originally had separate cuts for sliding window clusters and topo clusters
        //       where the min Et threshold for SW clusters was higher than for topoclusters
        // TODO: remove this distinction since there are only topo clusters in the reconstruction since 2017
        Gaudi::Property<double> m_high_et_min  {this, "high_et_min", 3500. , "Minimum Et cut for higg Et photons" };
        Gaudi::Property<double> m_overlap_el_ph{this, "overlap_el_ph", 0.01 , "Overlap dR for electrons and photons" };
        Gaudi::Property<double> m_overlap_el_mu{this, "overlap_el_mu", 0.001 , "Overlap dR for electrons and muons" };
        Gaudi::Property<double> m_far_fsr_drcut{this, "far_fsr_drcut", 0.15 , "Minimum dR cut for far fsr" };
        Gaudi::Property<double> m_far_fsr_etcut{this, "far_fsr_etcut", 10000.0 , "Minimum et cut for far fsr" };
        Gaudi::Property<double> m_drcut     {this, "drcut", 0.15 , "Maximun dR cut to be near fsr" };
        Gaudi::Property<double> m_etcut     {this, "etcut", 1000.0 , "Minimum Et cut for near fsr" };
        Gaudi::Property<double> m_f1cut     {this, "f1cut",  0.1 , "f1 cut for high Et clusters" };
        Gaudi::Property<double> m_topo_drcut{this, "topo_drcut",  0.08 , "Maximum dR cut for low Et clusters" };
        Gaudi::Property<double> m_topo_f1cut{this, "topo_f1cut", 0.2 , "Minimum f1 cut for low Et clusters" };
        Gaudi::Property<std::string> m_far_fsr_isoWorkingPoint{this, "far_fsr_isoWorkingPoint", "FixedCutLoose", "Far fsr isolation working point" };
        Gaudi::Property<std::string> m_energyRescalerName{this, "egCalibToolName", "" , "EnergyRescale tool to calibrate photons as electrons" };
        Gaudi::Property<bool> m_suppresEnergyRescaling{this, "SuppressEnergyRescaling", false , "flag to suppress energy rescaling - should only be used for testing purposes, e.g. test w/o calib for PhysLite " };
        Gaudi::Property<bool> m_AFII_corr{this, "AFII_corr", false , "Is AFII for isolation correction" };
        Gaudi::Property<bool> m_is_mc{this, "IsMC", true , "Is MC" };
        //Flag to use isolation variables with 'CloseByCorr' suffix which have been corrected for close-by leptons and photons. If false, the uncorrected variables are used 
        Gaudi::Property<bool> m_doCloseByIso{this, "DoCloseByCorrection", true , "flag to use isolation variables with 'CloseByCorr' suffix " };

        std::vector<FsrCandidate> m_fsrPhotons;
        FsrCandidate::FsrType     m_fsr_type;

        ToolHandle<CP::IIsolationSelectionTool>           m_isoSelTool;
        ToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_energyRescaler;
        ToolHandle<IAsgSelectionTool>                     m_elIsGoodOQSelectionTool;
        ToolHandle<IAsgSelectionTool>                     m_phIsGoodOQSelectionTool;


    }; // class FsrPhotonTool

} // namespace FSR

#endif // FSRUTILS_FsrPhotonTool_H
