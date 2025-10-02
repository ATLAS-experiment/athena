/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Framework include(s):
#include "AsgDataHandles/ReadHandle.h"
#include "PathResolver/PathResolver.h"
#include "FourMomUtils/xAODP4Helpers.h"
// Local include(s):
#include <cmath>
#include "TRandom3.h"

#include "MuonMomentumCorrections/MuonCalibIntSagittaTool.h"
#include "MuonMomentumCorrections/EnumDef.h"
#include <MuonMomentumCorrections/CalibInitializer.h>

namespace CP
{

    MuonCalibIntSagittaTool::MuonCalibIntSagittaTool(const std::string &name) : asg::AsgTool(name)
    {    }

    StatusCode MuonCalibIntSagittaTool::initialize()
    {
        // Greet the user:
        ATH_MSG_INFO("Initializing MuonCalibIntSagittaTool");

        // Get the m_eventinfo container
        ATH_CHECK(m_eventInfo.initialize());

        // Read the recommendations
        for(const auto& year: MCP::dataYearList)
        {
            m_sagittaCorrConst_ID[year] = MCP::CalibInitializer::createSagittaCorrMap(year, MCP::TrackType::ID, m_release, "data");
            m_sagittaCorrConst_ME[year] = MCP::CalibInitializer::createSagittaCorrMap(year, MCP::TrackType::ME, m_release, "data");
            m_sagittaCorrConst_CB[year] = MCP::CalibInitializer::createSagittaCorrMap(year, MCP::TrackType::CB, m_release, "data");

            // Corrections from the MC - what is typically referred to as the kinematic term
            m_sagittaCorrConst_mcID[year] = MCP::CalibInitializer::createSagittaCorrMap(year, MCP::TrackType::ID, m_release, "mc");
            m_sagittaCorrConst_mcME[year] = MCP::CalibInitializer::createSagittaCorrMap(year, MCP::TrackType::ME, m_release, "mc");
            m_sagittaCorrConst_mcCB[year] = MCP::CalibInitializer::createSagittaCorrMap(year, MCP::TrackType::CB, m_release, "mc");
            ATH_MSG_VERBOSE("Sagitta initilised: Year " <<m_release);
        }

        m_currentParameters = nullptr;
        // Init the systematics
        m_Parameters.initialize(affectingSystematics(), [this](const SystematicSet &systConfig, ParameterSetSagitta &param)
                                { return calcSystematicVariation(systConfig, param); });
        if (!applySystematicVariation(SystematicSet()))
        {
            ATH_MSG_ERROR("Unable to run with no systematic");
            return StatusCode::FAILURE;
        }
        SystematicRegistry &registry = SystematicRegistry::getInstance();
        if (registry.registerSystematics(*this) != StatusCode::SUCCESS)
        {
            ATH_MSG_ERROR("Unkown systematic list");
            return StatusCode::FAILURE;
        }
        // Return gracefully:
        return StatusCode::SUCCESS;
    }

    CorrectionCode MuonCalibIntSagittaTool::applyCorrection(MCP::MuonObj& mu) const
    {
        auto IDCorrections = getCorrections(mu.ID);
        auto MECorrections = getCorrections(mu.ME);
        auto CBCorrections = getCorrections(mu.CB);

        // Directly correct all three tracks
        double corrIDpT = mu.ID.calib_pt;
        applySagittaCorrection(corrIDpT, IDCorrections, mu.ID.calib_charge, mu.ID.isData);

        double corrMEpT = mu.ME.calib_pt;
        applySagittaCorrection(corrMEpT, MECorrections, mu.ME.calib_charge, mu.ME.isData);

        double corrDirectCBpT = mu.CB.calib_pt;
        applySagittaCorrection(corrDirectCBpT, CBCorrections, mu.CB.calib_charge, mu.CB.isData);


        // Perform the statistical combination of ID + ME, before and after correction. Apply that ratio as a correction to create the weighted rho
        double corrStatCombCBpT = statCombCorrection(mu, corrIDpT, corrMEpT, mu.CB.calib_pt);

        // Calculation of rho 
        // The central value is average value and width of muons coming Z
        // The idea of this rho calculationsi that we want the muons at 45 GeV to get the CB correction
        // And for it to slowly taper to ID+MS at the low and high pT specturm
        // All this if rho is dynamically set and not fixed
        double central(45.2), width(15.5);
        double rho = 0;

        // If we are doing the systematic, shift the central values by the expected resolution of the muon
        if (m_currentParameters->SagittaRho != MCP::SystVariation::Default) 
        {
            double sigmaID = mu.expectedResID;
            double sigmaME = mu.expectedResME;
            double denominator = mu.CB.calib_pt * std::sqrt(sigmaID * sigmaID + sigmaME * sigmaME);
            double res = denominator ? M_SQRT2 * sigmaID * sigmaME / denominator : 0.;

            if (m_currentParameters->SagittaRho == MCP::SystVariation::Up) 
            {
                central += std::abs(0.5 * res * central);
            } else if (m_currentParameters->SagittaRho == MCP::SystVariation::Down) 
            {
                central -= std::abs(0.5 * res * central);
            }
        }

        // Calculate the rho
        if (!m_useFixedRho)
        {
            double sigmas = std::max(1.,(std::abs(mu.CB.calib_pt - central) / width));
            rho = 1. / sigmas;
           
        }
        else rho = m_fixedRhoValue;

        // Caculated the corrected pT
        double corrCBpT = rho * corrDirectCBpT + (1 - rho) * corrStatCombCBpT;

        ATH_MSG_DEBUG("Saggita correction - corrDirectCBpT: " << corrDirectCBpT << " corrStatCombCBpT: " << corrStatCombCBpT << " rho " << rho<<" corrCBPt: "<<corrCBpT);

        // Write the pT into the object
        mu.ID.calib_pt = corrIDpT;
        mu.ME.calib_pt = corrMEpT;
	if(m_doDirectCBCalib) mu.CB.calib_pt = corrCBpT;
	else mu.CB.calib_pt = corrStatCombCBpT;

        // Return gracefully:
        return CorrectionCode::Ok;
    }

    // This function applies the vector of corrections iteratively to the pT
    void MuonCalibIntSagittaTool::applySagittaCorrection(double& pt, const std::vector<double>& correction, const int& charge, const bool& isData) const
    {
        for(const auto& corr: correction)
        {
            double originalPt = pt;
            if(isData) pt = pt / (1 + charge * corr * pt);
            else pt = pt / (1 - charge * corr * pt);
            ATH_MSG_DEBUG("CorrectForCharge - in pT: " << originalPt << " corrPt: " << pt << " applied saggita: " << corr);

        }
    }


    // Get the set of correction, based on data + systematic setting
    std::vector<double> MuonCalibIntSagittaTool::getCorrections(const MCP::TrackCalibObj& trk) const
    {        
        double eta = trk.eta;
        double pT = trk.calib_pt;
        SagittaCorrConstMap const *corrMap{nullptr};
        SagittaCorrConstMap const *kinematicTermMap{nullptr};
        if(trk.type == MCP::TrackType::CB)
        {
            corrMap          = &m_sagittaCorrConst_CB.at(trk.year);
            kinematicTermMap = &m_sagittaCorrConst_mcCB.at(trk.year);
        } 
        else if(trk.type == MCP::TrackType::ID)
        { 
            corrMap          = &m_sagittaCorrConst_ID.at(trk.year);
            kinematicTermMap = &m_sagittaCorrConst_mcID.at(trk.year);
        }
        else if(trk.type == MCP::TrackType::ME)
        { 
            corrMap          = &m_sagittaCorrConst_ME.at(trk.year);
            kinematicTermMap = &m_sagittaCorrConst_mcME.at(trk.year);
        }
        
        std::vector<double> corrections;
        if(trk.isData)
        {
            if(!m_applyCorrectionOnData) return corrections;
            else if (m_applyCorrectionOnData)
            {
                // Apply data - kinematic term for the nominal
                double corr = corrMap->at(MCP::SagittaCorrection::Nominal)->getCalibConstant(trk);
                corr -= kinematicTermMap->at(MCP::SagittaCorrection::Nominal)->getCalibConstant(trk);
                corrections.push_back(corr);
            }
        }
        else // if it is MC
        {
            // Otherwise do as needed
            if(m_currentParameters->SagittaBias != MCP::SystVariation::Default)
            {
                int scale = (m_currentParameters->SagittaBias ==  MCP::SystVariation::Up) ? 1: -1;
                // Sanity check
                if((m_currentParameters->SagittaEtaSlice != MCP::SystVariation::Default) && !m_applyCorrectionOnData)
                {
                    ATH_MSG_ERROR("Sagitta correction is not applied to data, yet Eta dependant systematics are requested. This configuration is not supported");
                    return corrections;
                }

                // if no apply correction on data, apply 100% of (Data - kinematic term)
                // TODO: Check if it 50% or 100%? 
                if(!m_applyCorrectionOnData) 
                {
                    // Apply data - kinematic term for the nominal
                    double corr =  corrMap->at(MCP::SagittaCorrection::Nominal)->getCalibConstant(trk);
                    ATH_MSG_VERBOSE("Raw corr: "<<corr);
                    // corr -= kinematicTermMap->at(MCP::SagittaCorrection::Nominal)->getCalibConstant(eta, phi);
                    // ATH_MSG_VERBOSE("Raw - Kinematic corr: "<<corr);
                    corr *= (scale);
                    ATH_MSG_VERBOSE("corr: "<<corr);
                    corrections.push_back(corr);
                }

                // 100% of resbiasMap added as res bias 
                double corr =  corrMap->at(MCP::SagittaCorrection::Residual__1up)->getCalibConstant(trk);
                corr *= scale;
                ATH_MSG_VERBOSE("Residual corr: "<<corr);

                // If eta dependant, set the p2 to 0, if it not in the given eta slices
                if ((m_currentParameters->SagittaEtaSlice == MCP::SystVariation::Up) && eta < 0) corr = 0;
                if ((m_currentParameters->SagittaEtaSlice == MCP::SystVariation::Down) && eta > 0) corr = 0;

                corrections.push_back(corr);
                ATH_MSG_VERBOSE("final corr: "<<corr);
            }
            else if(m_currentParameters->SagittaGlobal != MCP::SystVariation::Default)
            {
                int scale = (m_currentParameters->SagittaGlobal ==  MCP::SystVariation::Up) ? 1: -1;

                double deltas = m_extraRebiasSys;
                // systematic for Run3 data 2022
                if (trk.year==MCP::DataYear::Data22) {
                   if(m_release.value().find("Recs2023") != std::string::npos) deltas = 1.2 * deltas;
                   else if(std::abs(eta)>1.05)  deltas = 1.5 * deltas;
                }
                double corr = deltas * scale;
                ATH_MSG_VERBOSE("Deltas corr: "<<deltas);

                corrections.push_back(corr);
                ATH_MSG_VERBOSE("final corr: "<<corr);
 
            }
            else if(m_currentParameters->SagittaPtExtra != MCP::SystVariation::Default)
            {
                int scale = (m_currentParameters->SagittaPtExtra ==  MCP::SystVariation::Up) ? 1: -1;

                // Extra scaling to cover for non-closure in the forward and transition region
                // As seen in the Rel 21 validation of the sagitta correction
                // It is only concetrateed in a few high eta bin. Idea is to apply a linearly increasing sys
                // Till it reach 450 GeV and then to flatten it. 
                // The value is chosen in an arbitrary fashion. To be replaced and fixed, once we have a better idea of 
                double corr = 0;
                double deltas = 0.00002;
               
                if (corrMap->at(MCP::SagittaCorrection::PtExtra__1up)->mapExist()) {
                  deltas = corrMap->at(MCP::SagittaCorrection::PtExtra__1up)->getCalibConstant(trk);
                  if (pT > 450.0)
                    corr += std::abs(450.0 - 45) * deltas;  // Above 450 GeV flat
                  else
                    corr += std::abs(pT - 45) * deltas;
                } else {
                  // old style uncertainty for Run2 and early Run3 recommendations 
                  if (eta > 2 || (eta > -2 && eta < -1.05)) {
                      if (pT > 450.0)
                          corr += std::abs(450.0 - 45) / 100 * deltas;  // Above 450 GeV flat
                      else
                          corr += std::abs(pT - 45) / 100 * deltas;
                  }
                  if (eta < -2 || (eta < 2 && eta> 1.5)) {
                      if (pT > 450.0)
                          corr += std::abs(450.0 - 45) / 200 * deltas;  // Above 450 GeV flat
                      else
                          corr += std::abs(pT - 45) / 200 * deltas;
                  }
                  // additional uncertainties for 2022 data
                  if (m_release.value().find("Recs2023") != std::string::npos) {
                    if ( (trk.year==MCP::DataYear::Data22) && pT > 100.0) {
                      if (eta < 0 && eta> -0.5) corr += 2.1*deltas;
                      else if (eta < -1.05) corr += 1.1*deltas;
                      else if (eta > 0.5 ) corr += 0.8*deltas;
                    }
                  } else {
                    if ( trk.year==MCP::DataYear::Data22 ) {
                      if (eta > -2 && eta < -1.05) corr = corr*2.5;
                      if (eta < -2) corr = corr*6;
                      if (eta > 1.5 && eta < 2) {
                        if (pT > 450.0)
                          corr += std::abs(450.0 - 45) / 80 * deltas;  // Above 450 GeV flat
                        else
                          corr += std::abs(pT - 45) / 80 * deltas;
                      }
                      if (eta > 1.05 && eta < 1.5) {
                        if (pT > 450.0)
                          corr += std::abs(450.0 - 45) / 40 * deltas;  // Above 450 GeV flat
                        else
                          corr += std::abs(pT - 45) / 40 * deltas;
                      }
                    }
                  }
                  // done for old style uncertainty for Run2 and early Run3 recommendations 
                }

                corrections.push_back(corr*scale);
                ATH_MSG_VERBOSE("High pT variation for pT "<<pT<<" deltas "<<deltas<<"  final corr: "<<corr);

            }
            else if(m_currentParameters->SagittaDataStat != MCP::SystVariation::Default)
            {
                int scale = (m_currentParameters->SagittaDataStat ==  MCP::SystVariation::Up) ? 1: -1;
                // Apply 50% of the datastat map, up and down as stat error
                // TODO change to 100%
                // -1 is to match the convention
                double corr = corrMap->at(MCP::SagittaCorrection::Datastat__1up)->getCalibConstant(trk);
                corr *= (0.5 * scale);
                ATH_MSG_VERBOSE("data stat final corr: "<<corr);
                corrections.push_back(corr);
            }
            else if(m_currentParameters->SagittaRho != MCP::SystVariation::Default)
            {
                // For rho systematic, nothing happens right now
            }
            else
            {
                // if it is MC, then for nominal corrections - we don't apply anything
            }
        }

        return corrections;
    }


    double MuonCalibIntSagittaTool::statCombCorrection(const MCP::MuonObj& mu, double corrIDpT, double corrMEpT, double CBpT) const
    {
        ATH_MSG_VERBOSE("Sag mu.ID.calib_pt: "<<mu.ID.calib_pt);
        ATH_MSG_VERBOSE("Sag mu.ME.calib_pt: "<<mu.ME.calib_pt);
        ATH_MSG_VERBOSE("Sag mu.CB.calib_pt: "<<mu.CB.calib_pt);

        // Corner cases to prevent a NaN in the combination
        if(mu.ID.calib_pt == 0) return CBpT;
        if(mu.ME.calib_pt == 0) return CBpT;
        if(corrIDpT == 0) return CBpT;
        if(corrMEpT == 0) return CBpT;
        
        double chi2Nom = -999;
        AmgVector(5)        parsCBNom   = mu.CB.pars;
        AmgVector(5)        parsID      = mu.ID.pars;
        AmgVector(5)        parsMS      = mu.ME.pars;
        AmgSymMatrix(5)     covCBNom    = mu.CB.covariance;
        AmgSymMatrix(5)     covID       = mu.ID.covariance;
        AmgSymMatrix(5)     covMS       = mu.ME.covariance;
           
        AmgVector(5)        parsCBCorr;
        AmgSymMatrix(5)     covCBCorr;

        // Do the statistical combination, with the original pTs
        // create the TLV with original ID pT
        using TLV = ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double>>;
        TLV tlv{mu.ID.calib_pt, mu.ID.eta, mu.ID.phi, mu.ID.mass};
        // Now modify the ID covariance matrix, and convert it to MeV
        if(tlv.P() == 0) parsID[4] = 1e12;
        else parsID[4] = 1.0 / (tlv.P() * 1e3);

        tlv.SetCoordinates(mu.ME.calib_pt, mu.ME.eta, mu.ME.phi, mu.ME.mass);   
        // Now modify the ME covariance matrix
        if(tlv.P() == 0) parsMS[4] = 1e12;
        else parsMS[4] = 1.0 / (tlv.P() * 1e3);

        CorrectionCode SysCorrCode = applyStatCombination(parsID, covID, parsMS, covMS, mu.CB.calib_charge, parsCBNom, covCBNom, chi2Nom);
        if (SysCorrCode != CorrectionCode::Ok) return CBpT;


        // Do the statistical combination, with the original pTs
        // create the TLV with original ID pT
        tlv.SetCoordinates(corrIDpT,  mu.ID.eta, mu.ID.phi, mu.ID.mass);
        // Now modify the ID covariance matrix, and convert it to MeV
        if(tlv.P() == 0) parsID[4] = 1e12;
        else parsID[4] = 1.0 / (tlv.P() * 1e3);

        tlv.SetCoordinates(corrMEpT, mu.ME.eta, mu.ME.phi, mu.ME.mass);   
        // Now modify the ME covariance matrix
        if(tlv.P() == 0) parsMS[4] = 1e12;
        else parsMS[4] = 1.0 / (tlv.P() * 1e3);

        SysCorrCode = applyStatCombination(parsID, covID, parsMS, covMS, mu.CB.calib_charge, parsCBCorr, covCBCorr, chi2Nom);
        if (SysCorrCode != CorrectionCode::Ok) return CBpT;


        double statCombPtNom  = std::sin(parsCBNom[3]) / std::abs(parsCBNom[4]);
        double statCombPtCorr = std::sin(parsCBCorr[3]) / std::abs(parsCBCorr[4]);
        double corrCBpT = CBpT * (statCombPtCorr / statCombPtNom);

        return corrCBpT;

    }
    CorrectionCode MuonCalibIntSagittaTool::applyStatCombination(AmgVector(5) parsID, const AmgSymMatrix(5)& covID, AmgVector(5) parsMS,
                                                                        const AmgSymMatrix(5)& covMS, int charge, AmgVector(5) & parsCB,
                                                                        AmgSymMatrix(5) & covCB, double& chi2) const {
        chi2 = 1e20;
        parsID[4] = std::abs(parsID[4]);
        parsMS[4] = std::abs(parsMS[4]);


        Eigen::FullPivLU<AmgSymMatrix(5)> matID(covID);
        if (!matID.isInvertible()) {
            ATH_MSG_DEBUG(" ID weight matrix computation failed     ");
            return CorrectionCode::Error;
        }
        const AmgSymMatrix(5) weightID = matID.inverse();


        Eigen::FullPivLU<AmgSymMatrix(5)> matMS(covMS);
        if (!matMS.isInvertible()) {
            ATH_MSG_DEBUG(" MS weight matrix computation failed     ");
            return CorrectionCode::Error;
        }
        const AmgSymMatrix(5) weightMS = matMS.inverse();


        Eigen::FullPivLU<AmgSymMatrix(5)> matCB(weightID + weightMS);
        if (!matCB.isInvertible()) {
            ATH_MSG_DEBUG(" Inversion of weightCB failed ");
            return CorrectionCode::Error;
        }
        covCB = matCB.inverse();


        Eigen::FullPivLU<AmgSymMatrix(5)> matSum(covID + covMS);
        if (!matSum.isInvertible()) {
            ATH_MSG_DEBUG(" Inversion of weightCB failed ");
            return CorrectionCode::Error;
        }
        AmgSymMatrix(5) invCovSum = matSum.inverse();


        AmgVector(5) diffPars = parsID - parsMS;
        chi2 = diffPars.transpose() * invCovSum * diffPars;
        chi2 = chi2 / 5.;

        parsCB = covCB * (weightID * parsID + weightMS * parsMS);
        parsCB[4] *= charge;

        if (parsCB[2] > M_PI)
            parsCB[2] -= 2. * M_PI;
        else if (parsCB[2] < -M_PI)
            parsCB[2] += 2. * M_PI;
        return CorrectionCode::Ok;
    }



    bool MuonCalibIntSagittaTool::isAffectedBySystematic(const SystematicVariation &systematic) const
    {
        SystematicSet sys = affectingSystematics();
        return sys.find(systematic) != sys.end();
    }

    SystematicSet MuonCalibIntSagittaTool::affectingSystematics() const
    {
        SystematicSet result;
        //////////////////////
        /// Sagitta Systematis
        //////////////////////
        if (!m_useFixedRho || m_sysScheme == "AllSys") {
            result.insert(SystematicVariation("MUON_SAGITTA_RHO", 1));
            result.insert(SystematicVariation("MUON_SAGITTA_RHO", -1));
        }

        if (m_doEtaSagittaSys) {
            // Sagitta correction residual bias
            result.insert(SystematicVariation("MUON_SAGITTA_RESBIAS_NEGETA", 1));
            result.insert(SystematicVariation("MUON_SAGITTA_RESBIAS_NEGETA", -1));

            result.insert(SystematicVariation("MUON_SAGITTA_RESBIAS_POSETA", 1));
            result.insert(SystematicVariation("MUON_SAGITTA_RESBIAS_POSETA", -1));

        } 
        else {
            // Sagitta correction residual bias
            result.insert(SystematicVariation("MUON_SAGITTA_RESBIAS", 1));
            result.insert(SystematicVariation("MUON_SAGITTA_RESBIAS", -1));
        }

        result.insert(SystematicVariation("MUON_SAGITTA_DATASTAT", 1));
        result.insert(SystematicVariation("MUON_SAGITTA_DATASTAT", -1));
        
        result.insert(SystematicVariation("MUON_SAGITTA_GLOBAL", 1));
        result.insert(SystematicVariation("MUON_SAGITTA_GLOBAL", -1));

        result.insert(SystematicVariation("MUON_SAGITTA_PTEXTRA", 1));
        result.insert(SystematicVariation("MUON_SAGITTA_PTEXTRA", -1));

        return result;
    }

    SystematicSet MuonCalibIntSagittaTool::recommendedSystematics() const { return affectingSystematics(); }

    StatusCode MuonCalibIntSagittaTool::calcSystematicVariation(const SystematicSet &systConfig, ParameterSetSagitta &param) const
    {
        param.SagittaRho        = MCP::SystVariation::Default;
        param.SagittaBias       = MCP::SystVariation::Default;
        param.SagittaGlobal     = MCP::SystVariation::Default;
        param.SagittaPtExtra    = MCP::SystVariation::Default;
        param.SagittaDataStat   = MCP::SystVariation::Default;
        param.SagittaEtaSlice   = MCP::SystVariation::Default;

        // Sagitta Rho systematics
        SystematicVariation syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_RHO");

        if      (syst == SystematicVariation("MUON_SAGITTA_RHO",  1)) param.SagittaRho = MCP::SystVariation::Down;
        else if (syst == SystematicVariation("MUON_SAGITTA_RHO", -1)) param.SagittaRho = MCP::SystVariation::Up;
        else if (!syst.empty()) return StatusCode::FAILURE;

        // Sagitta Residual Bias systematics
        syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_RESBIAS");

        if      (syst == SystematicVariation("MUON_SAGITTA_RESBIAS",  1)) param.SagittaBias = MCP::SystVariation::Down;
        else if (syst == SystematicVariation("MUON_SAGITTA_RESBIAS", -1)) param.SagittaBias = MCP::SystVariation::Up;
        else if (!syst.empty()) return StatusCode::FAILURE;

        // Sagitta Residual Global Bias systematics
        syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_GLOBAL");

        if      (syst == SystematicVariation("MUON_SAGITTA_GLOBAL",  1)) param.SagittaGlobal = MCP::SystVariation::Down;
        else if (syst == SystematicVariation("MUON_SAGITTA_GLOBAL", -1)) param.SagittaGlobal = MCP::SystVariation::Up;
        else if (!syst.empty()) return StatusCode::FAILURE;

        // Sagitta Residual Bias systematics pt extrapolation
        syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_PTEXTRA");

        if      (syst == SystematicVariation("MUON_SAGITTA_PTEXTRA",  1)) param.SagittaPtExtra = MCP::SystVariation::Down;
        else if (syst == SystematicVariation("MUON_SAGITTA_PTEXTRA", -1)) param.SagittaPtExtra = MCP::SystVariation::Up;
        else if (!syst.empty()) return StatusCode::FAILURE;

        // Sagitta Residual Bias systematics
        syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_RESBIAS_POSETA");

        if (syst == SystematicVariation("MUON_SAGITTA_RESBIAS_POSETA", 1)) 
        {
            param.SagittaBias = MCP::SystVariation::Down;
            param.SagittaEtaSlice = MCP::SystVariation::Up;
        } 
        else if (syst == SystematicVariation("MUON_SAGITTA_RESBIAS_POSETA", -1)) 
        {
            param.SagittaBias = MCP::SystVariation::Up;
            param.SagittaEtaSlice = MCP::SystVariation::Up;
        } 
        else if (!syst.empty()) return StatusCode::FAILURE;

        // Sagitta Residual Bias systematics
        syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_RESBIAS_NEGETA");

        if (syst == SystematicVariation("MUON_SAGITTA_RESBIAS_NEGETA", 1)) 
        {
            param.SagittaBias = MCP::SystVariation::Down;
            param.SagittaEtaSlice = MCP::SystVariation::Down;
        }
        else if (syst == SystematicVariation("MUON_SAGITTA_RESBIAS_NEGETA", -1)) 
        {
            param.SagittaBias = MCP::SystVariation::Up;
            param.SagittaEtaSlice = MCP::SystVariation::Down;
        } 
        else if (!syst.empty()) return StatusCode::FAILURE;

        // Sagitta Residual Bias systematics
        syst = systConfig.getSystematicByBaseName("MUON_SAGITTA_DATASTAT");

        if      (syst == SystematicVariation("MUON_SAGITTA_DATASTAT",  1)) param.SagittaDataStat = MCP::SystVariation::Up;
        else if (syst == SystematicVariation("MUON_SAGITTA_DATASTAT", -1)) param.SagittaDataStat = MCP::SystVariation::Down;
        else if (!syst.empty()) return StatusCode::FAILURE;

        //
        ATH_MSG_DEBUG("Systematic variation's parameters, SagittaRho: " << param.SagittaRho);
        ATH_MSG_DEBUG("Systematic variation's parameters, SagittaBias: " << param.SagittaBias);
        ATH_MSG_DEBUG("Systematic variation's parameters, SagittaGlobal: " << param.SagittaGlobal);
        ATH_MSG_DEBUG("Systematic variation's parameters, SagittaPtExtra: " << param.SagittaPtExtra);
        ATH_MSG_DEBUG("Systematic variation's parameters, SagittaDataStat: " << param.SagittaDataStat);
        ATH_MSG_DEBUG("Systematic variation's parameters, SagittaEtaSlice: " << param.SagittaEtaSlice);

        return StatusCode::SUCCESS;
    }

    StatusCode MuonCalibIntSagittaTool::applySystematicVariation(const SystematicSet &systConfig)
    {
        return m_Parameters.get(systConfig, m_currentParameters);
    }



} // namespace CP
