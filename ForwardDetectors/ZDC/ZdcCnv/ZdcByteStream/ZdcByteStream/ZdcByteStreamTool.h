/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcByteZtreamTool.h
 *
 *  Created on: May 27, 2009
 *      Author: leite
 *              based on the L1Calo ByteStream Tool
 *
 */

#ifndef ZDCBYTESTREAMTOOL_H
#define ZDCBYTESTREAMTOOL_H

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "AthContainers/DataVector.h"
#include "eformat/SourceIdentifier.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"

class ZdcID;

class IInterface;
class InterfaceID;
class StatusCode;

template<class T> class FullEventAssembler;

//Use ZdcDigits onde it's fixed
//class ZdcRdo;
//class ZdcRdoCollection;
class ZdcDigits;
class ZdcDigitsCollection;

class ZdcSrcIdMap;
class ZdcPpmSubBlock;

/** Tool to perform ROB fragments to ZdcDigits and from ZdcDigits
 *  to raw data conversions.
 *
 *  Based on ROD Tool from Level1 from  Peter Faulkner
 */

class ZdcByteStreamTool: public AthAlgTool
{

public:
	ZdcByteStreamTool(const std::string& type,
						  const std::string& name,
						  const IInterface* parent);

	/// AlgTool InterfaceID
	static const InterfaceID& interfaceID();

	virtual StatusCode initialize() override;
	virtual StatusCode finalize() override;

	/// Convert ROB fragments to ZdcCollection
	StatusCode convert(	const IROBDataProviderSvc::VROBFRAG& robFrags,
							ZdcDigitsCollection* ttCollection);

	/// Convert ZdcCollection to ROB fragments
	static StatusCode convert(const ZdcDigitsCollection* ttCollection,
	                       RawEventWrite* re);

	/// Return reference to vector with all possible Source Identifiers
	const std::vector<uint32_t>& sourceIDs();

private:
//	typedef DataVector<ZdcDigits> ZdcDigitsCollection;
	typedef std::map<unsigned int, ZdcDigits*> ZdcDigitsMap;
	typedef std::map<unsigned int, const ZdcDigits*> ZdcDigitsMapConst;
	typedef IROBDataProviderSvc::VROBFRAG::const_iterator ROBIterator;
	typedef OFFLINE_FRAGMENTS_NAMESPACE::PointerType RODPointer;

	/// Add compression stats to totals
	void addCompStats(const std::vector<uint32_t>& stats);
	/// Print compression stats
	void printCompStats() const;
	/// Print a vector
	void printVec(const std::vector<int>& vec) const;

	/// Sub_block header version
	//int m_version;
	/// Data compression format
	Gaudi::Property<int> m_dataFormat{this, "DataFormat", 1, "Format identifier (0-3) in sub-block header"};
	/// Compression version
	//int m_compVers;
	/// Compression statistics print flag
	Gaudi::Property<int> m_printCompStats{this, "PrintCompStats", 0, "Print compressed format statistics"};
	/// Number of channels per module (may not all be used)
	int m_channels{64};
	/// Number of crates
	int m_crates{8};
	/// Number of modules per crate (may not all exist)
	//int m_modules;
	/// Number of slinks per crate when writing out bytestream
	Gaudi::Property<int> m_slinks{this, "SlinksPerCrate", 4, "The number of S-Links per crate"};
	/// Default number of LUT slices in simulation
	Gaudi::Property<int> m_dfltSlicesLut{this, "SimulSlicesLUT", 1, "The number of LUT slices in the simulation"};
	/// Default number of FADC slices in simulation
	Gaudi::Property<int> m_dfltSlicesFadc{this, "SimulSlicesFADC", 7, "The number of FADC slices in the simulation"};
	/// Force number of LUT slices in bytestream
	Gaudi::Property<int> m_forceSlicesLut{this, "ForceSlicesLUT", 0, "If >0, the number of LUT slices in bytestream"};
	/// Force number of FADC slices in bytestream
	Gaudi::Property<int> m_forceSlicesFadc{this, "ForceSlicesFADC", 0, "If >0, the number of FADC slices in bytestream"};
	/// Pedestal value
	Gaudi::Property<int> m_pedestal{this, "PedestalValue", 10, "Pedestal value - needed for compressed formats 0,1 only"};
	/// FADC baseline lower bound
	Gaudi::Property<int> m_fadcBaseline{this, "FADCBaseline", 0, "FADC baseline lower bound for compressed formats"};
	/// FADC threshold for super-compressed format
	Gaudi::Property<int> m_fadcThreshold{this, "FADCThreshold", 1, "FADC threshold for super-compressed format"};
	/// Zero suppression on input
	Gaudi::Property<int> m_zeroSuppress{this, "ZeroSuppress", 0, "Only make trigger towers with non-zero EM or Had energy"};
	/// ROB source IDs
	Gaudi::Property<std::vector<uint32_t>> m_sourceIDs{this, "ROBSourceIDs", {}, "ROB fragment source identifiers"};
	/// Sub-detector type
	eformat::SubDetector m_subDetector{eformat::FORWARD_ZDC};
	/// Source ID converter
	ZdcSrcIdMap* m_srcIdMap{nullptr};
	/// Trigger tower key provider
	ZdcPpmSubBlock* m_errorBlock{};
	/// Vector for current PPM sub-blocks
	DataVector<ZdcPpmSubBlock> m_ppmBlocks;
	/// Vector for compression statistics
	std::vector<uint32_t> m_compStats;
	/// ROD Status words
	std::vector<uint32_t>* m_rodStatus{nullptr};
	/// ROD status map
	std::map<uint32_t, std::vector<uint32_t>*> m_rodStatusMap;

	const ZdcID* m_zdcID{};

};

#endif /* ZDCBYTESTREAMTOOL_H*/
