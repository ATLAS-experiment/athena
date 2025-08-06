/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//////////////////////////////
//
/**  @file TRTStrawStatusRead.cxx
 *   Algoritm to dump TRT dead channel info in CondStore to a text file
 *
 *
 * @author Peter Hansen <phansen@nbi.dk>
 **/

#include "TRT_ConditionsAlgs/TRTStrawStatusRead.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "TRT_ConditionsData/StrawStatusMultChanContainer.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "StoreGate/ReadCondHandle.h"
#include "TRT_ConditionsData/ExpandedIdentifier.h"

TRTStrawStatusRead::TRTStrawStatusRead(const std::string &name, ISvcLocator *pSvcLocator) : AthAlgorithm(name, pSvcLocator)
{
}

StatusCode TRTStrawStatusRead::initialize()
{
    // Get ID helper
    ATH_MSG_INFO("TRTStrawStatusRead::initialize");
    ATH_CHECK(detStore()->retrieve(m_trtid, "TRT_ID"));

    // Get tool
    ATH_CHECK(m_status.retrieve());

    // Read keys
    ATH_CHECK(m_statReadKey.initialize());
    ATH_CHECK(m_permReadKey.initialize());
    ATH_CHECK(m_statHTReadKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode TRTStrawStatusRead::execute()
{

    StatusCode sc = StatusCode::SUCCESS;

    ATH_MSG_INFO(" Dump the Straw Status to text file:");
    sc = writeToTextFile("StrawStatusDump_Writer.txt");
    if (sc.isFailure())
    {
        ATH_MSG_ERROR(" Error writing the text file");
    }

    return sc;
}

StatusCode TRTStrawStatusRead::finalize()
{
    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////

StatusCode TRTStrawStatusRead::writeToTextFile(const std::string &filename)
{
    std::ofstream outfile(filename.c_str());

    int stat, bec, lay, sec, slay, straw, level;
    int deadba0[32];
    int deadba1[32];
    int deadba2[32];
    int deadbc0[32];
    int deadbc1[32];
    int deadbc2[32];
    int deadea[14];
    int deadec[14];
    for (int i = 0; i < 32; i++)
    {
        deadba0[i] = 0;
        deadba1[i] = 0;
        deadba2[i] = 0;
        deadbc0[i] = 0;
        deadbc1[i] = 0;
        deadbc2[i] = 0;
    }
    for (int i = 0; i < 14; i++)
    {
        deadea[i] = 0;
        deadec[i] = 0;
    }
    int ngood = 0;

    if (m_printfolder == "Status")
    {
        ATH_MSG_INFO(" Dump To File: StrawStatus ");
        TRTCond::StrawStatusLayerContainer::FlatContainer flatcontainer;

        getStrawStatusContainer()->getall(flatcontainer);

        std::cout << " Status write output file in the same format as input " << std::endl;
        for (TRTCond::StrawStatusContainer::FlatContainer::const_iterator
                 it = flatcontainer.begin();
             it != flatcontainer.end(); ++it)
        {
            TRTCond::ExpandedIdentifier id = it->first;
            const TRTCond::StrawStatus *status = it->second;
            bec = id.becindextoid(id.index(1));
            lay = id.index(2);
            sec = id.index(3);
            slay = id.index(4);
            straw = id.index(5);
            level = id.level();
            stat = int(status->getstatus());
            if (stat != TRTCond::StrawStatus::Good)
            {
                // record this straw in the output file in same format as in the original input text file
                if (level == 4)
                    straw = -1;
                if (level == 3)
                {
                    straw = -1;
                    slay = -1;
                }
                if (level == 2)
                {
                    straw = -1;
                    slay = -1;
                    sec = -1;
                }
                outfile << bec << " " << sec << " " << slay << " " << straw << " " << lay << " " << TRTCond::StrawStatus::Dead << std::endl;
            }
        }
        std::cout << " Check all straws: " << " ( Good = " << TRTCond::StrawStatus::Good << " Dead = " << TRTCond::StrawStatus::Dead << std::endl;
        for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
        {
            int nStrawsInLayer = m_trtid->straw_max(*it);
            for (int i = 0; i <= nStrawsInLayer; i++)
            {

                Identifier id = m_trtid->straw_id(*it, i);
                bec = m_trtid->barrel_ec(id);
                lay = m_trtid->layer_or_wheel(id);
                sec = m_trtid->phi_module(id);
                slay = m_trtid->straw_layer(id);
                straw = m_trtid->straw(id);

                stat = m_status->getStatus(id, Gaudi::Hive::currentContext());
                if (stat != TRTCond::StrawStatus::Good)
                {
                    if (bec == -2)
                    {
                        deadec[lay]++;
                    }
                    else if (bec == 2)
                    {
                        deadea[lay]++;
                    }
                    else if (bec == -1)
                    {
                        if (lay == 0)
                            deadbc0[sec]++;
                        if (lay == 1)
                            deadbc1[sec]++;
                        if (lay == 2)
                            deadbc2[sec]++;
                    }
                    else if (bec == 1)
                    {
                        if (lay == 0)
                            deadba0[sec]++;
                        if (lay == 1)
                            deadba1[sec]++;
                        if (lay == 2)
                            deadba2[sec]++;
                    }
                }
            }
        }
    }

    if (m_printfolder == "StatusHT")
    {
        ATH_MSG_INFO(" Dump To File: StrawStatus HT ");
        TRTCond::StrawStatusLayerContainer::FlatContainer flatcontainerHT;
        getStrawStatusHTContainer()->getall(flatcontainerHT);

        ATH_MSG_INFO(" bec lay gas (Xe=2 Ar=4)");
        for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
        {
            Identifier id = m_trtid->straw_id(*it, 1);
            bec = m_trtid->barrel_ec(id);
            lay = m_trtid->layer_or_wheel(id);
            sec = m_trtid->phi_module(id);
            slay = m_trtid->straw_layer(id);
            // print gas status for sector 1, strawlayer 1, straw 1 in this layer-or-wheel
            if (sec == 1 && slay == 1)
            {

                int stat = int(m_status->getStatusHT(id, Gaudi::Hive::currentContext()));

                ATH_MSG_INFO(bec << "  " << lay << " " << stat);
            }
        }

        for (TRTCond::StrawStatusContainer::FlatContainer::const_iterator
                 it = flatcontainerHT.begin();
             it != flatcontainerHT.end(); ++it)
        {
            TRTCond::ExpandedIdentifier id = it->first;
            const TRTCond::StrawStatus *status = it->second;
            outfile << id << " " << int(status->getstatus()) << std::endl;
        }
    }

    if (m_printfolder == "StatusPermanent")
    {
        ATH_MSG_INFO(" Dump To File: StrawStatus permanent ");

        TRTCond::StrawStatusLayerContainer::FlatContainer flatcontainerpermanent;

        getStrawStatusPermanentContainer()->getall(flatcontainerpermanent);

        std::cout << " StatusPermanent cobntainer first straws: " << std::endl;
        for (TRTCond::StrawStatusContainer::FlatContainer::const_iterator
                 it = flatcontainerpermanent.begin();
             it != flatcontainerpermanent.end(); ++it)
        {
            TRTCond::ExpandedIdentifier id = it->first;
            const TRTCond::StrawStatus *status = it->second;
            bec = id.becindextoid(id.index(1));
            lay = id.index(2);
            sec = id.index(3);
            slay = id.index(4);
            straw = id.index(5);
            stat = int(status->getstatus());
            if (stat != TRTCond::StrawStatus::Good)
            {
                outfile << bec << " " << sec << " " << straw << " " << slay << " " << lay << " " << stat << std::endl;

                if (bec == -2)
                {
                    deadec[lay]++;
                }
                else if (bec == 2)
                {
                    deadea[lay]++;
                }
                else if (bec == -1)
                {
                    if (lay == 0)
                        deadbc0[sec]++;
                    if (lay == 1)
                        deadbc1[sec]++;
                    if (lay == 2)
                        deadbc2[sec]++;
                }
                else if (bec == 1)
                {
                    if (lay == 0)
                        deadba0[sec]++;
                    if (lay == 1)
                        deadba1[sec]++;
                    if (lay == 2)
                        deadba2[sec]++;
                }
            }
            else
            {
                ngood++;
            }

            if (straw == 0 && slay == 0)
                std::cout << bec << " " << sec << " " << straw << " " << slay << " " << lay << " " << stat << std::endl;
        }
        std::cout << " First straws: " << " ( Good = " << TRTCond::StrawStatus::Good << " Dead = " << TRTCond::StrawStatus::Dead << " ) " << std::endl;

        for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
        {
            int nStrawsInLayer = m_trtid->straw_max(*it);
            for (int i = 0; i <= nStrawsInLayer; i++)
            {

                Identifier id = m_trtid->straw_id(*it, i);
                bec = m_trtid->barrel_ec(id);
                lay = m_trtid->layer_or_wheel(id);
                sec = m_trtid->phi_module(id);
                slay = m_trtid->straw_layer(id);
                straw = m_trtid->straw(id);
                stat = int(m_status->getStatusPermanent(id, Gaudi::Hive::currentContext()));

                if (slay == 0 && straw == 0)
                    std::cout << bec << " " << lay << " " << sec << " " << stat << std::endl;
            }
        }
    }

    if(m_printfolder=="Status" || m_printfolder=="StatusPermanent"){
        ATH_MSG_INFO ( " Dead straws BA layer 0" );
        ATH_MSG_INFO ( "deadba0[] = { " << deadba0[0] << ", " << deadba0[1] << ", " << deadba0[2] << ", " << deadba0[3] << ", " <<
                                            deadba0[4] << ", " << deadba0[5] << ", " << deadba0[6] << ", " << deadba0[7] << ", " <<
                                            deadba0[8] << ", " << deadba0[9] << ", " << deadba0[10] << ", " << deadba0[11] << ", " <<
                                            deadba0[12] << ", " << deadba0[13] << ", " << deadba0[14] << ", " << deadba0[15] << ", " <<
                                            deadba0[16] << ", " << deadba0[17] << ", " << deadba0[18] << ", " << deadba0[19] << ", " <<
                                            deadba0[20] << ", " << deadba0[21] << ", " << deadba0[22] << ", " << deadba0[23] << ", " <<
                                            deadba0[24] << ", " << deadba0[25] << ", " << deadba0[26] << ", " << deadba0[27] << ", " <<
                                            deadba0[28] << ", " << deadba0[29] << ", " << deadba0[30] << ", " << deadba0[31] << "}; " );
        ATH_MSG_INFO ( " Dead straws BA layer 1 ");
        ATH_MSG_INFO ( "deadba1[] = { " << deadba1[0] << ", " << deadba1[1] << ", " << deadba1[2] << ", " << deadba1[3] << ", " <<
                                            deadba1[4] << ", " << deadba1[5] << ", " << deadba1[6] << ", " << deadba1[7] << ", " <<
                                            deadba1[8] << ", " << deadba1[9] << ", " << deadba1[10] << ", " << deadba1[11] << ", " <<
                                            deadba1[12] << ", " << deadba1[13] << ", " << deadba1[14] << ", " << deadba1[15] << ", " <<
                                            deadba1[16] << ", " << deadba1[17] << ", " << deadba1[18] << ", " << deadba1[19] << ", " <<
                                            deadba1[20] << ", " << deadba1[21] << ", " << deadba1[22] << ", " << deadba1[23] << ", " <<
                                            deadba1[24] << ", " << deadba1[25] << ", " << deadba1[26] << ", " << deadba1[27] << ", " <<
                                            deadba1[28] << ", " << deadba1[29] << ", " << deadba1[30] << ", " << deadba1[31] << "};");
        ATH_MSG_INFO (" Dead straws BA layer 2");
        ATH_MSG_INFO ( "deadba2[]= { " << deadba2[0] << ", " << deadba2[1] << ", " << deadba2[2] << ", " << deadba2[3] << ", " <<
                                            deadba2[4] << ", " << deadba2[5] << ", " << deadba2[6] << ", " << deadba2[7] << ", " <<
                                            deadba2[8] << ", " << deadba2[9] << ", " << deadba2[10] << ", " << deadba2[11] << ", " <<
                                            deadba2[12] << ", " << deadba2[13] << ", " << deadba2[14] << ", " << deadba2[15] << ", " <<
                                            deadba2[16] << ", " << deadba2[17] << ", " << deadba2[18] << ", " << deadba2[19] << ", " <<
                                            deadba2[20] << ", " << deadba2[21] << ", " << deadba2[22] << ", " << deadba2[23] << ", " <<
                                            deadba2[24] << ", " << deadba2[25] << ", " << deadba2[26] << ", " << deadba2[27] << ", " <<
                                            deadba2[28] << ", " << deadba2[29] << ", " << deadba2[30] << ", " << deadba2[31] << "}; ");

        ATH_MSG_INFO ( " Dead straws BC layer 0" );
        ATH_MSG_INFO ( "deadbc0[] = { " << deadbc0[0] << ", " << deadbc0[1] << ", " << deadbc0[2] << ", " << deadbc0[3] << ", " <<
                                            deadbc0[4] << ", " << deadbc0[5] << ", " << deadbc0[6] << ", " << deadbc0[7] << ", " <<
                                            deadbc0[8] << ", " << deadbc0[9] << ", " << deadbc0[10] << ", " << deadbc0[11] << ", " <<
                                            deadbc0[12] << ", " << deadbc0[13] << ", " << deadbc0[14] << ", " << deadbc0[15] << ", " <<
                                            deadbc0[16] << ", " << deadbc0[17] << ", " << deadbc0[18] << ", " << deadbc0[19] << ", " <<
                                            deadbc0[20] << ", " << deadbc0[21] << ", " << deadbc0[22] << ", " << deadbc0[23] << ", " <<
                                            deadbc0[24] << ", " << deadbc0[25] << ", " << deadbc0[26] << ", " << deadbc0[27] << ", " <<
                                            deadbc0[28] << ", " << deadbc0[29] << ", " << deadbc0[30] << ", " << deadbc0[31] << "}; " );
        ATH_MSG_INFO ( " Dead straws BC layer 1 ");
        ATH_MSG_INFO ( "deadbc1[] = { " << deadbc1[0] << ", " << deadbc1[1] << ", " << deadbc1[2] << ", " << deadbc1[3] << ", " <<
                                            deadbc1[4] << ", " << deadbc1[5] << ", " << deadbc1[6] << ", " << deadbc1[7] << ", " <<
                                            deadbc1[8] << ", " << deadbc1[9] << ", " << deadbc1[10] << ", " << deadbc1[11] << ", " <<
                                            deadbc1[12] << ", " << deadbc1[13] << ", " << deadbc1[14] << ", " << deadbc1[15] << ", " <<
                                            deadbc1[16] << ", " << deadbc1[17] << ", " << deadbc1[18] << ", " << deadbc1[19] << ", " <<
                                            deadbc1[20] << ", " << deadbc1[21] << ", " << deadbc1[22] << ", " << deadbc1[23] << ", " <<
                                            deadbc1[24] << ", " << deadbc1[25] << ", " << deadbc1[26] << ", " << deadbc1[27] << ", " <<
                                            deadbc1[28] << ", " << deadbc1[29] << ", " << deadbc1[30] << ", " << deadbc1[31] << "};");
        ATH_MSG_INFO (" Dead straws BC layer 2");
        ATH_MSG_INFO ( "deadbc2[]= { " << deadbc2[0] << ", " << deadbc2[1] << ", " << deadbc2[2] << ", " << deadbc2[3] << ", " <<
                                            deadbc2[4] << ", " << deadbc2[5] << ", " << deadbc2[6] << ", " << deadbc2[7] << ", " <<
                                            deadbc2[8] << ", " << deadbc2[9] << ", " << deadbc2[10] << ", " << deadbc2[11] << ", " <<
                                            deadbc2[12] << ", " << deadbc2[13] << ", " << deadbc2[14] << ", " << deadbc2[15] << ", " <<
                                            deadbc2[16] << ", " << deadbc2[17] << ", " << deadbc2[18] << ", " << deadbc2[19] << ", " <<
                                            deadbc2[20] << ", " << deadbc2[21] << ", " << deadbc2[22] << ", " << deadbc2[23] << ", " <<
                                            deadbc2[24] << ", " << deadbc2[25] << ", " << deadbc2[26] << ", " << deadbc2[27] << ", " <<
                                            deadbc2[28] << ", " << deadbc2[29] << ", " << deadbc2[30] << ", " << deadbc2[31] << "}; ");

        ATH_MSG_INFO (" Dead straws EA" );
        ATH_MSG_INFO ("deadea[] = { " << deadea[0] << ", " << deadea[1] << ", " << deadea[2] << ", " << deadea[3] << ", " <<
                                            deadea[4] << ", " << deadea[5] << ", " << deadea[6] << ", " << deadea[7] << ", " <<
                                            deadea[8] << ", " << deadea[9] << ", " << deadea[10] << ", " << deadea[11] << ", " <<
                                            deadea[12] << ", " << deadea[13] << "}; ");
        ATH_MSG_INFO (" Dead straws EC" );
        ATH_MSG_INFO ("deadec[] = { " << deadec[0] << ", " << deadec[1] << ", " << deadec[2] << ", " << deadec[3] << ", " <<
                                            deadec[4] << ", " << deadec[5] << ", " << deadec[6] << ", " << deadec[7] << ", " <<
                                            deadec[8] << ", " << deadec[9] << ", " << deadec[10] << ", " << deadec[11] << ", " <<
                                            deadec[12] << ", " << deadec[13] << "}; " );
        ATH_MSG_INFO("Number of good layers or wheels: " << ngood);
    }

    return StatusCode::SUCCESS;
}

const TRTStrawStatusRead::StrawStatusContainer *TRTStrawStatusRead::getStrawStatusContainer() const
{

    SG::ReadCondHandle<StrawStatusContainer> rtc(m_statReadKey);
    const StrawStatusContainer *strawstatuscontainer = *rtc;
    return strawstatuscontainer;
}

const TRTStrawStatusRead::StrawStatusContainer *TRTStrawStatusRead::getStrawStatusPermanentContainer() const
{

    SG::ReadCondHandle<StrawStatusContainer> rtc(m_permReadKey);
    const StrawStatusContainer *strawstatuscontainer = *rtc;
    return strawstatuscontainer;
}

const TRTStrawStatusRead::StrawStatusContainer *TRTStrawStatusRead::getStrawStatusHTContainer() const
{

    SG::ReadCondHandle<StrawStatusContainer> rtc(m_statHTReadKey);
    const StrawStatusContainer *strawstatuscontainer = *rtc;
    return strawstatuscontainer;
}
