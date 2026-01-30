/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexByteStreamTool  -  description
//                              -------------------
//     begin                : 20 07 2022
//     email                : cecilia.tosciri@cern.ch
//  ***************************************************************************/

#ifndef GFEXBYTESTREAMTOOL_H 
#define GFEXBYTESTREAMTOOL_H

// Trigger includes
#include "TrigT1ResultByteStream/IL1TriggerByteStreamTool.h"

#include "xAODTrigger/gFexJetRoI.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexJetRoIAuxContainer.h"

#include "xAODTrigger/gFexGlobalRoI.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIAuxContainer.h"
#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1ThrExtraInfo.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/WriteDecorHandleKey.h"

// Gaudi includes
#include "Gaudi/Property.h"

// STL includes
#include <array>

/** @class gFEXRoIByteStreamTool
 *  @brief Implementation of a tool for L1 RoI conversion from BS to xAOD and from xAOD to BS
 *  (IL1TriggerByteStreamTool interface)
 **/
class gFexByteStreamTool : public extends<AthAlgTool, IL1TriggerByteStreamTool> {
    public:
        gFexByteStreamTool(const std::string& type, const std::string& name, const IInterface* parent);
        virtual ~gFexByteStreamTool() override = default;

        // ------------------------- IAlgTool methods --------------------------------
        virtual StatusCode initialize() override;
        virtual StatusCode start() override;

        // ------------------------- IL1TriggerByteStreamTool methods ----------------------
        /// BS->xAOD conversion
        virtual StatusCode convertFromBS(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vrobf, const EventContext& eventContext)const override;

        /// xAOD->BS conversion
        virtual StatusCode convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf, const EventContext& eventContext) override;

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
        Gaudi::Property<bool> m_saveExtendedTOBs {this, "SaveExtendedTOBs", false, "Decode and write xTOBs instead of TOBs"};

        int m_gJ_scale = 0;
        int m_gLJ_scale = 0;
        int m_gXE_scale = 0;
        int m_gTE_scale = 0;
    
        // Write handle keys for the L1Calo EDMs for BS->xAOD mode of operation
        SG::WriteHandleKey< xAOD::gFexJetRoIContainer    > m_gFexRhoWriteKey                {this,"gFexRhoOutputContainerWriteKey","L1_gFexRhoRoI","Write gFEX EDM gFexRho container"};
        SG::WriteHandleKey< xAOD::gFexJetRoIContainer    > m_gFexBlockWriteKey              {this,"gFexSRJetOutputContainerWriteKey","L1_gFexSRJetRoI","Write gFEX EDM gFexBlock container"};
        SG::WriteHandleKey< xAOD::gFexJetRoIContainer    > m_gFexJetWriteKey                {this,"gFexLRJetOutputContainerWriteKey","L1_gFexLRJetRoI","Write gFEX EDM gFexJet container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarEJwojWriteKey           {this,"gScalarEJwojOutputContainerWriteKey","L1_gScalarEJwoj","Write gFEX EDM Scalar MET and SumET (JwoJ) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gEspressoWriteKey              {this,"gEspressoOutputContainerWriteKey","L1_gEspresso","Write gFEX EDM gEspresso container"}; 
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsJwojWriteKey     {this,"gMETComponentsJwojOutputContainerWriteKey","L1_gMETComponentsJwoj","Write gFEX EDM total MET components (JwoJ) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMHTComponentsJwojWriteKey     {this,"gMHTComponentsJwojOutputContainerWriteKey","L1_gMHTComponentsJwoj","Write gFEX EDM hard MET components (JwoJ) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMSTComponentsJwojWriteKey     {this,"gMSTComponentsJwojOutputContainerWriteKey","L1_gMSTComponentsJwoj","Write gFEX EDM soft MET components (JwoJ) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsNoiseCutWriteKey {this,"gMETComponentsNoiseCutOutputContainerWriteKey","L1_gMETComponentsNoiseCut","Write gFEX EDM total MET components (NoiseCut) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsRmsWriteKey      {this,"gMETComponentsRmsOutputContainerWriteKey","L1_gMETComponentsRms","Write gFEX EDM total MET components (RMS) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarENoiseCutWriteKey       {this,"gScalarENoiseCutOutputContainerWriteKey","L1_gScalarENoiseCut","Write gFEX EDM Scalar MET and SumET (NoiseCut) container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarERmsWriteKey            {this,"gScalarERmsOutputContainerWriteKey","L1_gScalarERms","Write gFEX EDM Scalar MET and SumET (RMS) container"};

        // Multi-slice (out-of-time) write handle keys for Jet TOBs
        SG::WriteHandleKey< xAOD::gFexJetRoIContainer    > m_gFexRhoSliceWriteKey           {this,"gFexRhoSliceContainerWriteKey","","Write gFEX EDM gFexRho out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexJetRoIContainer    > m_gFexBlockSliceWriteKey         {this,"gFexSRJetSliceContainerWriteKey","","Write gFEX EDM gFexBlock out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexJetRoIContainer    > m_gFexJetSliceWriteKey           {this,"gFexLRJetSliceContainerWriteKey","","Write gFEX EDM gFexJet out-of-time container"};
        // Multi-slice (out-of-time) write handle keys for Global TOBs
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarEJwojSliceWriteKey      {this,"gScalarEJwojSliceContainerWriteKey","","Write gFEX EDM Scalar (JwoJ) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsJwojSliceWriteKey{this,"gMETComponentsJwojSliceContainerWriteKey","","Write gFEX EDM MET (JwoJ) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMHTComponentsJwojSliceWriteKey{this,"gMHTComponentsJwojSliceContainerWriteKey","","Write gFEX EDM MHT (JwoJ) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMSTComponentsJwojSliceWriteKey{this,"gMSTComponentsJwojSliceContainerWriteKey","","Write gFEX EDM MST (JwoJ) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gEspressoSliceWriteKey         {this,"gEspressoSliceContainerWriteKey","","Write gFEX EDM gEspresso out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsNoiseCutSliceWriteKey{this,"gMETComponentsNoiseCutSliceContainerWriteKey","","Write gFEX EDM MET (NoiseCut) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarENoiseCutSliceWriteKey  {this,"gScalarENoiseCutSliceContainerWriteKey","","Write gFEX EDM Scalar (NoiseCut) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsRmsSliceWriteKey {this,"gMETComponentsRmsSliceContainerWriteKey","","Write gFEX EDM MET (Rms) out-of-time container"};
        SG::WriteHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarERmsSliceWriteKey       {this,"gScalarERmsSliceContainerWriteKey","","Write gFEX EDM Scalar (Rms) out-of-time container"};

        // Slice number decoration keys for out-of-time containers only
        // L1A containers always contain slice 0, so decoration is redundant there
        // Out-of-time containers mix TOBs from slices 1,2,3,... so decoration is needed to identify which BC
        // Jet TOB decorations (out-of-time containers)
        SG::WriteDecorHandleKey<xAOD::gFexJetRoIContainer> m_gFexRhoOOTDecorKey   {this,"gFexRhoOOTDecorKey",m_gFexRhoSliceWriteKey,"sliceNumber","Slice number decoration for gFexRho out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexJetRoIContainer> m_gFexBlockOOTDecorKey {this,"gFexBlockOOTDecorKey",m_gFexBlockSliceWriteKey,"sliceNumber","Slice number decoration for gFexBlock out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexJetRoIContainer> m_gFexJetOOTDecorKey   {this,"gFexJetOOTDecorKey",m_gFexJetSliceWriteKey,"sliceNumber","Slice number decoration for gFexJet out-of-time"};
        // Global TOB decorations (out-of-time containers)
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gScalarEJwojOOTDecorKey        {this,"gScalarEJwojOOTDecorKey",m_gScalarEJwojSliceWriteKey,"sliceNumber","Slice number decoration for gScalarEJwoj out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gMETComponentsJwojOOTDecorKey  {this,"gMETComponentsJwojOOTDecorKey",m_gMETComponentsJwojSliceWriteKey,"sliceNumber","Slice number decoration for gMETComponentsJwoj out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gMHTComponentsJwojOOTDecorKey  {this,"gMHTComponentsJwojOOTDecorKey",m_gMHTComponentsJwojSliceWriteKey,"sliceNumber","Slice number decoration for gMHTComponentsJwoj out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gMSTComponentsJwojOOTDecorKey  {this,"gMSTComponentsJwojOOTDecorKey",m_gMSTComponentsJwojSliceWriteKey,"sliceNumber","Slice number decoration for gMSTComponentsJwoj out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gEspressoOOTDecorKey           {this,"gEspressoOOTDecorKey",m_gEspressoSliceWriteKey,"sliceNumber","Slice number decoration for gEspresso out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gMETComponentsNoiseCutOOTDecorKey{this,"gMETComponentsNoiseCutOOTDecorKey",m_gMETComponentsNoiseCutSliceWriteKey,"sliceNumber","Slice number decoration for gMETComponentsNoiseCut out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gScalarENoiseCutOOTDecorKey    {this,"gScalarENoiseCutOOTDecorKey",m_gScalarENoiseCutSliceWriteKey,"sliceNumber","Slice number decoration for gScalarENoiseCut out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gMETComponentsRmsOOTDecorKey   {this,"gMETComponentsRmsOOTDecorKey",m_gMETComponentsRmsSliceWriteKey,"sliceNumber","Slice number decoration for gMETComponentsRms out-of-time"};
        SG::WriteDecorHandleKey<xAOD::gFexGlobalRoIContainer> m_gScalarERmsOOTDecorKey         {this,"gScalarERmsOOTDecorKey",m_gScalarERmsSliceWriteKey,"sliceNumber","Slice number decoration for gScalarERms out-of-time"};

        // Read handle keys for the L1Calo EDMs for xAOD->BS mode of operation
        SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_gFexRhoReadKey                {this,"gFexRhoOutputContainerReadKey","L1_gFexRhoRoI","Read gFEX EDM gFexRho container"};
        SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_gFexBlockReadKey              {this,"gFexSRJetOutputContainerReadKey","L1_gFexSRJetRoI","Read gFEX EDM gFexBlock container"};
        SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_gFexJetReadKey                {this,"gFexLRJetOutputContainerReadKey","L1_gFexLRJetRoI","Read gFEX EDM gFexJet container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarEJwojReadKey           {this,"gScalarEJwojOutputContainerReadKey","L1_gScalarEJwoj","Read gFEX EDM Scalar MET and SumET (JwoJ) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gEspressoReadKey              {this,"gEspressoOutputContainerReadKey","L1_gEspresso","Read gFEX EDM gEspresso container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsJwojReadKey     {this,"gMETComponentsJwojOutputContainerReadKey","L1_gMETComponentsJwoj","Read gFEX EDM total MET components (JwoJ) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gMHTComponentsJwojReadKey     {this,"gMHTComponentsJwojOutputContainerReadKey","L1_gMHTComponentsJwoj","Read gFEX EDM hard MET components (JwoJ) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gMSTComponentsJwojReadKey     {this,"gMSTComponentsJwojOutputContainerReadKey","L1_gMSTComponentsJwoj","Read gFEX EDM soft MET components (JwoJ) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsNoiseCutReadKey {this,"gMETComponentsNoiseCutOutputContainerReadKey","L1_gMETComponentsNoiseCut","Read gFEX EDM total MET components (NoiseCut) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gMETComponentsRmsReadKey      {this,"gMETComponentsRmsOutputContainerReadKey","L1_gMETComponentsRms","Read gFEX EDM total MET components (RMS) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarENoiseCutReadKey       {this,"gScalarENoiseCutOutputContainerReadKey","L1_gScalarENoiseCut","Read gFEX EDM Scalar MET and SumET (NoiseCut) container"};
        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_gScalarERmsReadKey            {this,"gScalarERmsOutputContainerReadKey","L1_gScalarERms","Read gFEX EDM Scalar MET and SumET (RMS) container"};

        //Read handle key for the L1Menu
        SG::ReadHandleKey<TrigConf::L1Menu> m_l1MenuKey   {this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu","Name of the L1Menu object to read configuration from"}; 

        void decodeGfexTobSlice( const uint32_t dataArray[], uint32_t blockType) const;
        
        void printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const;
        
        int16_t fillGlobal(const std::array<uint32_t, 3> &tob, const int type, SG::WriteHandle<xAOD::gFexGlobalRoIContainer> &container, uint32_t sliceNumber, int16_t scalar = -1) const;
        
        static constexpr uint8_t m_DEBUG=0;
        static constexpr uint8_t m_WARNING=1;
        static constexpr uint8_t m_ERROR=2;
        static constexpr uint8_t m_FATAL=3;        
        
};

#endif // GFEXBYTESTREAMTOOL_H
