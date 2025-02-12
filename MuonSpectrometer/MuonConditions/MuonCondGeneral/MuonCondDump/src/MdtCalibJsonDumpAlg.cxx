/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCalibJsonDumpAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonIdHelpers/IdentifierByDetElSorter.h"
#include "Acts/Utilities/Enumerate.hpp"
#include <fstream>
#include <ranges>
#include <format>
#include <CxxUtils/StringUtils.h>

using namespace MuonCalib;

namespace {
    std::string whiteSpace(int n, const char space=' ') {
        std::string spaces{};
        spaces.assign(n, space);
        return spaces;
    }
    std::string rangeString(std::vector<ushort>& numbers){
        std::ranges::sort(numbers);

        std::stringstream sstr{};
        ushort low{numbers.front()}, high{numbers.front()};
        for (const ushort n : numbers) {
            if (high == n || high+1 == n) {
                high = n;
                continue;
            }
            /// <-
            if (low == high) {
                sstr<<low;
            } else if (low + 1!=high){
                sstr<<low<<"-"<<high;
            } else {
                sstr<<low<<";"<<high;
            }
            sstr<<";";
            low = high = n;
        }
        /// Single number at the end
        if (low == numbers.back()) {
            sstr<<low;
        } else if (low + 1 != high) {
            sstr<<low<<"-"<<high;
        } else {
            sstr<<low<<";"<<high;

        }
        /// 
        return sstr.str();
    }
}


namespace Muon {
    MdtCalibJsonDumpAlg::~MdtCalibJsonDumpAlg() = default;
    StatusCode MdtCalibJsonDumpAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        if (m_rtJSON.value().empty() || m_t0JSON.value().empty()) {
            ATH_MSG_FATAL("No output file has been given " << m_rtJSON<<" "<<m_t0JSON);
            return StatusCode::FAILURE;
        }
        if (m_t0JSON.value() == m_rtJSON.value()){
            ATH_MSG_FATAL("You can't dump the two streams into the same JSON "<<m_t0JSON<<" & "<<m_rtJSON);
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_rtDumpTree.init(this));
        ATH_CHECK(m_t0DumpTree.init(this));
        ATH_MSG_INFO("Dump calibration constants in " << m_rtJSON);
        return StatusCode::SUCCESS;
    }
    StatusCode MdtCalibJsonDumpAlg::finalize() {
        ATH_CHECK(m_rtDumpTree.write());
        ATH_CHECK(m_t0DumpTree.write());
        return StatusCode::SUCCESS;
    }
    StatusCode MdtCalibJsonDumpAlg::execute() {
        const EventContext &ctx{Gaudi::Hive::currentContext()};
        SG::ReadCondHandle calibHandle{m_readKey, ctx};
        ATH_CHECK(calibHandle.isValid());
        
        if (std::ranges::find(m_seenIDs, calibHandle.getRange()) != m_seenIDs.end()) {
            ATH_MSG_INFO("IOV  "<<calibHandle.getRange()<<" has already been dumped");
            return StatusCode::SUCCESS;
        }
        m_seenIDs.emplace_back(calibHandle.getRange());
        const MdtIdHelper &idHelper{m_idHelperSvc->mdtIdHelper()};
       
        T0Grouper t0Groups{};
        const CalibParamSorter sorter{std::pow(0.1, m_precision +2)}; 
        RtGrouper rtGroups{sorter};
        unsigned int rtCounter{0}, t0Counter{0};
        for (auto itr = idHelper.detectorElement_begin(); itr != idHelper.detectorElement_end(); ++itr) {
            const Identifier &detId{*itr};
            const MdtFullCalibData *calibData = calibHandle->getCalibData(detId, msgStream());
            if (!calibData || !calibData->rtRelation) {
                continue;
            }
            ++rtCounter;
            ATH_MSG_VERBOSE("Group rt constants of "<<m_idHelperSvc->toStringDetEl(detId));
            rtGroups[calibData->rtRelation.get()].insert(detId);
            const int ml = idHelper.multilayer(detId);
            
            T0PerChamb& chambT0{t0Groups.insert(std::make_pair(idHelper.elementID(detId), T0PerChamb{sorter})).first->second};

            for (int layer = 1; layer <= idHelper.tubeLayerMax(detId); ++layer) {
                for (int tube = 1; tube <= idHelper.tubeMax(detId); ++tube) {
                    bool valid{false};
                    const Identifier chId{idHelper.channelID(detId, ml, layer, tube, valid)};
                    if (!valid) {
                        continue;
                    }
                    const MuonCalib::MdtTubeCalibContainer::SingleTubeCalib* singleCalib = calibData->tubeCalib->getCalib(chId);
                    if (!singleCalib) {
                        continue;
                    }
                    chambT0[singleCalib].insert(chId);
                    ++t0Counter;
                }
            }
        }
        ATH_CHECK(dumpRtRelations(ctx, rtGroups, calibHandle.getRange()));
        ATH_CHECK(dumpTubeT0s(ctx, t0Groups, calibHandle.getRange()));
      
        unsigned t0GrpCounter{0};
        for (const auto&[chId, t0Channels] : t0Groups) {
            t0GrpCounter+=t0Channels.size();
        }
        if ((rtCounter == 0) or (t0Counter == 0)){
          ATH_MSG_ERROR("MdtCalibJsonDumpAlg::execute: Counter is zero in numerator");
          return StatusCode::FAILURE;
        }
        ATH_MSG_INFO(std::format("Grouped {:d} / {:d} ({:.2f}%) rt relations &  {:d}/ {:d}  ({:.2f}%) t0 calibration constants",
                        rtGroups.size(), rtCounter, (100.* rtGroups.size() / rtCounter), 
                        t0GrpCounter, t0Counter,(100.*t0GrpCounter / t0Counter)));
        return StatusCode::SUCCESS;
    }
    StatusCode MdtCalibJsonDumpAlg::dumpRtRelations(const EventContext& ctx, const RtGrouper& rtRelMap, const EventIDRange& eventRange) {
        std::ofstream outFileRT{toString(eventRange) + m_rtJSON};
        if (!outFileRT.good()) {
            ATH_MSG_FATAL("Failed to write "<<m_rtJSON);
            return StatusCode::FAILURE;
        }
        outFileRT<<"[\n";
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        nlohmann::json rtDump{};
        for (const auto& [counter, payload] : Acts::enumerate(rtRelMap)) {
            outFileRT<<whiteSpace(m_whiteSpace)<<"{\n";
            outFileRT<<whiteSpace(m_whiteSpace)<<" \"chambers\": [\n";
            const MdtRtRelation* rtRel = payload.first;
            const std::set<Identifier>& chambers = payload.second;

            for (const auto& [chCounter, chambId] : Acts::enumerate(chambers)) {
                outFileRT<<whiteSpace(2*m_whiteSpace)<<"{"
                         <<std::format("\"station\": \"{:}\", \"eta\": {:2d}, \"phi\": {:1d}, \"ml\": {:1d}",
                            idHelper.stationNameString(idHelper.stationName(chambId)), idHelper.stationEta(chambId), 
                            idHelper.stationPhi(chambId), idHelper.multilayer(chambId))<<"}";

                m_rt_stName.push_back(idHelper.stationNameString(idHelper.stationName(chambId)));
                m_rt_stEta.push_back(idHelper.stationEta(chambId));
                m_rt_stPhi.push_back(idHelper.stationPhi(chambId));
                m_rt_stMl.push_back(idHelper.multilayer(chambId));
                
                if (chCounter +1 != chambers.size()) {
                    outFileRT<<",\n";
                } else {
                    outFileRT<<"],\n";
                }
            }
            /// write the RT relation
            outFileRT<<whiteSpace(m_whiteSpace+1)<<"\"rtRelation\": {\n";            
            outFileRT<<whiteSpace(2*m_whiteSpace)<<"\"type\": \""<<rtRel->rt()->name()<<"\",\n";
            m_rt_type = rtRel->rt()->name();
            m_rt_pars = rtRel->rt()->parameters();
            outFileRT<<whiteSpace(2*m_whiteSpace)<<"\"params\": "<<dump(rtRel->rt()->parameters())<<"},\n";
            /// write the TR relation
            if (rtRel->tr()->name() != "TrRelationLookUp") {
                outFileRT<<whiteSpace(m_whiteSpace+1)<<"\"trRelation\": {\n";            
                outFileRT<<whiteSpace(2*m_whiteSpace)<<"\"type\": \""<<rtRel->tr()->name()<<"\",\n";
                outFileRT<<whiteSpace(2*m_whiteSpace)<<"\"params\": "<<dump(rtRel->tr()->parameters())<<"},\n";
                m_tr_type = rtRel->tr()->name();
                m_tr_pars = rtRel->tr()->parameters();
            }
            /// Write the resolution
            outFileRT<<whiteSpace(m_whiteSpace+1)<<"\"rtReso\": {\n";
            outFileRT<<whiteSpace(2*m_whiteSpace)<<"\"type\": \""<<rtRel->rtRes()->name()<<"\",\n";
            outFileRT<<whiteSpace(2*m_whiteSpace)<<"\"params\": "<<dump(rtRel->rtRes()->parameters())<<"}\n";
            m_rt_resoType = rtRel->rtRes()->name();
            m_rt_resoPars = rtRel->rtRes()->parameters();
            if (counter +1 != rtRelMap.size()) {
                outFileRT<<whiteSpace(m_whiteSpace)<<"},\n";
            } else {
                outFileRT<<whiteSpace(m_whiteSpace)<<"}\n";
            }
            m_rt_iov_start = eventRange.start();
            m_rt_iov_end = eventRange.stop();
            ATH_CHECK(m_rtDumpTree.fill(ctx));
        }
        outFileRT<<"]\n";       
        return StatusCode::SUCCESS;
    }
    std::string MdtCalibJsonDumpAlg::dump(const std::vector<double>& values) const{
        std::stringstream sstr{};
        sstr<<"[";
        for (const auto& [count, v] : Acts::enumerate(values)) {
            sstr<<dump(v);
            if (count + 1 != values.size())sstr<<", ";
        }
        sstr<<"]";
        return sstr.str();
    }
    std::string MdtCalibJsonDumpAlg::dump(const double v) const{
        std::stringstream sstr{};
        if (std::abs(v)< std::pow(0.1, m_precision + 1)) {
            sstr<<0;
        } else {
            sstr<<std::setprecision(m_precision + std::max(std::floor(std::log10(std::abs(v))), 1.))<<v;
        }
        return sstr.str();
    }
    StatusCode MdtCalibJsonDumpAlg::dumpTubeT0s(const EventContext& ctx, const T0Grouper& t0Map, const EventIDRange& eventRange) {
        std::ofstream outFileT0{toString(eventRange) + m_t0JSON};
        if (!outFileT0.good()) {
            ATH_MSG_FATAL("Failed to write "<<m_t0JSON);
            return StatusCode::FAILURE;
        }
        outFileT0<<"[\n";
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        for (const auto& [count, payload] : Acts::enumerate(t0Map)) {
            outFileT0<<whiteSpace(m_whiteSpace)<<"{\n";
            const Identifier& detId = payload.first;
            const T0PerChamb& calibConstants = payload.second;
            
            m_t0_stName = idHelper.stationNameString(idHelper.stationName(detId));
            m_t0_stEta = idHelper.stationEta(detId);
            m_t0_stPhi = idHelper.stationPhi(detId);

            outFileT0<<whiteSpace(m_whiteSpace+1)
                     <<std::format("\"station\": \"{:}\", \"eta\": {:2d}, \"phi\": {:1d}, \"calibConstants\":[\n",
                                  idHelper.stationNameString(idHelper.stationName(detId)), 
                                  idHelper.stationEta(detId), idHelper.stationPhi(detId));
            for (const auto& [calibCounter, calibPayload] : Acts::enumerate(calibConstants)) {
                outFileT0<<whiteSpace(2*m_whiteSpace)<<"{\"t0\": "<<dump(calibPayload.first->t0)<<", ";
                outFileT0<<"\"adc\": "<<dump(calibPayload.first->adcCal)<<", ";
                outFileT0<<"\"code\": "<<dump(calibPayload.first->statusCode)<<", ";
                outFileT0<<"\"tubes\": [";
                Identifier refId = (*calibPayload.second.begin());

                m_t0_t0= calibPayload.first->t0;
                m_t0_adc = calibPayload.first->adcCal;
                m_t0_code = calibPayload.first->statusCode;
                std::vector<u_short> tubes{};
                auto dumpTubeLayer = [&tubes, &outFileT0, &idHelper, &refId](const Identifier& newLayId) {
                    outFileT0<<std::format("{{\"ml\": {:1d}, \"tl\": {:1d}, \"no\": \"{:}\"}}",
                                            idHelper.multilayer(refId), idHelper.tubeLayer(refId),
                                            rangeString(tubes));
                    tubes.clear();
                    refId = newLayId;
                };
                for (const Identifier& tubeId : calibPayload.second){                    
                    const int ml = idHelper.multilayer(tubeId);
                    const int tl = idHelper.tubeLayer(tubeId);
                    m_t0_multiLayer.push_back(ml);
                    m_t0_tubeLayer.push_back(tl);                    
                    m_t0_tube.push_back(idHelper.tube(tubeId));
                    if (tl != idHelper.tubeLayer(refId) || ml != idHelper.multilayer(refId)) {
                        dumpTubeLayer(tubeId);
                        outFileT0<<", ";
                    } 
                    tubes.push_back(idHelper.tube(tubeId));
                }
                dumpTubeLayer(refId);
                outFileT0<<"]";
                if (calibCounter +1 != calibConstants.size()){
                    outFileT0<<"},\n";
                } else {
                    outFileT0<<"}]"<<std::endl;
                }
            }
            
            if (count +1 != t0Map.size()){
                outFileT0<<whiteSpace(m_whiteSpace)<<"},\n";
            } else {
                outFileT0<<whiteSpace(m_whiteSpace)<<"}"<<std::endl;
            }
            m_t0_iov_start = eventRange.start();
            m_t0_iov_end = eventRange.stop();
            ATH_CHECK(m_t0DumpTree.fill(ctx));
        }
        outFileT0<<"]\n";
        return StatusCode::SUCCESS;
    }
    std::string MdtCalibJsonDumpAlg::toString(const EventIDRange& range) const {
        if (m_savePerIOV){
           std::stringstream sstr{};
           sstr<<range;
           std::string outStr{sstr.str()};
           outStr.erase(std::remove_if(outStr.begin(),outStr.end(),
                  [](const unsigned char c){
                    return !std::isalnum(c) && c!='-' && c !='_';
                   }), outStr.end());
           return outStr+"_";
        }
        return {};
    }
}