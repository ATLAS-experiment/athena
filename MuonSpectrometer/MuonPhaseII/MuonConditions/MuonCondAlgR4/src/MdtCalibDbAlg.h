/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
   MdtCalibDbAlgR4 reads raw condition data and writes derived condition data to the condition store
*/

#ifndef MUONCALIBR4_MDTCALIBDBALGR4_H
#define MUONCALIBR4_MDTCALIBDBALGR4_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include <nlohmann/json.hpp>


class TTree;

namespace MuonCalibR4 {
    class MdtCalibDbAlg : public AthReentrantAlgorithm {
        public:
            
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~MdtCalibDbAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
            virtual bool isReEntrant() const override { return false; }

        private:

            using CorrectionPtr = MuonCalib::MdtCalibDataContainer::CorrectionPtr;
            using RtRelationPtr = MuonCalib::MdtCalibDataContainer::RtRelationPtr;
            using TubeContainerPtr = MuonCalib::MdtCalibDataContainer::TubeContainerPtr;

            /***   @brief Translates a r-t JSON payload into Mdt calibration constants
               *   @param rtBlob: JSON payload to translated
               *   @param outContainer: Container into which the Rt relations are appended */
            StatusCode parseRtPayload(const nlohmann::json& rtBlob,
                                      MuonCalib::MdtCalibDataContainer& outContainer) const;

            /** @brief Reads a r-t ROOT tree and stores its content in the calibration constant container
             *  @param rtTree: TTree to read the data from
             *  @param outContainer: Output container append the Rt relations */
            StatusCode parseRtPayload(TTree& rtTree,
                                      MuonCalib::MdtCalibDataContainer& outContainer) const;
            /** @brief Creates a new rt function from the typeName & the list of parameters
             *  @param rtType: Rt-relation type to create
             *  @param pars: Parameter vector to parse to the Rt-relation */
            MuonCalib::IRtRelationPtr makeRt(const std::string& rtType,
                                             const std::vector<double>& pars) const;
            /** @brief Creates a new tr function from the typeName & the list of parameters
             *  @param trType: Tr-relation type to create
             *  @param pars: Parameter vector to parse to the Tr-relation */
            MuonCalib::ITrRelationPtr makeTr(const std::string& trType,
                                             const std::vector<double>& pars) const;
            /** @brief Creates a new resoltuion function from the typeName & the list of parameters
             *  @param trType: Tr-relation type to create
             *  @param pars: Parameter vector to parse to the Tr-relation
             *  @param rt: Point to the rt relation created before. */
            MuonCalib::IRtResolutionPtr makeReso(const std::string& resoType,
                                                 const std::vector<double>& pars,
                                                  MuonCalib::IRtRelationPtr rt) const;
            /** @brief Trnaslates a t0 - JSON payload into transient memory
             *  @param t0Blob: JSON payload to be translated
             *  @param outContainer: Container into which the t0 constants are stored */
            StatusCode parseT0Payload(const nlohmann::json& t0Blob,
                                      MuonCalib::MdtCalibDataContainer& outContainer) const;

            /** @brief Translates a T0 TTree payload into transient memory 
                @param t0Tree: Reference to the TTree to translate 
                @param outContainer: Container into which the t0 constants are stored */
            StatusCode parseT0Payload(TTree& t0Tree,
                                      MuonCalib::MdtCalibDataContainer& outContainer) const;


            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            SG::WriteCondHandleKey<MuonCalib::MdtCalibDataContainer> m_writeKey{this, "WriteKey",  "MdtCalibConstants",
                                                                                            "Conditions object containing the calibrations"};   
                                                    
            /** @brief External Rt & T0 JSON files */            
            Gaudi::Property<std::string> m_rtJSON{this, "RtJSON", ""};
            Gaudi::Property<std::string> m_t0JSON{this, "TubeT0JSON", ""};

            Gaudi::Property<std::string> m_rtRootFile{this, "RtROOT", "", 
                                                      "Path to an external root file to read the Rt relation from"};
            Gaudi::Property<std::string> m_t0RootFile{this, "TubeT0ROOT", "", 
                                                      "Path to an external root file to read the T0 constants from"};

            Gaudi::Property<std::string> m_rtTreeName{this, "RtTreeName", "RtCalibConstants"};
            Gaudi::Property<std::string> m_t0TreeName{this, "T0TreeName", "T0CalibConstants"};

            SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyRt{this, "ReadKeyRt", "/MDT/RTJSON", "DB folder containing the RT calibrations"};
            SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyTube{this, "ReadKeyTube", "/MDT/T0JSON", "DB folder containing the tube constants"};
            Gaudi::Property<std::string> m_dbPayloadType {this, "dbPayloadType","TTree", "specify the format of the payload in the database. Can be TTree or JSON"};

            Gaudi::Property<double> m_prop_beta{this, "PropagationSpeedBeta", 1., "Speed of the signal propagation"};
            /** @brief Precision cut off to treat 2 t0 constants as equivalent */
            Gaudi::Property<unsigned> m_t0CalibPrec{this, "T0CalibPrecision", 4};

            Gaudi::Property<bool> m_create_b_field_function{this, "CreateBFieldFunctions", false,
                "If set to true, the B-field correction functions are initialized for each rt-relation that is loaded."};

            Gaudi::Property<bool> m_createSlewingFunction{this, "CreateSlewingFunctions", false,
                "If set to true, the slewing correction functions are initialized for each rt-relation that is loaded."};


    };
}
#endif
