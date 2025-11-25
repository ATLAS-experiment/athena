/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for gFEXCondAlgo - Tool to read the COOL DB for gFEX
//                              -------------------
//     begin                : 23 10 2024
//     email                : jared.little@cern.ch
//***************************************************************************

#ifndef gFEXCondAlgo_H
#define gFEXCondAlgo_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "L1CaloFEXCond/gFEXDBCondData.h"

namespace LVL1
{

    class gFEXCondAlgo : public AthReentrantAlgorithm
    {

    public:
        /** Constructors **/
        gFEXCondAlgo(const std::string &name, ISvcLocator *svc);

        virtual StatusCode initialize() override;

        virtual StatusCode execute(const EventContext &) const override;
        virtual bool isReEntrant() const override final { return false; }

    private:
        // Default noise-cut parameters, in case DB values aren't valid
        constexpr static std::array<int, 12> m_AslopesDefault = {891, 905, 903, 954, 1128, 1106, 1093, 1034, 940, 887, 877, 874};
        constexpr static std::array<int, 12> m_BslopesDefault = {880, 869, 888, 923, 1011, 1060, 1062, 1109, 941, 912, 904, 882};
        constexpr static std::array<int, 12> m_CslopesDefault = {1024, 1024, 1306, 1011, 887, 878, 860, 888, 975, 1252, 1024, 1024};
        constexpr static std::array<int, 12> m_AnoiseCutsDefault = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};
        constexpr static std::array<int, 12> m_BnoiseCutsDefault = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};
        constexpr static std::array<int, 12> m_CnoiseCutsDefault = {176, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, 176};

        // Key for writing CondHandles
        SG::WriteCondHandleKey<gFEXDBCondData> m_gFEXDBParamsKey{this, "gFEXDBParamsKey", "gFEXDBParams", "Output for gFEX DB noise parameters"};

        // Key for reading CondHandles
        SG::ReadCondHandleKey<CondAttrListCollection> m_GfexNoiseCutsKey{this, "GfexNoiseCuts", "", "Key to store GfexNoiseCuts DB path"};

        UnsignedIntegerProperty m_dbBeginTimestamp{this, "BeginTimestamp", 1729591852, "Earliest timestamp that db parameters will be loaded. Default is start of 2024-10-22"};

    Gaudi::Property<bool> m_isMC {this, "IsMC", false, "For MC, always access the DB"};
    };

} // END OF LVL1 NAMESPACE

#endif