/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COSMICTRIGGERTIMETOOL_H
#define COSMICTRIGGERTIMETOOL_H

#include "AthenaKernel/ITriggerTime.h" 
#include "GaudiKernel/IIncidentListener.h"
#include "AthenaBaseComps/AthAlgTool.h"

class ComTime;

class CosmicTriggerTimeTool : public extends<AthAlgTool, ITriggerTime, IIncidentListener>
{

public:
   CosmicTriggerTimeTool(const std::string& type, 
			 const std::string& name,
			 const IInterface* parent);


   virtual StatusCode initialize() override;

   virtual ~CosmicTriggerTimeTool() {}

   /// returns the time offset of the current trigger
   virtual double time() override;

   virtual void handle(const Incident& incident) override;

    double larTime(); 
    double trackRecordTime()   ; 

    void setComTime(const ComTime* comTime ) 
	{ 
	  m_comTime = comTime; 
	  return; 
	}

private: 

   double m_time; 
   bool m_newEvent ; 
   const ComTime* m_comTime; 
   bool m_useLArTime;
};


#endif 
