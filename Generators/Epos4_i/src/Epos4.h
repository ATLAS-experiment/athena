/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORMODULESEPOS_H
#define GENERATORMODULESEPOS_H

#include "GeneratorModules/GenModule.h"
#include <vector>
#include <string>


/**
@class Epos4
@brief This code is used to get an Epos4 Monte Carlo event.
@author Andrii Verbytskyi
*/

class Epos4: public GenModule {
public:
    Epos4(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~Epos4() = default;

    virtual StatusCode genInitialize();
    virtual StatusCode callGenerator();
    virtual StatusCode genFinalize();
    virtual StatusCode fillEvt(HepMC::GenEvent* evt);

protected:

    std::string create_file(const std::string&  filein);

    // event counter
    int m_events;

    // setable properties
    double      m_beamMomentum;
    double      m_targetMomentum;

    //Gen_tf run args.
    IntegerProperty m_dsid{this, "Dsid", 999999};

    std::vector<long int> m_seeds;
};

#endif

