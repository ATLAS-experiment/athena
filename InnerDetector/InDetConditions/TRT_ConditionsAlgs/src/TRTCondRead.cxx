/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include <iostream>
#include <string>
#include "TRTCondRead.h"
#include "TRT_ConditionsData/BasicRtRelation.h"
#include "TRT_ConditionsData/DinesRtRelation.h"
#include "TRT_ConditionsData/BinnedRtRelation.h"
#include "TRT_ConditionsData/RtRelationFactory.h"

/**  @file TRTCondRead.cxx
 *   Algoritm for dumping TRT Calibration constants from db to text file
 *
 *
 * @author Peter Hansen <phansen@nbi.dk>
 **/
TRTCondRead::TRTCondRead(const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) {}

StatusCode TRTCondRead::initialize()
{

    ATH_MSG_DEBUG("TRTCondRead::initialize() called");

    ATH_CHECK(m_TRTCalDbTool.retrieve());

    // Get TRT ID helper
    ATH_CHECK(detStore()->retrieve(m_trtid, "TRT_ID"));

    return StatusCode::SUCCESS;
}

StatusCode TRTCondRead::execute()
{

    // Write text file.
    if (!m_par_caloutputfile.empty())
    {
        ATH_MSG_INFO(" Writing calibration constants to text file " << m_par_caloutputfile);
        std::ofstream outfile(m_par_caloutputfile);
        // if ((m_par_caloutputfile.value()).contains("caliboutput")) TO BE CORRECTED when C++23
        if (m_par_caloutputfile.value().find("caliboutput") != std::string::npos)
        {
            ATH_MSG_INFO("File name: "<< m_par_caloutputfile);
            ATH_CHECK(writeCalibTextFile(outfile));
        }
        // else if ((m_par_caloutputfile.value()).contains("erroroutput")) TO BE CORRECTED when C++23
        else if (m_par_caloutputfile.value().find("erroroutput") != std::string::npos)
        {
            ATH_MSG_INFO("File name: "<< m_par_caloutputfile);
            ATH_CHECK(writeErrorTextFile(outfile));
        }
        else
        {
            ATH_MSG_WARNING("The file name must contain either 'caliboutput' or 'erroroutput' ");
        }
        outfile.close();
    }

    // Quick spot check of the T0s
    ATH_MSG_INFO(" Spot check of T0 in every 10'th straw_layer in phi_modules 0, 10, 20 and 30");
    for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
    {
        Identifier id = m_trtid->straw_id(*it, 0);
        if (m_trtid->phi_module(id) % 10 == 0 && m_trtid->straw_layer(id) % 10 == 0)
        {
            float t0 = m_TRTCalDbTool->getT0(id);
            ATH_MSG_INFO(" bec: " << m_trtid->barrel_ec(id) << " lay: " << m_trtid->layer_or_wheel(id) << " phi: " << m_trtid->phi_module(id) << " slay: " << m_trtid->straw_layer(id) << " t0: " << t0);
        }
    }

    ATH_MSG_INFO("");
    ATH_MSG_INFO("List of straws with T0=0 :");

    int nzeros = 0;
    for (std::vector<Identifier>::const_iterator it = m_trtid->straw_layer_begin(); it != m_trtid->straw_layer_end(); ++it)
    {
        int nStrawsInLayer = m_trtid->straw_max(*it);
        for (int i = 0; i <= nStrawsInLayer; i++)
        {
            Identifier strid = m_trtid->straw_id(*it, i);
            if (m_TRTCalDbTool->getT0(strid) == 0)
            {
                ATH_MSG_INFO(" bec: " << m_trtid->barrel_ec(strid) << " lay: " << m_trtid->layer_or_wheel(strid)
                                      << " phi: " << m_trtid->phi_module(strid) << " slay: " << m_trtid->straw_layer(strid) << " straw: " << m_trtid->straw(strid));
                nzeros++;
            }
        }
    }

    if (nzeros > 10)
    {
        ATH_MSG_INFO(" Several cases of T0=0. Check the input text file and assign default T0 values instead of zero ");
    }
    ATH_MSG_INFO("");

    return StatusCode::SUCCESS;
}

StatusCode TRTCondRead::finalize()
{
    return StatusCode::SUCCESS;
}

StatusCode TRTCondRead::writeCalibTextFile(std::ostream &outfile) const
{
    const RtRelationContainer *rtContainer = m_TRTCalDbTool->getRtContainer();
    const StrawT0Container *t0Container = m_TRTCalDbTool->getT0Container();

    // first store rtrelations
    outfile << "# Rtrelation" << std::endl;
    RtRelationContainer::FlatContainer rtrelations;
    rtContainer->getall(rtrelations);
    for (RtRelationContainer::FlatContainer::iterator it = rtrelations.begin(); it != rtrelations.end(); ++it)
    {
        // write the identifier
        outfile << it->first << " : ";
        // write the rt-relation via the factory
        TRTCond::RtRelationFactory::writeToFile(outfile, **(it->second));
        outfile << std::endl;
    }

    // now store the t0s
    outfile << "# StrawT0" << std::endl;
    StrawT0Container::FlatContainer packedstrawdata;
    t0Container->getall(packedstrawdata);
    float t0(0), t0err(0);
    for (TRTCond::StrawT0Container::FlatContainer::iterator it = packedstrawdata.begin(); it != packedstrawdata.end(); ++it)
    {
        const TRTCond::ExpandedIdentifier &calid = it->first;
        t0Container->unpack(calid, *it->second, t0, t0err);
        outfile << calid << " : " << t0 << " " << t0err << std::endl;
    }

    return StatusCode::SUCCESS;
}

StatusCode TRTCondRead::writeErrorTextFile(std::ostream &outfile) const
{
    const RtRelationContainer *errContainer = m_TRTCalDbTool->getErrContainer();
    const RtRelationContainer *slopeContainer = m_TRTCalDbTool->getSlopeContainer();

    // then store errors2d
    outfile << "# RtErrors" << std::endl;
    RtRelationContainer::FlatContainer errors;
    errContainer->getall(errors);
    for (RtRelationContainer::FlatContainer::iterator it = errors.begin();
         it != errors.end(); ++it)
    {
        // write the identifier
        outfile << it->first << " : ";
        // write the errors via the factory
        TRTCond::RtRelationFactory::writeToFile(outfile, **(it->second));
        outfile << std::endl;
    }

    // then store slopes
    outfile << "# RtSlopes" << std::endl;
    RtRelationContainer::FlatContainer slopes;
    slopeContainer->getall(slopes);
    for (RtRelationContainer::FlatContainer::iterator it = slopes.begin();
         it != slopes.end(); ++it)
    {
        // write the identifier
        outfile << it->first << " : ";
        // write the slopes via the factory
        TRTCond::RtRelationFactory::writeToFile(outfile, **(it->second));
        outfile << std::endl;
    }

    return StatusCode::SUCCESS;
}

TRTCond::ExpandedIdentifier TRTCondRead::trtcondid(const Identifier &id, int level) const
{
    return TRTCond::ExpandedIdentifier(m_trtid->barrel_ec(id), m_trtid->layer_or_wheel(id),
                                       m_trtid->phi_module(id), m_trtid->straw_layer(id),
                                       m_trtid->straw(id), level);
}
