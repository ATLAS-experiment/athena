// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimNNMAP_H
#define FPGATrackSimNNMAP_H

/**
 * @file FPGATrackSimNNMap.h
 * @author Elliott Cheu
 * @date April 28, 2021
 * @brief Map for NN tracking
 *
 * A connection map is a text file that maps the first stage sectors to all possible second stage sectors
 *
 * The first line indicates the first stage sector ID and the following numbers are the IDs of all the 
 * matching second stage, i.e.:
 *
 * First_stage_sector_ID Second_stage_sector_ID_1 Second_stage_sector_ID_2 ... Second_stage_sector_ID_N
 */

#include <fstream>
#include <string>
#include <memory>

class FPGATrackSimNNMap
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        FPGATrackSimNNMap(const std::string & filepath);

        ///////////////////////////////////////////////////////////////////////
        // Simple Getters/Setters

        const std::string& getNNMap() const;

        ///////////////////////////////////////////////////////////////////////
        // Main Interface Functions

    private:

        ///////////////////////////////////////////////////////////////////////
        // Member Variables
        std::string m_weightsFileName;

        ///////////////////////////////////////////////////////////////////////
        // Helper Functions

};

#endif // FPGATrackSimNNMAP_H
