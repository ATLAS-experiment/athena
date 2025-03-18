/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MM_DigitizationTool.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "CLHEP/Random/RandFlat.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace
{
    constexpr double percentage(unsigned int numerator, unsigned int denom)
    {
        return 100. * numerator / std::max(denom, 1u);
    }
    // thresholds for the shortest and longest strips
    // values from https://indico.cern.ch/event/1131762/contributions/4749097/attachments/2431773/4164431/MMGcoord2022.04.26.pdf
    constexpr double maxNoiseSmall_eta1 = 2100;
    constexpr double minNoiseSmall_eta1 = 1000;
    constexpr double maxNoiseSmall_eta2 = 2600;
    constexpr double minNoiseSmall_eta2 = 2100;

    constexpr double maxNoiseLarge_eta1 = 2500;
    constexpr double minNoiseLarge_eta1 = 1200;
    constexpr double maxNoiseLarge_eta2 = 3000;
    constexpr double minNoiseLarge_eta2 = 2500;
}

namespace MuonR4 {

    MM_DigitizationTool::MM_DigitizationTool(const std::string &type, const std::string &name, const IInterface *pIID) : 
            MuonDigitizationTool{type, name, pIID} {}

    StatusCode MM_DigitizationTool::initialize() {
        ATH_MSG_DEBUG("MM_DigitizationTool:: in initialize()");

        ATH_CHECK(MuonDigitizationTool::initialize());

        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_uncertCalibKey.initialize());
        ATH_CHECK(m_condThrshldsKey.initialize(m_useCondThresholds));
        ATH_CHECK(m_fieldCondObjInputKey.initialize());
        ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
        ATH_CHECK(m_calibrationTool.retrieve());
        ATH_CHECK(m_smearingTool.retrieve());

        // get gas properties from calibration tool
        const NSWCalib::MicroMegaGas prop = m_calibrationTool->mmGasProperties();
        const double peakTime = m_calibrationTool->mmPeakTime();

        MM_StripsResponseSimulation::ConfigModule strip_cfg{};
        strip_cfg.NSWCalib::MicroMegaGas::operator=(prop);
        strip_cfg.writeOutputFile = m_writeOutputFile;
        strip_cfg.qThreshold = m_qThreshold;
        strip_cfg.driftGapWidth = m_driftGapWidth;
        strip_cfg.crossTalk1 = m_crossTalk1;
        strip_cfg.crossTalk2 = m_crossTalk2;
        strip_cfg.avalancheGain = m_avalancheGain;
        m_StripsResponseSimulation = std::make_unique<MM_StripsResponseSimulation>(std::move(strip_cfg));

        m_timeWindowLowerOffset += peakTime; // account for peak time in time window
        m_timeWindowUpperOffset += peakTime; // account for peak time in time window

        MM_ElectronicsResponseSimulation::ConfigModule elec_sim_cfg{};
        elec_sim_cfg.peakTime = peakTime;
        elec_sim_cfg.timeWindowLowerOffset = m_timeWindowLowerOffset;
        elec_sim_cfg.timeWindowUpperOffset = m_timeWindowUpperOffset;
        elec_sim_cfg.vmmDeadtime = m_vmmDeadtime;
        elec_sim_cfg.vmmUpperGrazeWindow = m_vmmUpperGrazeWindow;
        elec_sim_cfg.stripDeadTime = m_stripdeadtime;
        elec_sim_cfg.artDeadTime = m_ARTdeadtime;
        elec_sim_cfg.useNeighborLogic = m_vmmNeighborLogic;
        // ElectronicsResponseSimulation Creation
        m_ElectronicsResponseSimulation = std::make_unique<MM_ElectronicsResponseSimulation>(std::move(elec_sim_cfg));

        // Configuring various VMM modes of signal readout
        //
        std::string vmmReadoutMode = m_vmmReadoutMode;
        // convert vmmReadoutMode to lower case
        std::for_each(vmmReadoutMode.begin(), vmmReadoutMode.end(), [](char &c){ c = ::tolower(c); });
        if (vmmReadoutMode.find("peak") != std::string::npos) {
            m_vmmReadoutMode = "peak";
        } else if (vmmReadoutMode.find("threshold") != std::string::npos){
            m_vmmReadoutMode = "threshold";
        } else {
            ATH_MSG_ERROR("MM_DigitizationTool can't interperet vmmReadoutMode option! (Should be 'peak' or 'threshold'.) Contains: "
                          << m_vmmReadoutMode);
            return StatusCode::FAILURE;
        }
        std::string vmmARTMode = m_vmmARTMode;
        // convert vmmARTMode to lower case
        std::for_each(vmmARTMode.begin(), vmmARTMode.end(), [](char &c){ c = ::tolower(c); });
        if (vmmARTMode.find("peak") != std::string::npos) {
            m_vmmARTMode = "peak";
        }
        else if (vmmARTMode.find("threshold") != std::string::npos) {
            m_vmmARTMode = "threshold";
        }
        else {
            ATH_MSG_ERROR("MM_DigitizationTool can't interperet vmmARTMode option! (Should be 'peak' or 'threshold'.) Contains: " << m_vmmARTMode);
            return StatusCode::FAILURE;
        }

        if (m_doSmearing) {
            ATH_MSG_INFO("Running in smeared mode!");
        }

        // get shortest and longest strip length for threshold scaling
        Identifier tmpId{0}; // temporary identifier to work with ReadoutElement
        int stripNumberShortestStrip{-1}, stripNumberLongestStrip{-1};
        Identifier tmpIdShortestStrip{0}, tmpIdLongestStrip{0};
        double shortestStripLength{FLT_MAX}, longestStripLength{0};
        IdentifierHash tmpIdHashShortestStrip{0}, tmpIdHashLongestStrip{0};
        NoiseCalibConstants noise_smallEta1{}, noise_smallEta2{}, noise_largeEta1{}, noise_largeEta2{};
        //
        for (std::string sectorType : {"MML", "MMS"}) {
            for (int etaStation : {1, 2}) {
                // identifier for first gas gap in a MM sector, layer is eta layer
                tmpId = m_idHelperSvc->mmIdHelper().channelID(sectorType, etaStation, 1, 1, 1, 1);
                const MuonGMR4::MmReadoutElement *detectorReadoutElement = m_detMgr->getMmReadoutElement(tmpId);
                const MuonGMR4::StripDesign &design{detectorReadoutElement->stripLayer(tmpId).design()};
                stripNumberShortestStrip = design.firstStripNumber();
                tmpIdShortestStrip = m_idHelperSvc->mmIdHelper().channelID(sectorType, etaStation, 1, 1, 1, stripNumberShortestStrip); // identifier for the shortest strip
                tmpIdHashShortestStrip = detectorReadoutElement->measurementHash(tmpIdShortestStrip);
                shortestStripLength = detectorReadoutElement->stripLength(tmpIdHashShortestStrip);
                stripNumberLongestStrip = design.numStrips() + stripNumberShortestStrip - 1;
                tmpIdLongestStrip = m_idHelperSvc->mmIdHelper().channelID(sectorType, etaStation, 1, 1, 1, stripNumberLongestStrip); // identifier for the longest strip
                tmpIdHashLongestStrip = detectorReadoutElement->measurementHash(tmpIdLongestStrip);
                longestStripLength = design.stripLength(tmpIdHashLongestStrip);

                // now get the slope and intercept for the threshold scaling
                // function is m_noiseSlope * stripLength + m_noiseIntercept
                if (sectorType == "MMS") {
                    if (etaStation == 1) {
                        noise_smallEta1 = m_noiseParams[m_idHelperSvc->stationName(tmpId)];
                        noise_smallEta1.slope = (maxNoiseSmall_eta1 - minNoiseSmall_eta1) / (longestStripLength - shortestStripLength);
                        noise_smallEta1.intercept = minNoiseSmall_eta1 - noise_smallEta1.slope * shortestStripLength;
                    }
                    else if (etaStation == 2) {
                        noise_smallEta2 = m_noiseParams[m_idHelperSvc->stationName(tmpId) * 2];
                        noise_smallEta2.slope = (maxNoiseSmall_eta2 - minNoiseSmall_eta2) / (longestStripLength - shortestStripLength);
                        noise_smallEta2.intercept = minNoiseSmall_eta2 - noise_smallEta2.slope * shortestStripLength;
                    }
                } else if (sectorType == "MML") {
                    if (etaStation == 1) {
                        noise_largeEta1 = m_noiseParams[m_idHelperSvc->stationName(tmpId)];
                        noise_largeEta1.slope = (maxNoiseLarge_eta1 - minNoiseLarge_eta1) / (longestStripLength - shortestStripLength);
                        noise_largeEta1.intercept = minNoiseLarge_eta1 - noise_largeEta1.slope * shortestStripLength;
                    } else if (etaStation == 2) {
                        noise_largeEta2 = m_noiseParams[m_idHelperSvc->stationName(tmpId) * 2];
                        noise_largeEta2.slope = (maxNoiseLarge_eta2 - minNoiseLarge_eta2) / (longestStripLength - shortestStripLength);
                        noise_largeEta2.intercept = minNoiseLarge_eta2 - noise_largeEta2.slope * shortestStripLength;
                    }
                } // end of if over sector types
            } // end of loop over eta stations
        } // end of loop over sector types

        ATH_MSG_DEBUG("Configuration  MM_DigitizationTool ");
        ATH_MSG_DEBUG("OutputObjectName       " << m_writeKey.key());
        ATH_MSG_DEBUG("UseTimeWindow          " << m_useTimeWindow);
        ATH_MSG_DEBUG("CheckSimHits           " << m_checkMMSimHits);
        ATH_MSG_DEBUG("Threshold              " << m_qThreshold);
        ATH_MSG_DEBUG("TransverseDiffusSigma  " << m_StripsResponseSimulation->getTransversDiffusionSigma());
        ATH_MSG_DEBUG("LogitundinalDiffusSigma" << m_StripsResponseSimulation->getLongitudinalDiffusionSigma());
        ATH_MSG_DEBUG("Interaction density mean: " << m_StripsResponseSimulation->getInteractionDensityMean());
        ATH_MSG_DEBUG("Interaction density sigma: " << m_StripsResponseSimulation->getInteractionDensitySigma());
        ATH_MSG_DEBUG("DriftVelocity stripResponse: " << m_StripsResponseSimulation->getDriftVelocity());
        ATH_MSG_DEBUG("crossTalk1             " << m_crossTalk1);
        ATH_MSG_DEBUG("crossTalk2             " << m_crossTalk2);
        ATH_MSG_DEBUG("EnergyThreshold        " << m_energyThreshold);

        return StatusCode::SUCCESS;
    }

    StatusCode MM_DigitizationTool::finalize() {

        std::stringstream statstr{};
        unsigned allHits{0};
        for (unsigned int g = 0; g < m_allHits.size(); ++g) {
            allHits += m_allHits[g];
            statstr << " *** Layer " << (g + 1) << " " << percentage(m_acceptedHits[g], m_allHits[g])
                    << "% of " << m_allHits[g] << std::endl;
        }
        if (!allHits) {
            return StatusCode::SUCCESS;
        }
        ATH_MSG_INFO("Tried to convert " << allHits << " hits. Successes rate per layer  " << std::endl
                                         << statstr.str());
        return StatusCode::SUCCESS;
    }

    StatusCode MM_DigitizationTool::digitize(const EventContext &ctx,
                                             const TimedHits &hitsToDigit,
                                             xAOD::MuonSimHitContainer *sdoContainer) const {

        const MmIdHelper &idHelper{m_idHelperSvc->mmIdHelper()};
        CLHEP::HepRandomEngine *rndEngine = getRandomEngine(ctx);
        const ActsGeometryContext &gctx{getGeoCtx(ctx)};

        // Prepare the temporary cache
        DigiCache digitCache{};

        // Fetch the conditions for efficiency calculations
        const Muon::DigitEffiData *efficiencyMap{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_effiDataKey, efficiencyMap));

        const NswErrorCalibData *errorCalibDB{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_uncertCalibKey, errorCalibDB));

        MagField::AtlasFieldCache fieldCache;
        const AtlasFieldCacheCondObj *fieldCondObj{nullptr};
        ATH_CHECK(retrieveConditions(ctx, m_fieldCondObjInputKey, fieldCondObj));
        fieldCondObj->getInitializedCache(fieldCache);

        xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
        do {

            std::array<std::vector<MM_ElectronicsToolInput>, 8> v_stripDigitOutput{};
            DeadTimeMap deadTimes{};

            for (const TimedHit &simHit : viewer) {
                // ignore radiation if you want
              if (m_digitizeMuonOnly && !MC::isMuon(simHit)) {
                    ATH_MSG_VERBOSE("Hit is not from a muon - skipping ");
                    continue;
                }

                const Identifier hitId{simHit->identify()};

                const double hitKineticEnergy = simHit->kineticEnergy();
                // Don't consider electron hits below m_energyThreshold.
                // Electrons aren't consider for now in any case due to the cut above.
                // But this may change.
                if (hitKineticEnergy < m_energyThreshold && MC::isElectron(simHit)) {
                    continue;
                }
                const MuonGMR4::MmReadoutElement *readOutEle = m_detMgr->getMmReadoutElement(hitId);
                const MuonGMR4::StripDesign &design{readOutEle->stripLayer(hitId).design()};

                const Amg::Vector3D locPos{xAOD::toEigen(simHit->localPosition())};
                const Amg::Vector3D locDir{xAOD::toEigen(simHit->localDirection())};

                const Amg::Transform3D &locToGlobal{readOutEle->localToGlobalTrans(gctx, readOutEle->layerHash(hitId))};
                const Amg::Vector3D globalHitPosition{locToGlobal * locPos};

                // We could use the MuonDigitizationTool::hitTime which returns the global time
                // of the hit which is the sum of eventTime & individual hit time,
                // but we want to have access on both contributions.
                const double globalHitTime = simHit->globalTime();
                const double tofCorrection = globalHitPosition.mag() / CLHEP::c_light;
                const double eventTime = simHit.eventTime();
                const double bunchTime = globalHitTime - tofCorrection + eventTime;

                const HepMcParticleLink particleLink = simHit->genParticleLink();
                // Print some information about the MicroMegas hit
                ATH_MSG_VERBOSE("hitID  " << m_idHelperSvc->toString(hitId) <<" ("<< simHit.get()<< ") Hit bunch time  " << bunchTime << " tof/G4 hit time " << globalHitTime
                            << " globalHitPosition " << Amg::toString(globalHitPosition) << " hit: r " << globalHitPosition.perp() << " z " << globalHitPosition.z()
                            << " mclink " << particleLink << "Kinetic energy " << hitKineticEnergy);

                // Angles, Geometry, and Coordinates.
                // This is not an incident angle yet. It's atan(z/x),
                // ... so it's the complement of the angle w.r.t. a vector normal to the detector surface
                double inAngleCompliment_XZ = std::atan2(locDir.z(), locDir.x()) / CLHEP::degree;
                double inAngleCompliment_YZ = std::atan2(locDir.z(), locDir.y()) / CLHEP::degree;

                // This is basically to handle the atan ambiguity
                if (inAngleCompliment_XZ < 0.0)
                    inAngleCompliment_XZ += 180;
                if (inAngleCompliment_YZ < 0.0)
                    inAngleCompliment_YZ += 180;

                // This gets the actual incidence angle from its complement.
                double inAngle_XZ = 90. - inAngleCompliment_XZ;
                double inAngle_YZ = 90. - inAngleCompliment_YZ;

                const IdentifierHash measHash{readOutEle->measurementHash(hitId)};
                int readoutSide = readOutEle->readoutSide(measHash);
                if (readoutSide == 1) {
                    inAngle_XZ = (-inAngle_XZ);
                }

                // Move the hit position to the readout plane
                int gasGap = idHelper.gasGap(hitId);
                // TO DO: THERE IS NO DESING THICKNESS, BUT HOW TO GET DIFFERENT THICKNESSES LIKE BEFORE?
                // double shift = 0.5 * design.thickness();
                double shift = 0.5 * m_driftGapWidth;
                std::optional<double> lambda{std::nullopt};
                if (gasGap == 1 || gasGap == 3) {
                    lambda = Amg::intersect<3>(locPos, locDir, Amg::Vector3D::UnitZ(), -shift);
                } else if (gasGap == 2 || gasGap == 4) {
                    lambda = Amg::intersect<3>(locPos, locDir, Amg::Vector3D::UnitZ(), shift);
                }

                const Amg::Vector3D hitOnSurface = locPos + lambda.value_or(0.) * locDir;
                const Amg::Vector2D positionOnSurface{hitOnSurface.x(), hitOnSurface.y()};

                // Account For Time Offset
                const double shiftTimeOffset = (globalHitTime - tofCorrection) * m_StripsResponseSimulation->getDriftVelocity();
                const Amg::Vector3D hitAfterTimeShift(hitOnSurface.x(), hitOnSurface.y(), shiftTimeOffset);
                lambda = Amg::intersect<3>(hitAfterTimeShift, locDir, Amg::Vector3D::UnitZ(), -shiftTimeOffset);
                const Amg::Vector3D hitAfterTimeShiftOnSurface = hitAfterTimeShift + lambda.value_or(0.) * locDir;

                if (std::abs(hitAfterTimeShiftOnSurface.z()) > 0.1) {
                    ATH_MSG_WARNING("Bad propagation to surface after time shift " << Amg::toString(hitAfterTimeShiftOnSurface));
                }

                // Calculate the index for the global hit counter
                const unsigned int hitGapInNsw = (idHelper.multilayer(hitId) - 1) * 4 + idHelper.gasGap(hitId) - 1;
                ++m_allHits[hitGapInNsw];

                bool isValid{false};

                int channelNumber = design.stripNumber(positionOnSurface);
                // Checks if the hit is outside of the panel
                if (channelNumber < 0) {
                    // Checks if the point is inside the trapezoid where the strips are mounted.
                    if (!design.insideTrapezoid(positionOnSurface)) {
                        ATH_MSG_WARNING("Hit " << m_idHelperSvc->toString(hitId) << " " << Amg::toString(positionOnSurface)
                                               << " is outside bounds " << std::endl
                                               << design << " rejecting it");
                    }
                    continue;
                }
                const Identifier clusId = idHelper.channelID(hitId, idHelper.multilayer(hitId),
                                                             idHelper.gasGap(hitId), channelNumber, isValid);

                if (!isValid) {
                    ATH_MSG_WARNING("Invalid strip identifier for layer " << m_idHelperSvc->toStringGasGap(hitId)<< " channel " << channelNumber 
                                    << " positionOnSurface " << Amg::toString(positionOnSurface));
                    continue;
                }

                if (efficiencyMap && efficiencyMap->getEfficiency(clusId) < CLHEP::RandFlat::shoot(rndEngine, 0., 1.)) {
                    ATH_MSG_VERBOSE("Simulated hit " <<Amg::toString(locPos) << " hitting strip " << m_idHelperSvc->toString(clusId) 
                                    << " is rejected because of effiency modelling");
                    continue;
                }
                if (!passDeadTime(clusId, hitTime(simHit), m_deadTime, deadTimes)) {
                    /// Reject hit within the dead time interval
                    continue;
                }

                double distToStrip = design.distanceToStrip(positionOnSurface, channelNumber);

                // Obtain Magnetic Field At Detector Surface
                Amg::Vector3D hitOnSurfaceGlobal{locToGlobal * hitOnSurface};
                Amg::Vector3D magneticField{Amg::Vector3D::Zero()};
                fieldCache.getField(hitOnSurfaceGlobal.data(), magneticField.data());

                // B-field in local cordinate, X ~ #strip, increasing to outer R, Z ~ global Z but positive to IP
                Amg::Vector3D localMagneticField{locToGlobal.linear().inverse() * magneticField};
                if (readoutSide == -1) {
                    localMagneticField[Amg::y] = -localMagneticField[Amg::y];
                }

                // Strip Response Simulation For This Hit
                const MM_DigitToolInput stripDigitInput(channelNumber, distToStrip, inAngle_XZ, inAngle_YZ, localMagneticField,
                                                        design.firstStripNumber(),
                                                        design.numStrips() + design.firstStripNumber() - 1,
                                                        idHelper.gasGap(hitId), eventTime + globalHitTime);
                constexpr double gainFraction = 1.0;

                MM_StripToolOutput tmpStripOutput = m_StripsResponseSimulation->GetResponseFrom(stripDigitInput, gainFraction, design.stripPitch(), rndEngine);

                MM_ElectronicsToolInput stripDigitOutput(tmpStripOutput.NumberOfStripsPos(), tmpStripOutput.chipCharge(),
                                                         tmpStripOutput.chipTime(), clusId, hitKineticEnergy);

                ATH_MSG_VERBOSE(__func__ << "() : " << __LINE__ << " Prepared strip for digitization: " << m_idHelperSvc->toString(clusId));
                v_stripDigitOutput[hitGapInNsw].push_back(std::move(stripDigitOutput));

                addSDO(simHit, sdoContainer)->setIdentifier(clusId);
                ++m_acceptedHits[hitGapInNsw];
            } // end of loop over hits

            // Now at Detector Element Level (VMM)
            for (std::vector<MM_ElectronicsToolInput> &elecHitsPerLayer : v_stripDigitOutput) {
                if (elecHitsPerLayer.empty()) {
                    continue;
                }
                // VMM Simulation
                // Combine all strips (for this VMM) into a single VMM-level object
                MM_ElectronicsToolInput stripDigitOutputAllHits = combinedStripResponseAllHits(ctx, std::move(elecHitsPerLayer));

                // Create Electronics Output with peak or threshold finding setting
                MM_DigitToolOutput electronicsOutputForReadout{m_vmmReadoutMode == "peak" ? 
                        m_ElectronicsResponseSimulation->getPeakResponseFrom(std::move(stripDigitOutputAllHits)) : 
                        m_ElectronicsResponseSimulation->getThresholdResponseFrom(std::move(stripDigitOutputAllHits))};
                if (!electronicsOutputForReadout.isValid()) {
                    ATH_MSG_DEBUG(__func__ << "() : " << __LINE__
                            << " there is no electronics response (peak or threshold finding mode) even though there is a strip response.");
                }

                for (unsigned int firedCh = 0; firedCh < electronicsOutputForReadout.stripPos().size(); ++firedCh) {

                    const int channel = electronicsOutputForReadout.stripPos()[firedCh];
                    double time = electronicsOutputForReadout.stripTime()[firedCh];
                    double charge = electronicsOutputForReadout.stripCharge()[firedCh];
                    bool isValid{false};
                    const Identifier digitID = idHelper.channelID(stripDigitOutputAllHits.digitID(),
                                                                  idHelper.multilayer(stripDigitOutputAllHits.digitID()),
                                                                  idHelper.gasGap(stripDigitOutputAllHits.digitID()),
                                                                  channel, isValid);

                    if (!isValid) {
                        ATH_MSG_DEBUG("Ghost strip fired... Ghost busters... ");
                        continue;
                    }
                    ATH_MSG_VERBOSE(__func__ << "() " << __LINE__ <<": Final digitized strip: " << m_idHelperSvc->toString(digitID)
                                    <<", time: "<<time<<", charge: "<<charge);
                    MmDigitCollection *outColl = fetchCollection(digitID, digitCache);
                    outColl->push_back(std::make_unique<MmDigit>(digitID, time, charge));
                }
            }
        } while (viewer.next());
        // Write everything at the end into the final digit container
        ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache), idHelper.module_hash_max()));

        return StatusCode::SUCCESS;
    }

    MM_ElectronicsToolInput
    MM_DigitizationTool::combinedStripResponseAllHits(const EventContext &ctx,
                                                      const std::vector<MM_ElectronicsToolInput> &v_stripDigitOutput) const {
        // set up pointer to conditions object

        const MmIdHelper &idHelper{m_idHelperSvc->mmIdHelper()};

        const NswCalibDbThresholdData *thresholdData{nullptr};
        if (m_useCondThresholds && !retrieveConditions(ctx, m_condThrshldsKey, thresholdData).isSuccess()) {
            THROW_EXCEPTION("Cannot find conditions data container for VMM thresholds!");
        }

        std::vector<int> v_stripStripResponseAllHits{};
        std::vector<std::vector<float>> v_timeStripResponseAllHits{}, v_qStripResponseAllHits{};
        std::vector<float> v_stripThresholdResponseAllHits{};

        Identifier digitID = v_stripDigitOutput.at(0).digitID();
        double max_kineticEnergy = 0.0;

        // Loop over strip digit output elements
        for (const MM_ElectronicsToolInput &i_stripDigitOutput : v_stripDigitOutput) {
            //--- Just to get Digit id with the largest kinetic energy, but the Digit id is no longer meaningful
            if (i_stripDigitOutput.kineticEnergy() > max_kineticEnergy) {
                digitID = i_stripDigitOutput.digitID();
                max_kineticEnergy = i_stripDigitOutput.kineticEnergy();
            }
            //---
            for (size_t i = 0; i < i_stripDigitOutput.NumberOfStripsPos().size(); ++i) {
                int strip_id = i_stripDigitOutput.NumberOfStripsPos().at(i);
                bool found = false;

                for (size_t ii = 0; ii < v_stripStripResponseAllHits.size(); ++ii) {
                    if (v_stripStripResponseAllHits.at(ii) == strip_id) {
                        v_timeStripResponseAllHits.at(ii).insert(v_timeStripResponseAllHits.at(ii).end(),
                                                                 i_stripDigitOutput.chipTime().at(i).begin(), i_stripDigitOutput.chipTime().at(i).end());

                        v_qStripResponseAllHits.at(ii).insert(v_qStripResponseAllHits.at(ii).end(),
                                                              i_stripDigitOutput.chipCharge().at(i).begin(), i_stripDigitOutput.chipCharge().at(i).end());
                        found = true;
                    }
                }

                if (found) {
                    continue;
                }
                
                v_stripStripResponseAllHits.push_back(strip_id);
                v_timeStripResponseAllHits.push_back(i_stripDigitOutput.chipTime().at(i));
                v_qStripResponseAllHits.push_back(i_stripDigitOutput.chipCharge().at(i));
                if (m_useCondThresholds) {
                    const Identifier id = idHelper.channelID(digitID,
                                                                idHelper.multilayer(digitID),
                                                                idHelper.gasGap(digitID), strip_id);
                    float threshold = 0;
                    if (!thresholdData->getThreshold(id, threshold)) {
                        THROW_EXCEPTION("Cannot find retrieve VMM threshold from conditions data base!");
                    }
                    v_stripThresholdResponseAllHits.push_back(threshold);
                } else if (m_useThresholdScaling) {
                    Identifier id = idHelper.channelID(digitID,
                                                        idHelper.multilayer(digitID),
                                                        idHelper.gasGap(digitID), strip_id);

                    const MuonGMR4::MmReadoutElement *detectorReadoutElement = m_detMgr->getMmReadoutElement(id);
                    const MuonGMR4::StripDesign &design{detectorReadoutElement->stripLayer(id).design()};
                    double stripLength = design.stripLength(strip_id);
                    const int noise_id = m_idHelperSvc->stationName(digitID) * std::abs(m_idHelperSvc->stationEta(digitID));
                    const NoiseCalibConstants &noise = m_noiseParams.at(noise_id);
                    double threshold = (noise.slope * stripLength + noise.intercept) * m_thresholdScaleFactor;
                    v_stripThresholdResponseAllHits.push_back(threshold);
                } else {
                    v_stripThresholdResponseAllHits.push_back(m_electronicsThreshold);
                }
            }
        }
        return MM_ElectronicsToolInput(std::move(v_stripStripResponseAllHits), std::move(v_qStripResponseAllHits),
                                       std::move(v_timeStripResponseAllHits), std::move(v_stripThresholdResponseAllHits),
                                       digitID, max_kineticEnergy);
    }

} // namespace MuonR4
