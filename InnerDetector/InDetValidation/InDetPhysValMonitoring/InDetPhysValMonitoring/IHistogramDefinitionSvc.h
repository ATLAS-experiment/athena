/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
//  IHistogramDefinitionSvc.h
//
//  Created by sroe on 07/07/2015.
//

#ifndef IHistogramDefinitionSvc_h
#define IHistogramDefinitionSvc_h
#include <limits>
#include <string_view>
#include <utility>
#include "GaudiKernel/IInterface.h"

class SingleHistogramDefinition;
///Interface class to get the histogram definition for a named histogram in a given directory
class IHistogramDefinitionSvc:virtual public IInterface{
public:
    DeclareInterfaceID(IHistogramDefinitionSvc,1,0);

    ///Format of the data source holding the histogram definition
	enum Formats{UNKNOWN,TEXT_XML,TEXT_PLAIN,NFORMATS};
	///Virtual destructor does nothing
	virtual ~IHistogramDefinitionSvc(){}
	///typedef for axes limits, (lower bound, upper bound)
	typedef std::pair<float, float> axesLimits_t ;
	///Return a histogram definition, retrieved by histogram identifier (and directory name, if supplied)
	virtual SingleHistogramDefinition definition(std::string_view name, std::string_view  dirName="") const =0;
	///Return Histogram type (TH1, TH2 etc) by histogram identifier (and directory name, if supplied)
	virtual std::string histoType(std::string_view name, std::string_view dirName="") const = 0;
	///Return Histogram title by histogram identifier (and directory name, if supplied)
	virtual std::string title(std::string_view name, std::string_view dirName="") const =0;
	///Return number of x bins by histogram identifier (and directory name, if supplied)
	virtual unsigned int nBinsX(std::string_view name, std::string_view dirName="") const = 0;
	///Return number of y bins by histogram identifier (and directory name, if supplied); default returns 0 for 1-D histos
	virtual unsigned int nBinsY(std::string_view /*name*/, std::string_view /*dirName*/="") const { return 0; }
	///Return number of z bins by histogram identifier (and directory name, if supplied); default returns 0 for 1-D histos
	virtual unsigned int nBinsZ(std::string_view /*name*/, std::string_view /*dirName*/="") const { return 0; }
	///Return x axes (lo,hi) by histogram identifier (and directory name, if supplied)
	virtual axesLimits_t xLimits(std::string_view name, std::string_view dirName="") const = 0;
	///Return y axes (lo,hi) by histogram identifier (and directory name, if supplied). Default returns (nan,nan).
	virtual axesLimits_t yLimits(std::string_view  /*name*/, std::string_view /*dirName*/="") const {return std::make_pair(std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN());}
	///Return z axes (lo,hi) by histogram identifier (and directory name, if supplied)
	virtual axesLimits_t zLimits(std::string_view name, std::string_view dirName="") const = 0;
	///Return x-axis title by histogram identifier (and directory name, if supplied)
	virtual std::string xTitle(std::string_view name, std::string_view dirName="") const = 0;
	///Return y-axis title by histogram identifier (and directory name, if supplied)
	virtual std::string yTitle(std::string_view name, std::string_view dirName="") const = 0;
	///Return z-axis title by histogram identifier (and directory name, if supplied)
	virtual std::string zTitle(std::string_view name, std::string_view dirName="") const = 0;
	//virtual bool initialise()=0;
	
};

#endif
