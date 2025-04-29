/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/CalibParamSorter.h"

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/EventIDBranch.h"

#include "nlohmann/json.hpp"

namespace Muon{

    class MdtCalibJsonDumpAlg : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;

            virtual ~MdtCalibJsonDumpAlg();  

            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
            virtual StatusCode finalize() override final;
        private:
            using CalibParamSorter = MuonCalib::CalibParamSorter;
            using SingleTubeCalib = CalibParamSorter::SingleTubeCalib;
            using RtGrouper = std::map<const MuonCalib::MdtRtRelation*, std::set<Identifier>, CalibParamSorter>;
            using T0PerChamb =  std::map<const SingleTubeCalib*, std::set<Identifier>, CalibParamSorter>;
            using T0Grouper = std::map<Identifier, T0PerChamb>;
            
            std::string dump(const std::vector<double>& values) const;
            std::string dump(const double v) const;
            std::string toString(const EventIDRange& range) const;

            /** @brief writes the RT relations into the JSON & output root file  */
            StatusCode dumpRtRelations(const EventContext& ctx, const RtGrouper& rtRelMap, const EventIDRange& eventRange);

            StatusCode dumpTubeT0s(const EventContext& ctx,
                                   const T0Grouper& t0Map, const EventIDRange& eventRange);

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            SG::ReadCondHandleKey<MuonCalib::MdtCalibDataContainer> m_readKey{this, "CalibKey",  "MdtCalibConstants",
                                                                              "Conditions object containing the calibrations"};   

            Gaudi::Property<std::string> m_rtJSON{this, "RtJSON", "RtRelations.json"};
            Gaudi::Property<std::string> m_t0JSON{this, "TubeT0JSON", "TubeT0.json"};
            Gaudi::Property<bool> m_savePerIOV{this, "JsonPerIOV", true};

            Gaudi::Property<unsigned> m_whiteSpace{this, "WhiteSpaces", 2};
            Gaudi::Property<unsigned> m_precision{this, "FloatPrecision", 4};


            std::vector<EventIDRange> m_seenIDs{};

            MuonVal::MuonTesterTree m_rtDumpTree{"RtCalibConstants", "MDTRTCALIBDUMP"};
            /** @brief string defining the type of the rt function */
            MuonVal::ScalarBranch<std::string>& m_rt_type{m_rtDumpTree.newScalar<std::string>("rtType")};
            /** @brief parameters of the rt-function */
            MuonVal::VectorBranch<double>& m_rt_pars{m_rtDumpTree.newVector<double>("rtParams")};
            /** @brief String defining the type of the tr function. Empty if it's a lookup table */
            MuonVal::ScalarBranch<std::string>& m_tr_type{m_rtDumpTree.newScalar<std::string>("trType", "")};
            /** @brief parameters of the tr-function */
            MuonVal::VectorBranch<double>& m_tr_pars{m_rtDumpTree.newVector<double>("trParams")};
            /** @brief string defining the type of the rt resolution function */
            MuonVal::ScalarBranch<std::string>& m_rt_resoType{m_rtDumpTree.newScalar<std::string>("resoType")};
            /** @brief parameters of the rt resolution function */
            MuonVal::VectorBranch<double>& m_rt_resoPars{m_rtDumpTree.newVector<double>("resoParams")};
            /** @brief Station name string of the associated chambers */
            MuonVal::VectorBranch<std::string>& m_rt_stName{m_rtDumpTree.newVector<std::string>("stationName")};
            /** @brief Station eta of the associated chambers */
            MuonVal::VectorBranch<short>& m_rt_stEta{m_rtDumpTree.newVector<short>("stationEta")};
            /** @brief Station phi of the associated chamber */
            MuonVal::VectorBranch<unsigned short>& m_rt_stPhi{m_rtDumpTree.newVector<unsigned short>("stationPhi")};
            /** @brief Multi layer of the associated chamber */
            MuonVal::VectorBranch<unsigned short>& m_rt_stMl{m_rtDumpTree.newVector<unsigned short>("multiLayer")};
            /** @brief Data mebmers to save the IOV range (Start point) */
            MuonVal::EventIDBranch m_rt_iov_start{m_rtDumpTree, "startIOV"};
            /** @brief Data mebmers to save the IOV range (End point) */
            MuonVal::EventIDBranch m_rt_iov_end{m_rtDumpTree, "endIOV"};

            MuonVal::MuonTesterTree m_t0DumpTree{"T0CalibConstants", "MDTT0CALIBDUMP"};
            MuonVal::ScalarBranch<std::string>& m_t0_stName{m_t0DumpTree.newScalar<std::string>("stationName")};
            MuonVal::ScalarBranch<short>& m_t0_stEta{m_t0DumpTree.newScalar<short>("stationEta")};
            MuonVal::ScalarBranch<unsigned short>& m_t0_stPhi{m_t0DumpTree.newScalar<unsigned short>("stationPhi")};

            MuonVal::ScalarBranch<float>& m_t0_t0{m_t0DumpTree.newScalar<float>("t0")};
            MuonVal::ScalarBranch<float>& m_t0_adc{m_t0DumpTree.newScalar<float>("adc")};
            MuonVal::ScalarBranch<unsigned short>& m_t0_code{m_t0DumpTree.newScalar<unsigned short>("code")};

            MuonVal::VectorBranch<unsigned short>& m_t0_multiLayer{m_t0DumpTree.newVector<unsigned short>("multiLayer")};
            MuonVal::VectorBranch<unsigned short>& m_t0_tubeLayer{m_t0DumpTree.newVector<unsigned short>("tubeLayer")};
            MuonVal::VectorBranch<unsigned short>& m_t0_tube{m_t0DumpTree.newVector<unsigned short>("tube")};
            /** @brief Data mebmers to save the IOV range (Start point) */
            MuonVal::EventIDBranch m_t0_iov_start{m_t0DumpTree, "startIOV"};
            /** @brief Data mebmers to save the IOV range (End point) */
            MuonVal::EventIDBranch m_t0_iov_end{m_t0DumpTree, "endIOV"};


    };
}