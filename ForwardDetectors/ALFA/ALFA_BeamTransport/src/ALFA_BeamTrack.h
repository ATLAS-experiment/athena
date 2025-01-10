/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

 #ifndef ALFA_BeamTrack_H
 #define ALFA_BeamTrack_H

//#include "baseAnalysis.h"
#include "GaudiKernel/MsgStream.h"

#include "AthenaBaseComps/AthAlgorithm.h"

#include "ALFA_FPConfig.h"

//FPTracker stuff
#include "FPTracker/setupBeamline.h"
#include "FPTracker/Beamline.h"
#include "FPTracker/Particle.h"
#include "FPTracker/getConfigFile.h"
#include "FPTracker/getMagnetConfigFiles.h"
#include "FPTracker/getAlfaMagnetConfigFiles.h"
#include "FPTracker/ConfigData.h"
//#include "FPTracker/readConfigData.h"
#include "FPTracker/FPTrackerConstants.h"
#include "FPTracker/Point.h"
#include <memory>

class ALFA_BeamTrack
{
  
  public:
    
    ALFA_BeamTrack() = default;
    ~ALFA_BeamTrack() = default;
    int initialize(const FPConfig &ConfigValues);
    	
	/**calculates the particle position and momentum at RP
	run first CalculatePosRP() then posRP() and momRP()*/
    	int CalculatePosRP(FPTracker::Particle particle);
	/**Gives Back Position*/
	const FPTracker::Point& PosRP();
	/**gives back Momentum*/
	const FPTracker::Point& MomRP();
    
  private:

    std::string m_ConfDir;
    FPTracker::ConfigData m_ConfigData;
    bool m_AlfaTwiss{true};
    int m_ip{1};
    int m_Magver{3};

    FPTracker::Side m_Side{FPTracker::beam1};
    std::shared_ptr< std::ifstream >  m_Magfile;
  
    //FPTracker::Particle m_Particle;
    FPTracker::Point m_PosParticleRP;
    FPTracker::Point m_MomParticleRP;

    FPTracker::Beamline m_Beamline_1;
    FPTracker::Beamline m_Beamline_2;
};

#endif
