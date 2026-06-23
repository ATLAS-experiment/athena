/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexRoiByteStreamTool  -  This tool decodes Run3 jFEX RoI data!
//                              -------------------
//     begin                : 01 01 2022
//     email                : Sergi.Rodriguez@cern.ch
//  ***************************************************************************/

#ifndef JFEXROIBYTESTREAMTOOL_H 
#define JFEXROIBYTESTREAMTOOL_H

// Trigger includes
#include "TrigT1ResultByteStream/IL1TriggerByteStreamTool.h"

#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIAuxContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexMETRoIAuxContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIAuxContainer.h"

#include "TrigConfData/L1Menu.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "PathResolver/PathResolver.h"

#include "AthenaMonitoringKernel/Monitored.h"

// Gaudi includes
#include "Gaudi/Property.h"

/** @class jFEXRoIByteStreamTool
 *  @brief Implementation of a tool for L1 RoI conversion from BS to xAOD and from xAOD to BS
 *  (IL1TriggerByteStreamTool interface)
 **/
class jFexRoiByteStreamTool : public extends<AthAlgTool, IL1TriggerByteStreamTool> {
    public:
        jFexRoiByteStreamTool(const std::string& type, const std::string& name, const IInterface* parent);
        virtual ~jFexRoiByteStreamTool() override = default;

        // ------------------------- IAlgTool methods --------------------------------
        virtual StatusCode initialize() override;
        virtual StatusCode start() override;

        // ------------------------- IL1TriggerByteStreamTool methods ----------------------
        /// BS->xAOD conversion
        virtual StatusCode convertFromBS(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vrobf, const EventContext& eventContext)const override;

        /// xAOD->BS conversion
        virtual StatusCode convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf,
                                       const xAOD::TrigCompositeContainer* tc,
                                       const EventContext& eventContext) override;

        /// Declare ROB IDs for conversion
        virtual const std::vector<uint32_t>& robIds() const override {
            return m_robIds.value();
        }

    private:
        // ------------------------- Properties --------------------------------------
        ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring tool"};
        bool m_UseMonitoring = false;        
        
        
        
        // ROBIDs property required by the interface
        Gaudi::Property<std::vector<uint32_t>> m_robIds {this, "ROBIDs", {}, "List of ROB IDs required for conversion to/from xAOD RoI"};
        Gaudi::Property<bool> m_convertExtendedTOBs {this, "ConvertExtendedTOBs", false, "Convert xTOBs instead of TOBs"};
        
        Gaudi::Property<std::string> m_TobMapping {this, "jFexTobMapping", "L1CaloFEXByteStream/2022-10-19/jFexTobMap.txt", "Text file to convert from hardware internal coordinates to eta-phi location"};
        
        //Read handle key for the L1Menu
        SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey   {this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"};


        // Write handle keys for the L1Calo EDMs for BS->xAOD mode of operation
        SG::WriteHandleKey< xAOD::jFexSRJetRoIContainer> m_jJWriteKey   {this,"jJRoIContainerWriteKey"  ,"L1_jFexSRJetRoI","Write jFexEDM SRjet container"};
        SG::WriteHandleKey< xAOD::jFexLRJetRoIContainer> m_jLJWriteKey  {this,"jLJRoIContainerWriteKey" ,"L1_jFexLRJetRoI","Write jFexEDM LRjet container"};
        SG::WriteHandleKey< xAOD::jFexTauRoIContainer  > m_jTauWriteKey {this,"jTauRoIContainerWriteKey","L1_jFexTauRoI"  ,"Write jFexEDM tau container"};
        SG::WriteHandleKey< xAOD::jFexSumETRoIContainer> m_jTEWriteKey  {this,"jTERoIContainerWriteKey" ,"L1_jFexSumETRoI","Write jFexEDM SumET container"};
        SG::WriteHandleKey< xAOD::jFexMETRoIContainer  > m_jXEWriteKey  {this,"jXERoIContainerWriteKey" ,"L1_jFexMETRoI"  ,"Write jFexEDM Met container"};
        SG::WriteHandleKey< xAOD::jFexFwdElRoIContainer> m_jEMWriteKey  {this,"jEMRoIContainerWriteKey" ,"L1_jFexFwdElRoI","Write jFexEDM fwdEl container"};
        
        // Decoder-side trailer parsers (BS word -> fields)
        std::array<uint32_t,6> TOBCounterTrailer (uint32_t word) const;
        std::array<uint32_t,4> xTOBCounterTrailer(uint32_t word) const;
        std::array<uint32_t,4> jFEXtoRODTrailer  (uint32_t word0, uint32_t word1) const;
        std::array<uint32_t,1> RODTrailer  (uint32_t word0, uint32_t word1) const;
        void     jFEXtoRODHeader   (uint32_t word0, uint32_t word1) const;

        // Encoder-side trailer builders (fields -> BS word)
        uint32_t buildTOBCounterTrailer (uint32_t n_jJ, uint32_t n_jLJ, uint32_t n_jTau,
                                         uint32_t n_jEM, uint32_t has_jTE, uint32_t has_jXE) const;
        uint32_t buildxTOBCounterTrailer(uint32_t n_xjJ, uint32_t n_xjLJ,
                                         uint32_t n_xjTau, uint32_t n_xjEM) const;
        std::array<uint32_t,2> buildjFEXtoRODTrailer(uint32_t payload, uint32_t fpga,
                                                    uint32_t jfex) const;
        
        
        //unpacking internal coordinates
        
        std::array<float,2> getEtaPhi  (unsigned int jfex, unsigned int fpga, uint32_t word, const char* obj) const;
        std::array<uint8_t,2> unpackLocalCoords  (uint32_t word) const;
        static const int s_etaBit   = 5;
        static const int s_phiBit   = 1;
        static const int s_etaMask  = 0x1f;
        static const int s_phiMask  = 0xf;
        
        int m_jTauRes = 0;
        int m_jJRes   = 0;
        int m_jLJRes  = 0;
        int m_jEMRes  = 0;
        int m_jXERes  = 0;
        int m_jTERes  = 0;

        
                
        // Read Mapping file to link internal word eta/phi to float Eta/Phi coordinates
        StatusCode ReadfromFile(const std::string&);
        
        // hash the index into one integer in the format 0xJFCCT (hexadecimal)
        constexpr static unsigned int mapIndex(unsigned int jfex, unsigned int fpga, unsigned int iEta, unsigned int iPhi);
        std::unordered_map<unsigned int, std::array<float,2> > m_TobPos_map; /// {map index, {eta,phi}}
        
        void printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const;
        
};

#endif // JFEXROIBYTESTREAMTOOL_H
