/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibDbAlg.h"

#include "AthenaKernel/IOVInfiniteRange.h"
#include "MdtCalibData/MdtFullCalibData.h"

#include "MdtCalibData/CalibParamSorter.h"
#include "MdtCalibData/RtRelationLookUp.h"
#include "MdtCalibData/RtResolutionLookUp.h"
#include "MdtCalibData/RtResolutionChebyshev.h"
#include "MdtCalibData/RadiusResolutionChebyshev.h"

#include "MdtCalibData/RtChebyshev.h"
#include "MdtCalibData/TrChebyshev.h"
#include "MdtCalibData/RtLegendre.h"
#include "MdtCalibData/TrLegendre.h"

#include "CxxUtils/StringUtils.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"

#include <unordered_map>
#include <fstream>

#include "TFile.h"
#include "TTree.h"


using namespace MuonCalib;
using RegionGranularity = MdtCalibDataContainer::RegionGranularity;



#define SET_BRANCHADDRESS(tree, branchName)                                  \
    {                                                                        \
        if (!tree.GetBranch(#branchName)) {                                  \
            ATH_MSG_FATAL("The branch "<<#branchName<<" does not exist.");   \
            return StatusCode::FAILURE;                                      \
        }                                                                    \
        if (tree.SetBranchAddress(#branchName,&branchName) != 0){            \
             ATH_MSG_FATAL("Failed to connect branch "<<#branchName<<".");   \
             return StatusCode::FAILURE;                                     \
        }                                                                    \
    }

namespace MuonCalibR4 {

StatusCode MdtCalibDbAlg::initialize() {
    ATH_MSG_VERBOSE("Initialize...");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    
    /// If not trees names are given, then there's no need to try accessing the ROOT file
    if (m_t0TreeName.value().empty()) {
        m_t0RootFile.value().clear();
    }
    if (m_rtTreeName.value().empty()) {
        m_rtRootFile.value().clear();
    }
    const bool initRt = (m_rtJSON.value().empty() && m_rtRootFile.value().empty());
    const bool initT0 = (m_t0JSON.value().empty() && m_t0RootFile.value().empty());
    ATH_CHECK(m_readKeyRt.initialize(initRt));
    ATH_CHECK(m_readKeyTube.initialize(initT0));

    if (m_rtRootFile.value().size()) {
        ATH_MSG_INFO("Load  RT - calibration constants from ROOT File "<<m_rtRootFile);
        m_rtJSON.clear();
    } else if (m_rtJSON.value().size()) {
        ATH_MSG_INFO("Load the RT- calibration constants from JSON file "<<m_rtJSON);
        m_rtTreeName.clear();
    } else {
        ATH_MSG_INFO("Load RT - calibration constants from  COOL: "<<m_readKeyRt.fullKey());
    }
    if (m_t0RootFile.value().size()) {
        ATH_MSG_INFO("Load  T0 - calibration constants from ROOT File "<<m_t0RootFile);
        m_t0JSON.clear();
    } else if (m_t0JSON.value().size()) {
        ATH_MSG_INFO("Load the T0 - calibration constants from JSON file "<<m_t0JSON);
        m_t0TreeName.clear();
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("Executing MdtCalibDbAlgR4");
    SG::WriteCondHandle writeHandle{m_writeKey, ctx};
    if(writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
    auto writeCdo = std::make_unique<MdtCalibDataContainer>(m_idHelperSvc.get(), RegionGranularity::OnePerMultiLayer);
    /** Load the t0 blob */
    writeCdo->setInversePropSpeed(1./ (m_prop_beta * Gaudi::Units::c_light));

    /** @brief Parse the constants from the database */
    if (!m_readKeyRt.empty()) {
        SG::ReadCondHandle readHandle{m_readKeyRt, ctx};
        ATH_CHECK(readHandle.isValid());
        writeHandle.addDependency(readHandle);
        for(CondAttrListCollection::const_iterator itr = readHandle->begin(); 
                                                   itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            if(atr["data"].specification().type() != typeid(coral::Blob)) {
                ATH_MSG_FATAL( "Data column is not of type blob!" );
                return StatusCode::FAILURE;
            }

            coral::Blob blob = atr["data"].data<coral::Blob>();

            if(m_dbPayloadType =="TTree"){
                std::unique_ptr<TTree> tree;
                if(!CoralUtilities::readBlobAsTTree(blob, tree, "RtCalibConstants")) {
                    ATH_MSG_FATAL( "Cannot retrieve data from coral blob!");
                    return StatusCode::FAILURE;
                }
                ATH_CHECK(parseRtPayload(*tree, *writeCdo));
            } else if (m_dbPayloadType=="JSON") {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                std::string data{};
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
                nlohmann::json rtBlob = nlohmann::json::parse(data);
                ATH_CHECK(parseRtPayload(std::move(rtBlob),*writeCdo));
            } else {
                ATH_MSG_FATAL("Payload type " << m_dbPayloadType << " not understood. Options are: TTree or JSON");
                return StatusCode::FAILURE;
            }
        }
    
    } else if (!m_rtRootFile.value().empty()) {
        std::unique_ptr<TFile> inFile{TFile::Open(m_rtRootFile.value().c_str(), "READ")};
        if(!inFile || inFile->IsZombie()) {
            ATH_MSG_FATAL("Failed to open file "<<m_rtRootFile);
            return StatusCode::FAILURE;
        }
        TTree* tree{nullptr};
        inFile->GetObject(m_rtTreeName.value().c_str(), tree);
        if (!tree) {
            ATH_MSG_FATAL("The object "<<m_rtRootFile<<" does not contain "<<m_rtTreeName);
            return StatusCode::FAILURE;
        }
        ATH_CHECK(parseRtPayload(*tree, *writeCdo));
    } else if (!m_rtJSON.value().empty()) {
        std::ifstream inFile{m_rtJSON};
        if (!inFile.good()) {
            ATH_MSG_FATAL("Failed to open "<<m_rtJSON);
            return StatusCode::FAILURE;
        }
        nlohmann::json rtBlob{};
        inFile >> rtBlob;
        ATH_CHECK(parseRtPayload(std::move(rtBlob),*writeCdo));
    }
    if (!m_readKeyTube.empty()) {
        SG::ReadCondHandle readHandle{m_readKeyTube, ctx};
        ATH_CHECK(readHandle.isValid());
        writeHandle.addDependency(readHandle);
        for(CondAttrListCollection::const_iterator itr = readHandle->begin(); 
                                                   itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            if(atr["data"].specification().type() != typeid(coral::Blob)) {
                ATH_MSG_FATAL( "Data column is not of type blob!" );
                return StatusCode::FAILURE;
            }

            coral::Blob blob = atr["data"].data<coral::Blob>();

            if(m_dbPayloadType =="TTree"){
                std::unique_ptr<TTree> tree;
                if(!CoralUtilities::readBlobAsTTree(blob, tree, "T0CalibConstants")) {
                    ATH_MSG_FATAL( "Cannot retrieve data from coral blob!");
                    return StatusCode::FAILURE;
                }
                ATH_CHECK(parseT0Payload(*tree, *writeCdo));
            } else if (m_dbPayloadType=="JSON") {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                std::string data{};
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
                nlohmann::json rtBlob = nlohmann::json::parse(data);
                ATH_CHECK(parseT0Payload(std::move(rtBlob),*writeCdo));
            } else {
                ATH_MSG_FATAL("Payload type " << m_dbPayloadType << " not understood. Options are: TTree or JSON");
                return StatusCode::FAILURE;
            }

        }
    } else if (!m_t0RootFile.value().empty()) {
        std::unique_ptr<TFile> inFile{TFile::Open(m_t0RootFile.value().c_str(), "READ")};
        if(!inFile || inFile->IsZombie()) {
            ATH_MSG_FATAL("Failed to open file "<<m_t0RootFile);
            return StatusCode::FAILURE;
        }
        TTree* tree{nullptr};
        inFile->GetObject(m_t0TreeName.value().c_str(), tree);
        if (!tree) {
            ATH_MSG_FATAL("The object "<<m_rtRootFile<<" does not contain "<<m_t0TreeName);
            return StatusCode::FAILURE;
        }
        ATH_CHECK(parseT0Payload(*tree, *writeCdo));
    } else {
        std::ifstream inFile{m_t0JSON};
        if (!inFile.good()) {
            ATH_MSG_FATAL("Failed to open "<<m_t0JSON);
            return StatusCode::FAILURE;
        }
        nlohmann::json t0Blob{};
        inFile >> t0Blob;
        ATH_CHECK(parseT0Payload(std::move(t0Blob), *writeCdo));
    }

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_INFO("Recorded succesfully "<<m_writeKey.fullKey()<<" "<<writeHandle.getRange());
    return StatusCode::SUCCESS;
}


StatusCode MdtCalibDbAlg::parseRtPayload(TTree& rtTree,
                                         MdtCalibDataContainer& outContainer) const {
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};

    std::string* rtType{nullptr}, *trType{nullptr}, *resoType{nullptr};

    std::vector<double>* rtParams{nullptr}, *trParams{nullptr}, *resoParams{nullptr};

    std::vector<std::string>* stationName{nullptr};
    std::vector<short>* stationEta{nullptr};
    std::vector<unsigned short>* stationPhi{nullptr}, *multiLayer{nullptr};

    SET_BRANCHADDRESS(rtTree, rtType);
    SET_BRANCHADDRESS(rtTree, trType);
    SET_BRANCHADDRESS(rtTree, resoType);
    SET_BRANCHADDRESS(rtTree, rtParams);
    SET_BRANCHADDRESS(rtTree, trParams);
    SET_BRANCHADDRESS(rtTree, resoParams);
    SET_BRANCHADDRESS(rtTree, stationName);
    SET_BRANCHADDRESS(rtTree, stationEta);
    SET_BRANCHADDRESS(rtTree, stationPhi);
    SET_BRANCHADDRESS(rtTree, multiLayer);

    for (Long64_t e = 0; e< rtTree.GetEntries(); ++e) {
        rtTree.GetEntry(e);
        ATH_MSG_VERBOSE("Load "<<e<<"-th calibration constant Valid for "<<stationName->size()<< " det elements.");
        std::vector<Identifier> detIds{};
        for (unsigned ch = 0; ch < stationName->size(); ++ch){
            const Identifier detElId = idHelper.channelID(stationName->at(ch), stationEta->at(ch), stationPhi->at(ch),
                                                          multiLayer->at(ch), 1, 1);
            detIds.emplace_back(detElId);
        }
        IRtRelationPtr rtRel{makeRt(*rtType, *rtParams)};
        if(!rtRel) {
            return StatusCode::FAILURE;
        } 
        ITrRelationPtr trRel = (*trType).size() ? makeTr(*trType, *trParams) : nullptr;
        if ((*trType).size() && !trRel) {
            return StatusCode::FAILURE;
        }
        IRtResolutionPtr rtReso = makeReso(*resoType, *resoParams, rtRel);
        if (!rtReso) {
            return StatusCode::FAILURE;
        }
        MdtFullCalibData::RtRelationPtr mdtRel =  
                std::make_unique<MdtRtRelation>(std::move(rtRel), std::move(rtReso), std::move(trRel));
        for (const Identifier& detId : detIds) {
             if (!outContainer.storeData(detId, mdtRel, msgStream())){
                return StatusCode::FAILURE;
             }
        }
    }
    return StatusCode::SUCCESS;
}
IRtRelationPtr MdtCalibDbAlg::makeRt(const std::string& rtType, const std::vector<double>& rtParams) const {
    IRtRelationPtr rtRel{};
    if (rtType == "RtRelationLookUp") {
        rtRel = std::make_unique<RtRelationLookUp>(rtParams);
    } else if (rtType == "RtLegendre"){
        rtRel = std::make_unique<RtLegendre>(rtParams);
    } else if (rtType == "RtChebyshev") {
        rtRel = std::make_unique<RtChebyshev>(rtParams);
    } 
    if (rtRel) {
        ATH_MSG_VERBOSE("Fetched new rt-relation <"<<rtType<<"> valid for drift times ranging from "
                        <<rtRel->tLower()<<"-"<<rtRel->tUpper()<<".");
    } else {
        ATH_MSG_FATAL("The rt-relation function type <"<<rtType<<"> is not yet supported.");
    }
    return rtRel;
}

ITrRelationPtr MdtCalibDbAlg::makeTr(const std::string& trType, const std::vector<double>& trParams) const {
    ITrRelationPtr trRel{};
    if (trType == "TrChebyshev") {
        trRel = std::make_unique<TrChebyshev>(trParams);
    } else if(trType == "TrLegendre") {
        trRel = std::make_unique<TrLegendre>(trParams);
    }
    if (trRel) {
        ATH_MSG_VERBOSE("Fetched new tr-relation <"<<trType<<"> valid for drift radii ranging from "
                      <<trRel->minRadius()<<"-"<<trRel->maxRadius()<<".");
    } else {
        ATH_MSG_FATAL("The rt-relation function type <"<<trType<<"> is not yet supported.");       
    }
    return trRel;
}

IRtResolutionPtr MdtCalibDbAlg::makeReso(const std::string& resoType, const std::vector<double>& resoParams,
                                         IRtRelationPtr rt) const {
   IRtResolutionPtr rtReso{};
   if (resoType == "RtResolutionLookUp") {
        rtReso = std::make_unique<RtResolutionLookUp>(resoParams);
    } else if (resoType == "RtResolutionChebyshev") {
        rtReso = std::make_unique<RtResolutionChebyshev>(resoParams);
    } else if (resoType == "RadiusResolutionChebyshev") {
        rtReso = std::make_unique<RadiusResolutionChebyshev>(resoParams, rt);
    } else {
        ATH_MSG_FATAL("The rt resolution type <"<<resoType<<"> is not yet supported.");        
    }
   return rtReso;
}
StatusCode MdtCalibDbAlg::parseRtPayload(const nlohmann::json& rtBlob,
                                         MdtCalibDataContainer& outContainer) const{
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};

    /** Loop over the R-T blob payload */
    for (auto& payload : rtBlob.items()) {       
        std::vector<Identifier> detIds{};

        const nlohmann::json calibConstants = payload.value();
        /** Fetch the associated detector elements */
        for (auto& chambList : calibConstants["chambers"].items()) {                
            const nlohmann::json chambPayload = chambList.value();
            const std::string station = chambPayload["station"];
            const int eta = chambPayload["eta"];
            const int phi = chambPayload["phi"];
            const int ml = chambPayload["ml"];           
            const Identifier mlId{idHelper.channelID(station, eta, phi, ml, 1, 1)};
            detIds.emplace_back(mlId);
        }

        /// Defining constants of the rt-relation
        const nlohmann::json rtPayload = calibConstants["rtRelation"];
        ITrRelationPtr trRel{};
        IRtRelationPtr rtRel = makeRt(rtPayload["type"], rtPayload["params"]);
        if (!rtRel) {
            return StatusCode::FAILURE;
        }
        if (calibConstants.find("trRelation") != calibConstants.end()) {
            const nlohmann::json trPayload = calibConstants["trRelation"];
            trRel = makeTr(trPayload["type"], trPayload["params"]);
            if (!trRel) {
                return StatusCode::FAILURE;
            }
        }
        /// Defining constants of the rt-resolution
        const nlohmann::json resoPayload = calibConstants["rtReso"];
        IRtResolutionPtr rtReso =makeReso(resoPayload["type"], resoPayload["params"], rtRel);
        if (!rtReso) {
            return StatusCode::FAILURE;
        }        
        MdtFullCalibData::RtRelationPtr mdtRel =  
                std::make_unique<MdtRtRelation>(std::move(rtRel), std::move(rtReso), std::move(trRel));


        /** Load the rt constants to the assigned detector elements */
        for (const Identifier& mlId : detIds) {
            if (outContainer.hasDataForChannel(mlId,msgStream()) && 
                outContainer.getCalibData(mlId, msgStream())->rtRelation) {
                continue;
            }
            if (!outContainer.storeData(mlId, mdtRel, msgStream())) {
                return StatusCode::FAILURE;
            }
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::parseT0Payload(TTree& t0Tree,
                                         MuonCalib::MdtCalibDataContainer& outContainer) const {
    
    std::string* stationName{nullptr};
    short stationEta{0};
    unsigned short stationPhi{0}, code{0};
    float t0{0.f}, adc{0.f};
    std::vector<unsigned short>* multiLayer{nullptr}, *tubeLayer{nullptr}, *tube{nullptr};

    SET_BRANCHADDRESS(t0Tree, stationName);
    SET_BRANCHADDRESS(t0Tree, stationEta);
    SET_BRANCHADDRESS(t0Tree, stationPhi);
    SET_BRANCHADDRESS(t0Tree, code);
    SET_BRANCHADDRESS(t0Tree, t0);
    SET_BRANCHADDRESS(t0Tree, adc);
    SET_BRANCHADDRESS(t0Tree, multiLayer);
    SET_BRANCHADDRESS(t0Tree, tubeLayer);
    SET_BRANCHADDRESS(t0Tree, tube);
    
    using SingleTubeCalib = MdtTubeCalibContainer::SingleTubeCalib;
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};

    for (Long64_t e = 0 ; e <t0Tree.GetEntries(); ++e) {
        t0Tree.GetEntry(e);
        bool isValid{false};
        const Identifier detId = idHelper.elementID(*stationName, stationEta, stationPhi, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to create a valid Identifier from "<<(*stationName)<<", "<<stationEta<<", "
                        <<stationPhi);
            return StatusCode::FAILURE;
        }
        TubeContainerPtr calibChannels = std::make_unique<MdtTubeCalibContainer>(m_idHelperSvc.get(), detId);
        SingleTubeCalib t0Calib{};
        t0Calib.adcCal = adc;
        t0Calib.t0 = t0;
        t0Calib.statusCode = code;

        for (unsigned int ch = 0; ch < multiLayer->size(); ++ch){
            const Identifier tubeId{idHelper.channelID(detId, multiLayer->at(ch),
                                                        tubeLayer->at(ch), tube->at(ch), isValid)};
            if (!isValid) {
                ATH_MSG_FATAL("Failed to get a valid tube Identifier for "<<m_idHelperSvc->toStringChamber(detId)
                            <<", ml: "<<multiLayer->at(ch)<<", tl: "<<tubeLayer->at(ch)<<", tube: "<<tube->at(ch));
                return StatusCode::FAILURE;
            }
            if (!calibChannels->setCalib(t0Calib, tubeId, msgStream())) {
                return StatusCode::FAILURE;
            }
        }
        if (!outContainer.storeData(detId, calibChannels, msgStream())){
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode MdtCalibDbAlg::parseT0Payload(const nlohmann::json& t0Blob,
                                         MdtCalibDataContainer& outContainer) const {
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    using SingleTubeCalib = MdtTubeCalibContainer::SingleTubeCalib;
    
    /** Loop over the R-T blob payload */
    for (auto& dbEntry : t0Blob.items()) {
        const nlohmann::json payload = dbEntry.value();
        const std::string station = payload["station"];
        const int eta = payload["eta"];
        const int phi = payload["phi"];
        bool isValid{false};
        const Identifier detId = idHelper.elementID(station, eta, phi, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to create a valid identifier from "<<station<<", "<<eta<<", "<<phi);
            return StatusCode::FAILURE;
        }
        TubeContainerPtr calibChannels = std::make_unique<MdtTubeCalibContainer>(m_idHelperSvc.get(), detId);

        for (auto& calibEntry : payload["calibConstants"].items()){
            const nlohmann::json calibPayload = calibEntry.value();
            SingleTubeCalib calibConstant{};
            calibConstant.t0 = calibPayload["t0"];
            calibConstant.adcCal = calibPayload["adc"];
            calibConstant.statusCode = calibPayload["code"];
            for (auto& tubeEntry : calibPayload["tubes"].items()) {
                 const nlohmann::json tubePayload = tubeEntry.value();
                 const int ml = tubePayload["ml"];
                 const int tubeLayer = tubePayload["tl"];
                 const std::string tubeStr = tubePayload["no"];
                 const std::vector<std::string> tubeTokens{CxxUtils::tokenize(tubeStr, ";")};
                 std::vector<int> tubes{};
                 for (const std::string& token: tubeTokens) {
                    if (token.find("-") == std::string::npos) {
                        tubes.emplace_back(CxxUtils::atoi(token));
                    } else {
                        const std::vector<std::string> rangeToken{CxxUtils::tokenize(token,"-")};
                        if (rangeToken.size() != 2) {
                            ATH_MSG_FATAL("No valid tube range token: "<<token);
                            return StatusCode::FAILURE;
                        }
                        const int tubeLow{CxxUtils::atoi(rangeToken[0])}, tubeHigh{CxxUtils::atoi(rangeToken[1])};
                        if (tubeLow >= tubeHigh){
                            ATH_MSG_FATAL("Invalid range "<<tubeLow<<"-"<<tubeHigh
                                        <<". The lower end must be strictly smaller than the upper one");
                            return StatusCode::FAILURE;
                        }
                        for (int tube = tubeLow; tube<=tubeHigh; ++tube) {
                            tubes.push_back(tube);
                        }
                    }
                 }
                 for (const int tube : tubes) {
                     const Identifier tubeId = idHelper.channelID(detId,ml, tubeLayer, tube, isValid);
                     if (!isValid) {
                        ATH_MSG_FATAL("No valid tube identifier from "<<m_idHelperSvc->toStringDetEl(detId)
                                    <<", layer: "<<tubeLayer<<", tube: "<<tube);
                        return StatusCode::FAILURE;
                    }
                    if (!calibChannels->setCalib(calibConstant, tubeId, msgStream())){
                        return StatusCode::FAILURE;
                    }
                }
            }
        }
        if (!outContainer.storeData(detId, std::move(calibChannels), msgStream())) {
            return StatusCode::FAILURE;
        }
    }
    
    return StatusCode::SUCCESS;
}

}


