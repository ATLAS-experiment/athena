/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/MdtCalibDbAlg.h"

#include "AthenaKernel/IOVInfiniteRange.h"
#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaKernel/RNGWrapper.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "MdtCalibData/BFieldCorFunc.h"
#include "MdtCalibData/RtRelationLookUp.h"
#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtSlewCorFuncHardcoded.h"
#include "MdtCalibData/RtFromPoints.h"
#include "MdtCalibData/RtResolutionFromPoints.h"
#include "MdtCalibUtils/RtDataFromFile.h"
#include "MuonCalibIdentifier/MdtCalibCreationFlags.h"
#include "MuonCalibIdentifier/MuonFixedId.h"
#include "MuonCalibMath/SamplePoint.h"
#include "MuonCalibTools/IdToFixedIdTool.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/TransientAddress.h"

#include "CxxUtils/StringUtils.h"


#include <fstream>

#include "TFile.h"
#include "TSpline.h"
#include "GeoModelKernel/throwExcept.h"

using namespace MuonCalib;
using namespace CxxUtils;
using CorrectionPtr = MdtFullCalibData::CorrectionPtr;
using TubeContainerPtr = MdtFullCalibData::TubeContainerPtr;
using RegionGranularity = MdtCalibDataContainer::RegionGranularity;

MdtCalibDbAlg::MdtCalibDbAlg(const std::string &name, ISvcLocator *pSvcLocator) :
    AthCondAlgorithm(name, pSvcLocator) {}

StatusCode MdtCalibDbAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());

    // if timeslew correction vector m_MeanCorrectionVsR has non-zero size then set
    // m_TsCorrectionT0=m_MeanCorrectionVsR[0] and subtract this each value in the vector.
    if (m_MeanCorrectionVsR.size()) {
        m_TsCorrectionT0 = m_MeanCorrectionVsR[0];
        for (float & it : m_MeanCorrectionVsR) {
            it -= m_TsCorrectionT0;
        }
    }

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_idToFixedIdTool.retrieve());
    // initiallize random number generator if doing t0 smearing (for robustness studies)
    if (m_t0Spread != 0.) {
        ATH_CHECK(m_AthRNGSvc.retrieve());
        ATH_MSG_DEBUG(" initialize Random Number Service: running with t0 shift " 
                     << m_t0Shift << " spread " << m_t0Spread << " rt shift " << m_rtShift);
        // getting our random numbers stream
        m_RNGWrapper = m_AthRNGSvc->getEngine(this, m_randomStream);
        if (!m_RNGWrapper) {
            ATH_MSG_ERROR("Could not get random number engine from AthRNGSvc. Abort.");
            return StatusCode::FAILURE;
        }
    }

    if (m_rtShift != 0. || m_rtScale != 1. || m_t0Shift != 0. || m_t0Spread != 0.) {
        ATH_MSG_INFO("************************************" << std::endl
                                                            << " Running with Calibration Deformations! " << std::endl
                                                            << " For performance studies only!" << std::endl
                                                            << " **************************************");
        ATH_MSG_DEBUG(" rt scale " << m_rtScale << " t0 shift " << m_t0Shift << " spread " << m_t0Spread << " rt shift " << m_rtShift);
    }

    ATH_CHECK(m_readKeyRt.initialize());
    ATH_CHECK(m_readKeyTube.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKeyDCS.initialize(m_create_b_field_function && !m_readKeyDCS.empty()));
    
    if (m_useNewGeo) ATH_CHECK(detStore()->retrieve(m_r4detMgr));
    else ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
 StatusCode MdtCalibDbAlg::declareDependency(const EventContext& ctx, 
                                             SG::WriteCondHandle<MuonCalib::MdtCalibDataContainer>& writeHandle) const {
    
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    for (const SG::ReadCondHandleKey<CondAttrListCollection>& key : {m_readKeyTube, m_readKeyRt}) {
        if (key.empty()) continue;
        SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve conditions object "<<readHandle.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        ATH_MSG_INFO("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdoRt->size()= " << readHandle->size());
        ATH_MSG_INFO("Range of input is " << readHandle.getRange());
    }
    if (m_readKeyDCS.empty()) return StatusCode::SUCCESS;
    SG::ReadCondHandle<MdtCondDbData> readHandle{m_readKeyDCS, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve conditions object "<<m_readKeyDCS.fullKey());
        return StatusCode::FAILURE;
    }
    if (readHandle->hasDCS()) {
        writeHandle.addDependency(readHandle);
    }
    return StatusCode::SUCCESS;
}
StatusCode MdtCalibDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    SG::WriteCondHandle<MuonCalib::MdtCalibDataContainer> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;   
    }
    ATH_CHECK(declareDependency(ctx, writeHandle));

    RegionGranularity gran{RegionGranularity::OnePerChamber};
    if (m_readKeyRt.key() == "/MDT/RTUNIQUE") {
        ATH_MSG_DEBUG("Save one set of Rt constants per chamber");
        gran = RegionGranularity::OneRt;
    } else if (m_UseMLRt) {
        ATH_MSG_DEBUG("Save one set of calibration constants per multi layer");
        gran = RegionGranularity::OnePerMultiLayer;
    } else  ATH_MSG_DEBUG("Save one set of calibration constants per chamber");
    std::unique_ptr<MuonCalib::MdtCalibDataContainer> writeCdo = std::make_unique<MuonCalib::MdtCalibDataContainer>(m_idHelperSvc.get(), gran);
   
    ATH_CHECK(loadRt(ctx, *writeCdo));
    ATH_CHECK(loadTube(ctx, *writeCdo));
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::defaultRt(MuonCalib::MdtCalibDataContainer& writeCdo, LoadedRtMap& loadedRts) const {
    ATH_MSG_DEBUG("defaultRt " << name());
    std::string fileName = PathResolver::find_calib_file(m_RTfileName);
    std::ifstream inputFile(fileName);
    if (!inputFile) {
        ATH_MSG_ERROR("Unable to open RT Ascii file: " << fileName);
        return StatusCode::FAILURE;
    } 
    ATH_MSG_DEBUG("Opened RT Ascii file: " << fileName);
    

    // Read the RTs from the text file
    MuonCalib::RtDataFromFile rts;
    rts.read(inputFile);
    ATH_MSG_VERBOSE("File contains " << rts.nRts() << " RT relations ");

    // Loop over all RTs in the file (but the default file only has 1 RT)
    // Use the first valid RT found in the file as the default for all chambers.
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    for (unsigned int n = 0; n < rts.nRts(); ++n) {
        std::unique_ptr<MuonCalib::RtDataFromFile::RtRelation> rt(rts.getRt(n));

        const MuonCalib::RtDataFromFile::RtRelation::DataVec &times = rt->times();
        const MuonCalib::RtDataFromFile::RtRelation::DataVec &radii = rt->radii();
        const MuonCalib::RtDataFromFile::RtRelation::DataVec &reso = rt->resolution();

        // check if rt contains data, at least two points on the rt are required
        if (times.size() < 2) {
            ATH_MSG_ERROR(" defaultRt rt table has too few entries");
            return StatusCode::FAILURE;
        }
        // check if all tables have same size
        if (times.size() != radii.size() || times.size() != reso.size()) {
            ATH_MSG_ERROR("defaultRt rt table size mismatch ");
            return StatusCode::FAILURE;
        }
        // check for negative time bins, i.e. decreasing time value with radius
        double t_min = times[0];
        double bin_size = times[1] - t_min;
        if (bin_size <= 0) {
            ATH_MSG_ERROR("defaultRt rt table negative binsize ");
            return StatusCode::FAILURE;
        }

        // create a vector to hold the r values,
        // we need two extra fields to store t_min and bin_size
        MuonCalib::CalibFunc::ParVec rtPars;
        rtPars.push_back(t_min);
        rtPars.push_back(bin_size);

        // copy r values into vector
        rtPars.insert(rtPars.end(), radii.begin(), radii.end());

        ATH_MSG_DEBUG("defaultRt new MuonCalib::IRtRelation");

        MuonCalib::CalibFunc::ParVec resoPars{t_min, bin_size};
        // copy r values into vector
        resoPars.insert(resoPars.end(), reso.begin(), reso.end());

        ATH_MSG_DEBUG("defaultRt new MuonCalib::IRtResolution");

        // create RT and resolution "I" objects
        std::shared_ptr<MuonCalib::IRtRelation> rtRel{std::make_shared<RtRelationLookUp>(rtPars)};
        std::shared_ptr<MuonCalib::IRtResolution> resoRel{std::make_shared<RtResolutionLookUp>(resoPars)};


        // Since the same RT is loaded for all chambers you might be tempted to create it once
        // and simply store the same pointer in writeCdoRt for all regions.
        // However it seems that when StoreGate clears writeCdoRt (which will happen in LoadRt
        // by detStore()->removeDataAndProxy) it will crash unless there are unique pointers/objects
        // for rtRel, resoRel, and MdtRtRelation

        // Loop over RT regions and store the default RT in each
        RtRelationPtr MdtRt = std::make_shared<MuonCalib::MdtRtRelation>(rtRel, resoRel);
        
        for(auto itr = idHelper.detectorElement_begin();
                 itr!= idHelper.detectorElement_end();++itr){
            const Identifier detElId{*itr};
            if (writeCdo.hasDataForChannel(detElId, msgStream())) {
                const MdtFullCalibData* dataObj =  writeCdo.getCalibData(detElId, msgStream());
                if (dataObj->rtRelation) {
                    ATH_MSG_DEBUG("Rt relation constants for "<<m_idHelperSvc->toString(detElId)<<" already exists");
                    continue;
                }
            }
            /// load the calibration constants of the second multilayer from the first one
            RtRelationPtr storeMe = MdtRt;
            if (idHelper.multilayer(detElId) == 2) {
                if (writeCdo.granularity() != RegionGranularity::OnePerMultiLayer) continue;
                const Identifier firstML = idHelper.multilayerID(detElId, 1);
                if (writeCdo.hasDataForChannel(firstML, msgStream())) {
                    const MdtFullCalibData* dataObj =  writeCdo.getCalibData(firstML, msgStream());
                    if (dataObj->rtRelation) {
                        ATH_MSG_DEBUG("Copy Rt constanst from the first multi layer for "<<m_idHelperSvc->toString(detElId));
                        storeMe = dataObj->rtRelation;
                    }
                }
            }
            ATH_MSG_DEBUG("Add default rt constants for "<<m_idHelperSvc->toString(detElId));
            if (!writeCdo.storeData(detElId, storeMe, msgStream())) {
                ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to save default rts for "<<m_idHelperSvc->toString(detElId));
                return StatusCode::FAILURE;
            }
            
            if (!(m_create_b_field_function || m_createSlewingFunction)) continue;
            loadedRts[detElId] = MdtRt;

        }

        // if VERBOSE enabled print out RT function
        if (msgLvl(MSG::VERBOSE)) {
            int npoints = rtRel->nPar() - 2;
            ATH_MSG_VERBOSE("defaultRt npoints from rtRel=" << npoints);
            for (int ipt = 0; ipt < npoints; ++ipt) {
                double t = t_min + ipt * bin_size;
                ATH_MSG_VERBOSE(" " << ipt << " " << t << " " << rtRel->radius(t) << " " << resoRel->resolution(t));
            }
        }
        break;  // only need the first good RT from the text file

    }  // end loop over RTs in file

    return StatusCode::SUCCESS;
}
std::optional<double> MdtCalibDbAlg::getInnerTubeRadius(const Identifier& id) const {
    static std::atomic<bool> rtWarningPrinted = false;
            
    if (m_detMgr) {
        const MuonGM::MdtReadoutElement *detEl = m_detMgr->getMdtReadoutElement(id);        
        if (detEl) { return std::make_optional<double>(detEl->innerTubeRadius()); }
       
    } else if (m_r4detMgr) {
        const MuonGMR4::MdtReadoutElement* detEl = m_r4detMgr->getMdtReadoutElement(id);
        if (detEl) { return std::make_optional<double>(detEl->innerTubeRadius()); }
    }
    if (!rtWarningPrinted) {
        ATH_MSG_WARNING("getInnerTubeRadius() - no Muon station known under the name "
                            << m_idHelperSvc->toString(id));
        rtWarningPrinted = true;
    }
    return std::nullopt;
}

StatusCode MdtCalibDbAlg::legacyRtPayloadToJSON(const coral::AttributeList& attr, nlohmann::json & json) const {
    std::string data{}, delim{};
    if (attr["data"].specification().type() == typeid(coral::Blob)) {
        ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
        if (!CoralUtilities::readBlobAsString(attr["data"].data<coral::Blob>(), data)) {
            ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
            return StatusCode::FAILURE;
        }
        delim = "\n";
    } else {
        data = *(static_cast<const std::string *>((attr["data"]).addressOfData()));
        delim = " ";
    }    
    const std::vector<std::string> tokens = tokenize(data, delim);
    if (tokens.size() < 2) {
        ATH_MSG_FATAL("The line "<<data<<" cannot be resolved into header & payload");
        return StatusCode::FAILURE;
    }
    const std::string& header = tokens[0];
    const std::string& payload = tokens[1];
    ATH_MSG_DEBUG("Header: '"<<header<<"'  payload: '"<<payload<<"'");
    /// Extract first the number of points and the Calib identifier
    unsigned int numPoints{0};
    nlohmann::json channel{};
    const bool rt_ts_applied = (attr["tech"].data<int>() & MuonCalib::TIME_SLEWING_CORRECTION_APPLIED);
    channel["appliedRT"] = rt_ts_applied;
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    {       

       std::vector<int> tokensHeader = tokenizeInt(header, ",");
       if(tokensHeader.size()< 2){
            ATH_MSG_FATAL("Failed to deduce extract number of points & calib Identifier from "<<header);
            return StatusCode::FAILURE;
       }
       unsigned int calibId = tokensHeader[0];
       numPoints = tokensHeader[1];
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
       channel["station"] = m_idHelperSvc->stationNameString(athenaId);
       channel["eta"] = m_idHelperSvc->stationEta(athenaId);
       channel["phi"] = m_idHelperSvc->stationPhi(athenaId);
       channel["ml"] =  idHelper.multilayer(athenaId);
       channel["layer"] = idHelper.tubeLayer(athenaId);
       channel["tube"] = idHelper.tube(athenaId);
    }
    /// Next convert the rt relations to 
    const std::vector<double> dataPoints = tokenizeDouble(payload, ",");
    std::vector<double> radii{}, times{}, resos{};
    radii.reserve(numPoints);
    times.reserve(numPoints);
    resos.reserve(numPoints);
    /// Another code beauty here. The legacy payload was structured such that a triplet of three numbers
    /// in the vector represents a single data point, the first one is the radius, the second one the time
    /// and the last one is the resolution
    for (unsigned int k = 0 ; k < dataPoints.size(); ++k) {
        const double value = dataPoints[k];
        switch (k%3) {
            case 0:
                radii.push_back(value);
                break;
            case 1:
                times.push_back(value);
                break;
            case 2:
                resos.push_back(value);
                break;
            default:
                break;
        }
    }
    /// Check that we picked up the right amount of points
    if (radii.size() != numPoints ||
        times.size() != numPoints ||
        resos.size() != numPoints) {
        ATH_MSG_FATAL("Payload "<<payload<<" does not lead to the expected number of points "<<numPoints<<" vs. "<<dataPoints.size());
        return StatusCode::FAILURE;
    }
    channel["radii"] = std::move(radii);
    channel["times"] = std::move(times);
    channel["resolutions"] = std::move(resos);
    json.push_back(channel);
    return StatusCode::SUCCESS;
}

StatusCode MdtCalibDbAlg::loadRt(const EventContext& ctx, MuonCalib::MdtCalibDataContainer& writeCdo) const {
    ATH_MSG_DEBUG("loadRt " << name());

    // Read Cond Handle
    SG::ReadCondHandle<CondAttrListCollection> readHandleRt{m_readKeyRt, ctx};
    if (!readHandleRt.isValid()) {
        ATH_MSG_ERROR("readCdoRt==nullptr");
        return StatusCode::FAILURE;
    }
    // read new-style format 2020

    nlohmann::json rtCalibJson = nlohmann::json::array();
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    if (m_newFormat2020) {
        for (CondAttrListCollection::const_iterator itr = readHandleRt->begin(); 
                                                    itr != readHandleRt->end(); ++itr) {
            const coral::AttributeList &atr = itr->second;
            std::string data{};
            if (atr["data"].specification().type() == typeid(coral::Blob)) {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
            } else {
                ATH_MSG_VERBOSE("Loading data as a STRING");
                data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
            }
            // unwrap the json and build the data vector
            nlohmann::json yy = nlohmann::json::parse(data);
            for (auto &it : yy.items()) {
                nlohmann::json yx = it.value();
                rtCalibJson.push_back(yx);
            }
        }
    }
    // read old-style format
    else {
        for (CondAttrListCollection::const_iterator itr = readHandleRt->begin(); 
             itr != readHandleRt->end(); ++itr) {
            ATH_CHECK(legacyRtPayloadToJSON(itr->second, rtCalibJson));
        }
    }
    /// List of loaded Rt relations to attach the proper corrections later
    LoadedRtMap loadedRtRel{};
    // unpack the strings in the collection and update the writeCdoRt
    for (const auto& payload : rtCalibJson) {
        const bool rt_ts_applied = payload["appliedRT"];
        /// Athena Identifier
        const std::string stName = payload["station"];
        const Identifier athenaId =  idHelper.channelID(stName, payload["eta"], payload["phi"],
                                                        payload["ml"], payload["layer"], payload["tube"]);

        std::optional<double> innerTubeRadius = getInnerTubeRadius(idHelper.multilayerID(athenaId, 1));
        if (!innerTubeRadius) continue;


        const std::vector<double> radii = payload["radii"];
        const std::vector<double> times = payload["times"];
        const std::vector<double> resolutions = payload["resolutions"];

        if (writeCdo.hasDataForChannel(athenaId, msgStream())) {
            const MdtFullCalibData* dataObj =  writeCdo.getCalibData(athenaId, msgStream());
            if (dataObj->rtRelation) {
                ATH_MSG_DEBUG("Rt relation constants for "<<m_idHelperSvc->toString(athenaId)<<" already exists");
                continue;
            }
        }

        MuonCalib::CalibFunc::ParVec rtPars{}, resoPars{};

        MuonCalib::SamplePoint tr_point, ts_point;  // pairs of numbers; tr = (time,radius); ts = (time,sigma)  [sigma=resolution]
        std::vector<MuonCalib::SamplePoint> tr_points{}, ts_points{};
        /// all the points in time,radius [RT] and time,sigma [resolution func]
        float multilayer_tmax_diff{-std::numeric_limits<float>::max()};

        // loop over RT function payload (triplets of radius,time,sigma(=resolution) )
        for (unsigned int k = 0; k < radii.size(); ++k) {
            float radius = radii[k];
            if (m_rtShift != 0.) {
                float oldradius = radius;
                // TODO: What is this magic number
                float rshift = m_rtShift * 1.87652e-2 * radius * (radius - *innerTubeRadius);
                radius = oldradius + rshift;
                ATH_MSG_DEBUG("DEFORM RT: old radius " << oldradius << " new radius " << radius << " shift " << rshift 
                            << " max shift " << m_rtShift);
            }

            if (m_rtScale != 1.) {
                float oldradius = radius;
                radius = radius * m_rtScale;
                ATH_MSG_DEBUG("DEFORM RT: old radius " << oldradius << " new radius " << radius << " scale factor " << m_rtScale);
            }
            tr_point.set_x2(radius);

            float time = times[k];
            tr_point.set_x1(time);
            ts_point.set_x1(time);

            float sigma = resolutions[k];
            ts_point.set_x2(sigma);
            ts_point.set_error(1.0);
            tr_point.set_error(1.0);
            if (tr_point.x2() < -99) {  // if radius is < -99 then treat time as ML Tmax difference
                multilayer_tmax_diff = tr_point.x1();
            } else if (tr_points.empty() || (tr_points.back().x1() < tr_point.x1() && tr_points.back().x2() < tr_point.x2())) {
                tr_points.push_back(tr_point);
                ts_points.push_back(ts_point);
            }
        }  // end loop over RT function payload (triplets of radius,time,resolution)

        /// Must have at least 3 points to have a valid RT
        if (ts_points.size() < 3) {
            ATH_MSG_FATAL("Rt relation broken!");
            return StatusCode::FAILURE;
        }

        if (rt_ts_applied != m_TimeSlewingCorrection) {
            float sign(rt_ts_applied ? -1.0 : 1.0);
            float slice_width = (*innerTubeRadius) / static_cast<float>(m_MeanCorrectionVsR.size());
            for (auto & tr_point : tr_points) {
                int slice_number = static_cast<int>(std::floor(tr_point.x2() / slice_width));
                if (slice_number < 0) slice_number = 0;
                if (slice_number >= static_cast<int>(m_MeanCorrectionVsR.size()))
                    slice_number = static_cast<int>(m_MeanCorrectionVsR.size()) - 1;
                tr_point.set_x1(tr_point.x1() + sign * m_MeanCorrectionVsR[slice_number]);
            }
        }

        // Create resolution function from ts_points
        std::unique_ptr<MuonCalib::IRtResolution> reso = getRtResolutionInterpolation(ts_points);
        if (msgLvl(MSG::VERBOSE)) {
            ATH_MSG_VERBOSE("Resolution points :");
            for (const MuonCalib::SamplePoint& point : tr_points) {
                ATH_MSG_VERBOSE(point.x1() << "|" << point.x2() << "|" << point.error());
            }
            ATH_MSG_DEBUG("Resolution parameters :");
            for (unsigned int i = 0; i < reso->nPar(); i++) { ATH_MSG_VERBOSE(i << " " << reso->par(i)); }
        }

        // Create RT function from tr_points and load RT and resolution functions
        std::unique_ptr<MuonCalib::IRtRelation> rt = MuonCalib::RtFromPoints::getRtRelationLookUp(tr_points);
        if (!reso || !rt) { continue; }

        if (rt->par(1) == 0.) {
            ATH_MSG_FATAL("Bin size is 0");
            for (const MuonCalib::SamplePoint& it: tr_points)
                ATH_MSG_WARNING(it.x1() << " " << it.x2() << " " << it.error());
            return StatusCode::FAILURE;
        }
        // Save ML difference if it is available
        if (multilayer_tmax_diff > -8e8) { rt->SetTmaxDiff(multilayer_tmax_diff); }
        // Store RT and resolution functions for this region
        RtRelationPtr rt_rel = std::make_shared<MuonCalib::MdtRtRelation>(std::move(rt), std::move(reso));

        if (!writeCdo.storeData(athenaId ,rt_rel, msgStream())) return StatusCode::FAILURE;
        if (!(m_create_b_field_function || m_createSlewingFunction)) continue;
        loadedRtRel[athenaId] = rt_rel;
        
    }  // end loop over itr (strings read from COOL)        
    ATH_CHECK(defaultRt(writeCdo, loadedRtRel));

    if (loadedRtRel.empty()) {
        return StatusCode::SUCCESS;
    }
   
    ATH_MSG_DEBUG("Initializing " << loadedRtRel.size()<< " b-field functions");
    const MdtCondDbData* condDbData{nullptr};
    if (!m_readKeyDCS.empty()) {
        SG::ReadCondHandle<MdtCondDbData> readCondHandleDb{m_readKeyDCS, ctx};
        /// The Mdt conditions data is only of value if it's also DCS constants
        if (readCondHandleDb->hasDCS()) {
            condDbData = readCondHandleDb.cptr();
        } else {
            ATH_MSG_INFO("Do not retrieve the HV from DCS. Fall back to 2730 & 3080");
        }
    }
  
  
    for (const auto& [athenaId, rtRelation] : loadedRtRel) {
        CorrectionPtr corrFuncSet = std::make_unique<MuonCalib::MdtCorFuncSet>();

        if (m_create_b_field_function) {
            std::vector<double> corr_params(2);
            bool loadDefault{false};
            if (condDbData){
                const MuonCond::DcsConstants& dcs{condDbData->getHvState(athenaId)};
                corr_params[0] = dcs.readyVolt;
                /// SKip everything that's switched off
                if (corr_params[0] < std::numeric_limits<float>::epsilon()) {
                    ATH_MSG_DEBUG("Chamber "<<m_idHelperSvc->toString(athenaId)<<" is switched off "<<dcs);
                    loadDefault = true;
                }
            } else loadDefault = true;
            if (loadDefault) {
                if (m_idHelperSvc->issMdt(athenaId)) {
                    corr_params[0] = 2730.0;
                } else {
                    corr_params[0] = 3080.0;  
                }
            }
            corr_params[1] = 0.11;    // epsilon parameter
            corrFuncSet->setBField(std::make_unique<MuonCalib::BFieldCorFunc>("medium", corr_params, rtRelation->rt()));
        }
        if (m_createSlewingFunction) {
            corrFuncSet->setSlewing(std::make_unique<MuonCalib::MdtSlewCorFuncHardcoded>(MuonCalib::CalibFunc::ParVec()));
        }
        if (!writeCdo.storeData(athenaId, corrFuncSet, msgStream())) return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
}

// build the transient structure and load some defaults for T0s
StatusCode MdtCalibDbAlg::defaultT0s(MuonCalib::MdtCalibDataContainer& writeCdo) const {
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};

    // loop over modules (MDT chambers) and create an MdtTubeContainer for each
    MdtIdHelper::const_id_iterator it = id_helper.module_begin();
    MdtIdHelper::const_id_iterator it_end = id_helper.module_end();
    for (; it != it_end; ++it) {
        
        if (writeCdo.hasDataForChannel(*it, msgStream())) {
            const MdtFullCalibData* dataObj =  writeCdo.getCalibData(*it, msgStream());
            if (dataObj->tubeCalib) {
                ATH_MSG_DEBUG("Rt relation constants for "<<m_idHelperSvc->toString(*it)<<" already exists");
                continue;
            }
        }
        // create an MdtTubeContainer
        TubeContainerPtr tubes = std::make_unique<MuonCalib::MdtTubeCalibContainer>(m_idHelperSvc.get(), *it);
        if (!writeCdo.storeData(*it, tubes, msgStream())) return StatusCode::FAILURE;
        
        // is tubes ever 0?  how could that happen?
        double t0 = m_defaultT0;

        unsigned int nml = tubes->numMultilayers();
        unsigned int nlayers = tubes->numLayers();
        unsigned int ntubes = tubes->numTubes();
        int size = nml * nlayers * ntubes;

        ATH_MSG_VERBOSE("Adding chamber " << m_idHelperSvc->toString(*it)
                      <<" size " << size << " ml " << nml << " l " << nlayers << " t " << ntubes);
        for (unsigned int ml = 1; ml <= nml; ++ml) {
            for (unsigned int l = 1; l <= nlayers; ++l) {
                for (unsigned int t = 1; t <= ntubes; ++t) {
                    MuonCalib::MdtTubeCalibContainer::SingleTubeCalib data{};
                    const Identifier tubeId = id_helper.channelID(*it, ml, l, t);
                    data.t0 = t0;
                    data.adcCal = 1.;                   
                    tubes->setCalib(std::move(data), tubeId, msgStream());
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode MdtCalibDbAlg::legacyTubePayloadToJSON(const coral::AttributeList& attr,nlohmann::json & json) const {
    std::string data{};
    if (attr["data"].specification().type() == typeid(coral::Blob)) {
        ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
        if (!CoralUtilities::readBlobAsString(attr["data"].data<coral::Blob>(), data)) {
            ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
            return StatusCode::FAILURE;
        }
        
    } else {
        data = *(static_cast<const std::string *>((attr["data"]).addressOfData()));
    }
    std::vector<std::string> tokens = tokenize(data, "\n");
    if (tokens.size() < 2) {
        ATH_MSG_FATAL("The line "<<data<<" cannot be resolved into header & payload");
        return StatusCode::FAILURE;
    }
    std::string& header = tokens[0];
    const std::string& payload = tokens[1];
    
    /// A  typical header looks like: T0BIL_1_-6,v0.0,1,288
    /// BIL is the station name, the numbers next do the underscores represent the station phi and the
    /// station eta. That's followed by a mysterious version number which was never incremented throughout time -- soo sad.
    /// The last two fields are the region number and the total number of tubes in the chamber.  
    const std::string stName = header.substr(2,3);
    int eta{0}, phi{0}, nTubes{0};
    {
        std::replace(header.begin(), header.end(),'_', ',');
        const std::vector<std::string> headerTokens = tokenize(header, ",");
        phi = atoi(headerTokens[1]);
        eta = atoi(headerTokens[2]);
        nTubes = atoi(headerTokens[5]);
    }
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    bool isValid{false};
    const Identifier chamID = idHelper.elementID(stName, eta, phi, isValid);
    if (!isValid) {
        static std::atomic<bool> idWarningPrinted = false;
        if (!idWarningPrinted) {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" Identifier: "<<stName<<","<<eta<<","<<phi
                            <<" is invalid. Skipping");
            idWarningPrinted.store(true, std::memory_order_relaxed);
        }
        return StatusCode::SUCCESS;
    }
    nlohmann::json channel{};
    const bool t0_ts_applied = (attr["tech"].data<int>() & MuonCalib::TIME_SLEWING_CORRECTION_APPLIED);
    channel["appliedT0"] = t0_ts_applied;
    channel["station"] = stName;
    channel["eta"] = eta;
    channel["phi"] = phi;

    const std::vector<double> payLoadData = tokenizeDouble(payload, ",");
    std::vector<double> tzeros{}, meanAdcs{};
    std::vector<int> statusCodes{};
    /// The payload comes along in triplets. The first element of the triplet represents the
    /// T0 shift of the single tube, the second one the status and the last one the mean Adc value
    for (unsigned int k = 0; k < payLoadData.size(); ++k){
        const double value = payLoadData[k];
        switch (k%3) {
            case 0:               
                tzeros.push_back(value);
                break;
            case 1:                
                statusCodes.push_back(value);
                break;
            case 2:                
                meanAdcs.push_back(value);
                break;
            default:
                break;
        }
    }
    if (statusCodes.size() != tzeros.size() ||
        statusCodes.size() != meanAdcs.size() ||
        statusCodes.empty()) {
        ATH_MSG_FATAL("Failed to properly readt t0 calibrations for chamber "<<m_idHelperSvc->toStringChamber(chamID));
        return StatusCode::FAILURE;
    }
    /// pack everything into a json array
    int ml{1}, layer{1}, tube{1};

    const int numMl = idHelper.numberOfMultilayers(chamID);
    const Identifier secondMlID = idHelper.multilayerID(chamID, numMl);
    const int tubesPerLay = std::max(idHelper.tubeMax(chamID), idHelper.tubeMax(secondMlID));
    const int numLayers = std::max(idHelper.tubeLayerMax(chamID), idHelper.tubeLayerMax(secondMlID));
    if (m_checkTubes &&  (numMl * numLayers * tubesPerLay) != nTubes) {
        ATH_MSG_FATAL("Calibration database differs in terms of number of tubes for chamber "
                     <<m_idHelperSvc->toStringChamber(chamID)<<". Expected "<<(numMl * numLayers * tubesPerLay)
                     <<" vs. observed "<<nTubes);
        return StatusCode::FAILURE;
    }
    nlohmann::json calibData = nlohmann::json::array();
    for (unsigned int k = 0; k < tzeros.size(); ++k) {
        nlohmann::json channelData{};
        channelData["ml"] = ml;
        channelData["layer"] =layer;
        channelData["tube"] = tube;
        channelData["t0"] = tzeros[k];
        channelData["meanAdc"] = meanAdcs[k];
        channelData["status"] = statusCodes[k];
        ++tube;
        if (tube > tubesPerLay){
            tube = 1;
            ++layer;
        }
        if (layer > numLayers){
            layer = 1;
            ++ml;
        }
        calibData.push_back(std::move(channelData));
    }
    channel["calibConstants"] = std::move(calibData);
    json.push_back(std::move(channel));
    return StatusCode::SUCCESS;
}
StatusCode MdtCalibDbAlg::loadTube(const EventContext& ctx, MuonCalib::MdtCalibDataContainer& writeCdo) const {
    ATH_MSG_DEBUG("loadTube " << name());
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};

    // Read Cond Handle
    SG::ReadCondHandle<CondAttrListCollection> readHandleTube{m_readKeyTube, ctx};
    // read new-style format 2020
    nlohmann::json t0CalibJson = nlohmann::json::array();
    if (m_newFormat2020) {
        for (CondAttrListCollection::const_iterator itr = readHandleTube->begin(); 
                                                    itr != readHandleTube->end(); ++itr) {
            const coral::AttributeList &atr = itr->second;
            std::string data{};
            if (atr["data"].specification().type() == typeid(coral::Blob)) {
                ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                    ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                    return StatusCode::FAILURE;
                }
            } else {
                ATH_MSG_VERBOSE("Loading data as a STRING");
                data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));
            }
            // unwrap the json and build the data vector
            nlohmann::json yy = nlohmann::json::parse(data);
            for (auto &it : yy.items()) {
                nlohmann::json yx = it.value();
                t0CalibJson.push_back(yx);
            }
        }
    }
    // read old-style format
    else {         
         for (CondAttrListCollection::const_iterator itr = readHandleTube->begin(); 
                    itr != readHandleTube->end(); ++itr) {
             ATH_CHECK(legacyTubePayloadToJSON(itr->second, t0CalibJson));
        }
    }

    // Inverse of wire propagation speed
    const float inversePropSpeed = 1. / (Gaudi::Units::c_light * m_prop_beta);
    writeCdo.setInversePropSpeed(inversePropSpeed);


    // unpack the strings in the collection and update the
    // MdtTubeCalibContainers in TDS
    for (const auto& chambChannel : t0CalibJson) {
        const std::string stName = chambChannel["station"];
        const int ieta = chambChannel["eta"];
        const int iphi = chambChannel["phi"];
        const bool t0_ts_applied = chambChannel["appliedT0"];
        // need to check validity of Identifier since database contains all Run 2 MDT chambers, e.g. also EI chambers which are
        // potentially replaced by NSW
        bool isValid{false};  // the elementID takes a bool pointer to check the validity of the Identifier
        const Identifier chId = idHelper.elementID(stName, ieta, iphi, isValid);
        if (!isValid) {
            static std::atomic<bool> idWarningPrinted = false;
            if (!idWarningPrinted) {
                ATH_MSG_WARNING("Element Identifier " << chId.get_compact() << " retrieved for station name " << stName
                                                      << " is not valid, skipping");
                idWarningPrinted.store(true, std::memory_order_relaxed);
            }
            continue;
        }

        if (writeCdo.hasDataForChannel(chId, msgStream())) {
            const MdtFullCalibData* dataObj =  writeCdo.getCalibData(chId, msgStream());
            if (dataObj->tubeCalib) {
                ATH_MSG_DEBUG("Rt relation constants for "<<m_idHelperSvc->toString(chId)<<" already exists");
                continue;
            }
        }
        
        TubeContainerPtr tubes = std::make_unique<MdtTubeCalibContainer>(m_idHelperSvc.get(), chId);
        if (!writeCdo.storeData(chId, tubes, msgStream())) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Failed to add chamber "<<m_idHelperSvc->toString(chId)
                        <<" ID fields: "<<stName<<","<<ieta<<","<<iphi);
            return StatusCode::FAILURE;
        }
        //const ref. No copy
        const nlohmann::json& tubeConstants = chambChannel["calibConstants"];
        for (const auto& tubeChannel : tubeConstants) {
            const int ml = tubeChannel["ml"]; 
            const int l = tubeChannel["layer"];
            const int t = tubeChannel["tube"];
            double tzero = tubeChannel["t0"];
            if (m_t0Shift != 0.) {
                tzero += m_t0Shift;
                ATH_MSG_VERBOSE("T0 shift " << m_t0Shift << " t0 " << tzero << " id " << ml << " " << l << " " << t);
            }
            if (m_t0Spread != 0.) {
                CLHEP::HepRandomEngine *engine = m_RNGWrapper->getEngine(ctx);
                double sh = CLHEP::RandGaussZiggurat::shoot(engine, 0., m_t0Spread);
                tzero += sh;
                ATH_MSG_VERBOSE("T0 spread " << sh << " t0 " << tzero << " id " << ml << " " << l << " " << t);
            }
            if (!t0_ts_applied && m_TimeSlewingCorrection) { tzero += m_TsCorrectionT0; }
            if (t0_ts_applied && !m_TimeSlewingCorrection) { tzero -= m_TsCorrectionT0; }
            
            const int statusCode = tubeChannel["status"];
            const double meanAdc = tubeChannel["meanAdc"];
            MuonCalib::MdtTubeCalibContainer::SingleTubeCalib datatube{};
            datatube.statusCode = statusCode;
            datatube.t0 = tzero;
            datatube.adcCal = meanAdc;
            const Identifier tubeId = idHelper.channelID(chId, ml, l, t);
            tubes->setCalib(std::move(datatube), tubeId, msgStream());
        }
    }  // end loop over readCdoTube

    ATH_CHECK(defaultT0s(writeCdo));
    // finally record writeCdo
    return StatusCode::SUCCESS;
}

std::unique_ptr<MuonCalib::RtResolutionLookUp> MdtCalibDbAlg::getRtResolutionInterpolation(const std::vector<MuonCalib::SamplePoint> &sample_points) {
    ///////////////
    // VARIABLES //
    ///////////////
    std::vector<Double_t> x(sample_points.size(),0);
    std::vector<Double_t> y(sample_points.size(),0);

    for (unsigned int i = 0; i < sample_points.size(); i++) {
        x[i] = sample_points[i].x1();
        y[i] = sample_points[i].x2();
    }
    TSpline3 sp("Rt Res Tmp", x.data(), y.data(), sample_points.size());
    ///////////////////////////////////////////////////////////////////
    // CREATE AN RtRelationLookUp OBJECT WITH THE CORRECT PARAMETERS //
    ///////////////////////////////////////////////////////////////////
    unsigned int nb_points(100);
    std::vector<double> res_param(nb_points + 2);  // r-t parameters
    Double_t bin_width = (x[sample_points.size() - 1] - x[0]) / static_cast<Double_t>(nb_points);

    res_param[0] = x[0];
    res_param[1] = bin_width;
    for (unsigned int k = 0; k < nb_points; k++) {
      Double_t xx = x[0] + k * bin_width;
      res_param[k + 2] = sp.Eval(xx);
      if (std::isnan(res_param[k + 2])) {
        TFile outf("kacke.root", "RECREATE");
        sp.Write("kacke");
        THROW_EXCEPTION("MdtCalibDbAlg::getRtResolutionInterpolation "
                                 "encountered nan element");
      }
    }
    return std::make_unique<MuonCalib::RtResolutionLookUp>(std::move(res_param));
}
