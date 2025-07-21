/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexInputByteStreamTool  -  This tool decodes Run3 gFEX input data!
//                              -------------------
//     begin                : 10 08 2022
//     email                : cecilia.tosciri@cern.ch
//  ***************************************************************************/

#ifndef GFEXINPUTBYTESTREAMTOOL_H
#define GFEXINPUTBYTESTREAMTOOL_H

// Trigger includes
#include "TrigT1ResultByteStream/IL1TriggerByteStreamTool.h"
#include "xAODTrigL1Calo/gFexTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerAuxContainer.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"

// Gaudi includes
#include "Gaudi/Property.h"

#include "L1CaloFEXByteStream/gFexPos.h"
#include <array>
#include <vector>
#include <cstdint>
namespace gPos = LVL1::gFEXPos;

/** @class gFexInputByteStreamTool
 *  @brief Implementation of a tool for L1 input data conversion from BS to xAOD and from xAOD to BS
 *  (IL1TriggerByteStreamTool interface)
 **/




class gFexInputByteStreamTool : public extends<AthAlgTool, IL1TriggerByteStreamTool> {
  public:
    typedef  std::array<std::array<uint32_t, 7>,  100>        gfiber;
    typedef  std::array<std::array<int,      6>,   32>        gEngines;
    typedef  std::array<std::array<int,      12>,  32>        gtFPGA;
    typedef  std::array<std::array<int,      20>,  100>       gFields;
    typedef  std::array<std::array<int,      16>,  100>       gCaloTwr;
    typedef  std::array<std::array<int,      8>,   100>       gSatur;
    typedef  std::array<std::array<char,     20>,  100>       gFieldsChar;
    typedef  std::array<std::array<int,      20>,  4>         gType;
    typedef  std::array<std::array<char,     20>,  4>         gTypeChar;
    gFexInputByteStreamTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~gFexInputByteStreamTool() override = default;

    // ------------------------- IAlgTool methods --------------------------------
    virtual StatusCode initialize() override;

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

    //Write handle keys for the L1Calo EDMs for BS->xAOD mode of operation
    SG::WriteHandleKey< xAOD::gFexTowerContainer> m_gTowersWriteKey   {this,"gTowersWriteKey"  ,"L1_gFexDataTowers", "Name of the gFEX Input Data Towers"};  // TODO: This will be the only output of this class in the future
    SG::WriteHandleKey< xAOD::gFexTowerContainer> m_gTowers50WriteKey   {this,"gTowers50WriteKey"  ,"", "Write gFexEDM Trigger Tower container with 50 MeV resolution"};
    SG::WriteHandleKey< xAOD::gFexTowerContainer> m_gTowers200WriteKey   {this,"gTowers200WriteKey"  ,"", "Write gFexEDM Trigger Tower container with 200 MeV resolution (default)"};

    // Read handle keys for the L1Calo EDMs for xAOD->BS mode of operation
    SG::ReadHandleKey < xAOD::gFexTowerContainer> m_gTowersReadKey    {this,"gTowersReadKey"   ,"L1_gFexDataTowers","Read gFexEDM Trigger Tower container"};

    virtual void a_gtrx_map( const gfiber &inputData, gfiber &jf_lar_rx_data) const;

    virtual void b_gtrx_map( const gfiber &inputData, gfiber &jf_lar_rx_data) const;

    virtual void c_gtrx_map( const gfiber &inputData, gfiber &outputData) const;

    virtual void gtReconstructABC(  int XFPGA,
                                    const gfiber & Xfiber,
                                    int Xin,
                                    gtFPGA &XgtF,
                                    gtFPGA &Xgt,
                                    int *BCIDptr,
                                    int do_lconv,
                                    const std::array<int, gPos::MAX_FIBERS> &XMPD_NFI,
                                    const std::array<int, gPos::MAX_FIBERS> &XCALO_TYPE,
                                    const gCaloTwr & XMPD_GTRN_ARR,
                                    const gType & XMPD_DSTRT_ARR,
                                    gTypeChar XMPD_DTYP_ARR,
                                    const std::array<int, gPos::MAX_FIBERS> &XMSK,
                                    gtFPGA &Xsatur,
                                    std::array<int, (gPos::AB_FIBERS*gPos::MAX_E_FIELDS)> &FiberTower,
                                    std::array<int, (gPos::AB_FIBERS*gPos::MAX_E_FIELDS)> &FiberTowerSatur) const;

    virtual int crc9d32(const std::array<uint32_t, 6> &inWords,int numWords,int reverse) const;

    uint32_t crc9d23(uint32_t inword, uint32_t in_crc, int  reverse ) const;

    virtual void undoMLE(int &datumPtr ) const;

    virtual void getEtaPhi(float &Eta, float &Phi, int iEta, int iPhi, int gFEXtowerID) const;

    virtual void signExtend(int *xptr, int upto) const;

    virtual void gtCalib(gtFPGA &gtf, int towerLSB,  int fpga, unsigned int offset) const;



    void printError(const std::string& location, const std::string& title, MSG::Level type, const std::string& detail) const;

};

#endif // GFEXINPUTBYTESTREAMTOOL_H
