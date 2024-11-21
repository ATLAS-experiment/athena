// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimNNMap.cxx
 * @author Elliott Cheu
 * @date April 27, 2021
 * @brief See FPGATrackSimNNMap.h
 */

#include "FPGATrackSimMaps/FPGATrackSimNNMap.h"

#include <string>
#include <sstream>
#include <exception>
#include <AsgMessaging/MessageCheck.h>

using namespace std;
using namespace asg::msgUserCode;

///////////////////////////////////////////////////////////////////////////////
// Constructor/Desctructor
///////////////////////////////////////////////////////////////////////////////



FPGATrackSimNNMap::FPGATrackSimNNMap(const std::string & filepath) :
    m_weightsFileName(filepath)
{
    // Open file with NN weights
    std::ifstream input_cfg(m_weightsFileName.c_str());
    if (input_cfg.is_open()) 
      ANA_MSG_INFO("Opened file: " << m_weightsFileName);
    else {
        ANA_MSG_FATAL("Unable to open file: " << m_weightsFileName);
        throw ("FPGATrackSimNNMap could not open " + m_weightsFileName);
    }

}


const std::string& FPGATrackSimNNMap::getNNMap() const {return m_weightsFileName;}
