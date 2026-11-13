/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcByteStreamRawData.h
 *
 *  Created on: Jun 29, 2009
 *      Author: leite
 *
 *
 */

#ifndef ZDCBYTESTREAMRAWDATA_H_
#define ZDCBYTESTREAMRAWDATA_H_

#include <map>
#include <string>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthContainers/DataVector.h"
#include "Gaudi/Property.h"
#include "ZdcByteStream/ZdcDefs.h"

class ISvcLocator;
class StatusCode;

class ZdcDigits;
class ZdcDigitsCollection;

class ZdcByteStreamRawData: public AthAlgorithm
{
public:
	ZdcByteStreamRawData(const std::string& name, ISvcLocator* pSvcLocator);

	virtual StatusCode execute(const EventContext& ctx) override;

private:
	typedef std::map<unsigned int, const ZdcDigits*> ZdcDigitsMap;

	void printVec(const std::vector<int>& vec) const;
	void printZdcDigits() const;

	/// ZDC Container and collection
	/// StoreGate key
	std::string m_ZdcDigitsLocation;
	Gaudi::Property<std::string> m_ZdcDigitsCollectionLocation{this, "ZdcDigitsCollectionLocation", ZdcDefs::ZdcDigitsCollectionLocation};

	void setupZdcDigitsMap(const ZdcDigitsCollection* jeCollection);

	///REMOVE Force number of LUT slices
	///REMOVE
	Gaudi::Property<int> m_forceSlicesLut{this, "ForceSlicesLUT", 0};
	///REMOVE Force number of FADC slices
	///REMOVE
	Gaudi::Property<int> m_forceSlicesFadc{this, "ForceSlicesFADC", 0};
	ZdcDigitsMap m_ZdcDigitsMap;
};

#endif /* ZDCBYTESTREAMRAWDATA_H_ */
