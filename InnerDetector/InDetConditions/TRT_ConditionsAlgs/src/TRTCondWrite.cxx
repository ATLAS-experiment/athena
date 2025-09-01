/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include <iostream>
#include <string>
#include "TRT_ConditionsAlgs/TRTCondWrite.h"
#include "TRT_ConditionsData/BasicRtRelation.h"
#include "TRT_ConditionsData/DinesRtRelation.h"
#include "TRT_ConditionsData/BinnedRtRelation.h"
#include "TRT_ConditionsData/RtRelationFactory.h"
#include "StoreGate/WriteCondHandle.h"

/**  @file TRTCondWrite.cxx
 *   CondAlg for publishing TRT Calibration constants from a text file to CondStore
 *
 *
 *
 * @author Peter Hansen <phansen@nbi.dk>
 **/
TRTCondWrite::TRTCondWrite(const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) {}

StatusCode TRTCondWrite::initialize()
{

    ATH_MSG_DEBUG("TRTCondWrite::initialize() called");

    // Get ID helper
    ATH_CHECK(detStore()->retrieve(m_trtid, "TRT_ID"));

    // Get condSvc
    ATH_CHECK(m_condSvc.retrieve());

    // Format of input text file
    int format = 0;
    if (m_par_caltextfile != "")
    {
        ATH_MSG_INFO(" input text file supplied " << m_par_caltextfile);
        if (StatusCode::SUCCESS != checkTextFile(m_par_caltextfile, format))
        {
            ATH_MSG_INFO("Could not find or could not read text file" << m_par_caltextfile);
            return StatusCode::SUCCESS;
        }
        ATH_MSG_INFO(" Found format " << format << " in text file " << m_par_caltextfile);
    }
    else
    {
        ATH_MSG_ERROR(" No input text file supplied. Initialization done. ");
        return StatusCode::FAILURE;
    }

    // Write keys
    // If an input text file is specified, initialize write keys. The corresponding folders must be blocked.

    if (m_par_caltextfile != "" && format > 0)
    {

        ATH_CHECK(m_rtWriteKey.initialize());
        if (m_condSvc->regHandle(this, m_rtWriteKey).isFailure())
            ATH_MSG_ERROR("unable to register WriteCondHandle " << m_rtWriteKey.fullKey());
        ATH_MSG_INFO("Registered WriteCondHandle " << m_rtWriteKey.fullKey());
        ATH_CHECK(m_t0WriteKey.initialize());
        if (m_condSvc->regHandle(this, m_t0WriteKey).isFailure())
            ATH_MSG_ERROR("unable to register WriteCondHandle " << m_t0WriteKey.fullKey());
        ATH_MSG_INFO("Registered WriteCondHandle " << m_t0WriteKey.fullKey());
    }
    ATH_MSG_INFO(" Initilization done with WriteKey Registraton ");

    return StatusCode::SUCCESS;
}

StatusCode TRTCondWrite::execute()
{

    // Read from text file?
    if (!m_par_caltextfile.empty())
    {
        std::ifstream infile(m_par_caltextfile.value().c_str());
        if (infile)
        {
            ATH_MSG_INFO(" Read calibration constants from text file " << m_par_caltextfile);
            int format = 0;
            if (StatusCode::SUCCESS != readTextFile(m_par_caltextfile, format))
            {
                ATH_MSG_FATAL("Could not read calibration objects from text file " << m_par_caltextfile);
                return StatusCode::FAILURE;
            }
        }
        else
        {
            ATH_MSG_ERROR("Input file does not exist " << m_par_caltextfile.value().c_str());
            return StatusCode::FAILURE;
        }
        infile.close();
    }
    else
    {
        ATH_MSG_ERROR("No input filename supplied ");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
    ;
}

StatusCode TRTCondWrite::finalize()
{
    return StatusCode::SUCCESS;
}

StatusCode TRTCondWrite::checkTextFile(const std::string &filename, int &format)
{

    StatusCode sc = StatusCode::SUCCESS;
    std::ifstream infile(filename.c_str());
    if (!infile)
    {
        sc = StatusCode::FAILURE;
    }
    else
    {
        // read the format tag. if none, default to 0
        format = 0;
        char line[512];
        infile.getline(line, 512);
        std::string linestring(line);
        size_t pos = linestring.find("Fileformat");
        if (pos != std::string::npos)
        {
            sscanf(line, "# Fileformat=%d", &format);
        }
        else
        {
            ATH_MSG_WARNING("Input file has no Fileformat identifier. Assuming format=0.");
            // 'rewind' the file

            infile.close();
            infile.open(filename.c_str());
        }
    }
    infile.close();
    return sc;
}

StatusCode TRTCondWrite::readTextFile(const std::string &filename, int &format)
{

    StatusCode sc = StatusCode::SUCCESS;
    std::ifstream infile(filename.c_str());
    if (!infile)
    {
        ATH_MSG_ERROR("Cannot find input file " << filename);
        sc = StatusCode::FAILURE;
    }
    else
    {
        // read the format tag. if none, default to 0
        format = 0;
        char line[512];
        infile.getline(line, 512);
        std::string linestring(line);
        size_t pos = linestring.find("Fileformat");
        if (pos != std::string::npos)
        {
            sscanf(line, "# Fileformat=%d", &format);
        }
        else
        {
            ATH_MSG_WARNING("Input file has no Fileformat identifier. Assuming format=1");
            // 'rewind' the file

            infile.close();
            infile.open(filename.c_str());
        }
        ATH_MSG_INFO("Reading calibration data from text file " << filename << " format " << format);
        // force format 1 here
        sc = readTextFile_Format1(infile);
    }
    infile.close();
    return sc;
}

StatusCode TRTCondWrite::readTextFile_Format1(std::istream &infile)
{

    enum ReadMode
    {
        ReadingRtRelation,
        ReadingStrawT0,
        ReadingGarbage
    };
    ReadMode readmode = ReadingGarbage;

    // Make containers for access via ReadCondHandle for the reconstruction.
    std::unique_ptr<RtRelationContainer> rtCdo{std::make_unique<RtRelationContainer>()};
    std::unique_ptr<StrawT0Container> t0Cdo{std::make_unique<StrawT0Container>()};

    char line[512];
    int nrtrelations(0), nstrawt0(0);
    while (infile.getline(line, 512))
    {
        if (line[0] == '#')
        {
            // line with tag
            std::string linestring(line);
            if (linestring.find("RtRelation") != std::string::npos)
            {
                readmode = ReadingRtRelation;
                ATH_MSG_INFO(" Found line with Rt tag ");
            }
            else if (linestring.find("StrawT0") != std::string::npos)
            {
                readmode = ReadingStrawT0;
                ATH_MSG_INFO(" Found line with T0 tag ");
            }
            else
                readmode = ReadingGarbage;
        }
        else if (readmode != ReadingGarbage)
        {
            std::istringstream is(line);
            // read the id
            TRTCond::ExpandedIdentifier id;
            is >> id;
            // read the semicolon that end the id
            char dummy;
            is >> dummy;

            // read the object
            if (readmode == ReadingRtRelation)
            {

                TRTCond::RtRelation *rt = TRTCond::RtRelationFactory::readFromFile(is);
                rtCdo->set(id, rt);
                delete rt;
                ++nrtrelations;
            }
            else if (readmode == ReadingStrawT0)
            {

                float t0(0), t0err(0);
                is >> t0 >> t0err;
                if (t0 > 0)
                {
                    t0Cdo->setT0(id, t0, t0err);
                    ++nstrawt0;
                }
            }
        }
    }

    // Check that containers were filled

    size_t t0footprint = t0Cdo->footprint();
    size_t rtfootprint = rtCdo->footprint();

    ATH_MSG_INFO("read " << nstrawt0 << " t0 and " << nrtrelations << " rt from file. "
                         << " t0/rt footprints " << t0footprint << " / " << rtfootprint << " t0 footprint after crunch " << t0Cdo->footprint());

    // Record the containers for access via ReadCondHandle during reconstruction
    const EventIDRange rangeW = IOVInfRange();
    SG::WriteCondHandle<RtRelationContainer> rtWriteHandle{m_rtWriteKey};
    if (rtWriteHandle.isValid())
    {
        ATH_MSG_DEBUG(" RtRelationContainer already available ");
        return StatusCode::SUCCESS;
    }

    if (rtWriteHandle.record(rangeW, std::move(rtCdo)).isFailure())
    {
        ATH_MSG_ERROR("Could not record RT Container for key " << m_rtWriteKey.fullKey() << " with WriteHandle ");
        return StatusCode::FAILURE;
    }
    else
    {
        ATH_MSG_INFO("Recorded RT Container for key " << m_rtWriteKey.fullKey() << " with range " << rtWriteHandle.getRange());
    }

    SG::WriteCondHandle<StrawT0Container> t0WriteHandle{m_t0WriteKey};
    if (t0WriteHandle.isValid())
    {
        ATH_MSG_DEBUG(" StrawT0Container already available ");
        return StatusCode::SUCCESS;
    }

    if (t0Cdo->initialize().isFailure())
        ATH_MSG_WARNING("Could not initialize T0 Container for key " << m_t0WriteKey.fullKey());

    if (t0WriteHandle.record(rangeW, std::move(t0Cdo)).isFailure())
    {
        ATH_MSG_ERROR("Could not record T0 Container for key " << m_t0WriteKey.fullKey() << " with WriteHandle ");
        return StatusCode::FAILURE;
    }
    else
    {
        ATH_MSG_INFO("Recorded T0 Container for key " << m_t0WriteKey.fullKey() << " with range " << t0WriteHandle.getRange());
    }

    return StatusCode::SUCCESS;
}

TRTCond::ExpandedIdentifier TRTCondWrite::trtcondid(const Identifier &id, int level) const
{
    return TRTCond::ExpandedIdentifier(m_trtid->barrel_ec(id), m_trtid->layer_or_wheel(id),
                                       m_trtid->phi_module(id), m_trtid->straw_layer(id),
                                       m_trtid->straw(id), level);
}
EventIDRange TRTCondWrite::IOVInfRange() const
{
    const EventIDBase start{1, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, 0};
    const EventIDBase stop{EventIDBase::UNDEFNUM - 1, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM - 1};
    return EventIDRange{start, stop};
}
