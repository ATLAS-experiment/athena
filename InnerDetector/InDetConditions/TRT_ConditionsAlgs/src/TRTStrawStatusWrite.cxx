/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//////////////////////////////
//
/**  @file TRTStrawStatusWrite.cxx
 *   Algorithm for publishing TRT dead channels and/or gas geometry
 *   to cool and pool files for DB uploads
 *
 * @author Peter Hansen <phansen@nbi.dk>
 **/

#include "TRT_ConditionsAlgs/TRTStrawStatusWrite.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

TRTStrawStatusWrite::TRTStrawStatusWrite(const std::string &name, ISvcLocator *pSvcLocator) : AthAlgorithm(name, pSvcLocator){}

StatusCode TRTStrawStatusWrite::initialize()
{
    ATH_MSG_INFO(" Initializing TRTStrawStatusWrite ");
    ATH_CHECK(detStore()->retrieve(m_trtid, "TRT_ID"));

    if (!m_par_stattextfile.empty())
    {
        ATH_MSG_INFO("Recording StrawStatusContainer for key " << m_par_strawstatuscontainerkey);
        if (!readStatFromTextFile(m_par_stattextfile).isSuccess())
        {
            ATH_MSG_ERROR(" Could not read TRT StrawStatus objects ");
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO(" Filled " << m_par_strawstatuscontainerkey << " using input file " << m_par_stattextfile);
    }

    if (!m_par_stattextfilepermanent.empty())
    {
        ATH_MSG_INFO("Recording StrawStatusPermanent Container for key " << m_par_strawstatuspermanentcontainerkey);
        if (!readStatPermFromTextFile(m_par_stattextfilepermanent).isSuccess())
        {
            ATH_MSG_ERROR(" Could not read TRT StrawStatus permanent objects ");
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO(" Filled " << m_par_strawstatuspermanentcontainerkey << " using input file " << m_par_stattextfilepermanent);
    }

    if (!m_par_stattextfileHT.empty())
    {
        ATH_MSG_INFO("Recording StrawStatusHTContainer for key " << m_par_strawstatusHTcontainerkey);
        if (!readStatHTFromTextFile(m_par_stattextfileHT).isSuccess())
        {
            ATH_MSG_ERROR(" Could not read TRT StrawStatus HT objects ");
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO(" Filled " << m_par_strawstatusHTcontainerkey << " using input file " << m_par_stattextfileHT);
    }

    return StatusCode::SUCCESS;
}

StatusCode TRTStrawStatusWrite::execute()
{
    return StatusCode::SUCCESS;
}

StatusCode TRTStrawStatusWrite::finalize()
{
    return StatusCode::SUCCESS;
}

// set special bits//////////////////////////////////////////////////
void TRTStrawStatusWrite::set_status_temp(StrawStatusContainer *strawstatuscontainer, Identifier offlineID, bool set)
{
    int level = TRTCond::ExpandedIdentifier::STRAW;
    TRTCond::ExpandedIdentifier id = TRTCond::ExpandedIdentifier(m_trtid->barrel_ec(offlineID), m_trtid->layer_or_wheel(offlineID),
                                                                 m_trtid->phi_module(offlineID), m_trtid->straw_layer(offlineID),
                                                                 m_trtid->straw(offlineID), level);

    unsigned int org_stat = int((*strawstatuscontainer).get(id).getstatus());
    const unsigned int statusbitmask = 1 << 8; // 0000001 00000000

    if (set)
    {
        (*strawstatuscontainer).set(id, (org_stat | statusbitmask));
    }
    else
    {
        (*strawstatuscontainer).set(id, (org_stat & (~statusbitmask)));
    }
}

// set special bits////////////////////////////////////////////////
void TRTStrawStatusWrite::set_status_permanent(StrawStatusContainer *strawstatuspermanentcontainer, Identifier offlineID, bool set)
{

    int level = TRTCond::ExpandedIdentifier::STRAW;
    TRTCond::ExpandedIdentifier id = TRTCond::ExpandedIdentifier(m_trtid->barrel_ec(offlineID), m_trtid->layer_or_wheel(offlineID),
                                                                 m_trtid->phi_module(offlineID), m_trtid->straw_layer(offlineID),
                                                                 m_trtid->straw(offlineID), level);
    unsigned int org_stat = int((*strawstatuspermanentcontainer).get(id).getstatus());
    const unsigned int statusbitmask = 1 << 8; // 0000001 00000000

    if (set)
    {
        (*strawstatuspermanentcontainer).set(id, (org_stat | statusbitmask));
    }
    else
    {
        (*strawstatuspermanentcontainer).set(id, (org_stat & (~statusbitmask)));
    }
}

/////////////////////////////////////////////////////////////////
StatusCode TRTStrawStatusWrite::readStatFromTextFile(const std::string &filename)
{

    StrawStatusContainer *strawstatus = new TRTCond::StrawStatusMultChanContainer();

    strawstatus->clear();
    ATH_MSG_INFO(" ***************** TRTStrawStatusWrite ************************ ");
    ATH_MSG_INFO(" readStatFromTextFile called with file name " << filename);

    const InDetDD::TRT_DetectorManager *TRTDetectorManager;
    if ((detStore()->retrieve(TRTDetectorManager)).isFailure())
    {
        ATH_MSG_FATAL("Problem retrieving TRT_DetectorManager");
    }

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
    // initialize detector layers with status 'good'
    for (int l = 0; l < 3; l++)
    {
        TRTCond::ExpandedIdentifier idbc(-1, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
        strawstatus->setStatus(idbc, TRTCond::StrawStatus::Good);
        TRTCond::ExpandedIdentifier idba(1, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
        strawstatus->setStatus(idba, TRTCond::StrawStatus::Good);
    }
    for (int l = 0; l < 14; l++)
    {
        TRTCond::ExpandedIdentifier idec(-2, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
        strawstatus->setStatus(idec, TRTCond::StrawStatus::Good);
        TRTCond::ExpandedIdentifier idea(2, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
        strawstatus->setStatus(idea, TRTCond::StrawStatus::Good);
    }

    if (filename.empty())
    {
        ATH_MSG_FATAL(" Empty input file! ");
        return StatusCode::FAILURE;
    }
    std::ifstream ifs(filename.c_str());
    int bec, layer, sector, strawlayer, straw, status;
    // txt file format : bec sector straw strawlayer layer dead/alive
    int line = 0;
    ATH_MSG_INFO("StrawStatus levels:  strawlayer: " << TRTCond::ExpandedIdentifier::STRAWLAYER << " module: " << TRTCond::ExpandedIdentifier::MODULE << " layer: " << TRTCond::ExpandedIdentifier::LAYERWHEEL);
    while (ifs >> bec >> sector >> straw >> strawlayer >> layer >> status)
    {
        if ((status == 3) || (status == 4) || (status == 5))
            status = 1;
        if (status != 1)
        {
            ATH_MSG_FATAL(" The Status Temp : " << status << " IS NOT ACCEPTED, Use 1 for dead ");
            return StatusCode::FAILURE;
        }
        int level = TRTCond::ExpandedIdentifier::STRAW;
        line += 1;

        if (straw < 0)
        {
            level = TRTCond::ExpandedIdentifier::STRAWLAYER;
            straw = 0;
        }
        if (strawlayer < 0)
        {
            level = TRTCond::ExpandedIdentifier::MODULE;
            strawlayer = 0;
        }
        if (sector < 0)
        {
            level = TRTCond::ExpandedIdentifier::LAYERWHEEL;
            sector = 0;
        }
        if (layer < 0)
        {
            level = TRTCond::ExpandedIdentifier::BARRELEC;
            layer = 0;
        }
        if (bec < -2)
        {
            level = TRTCond::ExpandedIdentifier::DETECTOR;
            bec = -2;
        }

        if (level == TRTCond::ExpandedIdentifier::STRAW)
        {
            if (bec == -1 && sector >= 0 && sector < 32)
            {
                if (layer == 0)
                    deadbc0[sector]++;
                if (layer == 1)
                    deadbc1[sector]++;
                if (layer == 2)
                    deadbc2[sector]++;
            }
            if (bec == 1 && sector >= 0 && sector < 32)
            {
                if (layer == 0)
                    deadba0[sector]++;
                if (layer == 1)
                    deadba1[sector]++;
                if (layer == 2)
                    deadba2[sector]++;
            }
            if (bec == -2 && layer >= 0 && layer < 14)
                deadec[layer]++;
            if (bec == 2 && layer >= 0 && layer < 14)
                deadea[layer]++;
        }
        else
        {
            ATH_MSG_INFO("Entire Module dead: bec: " << bec << " layer: " << layer << " phi: " << sector << " stl:  " << strawlayer << " stw: " << straw << " level " << level);
        }

        TRTCond::ExpandedIdentifier id(bec, layer, sector, strawlayer, straw, level);
        // 1 means BAD
        strawstatus->setStatus(id, 1);

        // I don't know why to set this extra bit 8 here for dead straws. Been always like that. (PH)
        strawstatus->set(id, 1);
        Identifier ID = m_trtid->straw_id(bec, sector, layer, strawlayer, straw);
        set_status_temp(strawstatus, ID, status == 1 ? true : false);
    }

    ATH_MSG_INFO(" Dead straws BA layer 0");
    ATH_MSG_INFO("deadba0[] = { " << deadba0[0] << ", " << deadba0[1] << ", " << deadba0[2] << ", " << deadba0[3] << ", " << deadba0[4] << ", " << deadba0[5] << ", " << deadba0[6] << ", " << deadba0[7] << ", " << deadba0[8] << ", " << deadba0[9] << ", " << deadba0[10] << ", " << deadba0[11] << ", " << deadba0[12] << ", " << deadba0[13] << ", " << deadba0[14] << ", " << deadba0[15] << ", " << deadba0[16] << ", " << deadba0[17] << ", " << deadba0[18] << ", " << deadba0[19] << ", " << deadba0[20] << ", " << deadba0[21] << ", " << deadba0[22] << ", " << deadba0[23] << ", " << deadba0[24] << ", " << deadba0[25] << ", " << deadba0[26] << ", " << deadba0[27] << ", " << deadba0[28] << ", " << deadba0[29] << ", " << deadba0[30] << ", " << deadba0[31] << "}; ");
    ATH_MSG_INFO(" Dead straws BA layer 1 ");
    ATH_MSG_INFO("deadba1[] = { " << deadba1[0] << ", " << deadba1[1] << ", " << deadba1[2] << ", " << deadba1[3] << ", " << deadba1[4] << ", " << deadba1[5] << ", " << deadba1[6] << ", " << deadba1[7] << ", " << deadba1[8] << ", " << deadba1[9] << ", " << deadba1[10] << ", " << deadba1[11] << ", " << deadba1[12] << ", " << deadba1[13] << ", " << deadba1[14] << ", " << deadba1[15] << ", " << deadba1[16] << ", " << deadba1[17] << ", " << deadba1[18] << ", " << deadba1[19] << ", " << deadba1[20] << ", " << deadba1[21] << ", " << deadba1[22] << ", " << deadba1[23] << ", " << deadba1[24] << ", " << deadba1[25] << ", " << deadba1[26] << ", " << deadba1[27] << ", " << deadba1[28] << ", " << deadba1[29] << ", " << deadba1[30] << ", " << deadba1[31] << "};");
    ATH_MSG_INFO(" Dead straws BA layer 2");
    ATH_MSG_INFO("deadba2[]= { " << deadba2[0] << ", " << deadba2[1] << ", " << deadba2[2] << ", " << deadba2[3] << ", " << deadba2[4] << ", " << deadba2[5] << ", " << deadba2[6] << ", " << deadba2[7] << ", " << deadba2[8] << ", " << deadba2[9] << ", " << deadba2[10] << ", " << deadba2[11] << ", " << deadba2[12] << ", " << deadba2[13] << ", " << deadba2[14] << ", " << deadba2[15] << ", " << deadba2[16] << ", " << deadba2[17] << ", " << deadba2[18] << ", " << deadba2[19] << ", " << deadba2[20] << ", " << deadba2[21] << ", " << deadba2[22] << ", " << deadba2[23] << ", " << deadba2[24] << ", " << deadba2[25] << ", " << deadba2[26] << ", " << deadba2[27] << ", " << deadba2[28] << ", " << deadba2[29] << ", " << deadba2[30] << ", " << deadba2[31] << "}; ");

    ATH_MSG_INFO(" Dead straws BC layer 0");
    ATH_MSG_INFO("deadbc0[] = { " << deadbc0[0] << ", " << deadbc0[1] << ", " << deadbc0[2] << ", " << deadbc0[3] << ", " << deadbc0[4] << ", " << deadbc0[5] << ", " << deadbc0[6] << ", " << deadbc0[7] << ", " << deadbc0[8] << ", " << deadbc0[9] << ", " << deadbc0[10] << ", " << deadbc0[11] << ", " << deadbc0[12] << ", " << deadbc0[13] << ", " << deadbc0[14] << ", " << deadbc0[15] << ", " << deadbc0[16] << ", " << deadbc0[17] << ", " << deadbc0[18] << ", " << deadbc0[19] << ", " << deadbc0[20] << ", " << deadbc0[21] << ", " << deadbc0[22] << ", " << deadbc0[23] << ", " << deadbc0[24] << ", " << deadbc0[25] << ", " << deadbc0[26] << ", " << deadbc0[27] << ", " << deadbc0[28] << ", " << deadbc0[29] << ", " << deadbc0[30] << ", " << deadbc0[31] << "}; ");
    ATH_MSG_INFO(" Dead straws BC layer 1 ");
    ATH_MSG_INFO("deadbc1[] = { " << deadbc1[0] << ", " << deadbc1[1] << ", " << deadbc1[2] << ", " << deadbc1[3] << ", " << deadbc1[4] << ", " << deadbc1[5] << ", " << deadbc1[6] << ", " << deadbc1[7] << ", " << deadbc1[8] << ", " << deadbc1[9] << ", " << deadbc1[10] << ", " << deadbc1[11] << ", " << deadbc1[12] << ", " << deadbc1[13] << ", " << deadbc1[14] << ", " << deadbc1[15] << ", " << deadbc1[16] << ", " << deadbc1[17] << ", " << deadbc1[18] << ", " << deadbc1[19] << ", " << deadbc1[20] << ", " << deadbc1[21] << ", " << deadbc1[22] << ", " << deadbc1[23] << ", " << deadbc1[24] << ", " << deadbc1[25] << ", " << deadbc1[26] << ", " << deadbc1[27] << ", " << deadbc1[28] << ", " << deadbc1[29] << ", " << deadbc1[30] << ", " << deadbc1[31] << "};");
    ATH_MSG_INFO(" Dead straws BC leayer 2");
    ATH_MSG_INFO("deadbc2[]= { " << deadbc2[0] << ", " << deadbc2[1] << ", " << deadbc2[2] << ", " << deadbc2[3] << ", " << deadbc2[4] << ", " << deadbc2[5] << ", " << deadbc2[6] << ", " << deadbc2[7] << ", " << deadbc2[8] << ", " << deadbc2[9] << ", " << deadbc2[10] << ", " << deadbc2[11] << ", " << deadbc2[12] << ", " << deadbc2[13] << ", " << deadbc2[14] << ", " << deadbc2[15] << ", " << deadbc2[16] << ", " << deadbc2[17] << ", " << deadbc2[18] << ", " << deadbc2[19] << ", " << deadbc2[20] << ", " << deadbc2[21] << ", " << deadbc2[22] << ", " << deadbc2[23] << ", " << deadbc2[24] << ", " << deadbc2[25] << ", " << deadbc2[26] << ", " << deadbc2[27] << ", " << deadbc2[28] << ", " << deadbc2[29] << ", " << deadbc2[30] << ", " << deadbc2[31] << "}; ");

    ATH_MSG_INFO(" Dead straws EA");
    ATH_MSG_INFO("deadea[] = { " << deadea[0] << ", " << deadea[1] << ", " << deadea[2] << ", " << deadea[3] << ", " << deadea[4] << ", " << deadea[5] << ", " << deadea[6] << ", " << deadea[7] << ", " << deadea[8] << ", " << deadea[9] << ", " << deadea[10] << ", " << deadea[11] << ", " << deadea[12] << ", " << deadea[13] << "}; ");
    ATH_MSG_INFO(" Dead straws EC");
    ATH_MSG_INFO("deadec[] = { " << deadec[0] << ", " << deadec[1] << ", " << deadec[2] << ", " << deadec[3] << ", " << deadec[4] << ", " << deadec[5] << ", " << deadec[6] << ", " << deadec[7] << ", " << deadec[8] << ", " << deadec[9] << ", " << deadec[10] << ", " << deadec[11] << ", " << deadec[12] << ", " << deadec[13] << "}; ");

    ATH_MSG_INFO("Recording StrawStatus Container. Number of dead straws  " << line << " straws");
    if ((detStore()->record(strawstatus, m_par_strawstatuscontainerkey)) != StatusCode::SUCCESS)
    {
        ATH_MSG_ERROR("Could not record StrawStatus Container for key " << m_par_strawstatuscontainerkey);
        return StatusCode::FAILURE;
    }
    else
    {
        ATH_MSG_INFO("Recorded StrawStatus container " << m_par_strawstatuscontainerkey << " with DetStore ");
    }

    return StatusCode::SUCCESS;
}

StatusCode TRTStrawStatusWrite::readStatHTFromTextFile(const std::string &filename)
{

    StrawStatusContainer *strawstatusHT = new TRTCond::StrawStatusMultChanContainer();

    strawstatusHT->clear();

    ATH_MSG_INFO(" ***************** TRTStrawStatusWrite ************************ ");
    ATH_MSG_INFO(" readStatHTFromTextFile called with file name " << filename);

    std::ifstream ifsHT(filename.c_str());
    if (ifsHT)
    {

        const InDetDD::TRT_DetectorManager *TRTDetectorManager;
        if ((detStore()->retrieve(TRTDetectorManager)).isFailure())
        {
            ATH_MSG_FATAL("Problem retrieving TRT_DetectorManager");
        }

        // initialize detector layers with Good like in rel21 (PH: Good and Xenon is treated as the same)
        int lineXe = 0;
        TRTCond::ExpandedIdentifier iddet(0, 0, 0, 0, 0, TRTCond::ExpandedIdentifier::DETECTOR);
        strawstatusHT->setStatus(iddet, TRTCond::StrawStatus::Good);

        for (int l = 0; l < 3; l++)
        {

            TRTCond::ExpandedIdentifier idbc(-1, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
            strawstatusHT->setStatus(idbc, TRTCond::StrawStatus::Good);
            TRTCond::ExpandedIdentifier idba(1, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
            strawstatusHT->setStatus(idba, TRTCond::StrawStatus::Good);
            lineXe += 2;
        }
        for (int l = 0; l < 14; l++)
        {
            TRTCond::ExpandedIdentifier idec(-2, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
            strawstatusHT->setStatus(idec, TRTCond::StrawStatus::Good);
            TRTCond::ExpandedIdentifier idea(2, l, 0, 0, 0, TRTCond::ExpandedIdentifier::LAYERWHEEL);
            strawstatusHT->setStatus(idea, TRTCond::StrawStatus::Good);
            lineXe += 2;
        }

        int bec, layer, sector, strawlayer, straw, status;
        // txt file format : bec sector straw strawlayer layer dead/alive
        int line = 0;
        int lineAr = 0;
        int lineKr = 0;
        int lineEAr = 0;
        int lineEKr = 0;

        while (ifsHT >> bec >> sector >> strawlayer >> straw >> layer >> status)
        {
            int level = TRTCond::ExpandedIdentifier::STRAW;

            line += 1;
            if (straw < 0)
            {
                level = TRTCond::ExpandedIdentifier::STRAWLAYER;
                straw = 0;
            }
            if (strawlayer < 0)
            {
                level = TRTCond::ExpandedIdentifier::MODULE;
                strawlayer = 0;
            }
            if (sector < 0)
            {
                level = TRTCond::ExpandedIdentifier::LAYERWHEEL;
                sector = 0;
            }
            if (layer < 0)
            {
                level = TRTCond::ExpandedIdentifier::BARRELEC;
                layer = 0;
            }
            if (bec < -2)
            {
                level = TRTCond::ExpandedIdentifier::DETECTOR;
                bec = -2;
            }

            ATH_MSG_INFO("Layer/Wheel with Argon: bec: " << bec << " layer: " << layer << " phi: " << sector << " stl:  " << strawlayer << " stw: " << straw << " status (" << TRTCond::StrawStatus::Argon << " is Argon): " << status << " level (" << TRTCond::ExpandedIdentifier::LAYERWHEEL << " is LAYERWHEEL) " << level);
            TRTCond::ExpandedIdentifier id(bec, layer, sector, strawlayer, straw, level);
            if (status == 4)
            { // Argon
                strawstatusHT->setStatus(id, TRTCond::StrawStatus::Argon);
                lineAr += 1;
                lineXe -= 1;
            }
            else if (status == 5)
            { // Krypton
                strawstatusHT->setStatus(id, TRTCond::StrawStatus::Krypton);
                lineKr += 1;
                lineXe -= 1;
            }
            else if (status == 6)
            { // EmulateArgon
                strawstatusHT->setStatus(id, TRTCond::StrawStatus::EmulateArgon);
                lineEAr += 1;
            }
            else if (status == 7)
            { // EmulateKrypton
                strawstatusHT->setStatus(id, TRTCond::StrawStatus::EmulateKrypton);
                lineEKr += 1;
                lineXe -= 1;
            }
            else
            {
                ATH_MSG_FATAL(" The HT Status: " << status << " IS NOT ACCEPTED, 4 for Argon, 5 for Krypton!! "
                                                 << " Or 6 or 7 for emulated types!");
                return StatusCode::FAILURE;
            }
        }
        ATH_MSG_INFO("All the rest is running Xenon");
        ATH_MSG_INFO("HT Status. Read  " << line << " layers. " << "    Xenon: " << lineXe << " Argon:  " << lineAr << "  Krypton: " << lineKr << " EmulateArgon: " << lineEAr << " EmulateKrypton: " << lineEKr);

        if ((detStore()->record(strawstatusHT, m_par_strawstatusHTcontainerkey)) != StatusCode::SUCCESS)
        {
            ATH_MSG_ERROR("Could not record StrawStatusHT Container for key " << m_par_strawstatusHTcontainerkey);
            return StatusCode::FAILURE;
        }
        else
        {
            ATH_MSG_INFO("Recorded StrawStatusHT container " << m_par_strawstatusHTcontainerkey << " with DetStore ");
        }
    }
    else
    {
        ATH_MSG_WARNING(" The HT Status input file not found ");
    }
    return StatusCode::SUCCESS;
}

StatusCode TRTStrawStatusWrite::readStatPermFromTextFile(const std::string &filename)
{

    StrawStatusContainer *strawstatuspermanent = new TRTCond::StrawStatusMultChanContainer();
    strawstatuspermanent->clear();

    ATH_MSG_INFO(" readStatPermFromTextFile called with file name " << filename);

    std::ifstream ifspermanent(filename.c_str());

    if (ifspermanent)
    {

        // initialize all straws with status 'Good'
        for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
        {
            Identifier id = m_trtid->straw_id(*it, 0);
            int m = m_trtid->phi_module(id);
            int n = m_trtid->barrel_ec(id);
            int l = m_trtid->layer_or_wheel(id);
            int nStrawsInLayer = m_trtid->straw_max(*it);
            int sl = m_trtid->straw_layer(id);
            for (int i = 0; i <= nStrawsInLayer; i++)
            {
                id = m_trtid->straw_id(*it, i);
                TRTCond::ExpandedIdentifier expid(n, l, m, sl, i, TRTCond::ExpandedIdentifier::STRAW);
                strawstatuspermanent->setStatus(expid, TRTCond::StrawStatus::Good);
            }
        }
        int bec, layer, sector, strawlayer, straw, status;
        // txt file format : bec sector straw strawlayer layer dead/alive
        int line = 0;
        int nmodu = 0;
        while (ifspermanent >> bec >> sector >> straw >> strawlayer >> layer >> status)
        {
            if ((status == 3) || (status == 4) || (status == 5))
                status = 1;
            if (status != 1)
            {
                ATH_MSG_FATAL(" The Status Permanent: " << status << " IS NOT ACCEPTED, Use 1 for dead ");
                return StatusCode::FAILURE;
            }
            int level = TRTCond::ExpandedIdentifier::STRAW;
            line += 1;

            // Expect the line to ask for either an entire modules or an individual straw to be turned off
            if (strawlayer < 0)
            {
                std::cout << " Read permanent dead module from txt: " << bec << " ,sector " << sector << " ,straw " << straw << " ,strawlayer " << strawlayer << " ,layer " << layer << " ,status " << status << std::endl;
                for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
                {
                    Identifier id = m_trtid->straw_id(*it, 0);
                    int m = m_trtid->phi_module(id);
                    int n = m_trtid->barrel_ec(id);
                    int l = m_trtid->layer_or_wheel(id);
                    if (n != bec || m != sector || l != layer)
                        continue;

                    int nStrawsInLayer = m_trtid->straw_max(*it);
                    int sl = m_trtid->straw_layer(id);
                    for (int i = 0; i <= nStrawsInLayer; i++)
                    {
                        id = m_trtid->straw_id(*it, i);
                        TRTCond::ExpandedIdentifier expid(bec, layer, sector, sl, i, level);
                        strawstatuspermanent->setStatus(expid, TRTCond::StrawStatus::Dead);
                    }
                }
                nmodu++;
            }
            else
            {
                TRTCond::ExpandedIdentifier expid(bec, layer, sector, strawlayer, straw, level);
                strawstatuspermanent->setStatus(expid, TRTCond::StrawStatus::Dead);
            }

            ATH_MSG_DEBUG("Permanent dead ids: bec: " << bec << " layer: " << layer << " phi: " << sector << " stl:  " << strawlayer << " stw: " << straw);
        } // end reading input text file

        ATH_MSG_INFO(" Record Permanent Straw Status. Dead modules : " << nmodu << "  Number of lines:  " << line << " of dead identifiers ");
        if ((detStore()->record(strawstatuspermanent, m_par_strawstatuspermanentcontainerkey)) != StatusCode::SUCCESS)
        {
            ATH_MSG_ERROR("Could not record StrawStatusPermanent Container for key " << m_par_strawstatuspermanentcontainerkey);
            return StatusCode::FAILURE;
        }
        else
        {
            ATH_MSG_INFO("Recorded StrawStatusPermanent container " << m_par_strawstatuspermanentcontainerkey << " with DetStore ");
        }
    }
    return StatusCode::SUCCESS;
}
