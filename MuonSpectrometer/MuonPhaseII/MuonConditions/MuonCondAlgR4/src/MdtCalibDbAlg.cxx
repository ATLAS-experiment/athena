/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaKernel/IOVInfiniteRange.h"

#include "MdtCalibDbAlg.h"
#include "MdtCalibData/RtChebyshev.h"
#include "MdtCalibData/TrChebyshev.h"
#include <MuonCalibMath/BaseFunctionFitter.h>
#include <MuonCalibMath/ChebyshevPolynomial.h>
#include "MuonCalibIdentifier/MuonFixedId.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"

#include <unordered_map>

using RegionGranularity = MuonCalib::MdtCalibDataContainer::RegionGranularity;
using namespace MuonCalibR4;

MdtCalibDbAlg::MdtCalibDbAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MdtCalibDbAlg::initialize() {
    ATH_MSG_VERBOSE("Initialize...");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_idToFixedIdTool.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKeyRt.initialize());
    ATH_CHECK(m_readKeyTube.initialize());
    ATH_CHECK(detStore()->retrieve(m_r4detMgr));
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("Executing MdtCalibDbAlgR4");
    SG::WriteCondHandle<MuonCalib::MdtCalibDataContainer> writeHandle{m_writeKey, ctx};
    if(writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
    RegionGranularity gran{RegionGranularity::OnePerMultiLayer};
    auto writeCdo = std::make_unique<MuonCalib::MdtCalibDataContainer>(m_idHelperSvc.get(), gran);

    ATH_CHECK(loadRt(ctx,*writeCdo));
    ATH_CHECK(loadTube(*writeCdo));

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    

    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::loadRt(const EventContext& ctx, MuonCalib::MdtCalibDataContainer& writeHandle) const {
    ATH_MSG_DEBUG("loadRt " << name());


    SG::ReadCondHandle readHandleRt{m_readKeyRt, ctx};
    if(!readHandleRt.isValid()) {
        ATH_MSG_ERROR("No RT data found in " << m_readKeyRt.key());
        return StatusCode::FAILURE;
    }

    std::unordered_map<Identifier, RtPayload> rtCalibration{};
    for(CondAttrListCollection::const_iterator it = readHandleRt->begin(); 
                                            it != readHandleRt->end(); ++it) {
        if(!readRtPayload(it->second, rtCalibration)) {
            ATH_MSG_ERROR("Failed to read RT payload");
            return StatusCode::FAILURE;
        }
    }

    std::vector<const MuonGMR4::MdtReadoutElement*> detEls = m_r4detMgr->getAllMdtReadoutElements();
    for(const MuonGMR4::MuonReadoutElement* mdtDetEl : detEls){
        const Identifier& detElId = mdtDetEl->identify();
        ATH_MSG_DEBUG("Adding Rt relation for detector element " << m_idHelperSvc->toString(detElId));
        if(writeHandle.hasDataForChannel(detElId, msgStream())) {
            const MuonCalib::MdtFullCalibData* dataObj = writeHandle.getCalibData(detElId, msgStream());
            if(dataObj->rtRelation) {
                ATH_MSG_DEBUG("Rt relation constants for " << m_idHelperSvc->toString(detElId) << " already exist");
                continue;
            }
        }
        //find the calibration data for this detector element
        auto it = rtCalibration.find(detElId);
        RtPayload payload{};
        if(it == rtCalibration.end()) {
            // payload is not appearing in database for multilayer 2, so default it to multilayer 1 calibration
            if(m_idHelperSvc->mdtIdHelper().multilayer(detElId) == 2) {
                //try to find the calibration data for multilayer 1
                Identifier ml1Id = m_idHelperSvc->mdtIdHelper().elementID(m_idHelperSvc->stationName(detElId), 
                                                                        m_idHelperSvc->stationEta(detElId), 
                                                                        m_idHelperSvc->stationPhi(detElId));
                Identifier mlID = m_idHelperSvc->mdtIdHelper().multilayerID(ml1Id, 1);
                it = rtCalibration.find(mlID);
                if(it == rtCalibration.end()) {
                    ATH_MSG_WARNING("No calibration data found for " << m_idHelperSvc->toString(ml1Id) << " Identifier is " << ml1Id);
                    continue;
                } else {
                    ATH_MSG_DEBUG("Identifier " << m_idHelperSvc->toString(detElId) << " not found, using multilayer 1 calibration");
                    payload = it->second;
                }
            }
        } else{
            ATH_MSG_DEBUG("Found calibration data for " << m_idHelperSvc->toString(detElId) << " Identifier is " << detElId);
            payload = it->second;

        }

        //extract rt parameters
        std::vector<MuonCalib::SamplePoint> samplePointsRt{};
        for(size_t i = 0; i < payload.times.size(); ++i) {
            //remap t to [-1,1] range
            const double chebyShevT = 2.*(payload.times[i] - 0.5*(payload.times.front() + payload.times.back())) / (payload.times.back() - payload.times.front());
            samplePointsRt.emplace_back(chebyShevT, payload.radii[i], payload.resolutions[i]);            
        }

        MuonCalib::CalibFunc::ParVec rtPars{payload.times.front(), payload.times.back()}, resolutionPars{};
        MuonCalib::BaseFunctionFitter fitter{m_maxOrder};
        fitter.fit_parameters(samplePointsRt, 1, samplePointsRt.size(), MuonCalib::ChebyshevPolynomial{});
        for(const double c : fitter.coefficients()) {
            ATH_MSG_DEBUG("rtPars: " << c);
            rtPars.push_back(c);
        }

        //Extract tr parameters
        std::vector<MuonCalib::SamplePoint> samplePointsTr{};
        for(size_t i = 0; i < payload.radii.size(); ++i) {
            //remap r to [-1,1] range
            const double chebyShevR = 2.*(payload.radii[i] - 0.5*(payload.radii.front() + payload.radii.back())) / (payload.radii.back() - payload.radii.front());
            samplePointsTr.emplace_back(chebyShevR, payload.times[i], payload.resolutions[i]);
        }
        MuonCalib::CalibFunc::ParVec trPars{payload.radii.front(), payload.radii.back()};
        fitter.fit_parameters(samplePointsTr, 1, samplePointsTr.size(), MuonCalib::ChebyshevPolynomial{});
        for(const double c : fitter.coefficients()) {
            ATH_MSG_DEBUG("trPars: " << c);
            trPars.push_back(c);
        }

        auto rtRelRegion{std::make_shared<MuonCalib::RtChebyshev>(rtPars)};
        std::unique_ptr<MuonCalib::IRtResolution> rtResRegion{};
        auto trRelRegion{std::make_shared<MuonCalib::TrChebyshev>(rtRelRegion, trPars)};
        RtRelationPtr MdtRt = std::make_unique<MuonCalib::MdtRtRelation>(std::move(rtRelRegion), std::move(rtResRegion), std::move(trRelRegion), 0.);
        if(!writeHandle.storeData(detElId, MdtRt, msgStream())) {
            ATH_MSG_ERROR("Failed to store Rt for " << m_idHelperSvc->toString(detElId));
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::loadTube(MuonCalib::MdtCalibDataContainer& writeCdo) const {
    ATH_MSG_DEBUG("loadTube " << name());
    SG::ReadCondHandle readHandleTube{m_readKeyTube};
    if(!readHandleTube.isValid()) {
        ATH_MSG_ERROR("No tube data found in " << m_readKeyTube.key());
        return StatusCode::FAILURE;
    }
    std::unordered_map<Identifier, RtTubePayload> tubeCalibration{};
    for(CondAttrListCollection::const_iterator it = readHandleTube->begin(); 
                                            it != readHandleTube->end(); ++it) {
        if(!readTubePayload(it->second, tubeCalibration)) {
            ATH_MSG_ERROR("Failed to read tube payload");
            return StatusCode::FAILURE;
        }
    }
    std::vector<const MuonGMR4::MdtReadoutElement*> detEls = m_r4detMgr->getAllMdtReadoutElements();
    for(const MuonGMR4::MdtReadoutElement* mdtDetEl: detEls){
        const Identifier& chamberId = mdtDetEl->identify();
        if(writeCdo.hasDataForChannel(chamberId, msgStream())) {
            if(writeCdo.getCalibData(chamberId, msgStream())->tubeCalib) {
                ATH_MSG_DEBUG("Tube calibration constants for " << m_idHelperSvc->toString(chamberId) << " already exist");
                continue;
            }
        }
        TubeContainerPtr tubes = std::make_unique<MuonCalib::MdtTubeCalibContainer>(m_idHelperSvc.get(), chamberId);
        if(!writeCdo.storeData(chamberId, tubes, msgStream())) {
            ATH_MSG_FATAL("Failed to store chamber " << m_idHelperSvc->toString(chamberId));
            return StatusCode::FAILURE;
        }
        for(std::size_t layer = 1; layer <= mdtDetEl->numLayers(); ++layer){
            for(std::size_t tube = 1; tube <= mdtDetEl->numTubesInLay(); ++tube){
                const Identifier tubeId{mdtDetEl->measurementId(mdtDetEl->measurementHash(layer, tube))};
                auto it = tubeCalibration.find(tubeId);
                if(it == tubeCalibration.end()) {
                    ATH_MSG_WARNING("No calibration data found for " << m_idHelperSvc->toString(tubeId));
                    continue;
                }
                const RtTubePayload& payload = it->second;
                auto tubeCalibration = std::make_unique<MuonCalib::MdtTubeCalibContainer::SingleTubeCalib>();
                tubeCalibration->statusCode = payload.statusCode;
                tubeCalibration->t0 = payload.tZero;
                tubeCalibration->adcCal = payload.meanAdc;
                if(!tubes->setCalib(std::move(tubeCalibration), tubeId, msgStream())) {
                    ATH_MSG_ERROR("Failed to store tube calibration for " << m_idHelperSvc->toString(tubeId));
                    return StatusCode::FAILURE;
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::readRtPayload(const coral::AttributeList& attributeList, std::unordered_map<Identifier, RtPayload>& rtCalibration) const {
    ATH_MSG_DEBUG("readRtPayload " << name());
    std::string data{}, delim{};
    if (attributeList["data"].specification().type() == typeid(coral::Blob)) {
        if (!CoralUtilities::readBlobAsString(attributeList["data"].data<coral::Blob>(), data)) {
            ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
            return StatusCode::FAILURE;
        }
        delim = "\n";
    } else {
        data = *(static_cast<const std::string *>((attributeList["data"]).addressOfData()));
        delim = " ";
    }    
    const std::vector<std::string> tokens = CxxUtils::tokenize(data, delim);
    if (tokens.size() < 2) {
        ATH_MSG_FATAL("The line "<<data<<" cannot be resolved into header & payload");
        return StatusCode::FAILURE;
    }
    const std::string& header = tokens[0];
    const std::string& payload = tokens[1];
    const std::vector<int> tokensHeader = CxxUtils::tokenizeInt(header, ",");
    if(tokensHeader.size()< 2){
            ATH_MSG_FATAL("Failed to deduce extract number of points & calib Identifier from "<<header);
            return StatusCode::FAILURE;
    }
    const std::size_t calibId = tokensHeader[0];
    const std::size_t numPoints = tokensHeader[1];
    MuonCalib::MuonFixedId id(calibId);
    if (!id.is_mdt()) {
        ATH_MSG_FATAL("Found non-MDT MuonFixedId, continuing...");
        return StatusCode::FAILURE;
    }
    const Identifier athenaId = m_idToFixedIdTool->fixedIdToId(id);
    if (!m_idHelperSvc->isMuon(athenaId)) {
            ATH_MSG_WARNING("The translation from the calibration ID with station: "
                            <<id.stationNameString()<<"("<<id.stationName()<<") "
                            <<" eta:"<<id.eta()<<" phi: "<<id.phi());
    }

    RtPayload channelPayload{};
    channelPayload.station = m_idHelperSvc->stationNameString(athenaId);
    channelPayload.eta = m_idHelperSvc->stationEta(athenaId);
    channelPayload.phi = m_idHelperSvc->stationPhi(athenaId);
    channelPayload.ml = m_idHelperSvc->mdtIdHelper().multilayer(athenaId);
    channelPayload.layer = m_idHelperSvc->mdtIdHelper().tubeLayer(athenaId);
    channelPayload.tube = m_idHelperSvc->mdtIdHelper().tube(athenaId);
    channelPayload.appliedRT = (attributeList["tech"].data<int>());

    const std::vector<double> dataPoints = CxxUtils::tokenizeDouble(payload, ",");
    if (dataPoints.size() % 3 != 0) {
        ATH_MSG_FATAL("Payload does not contain a multiple of 3 elements. Size: " << dataPoints.size());
        return StatusCode::FAILURE;
    }

    // Calculate expected number of data points
    const std::size_t expectedPoints = dataPoints.size() / 3;
    if (expectedPoints != numPoints) {
        ATH_MSG_FATAL("Number of data points (" << expectedPoints << ") does not match numPoints in header (" << numPoints << ").");
        return StatusCode::FAILURE;
    }

    std::vector<double> radii(expectedPoints);
    std::vector<double> times(expectedPoints);
    std::vector<double> resos(expectedPoints);

    for (size_t i = 0; i < expectedPoints; ++i) {
        radii[i] = dataPoints[i * 3];
        times[i] = dataPoints[i * 3 + 1];
        resos[i] = dataPoints[i * 3 + 2];
    }
    channelPayload.radii = std::move(radii);
    channelPayload.times = std::move(times);
    channelPayload.resolutions = std::move(resos);

    rtCalibration[athenaId] = std::move(channelPayload);
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::readTubePayload(const coral::AttributeList& attributeList, std::unordered_map<Identifier, RtTubePayload>& tubeCalibration) const {
    ATH_MSG_DEBUG("readTubePayload " << name());

    std::string data;
    if (attributeList["data"].specification().type() == typeid(coral::Blob)) {
        ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
        if (!CoralUtilities::readBlobAsString(attributeList["data"].data<coral::Blob>(), data)) {
            ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
            return StatusCode::FAILURE;
        }
    } else {
        data = *(static_cast<const std::string*>(attributeList["data"].addressOfData()));
    }

    const std::vector<std::string> tokens = CxxUtils::tokenize(data, "\n");
    if (tokens.size() < 2) {
        ATH_MSG_FATAL("The line " << data << " cannot be resolved into header & payload");
        return StatusCode::FAILURE;
    }

    std::string header = tokens[0];
    const std::string& payload = tokens[1];

    std::string stName = header.substr(2, 3);
    std::replace(header.begin(), header.end(), '_', ',');
    const std::vector<std::string> headerTokens = CxxUtils::tokenize(header, ",");

    if (headerTokens.size() < 6) { // expecting at least 6 tokens
        ATH_MSG_FATAL("Invalid header format: " << header);
        return StatusCode::FAILURE;
    }

    int phi = std::atoi(headerTokens[1].c_str());
    int eta = std::atoi(headerTokens[2].c_str());
    Identifier chamID = m_idHelperSvc->mdtIdHelper().elementID(stName, eta, phi);

    RtTubePayload channelPayload{};
    channelPayload.station = stName;
    channelPayload.eta = eta;
    channelPayload.phi = phi;
    channelPayload.appliedT0 = (attributeList["tech"].data<int>());

    const std::vector<double> payLoadData = CxxUtils::tokenizeDouble(payload, ",");
    if (payLoadData.size() % 3 != 0) {
        ATH_MSG_FATAL("Payload size is not a multiple of 3. Size: " << payLoadData.size());
        return StatusCode::FAILURE;
    }

    std::size_t numPoints = payLoadData.size() / 3;
    std::vector<double> tZeros, meanAdcs;
    std::vector<int> statusCodes;

    tZeros.reserve(numPoints);
    meanAdcs.reserve(numPoints);
    statusCodes.reserve(numPoints);

    for (std::size_t i = 0; i < payLoadData.size(); i += 3) {
        tZeros.push_back(payLoadData[i]);
        statusCodes.push_back(static_cast<int>(payLoadData[i + 1]));
        meanAdcs.push_back(payLoadData[i + 2]);
    }

    if (statusCodes.size() != tZeros.size() || statusCodes.size() != meanAdcs.size() || statusCodes.empty()) {
        ATH_MSG_FATAL("Inconsistent payload data sizes for chamber " << m_idHelperSvc->toStringChamber(chamID));
        return StatusCode::FAILURE;
    }

    int ml = 1, layer = 1, tube = 1;
    const int numMl = m_idHelperSvc->mdtIdHelper().numberOfMultilayers(chamID);
    const Identifier secondMlID = m_idHelperSvc->mdtIdHelper().multilayerID(chamID, numMl);
    const int tubesPerLay = std::max(m_idHelperSvc->mdtIdHelper().tubeMax(chamID),
                               m_idHelperSvc->mdtIdHelper().tubeMax(secondMlID));
    const int numLayers = std::max(m_idHelperSvc->mdtIdHelper().tubeLayerMax(chamID),
                             m_idHelperSvc->mdtIdHelper().tubeLayerMax(secondMlID));

    for(std::size_t i = 0; i < tZeros.size(); ++i){
        Identifier tubeID = m_idHelperSvc->mdtIdHelper().channelID(chamID, ml, layer, tube);
        RtTubePayload tubePayload = channelPayload;
        tubePayload.ml = ml;
        tubePayload.layer = layer;
        tubePayload.tube = tube;
        tubePayload.tZero = tZeros[i];
        tubePayload.meanAdc = meanAdcs[i];
        tubePayload.statusCode = statusCodes[i];
        tubeCalibration[tubeID] = std::move(tubePayload);
        if (++tube > tubesPerLay) {
            tube = 1;
            if (++layer > numLayers) {
                layer = 1;
                ++ml;
            }
        }
    }
    return StatusCode::SUCCESS;
}


