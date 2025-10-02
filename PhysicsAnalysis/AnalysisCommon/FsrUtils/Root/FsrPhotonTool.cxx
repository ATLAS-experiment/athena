/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

// Local include(s):
#include "FsrUtils/FsrPhotonTool.h"
#include "IsolationSelection/IsolationSelectionTool.h"
#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisAlgorithms/EgammaIsGoodOQSelectionTool.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "AsgTools/AsgToolConfig.h"
#include "AthContainers/ConstDataVector.h"


namespace FSR {

    FsrPhotonTool::FsrPhotonTool( const std::string& name )
        : asg::AsgTool( name ),
          m_fsr_type(FsrCandidate::FsrUnknown),
          m_isoSelTool("", this),
          m_energyRescaler("", this),
          m_elIsGoodOQSelectionTool("", this),
          m_phIsGoodOQSelectionTool("", this)
    {}

    FsrPhotonTool::~FsrPhotonTool() 
    {}
    
    
    StatusCode FsrPhotonTool::initialize() {

        // Greet the user:
        ATH_MSG_INFO( "Initialising..." );

        ATH_MSG_INFO( "initialize: high_et_min " << m_high_et_min);
        ATH_MSG_INFO( "initialize: overlap_el_ph " << m_overlap_el_ph);
        ATH_MSG_INFO( "initialize: overlap_el_mu " << m_overlap_el_mu);
        ATH_MSG_INFO( "initialize: far_fsr_drcut " << m_far_fsr_drcut);
        ATH_MSG_INFO( "initialize: far_fsr_etcut " << m_far_fsr_etcut);
        ATH_MSG_INFO( "initialize: far_fsr_isoWorkingPoint " << m_far_fsr_isoWorkingPoint);
        ATH_MSG_INFO( "initialize: drcut " << m_drcut);
        ATH_MSG_INFO( "initialize: etcut " << m_etcut);
        ATH_MSG_INFO( "initialize: f1cut " << m_f1cut);
        ATH_MSG_INFO( "initialize: topo_drcut " << m_topo_drcut);
        ATH_MSG_INFO( "initialize: topo_f1cut " << m_topo_f1cut);
        ATH_MSG_INFO( "initialize: egCalibToolName " << m_energyRescalerName);
        
        // Create IsolationSelectionTool for far fsr selection. 
        // Will use the closeBy corrected isolation variables if DoCloseByCorrection is set to true, which is default.
        asg::AsgToolConfig config ("CP::IsolationSelectionTool/isolationSelectionTool");
        RETURN_CHECK("initialize", config.setProperty( "PhotonWP", m_far_fsr_isoWorkingPoint));

        if (m_doCloseByIso) {
            RETURN_CHECK("initialize", config.setProperty("IsoDecSuffix", "CloseByCorr")); 
            if (msg().level() <= MSG::DEBUG) {
                RETURN_CHECK( "initialize", config.setProperty( "OutputLevel", MSG::DEBUG) );
            }
        }
        RETURN_CHECK("initialize", config.makePrivateTool (m_isoSelTool));  
        ATH_MSG_INFO("initialize - photon IsolationSelectionTool initialized " << m_isoSelTool->name());

        RETURN_CHECK("initialize", m_isoSelTool.retrieve());

        // get egamma calibration tool - needed to recalibrate electron as a photon
        if (m_energyRescalerName.size()) {
            m_energyRescaler = ToolHandle<CP::IEgammaCalibrationAndSmearingTool>(m_energyRescalerName);
            if (m_energyRescaler.empty()) {
                ATH_MSG_ERROR("initialize - unable to get IEgammaCalibrationAndSmearingTool with name: " << m_energyRescalerName);
                return StatusCode::FAILURE;
            }

            ATH_MSG_INFO("initialize - found  IEgammaCalibrationAndSmearingTool with name: " << m_energyRescalerName);
        }
        else {
            ATH_MSG_ERROR("initialize - CANNOT retrive IEgammaCalibrationAndSmearingTool. Please set the property 'egCalibToolName' to be able to recalibrate fsr photon found as an electron ");
            return StatusCode::FAILURE;
        }
        
        // setup electron and photon object quality tools - checks object quality and Dead HV cell cuts

        // electron OQ
        asg::AsgToolConfig config1 ("CP::EgammaIsGoodOQSelectionTool/electronIsGoodOQSelectionTool");
        RETURN_CHECK("initialize", config1.setProperty( "Mask", xAOD::EgammaParameters::BADCLUSELECTRON));
        RETURN_CHECK("initialize", config1.makePrivateTool (m_elIsGoodOQSelectionTool));  
        RETURN_CHECK("initialize", m_elIsGoodOQSelectionTool.retrieve());      
        ATH_MSG_INFO("initialize - electronIsGoodOQSelectionTool initialized " << m_elIsGoodOQSelectionTool->name());

        // photon OQ
        asg::AsgToolConfig config2 ("CP::EgammaIsGoodOQSelectionTool/photonIsGoodOQSelectionTool");
        RETURN_CHECK("initialize", config2.setProperty( "Mask", xAOD::EgammaParameters::BADCLUSPHOTON));
        RETURN_CHECK("initialize", config2.makePrivateTool (m_phIsGoodOQSelectionTool));  
        RETURN_CHECK("initialize", m_phIsGoodOQSelectionTool.retrieve());      
        ATH_MSG_INFO("initialize - electronIsGoodOQSelectionTool initialized " << m_phIsGoodOQSelectionTool->name());

        // Return gracefully:
        return StatusCode::SUCCESS;
    }


    CP::CorrectionCode FsrPhotonTool::getFsrPhoton(const xAOD::IParticle* part, FsrCandidate& candidate,
                                                   xAOD::PhotonContainer* photons,
                                                   const xAOD::ElectronContainer* electrons) {

        if (photons == 0 ||electrons == 0) {
            ATH_MSG_ERROR( "getFsrPhoton: You MUST provide photon AND electron containers" );
            candidate = FsrCandidate();
            return CP::CorrectionCode::Error;
        }
        
            
        std::vector<FsrCandidate>* cands = getFsrCandidateList(part, photons, electrons);

        ATH_MSG_DEBUG( "Fsr candidate size = " << cands->size() );

        // Return gracefully:
        if (cands->size() > 0) {
            candidate = cands->at(0);
            return CP::CorrectionCode::Ok;
        }
        candidate = FsrCandidate();
        ATH_MSG_DEBUG( "Fsr candidate  f1 = " << candidate.f1
                       << ", deltaR = " << candidate.deltaR
                       << ", Et = " << candidate.Et
                       << ", Eta = " << candidate.eta
                       << ", Phi = " << candidate.phi
                       << ", type = " <<candidate.type);
        return CP::CorrectionCode::Error;

    }
       

    std::vector<FsrCandidate>* 
    FsrPhotonTool::getFsrCandidateList(const xAOD::IParticle* part,
                                       xAOD::PhotonContainer* photons,
                                       const xAOD::ElectronContainer* electrons) {
        xAOD::PhotonContainer* photons_cont = NULL;
        const xAOD::ElectronContainer* electrons_cont = NULL;

        // check for photon container
        if (0 == photons) {
            ATH_MSG_ERROR( "getFsrCandidateList: No Photon container provided" );
            return NULL;
        }
        else {
            // set the containers
            photons_cont   = photons;
        }
       
        // check for electron container
        if (0 == electrons) {
            ATH_MSG_ERROR( "getFsrCandidateList: No Electron container provided" );
            return NULL;
        }
        else {
            // set the containers
            electrons_cont = electrons;
        }

        /// Search for far fsr for for electron and muon
        /// Electron : far fsr is the only option  - return the result
        /// Muon     : In case collinar fsr not found, check the far fsr
    
        /// Set FSR type to far
        m_fsr_type = FsrCandidate::FsrType::FsrUnknown;
    
        const xAOD::Electron* electron = dynamic_cast<const xAOD::Electron*>(part);
        if (electron) {
            return getFarFsrCandidateList(part, photons_cont);
        }
    
        const xAOD::Muon* muon = dynamic_cast<const xAOD::Muon*>(part);
        if (muon) {
            std::vector<FsrCandidate>* nearlist = getNearFsrCandidateList(muon, photons_cont, electrons_cont);
            return (nearlist->size() > 0) ? nearlist : getFarFsrCandidateList(part, photons_cont);
        }
    
        ATH_MSG_WARNING( "The FSR search not done !" );
        return NULL;
    }
    
    std::vector<FsrCandidate>* FsrPhotonTool::getFarFsrCandidateList(const xAOD::IParticle* part, 
                                                                     xAOD::PhotonContainer* photons_cont) {


        static const SG::AuxElement::Accessor<char>  DFCommonPhotonsIsEMTight ("DFCommonPhotonsIsEMTight");
        static const SG::AuxElement::Accessor<float> topoetcone20             ("topoetcone20");

        /// Set FSR type to far
        m_fsr_type = FsrCandidate::FsrType::FsrFar;
        std::vector< std::pair <const xAOD::IParticle*, double> > farFsrCandList;
        farFsrCandList.clear();
        farFsrCandList.reserve(photons_cont->size());

        // Save muon or electron of overlap removal for isolation
        ConstDataVector<xAOD::IParticleContainer> parts(SG::VIEW_ELEMENTS);
        parts.push_back(part);
    
        ATH_MSG_DEBUG( "In getFarFsrCandidateList function : photon size = " << photons_cont->size());
    
        for (auto ph : *photons_cont) {

            bool oqIsOK = (bool) m_phIsGoodOQSelectionTool->accept(ph);
                
            bool is_tight_photon = DFCommonPhotonsIsEMTight(*ph);
            if ( oqIsOK && (ph->p4().Et() > m_far_fsr_etcut) && is_tight_photon) {
                // correct isolation leakage

                ATH_MSG_VERBOSE( "Far Fsr ph bef : pt   " << ph->pt() << " topoetcone20 = " << topoetcone20(*ph));

                // apply isolation selection
                bool farPhIsoOK = (bool) m_isoSelTool->accept(*ph);

                ATH_MSG_VERBOSE( "Far Fsr ph aft: pt   " << ph->pt() << " topoetcone20 = " << topoetcone20(*ph));

                bool far_fsr_drcut_isOK = false;
                if (farPhIsoOK) {
                    double dr = deltaR(part->eta(), part->phi(), ph->eta(), ph->phi());
                    far_fsr_drcut_isOK = (dr > m_far_fsr_drcut);

                    if (far_fsr_drcut_isOK && dr < 0.2) {
                        ATH_MSG_VERBOSE( "Far Fsr candidate kinematics : author  " << ph->author()
                                    << " Et = " << ph->p4().Et()
                                    << " dr = " << dr
                                    << " isoIsOK = " << farPhIsoOK );
                    }

                    if(far_fsr_drcut_isOK) farFsrCandList.push_back(std::make_pair(ph, dr));
                }
                    
                ATH_MSG_DEBUG( "Far Fsr candidate kinematics : author  " << ph->author()
                            << " Et = " << ph->p4().Et()
                            << " tight = " << is_tight_photon
                            << " farPhIsoOK = " << farPhIsoOK
                            << " far_fsr_drcut_isOK = " << far_fsr_drcut_isOK);
            }
        }
   
        /// return sorted list
        return sortFsrCandidates(farFsrCandList);
    }


    std::vector<FsrCandidate>* FsrPhotonTool::getNearFsrCandidateList(const xAOD::Muon* muon, 
                                                                      const xAOD::PhotonContainer* photons_cont, 
                                                                      const xAOD::ElectronContainer* electrons_cont) {
        /// Set FSR type to far
        m_fsr_type = FsrCandidate::FsrType::FsrNear;
        /// Start looking for collinar FSR for muons
        /// Loop over photon candidates and choose the FSR photons
        std::vector< std::pair <const xAOD::IParticle*, double> >  nearFsrCandList;
    
        nearFsrCandList.reserve(photons_cont->size()+electrons_cont->size());
        ATH_MSG_DEBUG( "In getNearFsrCandidateList function : photon size = " << photons_cont->size()
                        << ", electron size = " << electrons_cont->size());
    
        for (auto photon : *photons_cont) {
            float photon_f1;
            photon->showerShapeValue(photon_f1, xAOD::EgammaParameters::f1);

            bool oqIsOK = (bool) m_phIsGoodOQSelectionTool->accept(photon);

            // Selection is tighter for photons below high_et_min
            bool high_et_photon = (photon->p4().Et() > m_high_et_min);
            if(  ( oqIsOK && high_et_photon && (photon->p4().Et() > m_etcut) && (photon_f1 > m_f1cut) )
                 ||
                 ( oqIsOK && !high_et_photon && (photon_f1 > m_topo_f1cut)
                     && (m_etcut < photon->p4().Et())) ) {
   
                double dr = deltaR(muon->eta(), muon->phi(), photon->eta(), photon->phi());
   
                // ph_cl_eta/phi should be used in duplicate
                if (   (!high_et_photon && dr < m_topo_drcut)
                       || (high_et_photon && dr < m_drcut)  ) {
                    nearFsrCandList.push_back(std::make_pair(photon, dr));
                    ATH_MSG_DEBUG( "Near Fsr candidates ( photon ) kinematics ; author  "
                                   << photon->author()
                                   << " Et = " << photon->p4().Et()
                                   << " f1 = " << photon_f1
                                   << " dr = " << dr );
                }
            }
     	}
   
    	unsigned int nofPhFsr = nearFsrCandList.size();
   
        // look for fsr photons in electron container
        for (auto electron : *electrons_cont) {
            // remove overlap with fsr already found
            if ( (nearFsrCandList.size() > 0) && isOverlap(electron, nearFsrCandList, nofPhFsr) ) continue;

            // Check object quality
            // bool oqIsOK = electron->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON);
bool oqIsOK = (bool) m_elIsGoodOQSelectionTool->accept(electron);
   
            const xAOD::TrackParticle* electron_track = electron->trackParticle();
            const xAOD::TrackParticle* muon_track     = muon->primaryTrackParticle();

            bool elmutrackmatch = ( (fabs(electron_track->theta()- muon_track->theta()) < m_overlap_el_mu) &&
                                    (deltaPhi(electron_track->phi(),  muon_track->phi())   < m_overlap_el_mu) );
            float electron_f1;
            electron->showerShapeValue(electron_f1, xAOD::EgammaParameters::f1);

            // Get the corrected cluster energy:
            //  if the electron energy has been calibrated, we apply the change applied to the
            //  electron four vector assuming that the uncorrected energy is
            //  Eclus/cosh(track_eta). This will not work if an e-p combination has also been
            //  applied, and so the following will have to change.  RDS 04/2015.
            float eCorr = electron->p4().Et()/(electron->caloCluster()->e()/cosh(electron->trackParticle()->eta()));
            float clEt  = eCorr*electron->caloCluster()->et();

            ATH_MSG_VERBOSE( "Near Fsr candidate ( electron ) Et = " << clEt << " eCorr " << eCorr << " OQ is ok: " << (int) oqIsOK  << ", pt,eta,phi " << electron->pt()/1000 << ", " << electron->eta() << ", " << electron->phi());
        
            // Allow topo-clusters to come in as electrons - apply f1 sliding window cut for Et
            // > 3.5 GeV and f1 topo cut for Et 1-3.5 GeV
            if( elmutrackmatch && oqIsOK &&
                ((clEt < m_high_et_min && m_etcut < clEt && electron_f1 > m_topo_f1cut ) ||
                    (m_high_et_min < clEt                   && electron_f1 > m_f1cut )) ) {

                double dr = deltaR(muon->eta(), muon->phi(), electron->caloCluster()->eta(), electron->caloCluster()->phi());

                if ( dr < m_drcut ) {
                    nearFsrCandList.push_back(std::make_pair(electron, dr));
                    ATH_MSG_DEBUG( "Near Fsr candidates ( electron ) kinematics : author  "
                                    << electron->author()
                                    << " Et = " << clEt
                                    << " f1 = " << electron_f1
                                    << " dr = " << dr );
                }
                else ATH_MSG_VERBOSE( "FAILED Near Fsr candidates ( electron ) kinematics : author  "
                                        << electron->author()
                                        << " Et = " << clEt
                                        << " f1 = " << electron_f1
                                        << " dr = " << dr );
            }
            else {
                double dr = deltaR(muon->eta(), muon->phi(), electron->caloCluster()->eta(), electron->caloCluster()->phi());

                if (elmutrackmatch && dr < m_drcut ) {
                ATH_MSG_DEBUG( "FAILED Near Fsr candidates ( electron ) kinematics : author  "
                                << electron->author()
                                << " Et = " << clEt
                                << " f1 = " << electron_f1
                                << " dr = " << deltaR(muon->eta(), muon->phi(), electron->caloCluster()->eta(), electron->caloCluster()->phi())
                                << " theta/phi el/mu " << electron_track->theta() << "/" << muon->p4().Theta()
                                << "/" << electron_track->phi() << "/" << muon->phi()
                                << " theta/phi mu trk " << muon_track->theta() << "/" << muon_track->phi()
                                );
                }
            }
        }
   
        return sortFsrCandidates(nearFsrCandList);
   
    }  // end of getFsrPhotons


    std::vector<FsrCandidate>* FsrPhotonTool::sortFsrCandidates(
        const std::vector< std::pair <const xAOD::IParticle*, double> >& FsrCandList, 
        const std::string& option) {
   
        m_fsrPhotons.clear();
        m_fsrPhotons.reserve(FsrCandList.size());
   
        for (unsigned int i=0; i < FsrCandList.size(); i++ ) {
            FsrCandidate c;
            const xAOD::IParticle* particle = FsrCandList.at(i).first;
            const xAOD::Electron* electron = dynamic_cast<const xAOD::Electron*>(particle);
            const xAOD::Photon* photon = dynamic_cast<const xAOD::Photon*>(particle);
            float part_f1(-999);
            if ( electron ){
           	c.container =   "electron" ;

                // If we have an energy rescaler, calibrate correctly the electron as a photon
                if (!m_energyRescaler.empty()) {
                    xAOD::Photon photon;
                     photon.Egamma_v1::operator=(*electron);
                    if (m_energyRescaler->applyCorrection(photon) != CP::CorrectionCode::Ok) {
                        ATH_MSG_ERROR("FsrPhotonTool::sortFsrCandidates: Unable to applyCorrection to photon ");
                    }
                    if ( photon.showerShapeValue(part_f1, xAOD::EgammaParameters::f1) ) c.f1 = part_f1; 
                    c.Et  = photon.pt();
                    c.eta = photon.caloCluster()->eta();
                    c.phi = photon.caloCluster()->phi();

                    ATH_MSG_DEBUG("FsrPhotonTool::sortFsrCandidates: el/ph et " << electron->pt() << "/"
                                  << electron->caloCluster()->eta() << "/" << electron->caloCluster()->phi() << " "
                                  << photon.pt() << "/"
                                  << photon.caloCluster()->eta() << "/" << photon.caloCluster()->phi());
                }
                else {
                    // Use the cluster pt/eta/phi when considering the electron to be an FSR photon
                    // setup accessor for electron cluster corrected energy

                    // Get the corrected cluster energy:
                    //  if the electron energy has been calibrated, we apply the change applied to the
                    //  electron four vector assuming that the uncorrected energy is
                    //  Eclus/cosh(track_eta). This will not work if an e-p combination has also been
                    //  applied, and so the following will have to change.  RDS 04/2015.
                    float eCorr = electron->p4().Et()/(electron->caloCluster()->e()/cosh(electron->trackParticle()->eta()));
                    float clEt  = eCorr*electron->caloCluster()->et();
                    if ( electron->showerShapeValue(part_f1, xAOD::EgammaParameters::f1) ) c.f1 = part_f1; 
                    c.Et  = clEt;
                    c.eta = electron->caloCluster()->eta();
                    c.phi = electron->caloCluster()->phi();
                }

            }
            else if ( photon ) {
                c.container =   "photon";
            	if ( photon->showerShapeValue(part_f1, xAOD::EgammaParameters::f1) ) c.f1 = part_f1; 
                c.Et  = photon->p4().Et();
                c.eta = photon->eta();
                c.phi = photon->phi();
            }
            else {
                ATH_MSG_WARNING( "sortFsrCandidates: undefined particle - NOT electron nor photon. Should never get here!" );
           	c.container =   "";
                c.Et  = 0;
                c.eta = 0;
                c.phi = 0;
            }
            c.particle     = particle;
            c.deltaR       = FsrCandList.at(i).second;
            if(c.deltaR < 0.05) c.Et -= 400./cosh(particle->eta());
            c.type         = (c.deltaR > m_far_fsr_drcut) ? FsrCandidate::FsrFar : FsrCandidate::FsrNear;
            m_fsrPhotons.push_back(c);

            ATH_MSG_DEBUG( "sortFsrCandidates: save fsr candidate  f1 = " << c.f1
                           << ", deltaR = " << c.deltaR
                           << ", Et = " << c.Et
                           << ", Eta = " << c.eta
                           << ", Phi = " << c.phi
                           << ", type = " <<c.type);
            
        }
   
        ATH_MSG_DEBUG( "sortFsrCandidates: found " << m_fsrPhotons.size() << " FSR photons" );
        
        // sort FSR candidates w.r.t min dR or ET
        if (option=="ET") std::sort(m_fsrPhotons.begin(), m_fsrPhotons.end(), compareEt);
        else std::sort(m_fsrPhotons.begin(), m_fsrPhotons.end());
        return &m_fsrPhotons;
    }

    bool FsrPhotonTool::isOverlap(const xAOD::Electron_v1* electron, 
                                  const std::vector< std::pair <const xAOD::IParticle*, double> >& phfsr, 
                                  unsigned int nofPhFsr) {
        for (unsigned int indx=0; indx < nofPhFsr; indx++ ) {
                const xAOD::Photon* ph = dynamic_cast<const xAOD::Photon*>(phfsr.at(indx).first);
                const xAOD::CaloCluster* ph_cl = ph->caloCluster();
                const xAOD::CaloCluster* el_cl = electron->caloCluster();
                double dr = deltaR(el_cl->eta(), el_cl->phi(),
                                ph_cl->eta(), ph_cl->phi());
                ATH_MSG_VERBOSE( "isOverlap dr, ets " << (dr < m_overlap_el_ph) << "/" << dr << "/"
                                << ph_cl->et() << "/" << el_cl->et());
                if ( dr < m_overlap_el_ph ) return true;
        }
        return false;
    }
   
    double FsrPhotonTool::deltaPhi(float phi1, float phi2) const {
        double dphi= fabs(phi1 - phi2);
        if (dphi > M_PI) dphi=2*M_PI-dphi;
        return dphi;
    }
    

    double FsrPhotonTool::deltaR(float meta, float mphi, float peta, float pphi) const {
   
        double deta = fabs(meta - peta);
        double dphi = deltaPhi(mphi, pphi);
        double dR   = sqrt((dphi*dphi)+(deta*deta));
   
        return dR;
    }

} // namespace FSR
