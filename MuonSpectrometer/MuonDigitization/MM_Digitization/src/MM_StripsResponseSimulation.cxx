/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// PROJECTS
#include "MM_Digitization/MM_StripsResponseSimulation.h"

#include <TGraph.h>

#include <algorithm>
#include <mutex>
#include <thread>
#include <map>

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGamma.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandGeneral.h"
#include "CLHEP/Random/RandomEngine.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MM_Digitization/MM_IonizationCluster.h"
#include "MM_Digitization/MM_StripResponse.h"
#include "PathResolver/PathResolver.h"

/*

To Do List:

Check Lorentz Angle is in correct direction...

*/

namespace {
    constexpr float electronsToFC = 1. / 6241.;
    constexpr float polyaTheta = 1.765;
    static std::mutex fillMutex{};
}  // namespace

/*******************************************************************************/
MM_StripsResponseSimulation::MM_StripsResponseSimulation(ConfigModule&& mod) :
    AthMessaging("MMStripResponseSimulation"),
    m_cfg{std::move(mod)} {
    
    initHistos();
    // initialization for the random number generator for the number of secondary electrons
    // values are from G. Iakovidis   
    std::vector<double> NelectronProb{65.6, 15., 6.4,  3.5,  2.25,  1.55,  1.05,  0.81,  0.61,  0.49,  0.39, 0.3,
                                      0.25, 0.2, 0.16, 0.12, 0.095, 0.075, 0.063, 0.054, 0.049, 0.045, 0.044};
    NelectronProb.reserve(s_NelectronPropBins);
    for (unsigned int Nelectron = NelectronProb.size(); Nelectron < s_NelectronPropBins; ++Nelectron)
        NelectronProb.push_back(21.6 / ((Nelectron) * (Nelectron)));

    m_randNelectrons = std::make_unique<CLHEP::RandGeneral>(NelectronProb.data(), NelectronProb.size(), 1);  // 1 means non-continious random numbers

    ATH_MSG_DEBUG("MM_StripsResponseSimulation::initializationFrom set values");
    }

/*******************************************************************************/
void MM_StripsResponseSimulation::initHistos() {
    if (m_cfg.writeOutputFile) {
        m_outputFile = std::make_unique<TFile>("MM_StripsResponse_Plots.root", "RECREATE");
        if (m_outputFile) m_outputFile->cd();

        m_mapOfHistograms["nInteractions"] = std::make_unique<TH1F>("nInteractions", "nInteractions", 200, 0, 200);
        m_mapOfHistograms["nElectrons"] = std::make_unique<TH1F>("nElectrons", "nElectrons", 200, 0, 200);
        m_mapOfHistograms["lorentzAngle"] = std::make_unique<TH1F>("lorentzAngle", "lorentzAngle", 100, 0, 3);
        m_mapOfHistograms["effectiveCharge"] = std::make_unique<TH1F>("effectiveCharge", "effectiveCharge", 100, 0, 1e5);
        m_mapOfHistograms["effectiveNElectrons"] = std::make_unique<TH1F>("effectiveNElectrons", "effectiveNElectrons", 200, 0, 200);
        m_mapOfHistograms["totalEffectiveCharge"] = std::make_unique<TH1F>("totalEffectiveCharge", "totalEffectiveCharge", 400, 0, 200);
        m_mapOf2DHistograms["totalEffectiveChargeVsTheta"] = std::make_unique<TH2F>("totalEffectiveChargeVsTheta", "totalEffectiveChargeVsTheta", 17, 0, 35, 300, 0, 300);

        m_mapOf2DHistograms["pathLengthVsTheta"] = std::make_unique<TH2F>("pathLengthVsTheta", "pathLengthVsTheta", 100, 0, 10, 100, -3, 3);
        m_mapOf2DHistograms["pathLengthVsAlpha"] = std::make_unique<TH2F>("pathLengthVsAlpha", "pathLengthVsAlpha", 100, 0, 10, 100, -3, 3);

        m_mapOf2DHistograms["lorentzAngleVsTheta"] = std::make_unique<TH2F>("lorentzAngleVsTheta", "lorentzAngleVsTheta", 100, -3, 3, 100, -3, 3);
        m_mapOf2DHistograms["lorentzAngleVsBy"] = std::make_unique<TH2F>("lorentzAngleVsBy", "lorentzAngleVsBy", 100, -3, 3, 100, -2., 2.);

        m_mapOf2DHistograms["deltaLorentzAngleVsByForPosTheta"] =
            std::make_unique<TH2F>("deltaLorentzAngleVsByForPosTheta", "deltaLorentzAngleVsByForPosTheta", 100, -3, 3, 100, -2., 2.);
        m_mapOf2DHistograms["deltaLorentzAngleVsByForNegTheta"] =
            std::make_unique<TH2F>("deltaLorentzAngleVsByForNegTheta", "deltaLorentzAngleVsByForNegTheta", 100, -3, 3, 100, -2., 2.);

        m_mapOf2DHistograms["driftDistanceVsDriftTime"] =
            std::make_unique<TH2F>("driftDistanceVsDriftTime", "driftDistanceVsDriftTime", 100, 0, 10, 100, 0, 200);
    }
}
/*******************************************************************************/
void MM_StripsResponseSimulation::writeHistos() {
    if (!m_outputFile) {
        return;
    }
    for (auto& [histo_name, histo] : m_mapOfHistograms) {
        m_outputFile->WriteObject(histo.get(), histo_name.c_str());
    }
    for (auto& [histo_name, histo] : m_mapOf2DHistograms) {
        m_outputFile->WriteObject(histo.get(), histo_name.c_str());
    }
    m_mapOf2DHistograms.clear();
    m_mapOfHistograms.clear();
}
/*******************************************************************************/
MM_StripToolOutput MM_StripsResponseSimulation::GetResponseFrom(const MM_DigitToolInput& digiInput, 
                                                                double gainFraction, double stripPitch,
                                                                CLHEP::HepRandomEngine* rndmEngine) const {
    ATH_MSG_DEBUG("Starting to get response from strips");
    DataCache cache{};
    whichStrips(cache, digiInput, gainFraction, stripPitch, rndmEngine);

    ATH_MSG_DEBUG("Creating MmDigitToolOutput object");

    MM_StripToolOutput tmpStripToolOutput(std::move(cache.finalNumberofStrip), 
                                          std::move(cache.finalqStrip), 
                                          std::move(cache.finaltStrip));

    return tmpStripToolOutput;
}

//__________________________________________________________________________________________________
//==================================================================================================
// calculate the strips that got fired, charge and time. Assume any angle thetaDegrees....
//==================================================================================================
void MM_StripsResponseSimulation::whichStrips(DataCache& cache,
                                              const MM_DigitToolInput& digiInput, 
                                              double gainFraction, double stripPitch,
                                              CLHEP::HepRandomEngine* rndmEngine) const {
    ATH_MSG_DEBUG("Starting to calculate strips that got fired");
    const float hitx = digiInput.positionWithinStrip();
    const int stripID = digiInput.stripIDLocal();
    const float incidentAngleXZ = digiInput.incomingAngleXZ();  // degrees
    const float incidentAngleYZ = digiInput.incomingAngleYZ();  // degrees
    const int stripMinID = digiInput.stripMinID();
    const int stripMaxID = digiInput.stripMaxID();

    CLHEP::RandGeneral* randDist ATLAS_THREAD_SAFE = m_randNelectrons.get();
    const float eventTime = digiInput.eventTime();
    const float theta = incidentAngleXZ * Gaudi::Units::degree;  // Important for path length and strip distribution
    const float alpha = incidentAngleYZ * Gaudi::Units::degree;  // Important for path length

    int nPrimaryIons = 0;

    Amg::Vector3D b = digiInput.magneticField() * 1000.;

    // Still need to understand which sign is which... But I think this is correct...

    float lorentzAngle = (b.y() > 0. ? 1. : -1.) * m_cfg.lorentzAngleFunction(std::abs(b.y())) * Gaudi::Units::deg;  // in radians
    if (m_cfg.writeOutputFile) {
        std::lock_guard guard{fillMutex};
        m_mapOf2DHistograms.at("lorentzAngleVsTheta")->Fill(lorentzAngle, theta);
        m_mapOf2DHistograms.at("lorentzAngleVsBy")->Fill(lorentzAngle, b.y());
    }

    ATH_MSG_DEBUG("LorentzAngle vs theta: " << lorentzAngle << " " << theta);

    float pathLengthTraveled = getPathLengthTraveled(rndmEngine);

    float pathLength = m_cfg.driftGapWidth / std::cos(theta);  // Increasing path length based on XZ angle
    pathLength /= std::cos(alpha);                         // Further increasing path length for YZ angle

    if (m_cfg.writeOutputFile) {
        std::lock_guard guard{fillMutex};
        m_mapOf2DHistograms.at("pathLengthVsTheta")->Fill(pathLength, theta);
        m_mapOf2DHistograms.at("pathLengthVsAlpha")->Fill(pathLength, alpha);

        if (theta > 0)
            m_mapOf2DHistograms.at("deltaLorentzAngleVsByForPosTheta")->Fill(lorentzAngle - theta, b.y());
        else
            m_mapOf2DHistograms.at("deltaLorentzAngleVsByForNegTheta")->Fill(lorentzAngle - theta, b.y());
    }
    while (pathLengthTraveled < pathLength) {
        // N.B. Needs correction from alpha angle still...
        std::unique_ptr<MM_IonizationCluster> IonizationCluster =
            std::make_unique<MM_IonizationCluster>(hitx, pathLengthTraveled * std::sin(theta), pathLengthTraveled * std::cos(theta));

        // +1 since first entry in electron probability vector corresponds to 1 electron and not to 0 electrons
        int nElectrons = s_NelectronPropBins * randDist->shoot(rndmEngine) + 1;  
        IonizationCluster->createElectrons(nElectrons);

        const Amg::Vector2D& initialPosition = IonizationCluster->getIonizationStart();

        ATH_MSG_DEBUG("New interaction starting at x,y, pathLengthTraveled: " << initialPosition.x() << " " << initialPosition.y() << " "
                                                                              << pathLengthTraveled);

        for (auto& Electron : IonizationCluster->getElectrons()) {
            Electron->setOffsetPosition(getTransverseDiffusion(initialPosition.y(), rndmEngine),
                                        getLongitudinalDiffusion(initialPosition.y(), rndmEngine));
        }

        IonizationCluster->propagateElectrons(lorentzAngle, m_cfg.driftVelocity);

        int tmpEffectiveNElectrons = 0;

        for (auto& Electron : IonizationCluster->getElectrons()) {
            float effectiveCharge = getEffectiveCharge(rndmEngine) * gainFraction;

            Electron->setCharge(effectiveCharge);

            ATH_MSG_DEBUG("Electron's effective charge is  " << effectiveCharge);

            if (m_cfg.writeOutputFile) {
                std::lock_guard guard{fillMutex};
                m_mapOfHistograms.at("effectiveCharge")->Fill(effectiveCharge);
                m_mapOf2DHistograms.at("driftDistanceVsDriftTime")->Fill(Electron->getOffsetPosition().mag(), Electron->getTime());
            }
            // Add eventTime in Electron time
            Electron->setTime(Electron->getTime() + eventTime);

            tmpEffectiveNElectrons += effectiveCharge;
        }

        if (m_cfg.writeOutputFile) {
            std::lock_guard guard{fillMutex};
            m_mapOfHistograms.at("effectiveNElectrons")->Fill(tmpEffectiveNElectrons / m_cfg.avalancheGain);
        }
        //---
        cache.IonizationClusters.push_back(std::move(IonizationCluster));

        pathLengthTraveled += getPathLengthTraveled(rndmEngine);

        ATH_MSG_DEBUG("Path length traveled: " << pathLengthTraveled);

        ++nPrimaryIons;
        if (nPrimaryIons >= m_cfg.maxPrimaryIons) break;  // don't create more than "MaxPrimaryIons" along a track....

    }  // end of clusters loop

    float timeresolution = 0.01;  // ns

    MM_StripResponse stripResponseObject(cache.IonizationClusters, timeresolution, stripPitch, stripID, stripMinID, stripMaxID);
    stripResponseObject.timeOrderElectrons();
    stripResponseObject.calculateTimeSeries(incidentAngleXZ, digiInput.gasgap());
    stripResponseObject.simulateCrossTalk(m_cfg.crossTalk1, m_cfg.crossTalk2);
    stripResponseObject.calculateSummaries(m_cfg.qThreshold);

    // Connect the output with the rest of the existing code
    //
    cache.finalNumberofStrip = stripResponseObject.getStripVec();
    cache.finalqStrip = stripResponseObject.getTotalChargeVec();
    cache.finaltStrip = stripResponseObject.getTimeThresholdVec();
    cache.tStripElectronicsAbThr = stripResponseObject.getTimeMaxChargeVec();
    cache.qStripElectronics = stripResponseObject.getMaxChargeVec();

    // Output diagnostic hists and graphs
    //
    if (m_cfg.writeOutputFile) {
        std::lock_guard guard{fillMutex};
        m_mapOfHistograms.at("nInteractions")->Fill(nPrimaryIons);
        m_mapOfHistograms.at("nElectrons")->Fill(stripResponseObject.getNElectrons());
        m_mapOfHistograms.at("totalEffectiveCharge")->Fill(stripResponseObject.totalCharge() * electronsToFC);
        m_mapOf2DHistograms.at("totalEffectiveChargeVsTheta")->Fill(std::abs(incidentAngleXZ),
                                                                 stripResponseObject.totalCharge() * electronsToFC);
        ATH_MSG_DEBUG("incidentAngleXZ" << incidentAngleXZ << "charge [fC]" << stripResponseObject.totalCharge() / 6241.);
        m_mapOfHistograms.at("lorentzAngle")->Fill(lorentzAngle);
    }

    if (m_cfg.writeEventDisplays && m_outputFile) {
        std::lock_guard guard{fillMutex};
        static std::atomic<unsigned> written{0};
        TGraph grIonizationXZ(cache.IonizationClusters.size());
        for (size_t  iIonization = 0; iIonization < cache.IonizationClusters.size(); ++iIonization) {
            Amg::Vector2D ionizationPosition(cache.IonizationClusters.at(iIonization)->getIonizationStart());
            grIonizationXZ.SetPoint(iIonization, ionizationPosition.x(), ionizationPosition.y());
        }
        m_outputFile->WriteObject(&grIonizationXZ, Form("ionizationXZ_%u", ++written));

        TGraph grElectronsXZ(stripResponseObject.getNElectrons());
        int iElectron = 0;
        for (const auto& electron : stripResponseObject.getElectrons()) {
            grElectronsXZ.SetPoint(iElectron, electron->getX(), electron->getY());
            iElectron++;
        }
        m_outputFile->WriteObject(&grElectronsXZ, Form("electronsXZ_%u", ++written));
    }

}  // end of whichStrips()

float MM_StripsResponseSimulation::generateTransverseDiffusion(float posY, CLHEP::HepRandomEngine* rndmEngine) const {
    // this is a helper function used in getTransverseDiffusion, generating double gaussian distributions

    // avoid division by zero in calculation of scale if ypos is 0
    // also protect against negative values of ypos which should never happen
    if (posY <= 0.) posY = 0.001;

    // need to scale weigths since initial distributions were not normalized
    const double scale = 0.001 / (posY * m_cfg.transverseDiffusionSigma);

    const double uni = CLHEP::RandFlat::shoot(rndmEngine, 0, 1.0 + scale);
    if (uni < scale) return CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0.0, 1.0);
    return CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0., m_cfg.transverseDiffusionSigma * posY);
}

float MM_StripsResponseSimulation::getTransverseDiffusion(float posY, CLHEP::HepRandomEngine* rndmEngine) const {
    // the random numbers are generate from the following function:
    // "1.*TMath::Exp(-TMath::Power(x,2.)/(2.*[0]*[0])) + 0.001*TMath::Exp(-TMath::Power(x,2)/(2.*[1]*[1]))"
    // in the range from -1 to 1
    // Since sampling from a TF1 where its parameters are changed for every random number is slow,
    // the following approach is used to generate random numbers from the mixture of two Gaussian:
    // https://stats.stackexchange.com/questions/226834/sampling-from-a-mixture-of-two-gamma-distributions/226837#226837
    // this approach seems to be around 20000 times faster

    // if one of the diffusions is off, the tail is not present
    float diffusion{0.};
    if (m_cfg.longitudinalDiffusionSigma == 0 || m_cfg.transverseDiffusionSigma == 0) {
         // limit random number to be -1 < x < 1
        do {
            diffusion = CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0.0, posY * m_cfg.transverseDiffusionSigma);
            ATH_MSG_VERBOSE("getTransverseDiffusion() -- posY x transverseSigma: "<<(posY* m_cfg.transverseDiffusionSigma)<<" engine seed: "<<rndmEngine->getSeed()<<" "<<rndmEngine->flat()<<" diffusion "<<diffusion); 
        } while (std::abs(diffusion) > 1.);
        return diffusion;        
    }
    do {
        diffusion =  generateTransverseDiffusion(posY, rndmEngine);
        ATH_MSG_VERBOSE("getTransverseDiffusion() -- posY: "<<posY<<" engine seed: "<<rndmEngine->getSeed()<<" "<<rndmEngine->flat()<<" diffusion "<<diffusion); 
    } while (std::abs(diffusion)> 1.);
    return diffusion;    
}

float MM_StripsResponseSimulation::getLongitudinalDiffusion(float posY, CLHEP::HepRandomEngine* rndmEngine) const {
    float diffusion{0.};
    // We only want random numbers between -5 and 5
    do {
        diffusion= CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0.0, posY * m_cfg.longitudinalDiffusionSigma);
        ATH_MSG_VERBOSE("getLongitudinalDiffusion() -- posY: "<<posY<<" engine seed: "<<rndmEngine->getSeed()<<" "<<rndmEngine->flat()<<" diffusion "<<diffusion);
    } while (std::abs(diffusion) > 5);
    return diffusion;
}

float MM_StripsResponseSimulation::getEffectiveCharge(CLHEP::HepRandomEngine* rndmEngine) const {
    // charge fluctuatation is described by Polya function   
    // To calculate the gain from polya distibution we replace "alpha = 1+theta and beta = 1+theta/mean" in gamma PDF. With this gamma PDF
    // gives us same sampling values as we get from polya PDF. Idea taken from MR 40547
    return CLHEP::RandGamma::shoot(rndmEngine, 1. + polyaTheta, (1. + polyaTheta) / m_cfg.avalancheGain);
}

float MM_StripsResponseSimulation::getPathLengthTraveled(CLHEP::HepRandomEngine* rndmEngine) const {
    // Probability of having an interaction (per unit length traversed) is sampled from a gaussian provided by G. Iakovidis
    float rndGaus{0.};

    // gaussian random number should be in the range from 0 to 10
    do {
        rndGaus = CLHEP::RandGaussZiggurat::shoot(rndmEngine, m_cfg.interactionDensityMean, m_cfg.interactionDensitySigma);
        ATH_MSG_VERBOSE("getPathLengthTraveled() --  engine seed: "<<rndmEngine->getSeed()<<" "<<rndmEngine->flat()<<" rndGaus: "<<rndGaus);
    } while (rndGaus < 0. || rndGaus > 10.) ;

    return (1. / rndGaus) * -1. * std::log(CLHEP::RandFlat::shoot(rndmEngine));
}

/*******************************************************************************/
MM_StripsResponseSimulation::~MM_StripsResponseSimulation() {
    if (m_outputFile) { writeHistos(); }
}
/*******************************************************************************/
