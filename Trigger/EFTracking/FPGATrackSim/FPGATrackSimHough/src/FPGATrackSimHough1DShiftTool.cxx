// Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimHough1DShiftTool.cxx
 * @author Riley Xu - riley.xu@cern.ch
 * @date October 31st, 2020
 * @brief See header file.
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimHoughTransformTool.h"
#include "FPGATrackSimHough1DShiftTool.h"
#include "TH2.h"

#include <sstream>
#include <cmath>
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <iostream>

static inline std::string to_string(std::vector<int> v);
static inline std::string to_string(std::vector<size_t> v);
static inline std::string to_string(std::vector<boost::dynamic_bitset<>> & v);
static inline std::string instance_name(std::string const & s);
static inline boost::dynamic_bitset<> lshift(boost::dynamic_bitset<> const & b, int n);
static inline boost::dynamic_bitset<> rshift(boost::dynamic_bitset<> const & b, int n);
static inline void updateBinHits(std::vector<boost::dynamic_bitset<>> & binHits, unsigned layer, boost::dynamic_bitset<> const & b);
static inline int layersHit(FPGATrackSimRoad& r);

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimHough1DShiftTool::FPGATrackSimHough1DShiftTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    AthAlgTool(algname, name, ifc),
    m_EvtSel("FPGATrackSimEventSelectionSvc", name),
    m_FPGATrackSimMapping("TrigFPGATrackSimMappingSvc", name),
    m_FPGATrackSimBankSvc("TrigFPGATrackSimBankSvc", name),
    m_name(instance_name(name)),
    m_monitorFile((m_name + ".root").c_str(), "RECREATE")
{
    declareInterface<IFPGATrackSimRoadFinderTool>(this);
}


StatusCode FPGATrackSimHough1DShiftTool::initialize()
{
    // Config printout
    ATH_MSG_INFO("Phi range: (" << m_phiMin << "," << m_phiMax << "," << m_phiBins << ")");
    if (m_useDiff) ATH_MSG_INFO("useDiff Set True");
    if (m_variableExtend) ATH_MSG_INFO("variableExtend Set True");
    ATH_MSG_INFO("enhancedHighPt" << m_enhanceHighPt.value());
    ATH_MSG_INFO("applyDropable" << m_applyDropable.value());

    // Retrieve info
    ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st()->getNLogiLayers();

    // Error checking
    if (m_phiMin >= m_phiMax || m_phiBins == 0) {
        ATH_MSG_FATAL("initialize() Phi range invalid");
	return StatusCode::FAILURE;
    }

    // Fix inputs
    if (!m_hitExtendProperty.value().empty()) {
      if (m_hitExtendProperty.value().size() != m_nLayers) {
	ATH_MSG_FATAL("initialize() Hit extend must have size == nLayers");
	return StatusCode::FAILURE;
      } else {
	m_hitExtend = m_hitExtendProperty.value();
      }
    } else {
      m_hitExtend.resize(m_nLayers,0); // all 0
    }
    if (m_iterStep == 0) m_iterStep = m_hitExtend[m_iterLayer] * 2 + 1; // default 1

    // Copy correct r values
    m_r.resize(m_nLayers);
    if (m_radii_file.value().empty()) {
      ATH_MSG_ERROR("RADII FILE WAS EMPTY, We should not be in this configuration");
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("initialize() Reading Radii File: " << m_radii_file);
      readRadii(m_radii_file);
    }

    // Warnings / corrections
     if (m_idealGeoRoads)
    {
        if (m_useSectors)
        {
            ATH_MSG_WARNING("initialize() idealGeoRoads conflicts with useSectors, switching off FPGATrackSim sector matching");
            m_useSectors = false;
        }
        if (!m_traceHits)
        {
            ATH_MSG_WARNING("initialize() idealGeoRoads requires tracing hits, turning on automatically");
            m_traceHits = true;
        }
    }

    // Fill convenience variables
    m_phiStep = (m_phiMax - m_phiMin) / m_phiBins;
    for (unsigned i = 0; i <= m_phiBins; i++)
        m_bins.push_back(m_phiMin + m_phiStep * i);
    m_regionMin = m_EvtSel->getRegions()->getMin(m_EvtSel->getRegionID());
    m_regionMax = m_EvtSel->getRegions()->getMax(m_EvtSel->getRegionID());

    m_regionMin.phi = m_regionMin.phi - 0.01; // For phi boundary effect   
    m_regionMax.phi = m_regionMax.phi + 0.01; // For phi boundary effect 

    m_currentcounts.resize(m_phiBins);

    // Calculate the shift amounts
    if (m_bitShift_path.value().empty()) calculateShifts();
    else readShifts(m_bitShift_path);
    printShifts();


    return StatusCode::SUCCESS;
}


// Fills m_shifts
void FPGATrackSimHough1DShiftTool::calculateShifts()
{
    // Calculated d0shift if selected
    if (m_d0spread>0) calculated0Shifts();


    // Iterate over a steps of m_iterStep bin shift in m_iterLayer
    for (int sign = -1; sign<2; sign+=2) {

	// start at highest pT
	float iterShift = (sign>0) ? 0.0 : -1.0; // don't double count zero

	while (true)
	{
	    float qpt = qPt(m_r[m_iterLayer], m_phiStep * iterShift);
	    if (abs(qpt)>std::max(abs(m_qptMin),abs(m_qptMax))) break;

	    if ((qpt <= m_qptMax ) && (qpt >= m_qptMin )) {

		std::vector<int> shifts; // Fill both +iterShift and -iterShift here
		for (unsigned i = 0; i < m_nLayers; i++)
		{
		    float dPhi = deltaPhi(m_r[i], qpt);
		    if (m_useDiff){
			dPhi -= deltaPhi(m_r[0], qpt);
		    }

		    int shift = static_cast<int>(round(dPhi / m_phiStep));
		    if (shift >= static_cast<int>(m_phiBins) || shift <= -(static_cast<int>(m_phiBins))) return;
		    shifts.push_back(shift);
		}

		// if thre are d0shifts, apply them
		for (auto d0shift: m_d0shifts){
		    m_shifts.push_back(applyVariation(shifts,d0shift,1));
		    m_shifts.push_back(applyVariation(shifts,d0shift,-1));
		}

		m_qpt.push_back(qpt);
		m_shifts.push_back(shifts);

	    }

	    // smaller steps if in "enhanceHighPt" range
	    iterShift += ((std::abs(qpt)>m_enhanceHighPt) ?  sign*float(m_iterStep): sign*float(m_iterStep)/2.0) ;
	}
	ATH_MSG_INFO("Ending shift, qpt("<< m_qptMin << ", " << m_qptMax <<") sign:" <<  sign << " shift:" << iterShift);


    // figure out which patterns are the same after dropping a hit
    calculateDropable();
    }

}


// Fills m_shifts
//
// This reads a bitshift pattern file, which is a simple space-separated text file
// where each row respresents one shift set / pattern. Each row contains
// (nLayers - 1) columns representing the shifts in layers [1, nLayers)
// (implicit 0 shift in layer 0), followed by a column that is a 'droplist'.
// The droplist is a string of 0s and 1s indicating which layers can be dropped
// for 4/5 matching, i.e (layer 0 is right-most bit).
//
// See /eos/atlas/atlascerngroupdisk/det-htt/sectors_constants/BitShiftHoughPatterns/2021-02-20/
void FPGATrackSimHough1DShiftTool::readShifts(std::string const & filepath)
{
    // Open the file
    std::ifstream fin(filepath);
    if (!fin.is_open())
    {
        ATH_MSG_FATAL("Couldn't open " << filepath);
        throw ("FPGATrackSimHough1DShiftTool::readShifts couldn't open " + filepath);
    }

    // Variables to fill
    std::string line;
    bool ok = true;
    int subregion = -1;
    bool newsubr = false;
    bool headerdone = false;
    int nslices = 0;
    int nphi=0;


    // Parse the file
    while (getline(fin, line))
    {
	newsubr|=(line.empty());
        if (line.empty() || line[0] == '#') continue;
        std::istringstream sline(line);
	if (!headerdone) {
	    std::string towers, phi;
	    ok = ok && (sline >> towers);
	    if (towers!="towers") {
		ATH_MSG_FATAL("Parse Error expected 'towers' got "<< towers);
		throw ("FPGATrackSimHough1DShiftTool::readShifts couldn't parse " + filepath);
	    }
	    ok = ok && (sline >> nslices);
	    ATH_MSG_INFO("FPGATrackSimHough1DShiftTool::readShifts " << nslices << "slices in shift file");
	    ok = ok && (sline >> phi);
	    if (phi!="phi") {
		ATH_MSG_FATAL("Parse Error expected 'phi' got "<< phi);
		throw ("FPGATrackSimHough1DShiftTool::readShifts couldn't parse " + filepath);
	    }
	    ok = ok && (sline >> nphi);
	    headerdone=true;
	    continue;
	}

	if (newsubr) {
	    ok = ok && (sline >> subregion);
	    ATH_MSG_INFO("FPGATrackSimHough1DShiftTool::readShifts " << subregion << " looking for " << m_subRegion);
	    newsubr=false;
	    continue;
	}

	if (subregion==m_subRegion) {
	    ATH_MSG_INFO("FPGATrackSimHough1DShiftTool::readShifts found subregion " << m_subRegion);
	    // Shifts
	    std::vector<int> shifts;
	    shifts.push_back(0); // layer 0 is implicit 0 TODO
	    for (unsigned layer = 1; layer < m_nLayers; layer++)
	    {
		int shift = 0;
		ok = ok && (sline >> shift);
		if (!ok) break;
		shifts.push_back(shift);
	    }
	    if (!ok) break;
	    if (shifts[0]<-1000) {
		ATH_MSG_INFO("FPGATrackSimHough1DShiftTool::readShifts skipping line with shift out of range: " << line);
		continue;
	    }

	    // Dropable
	    std::string droplist;
	    ok = ok && (sline >> droplist);
	    ok = ok && (droplist.size() == m_nLayers);
	    if (!ok) break;

	    boost::dynamic_bitset<> drops(m_nLayers);
	    for (unsigned i = 0; i < m_nLayers; i++)
	      if (droplist[i] == '1')
                drops[m_nLayers - i - 1] = true; // Droplist is printed with MSB on left

	    // qpT
	    float qpt = 0;
	    ok = ok && (sline >> qpt);
	    if (!ok) break;
	    if ((qpt<m_qptMin) or (qpt>m_qptMax)) continue;

	    // Phi Vals
	    std::vector<float> phivals;
	    for (unsigned layer = 0; layer < m_nLayers; layer++)
	    {
		float phival = 0;
		ok = ok && (sline >> phival);
		if (!ok) break;
		phivals.push_back(phival);
	    }

	    m_qpt.push_back(qpt);
	    m_shifts.push_back(shifts);
	    m_dropable.push_back(drops);
	    m_phivals.push_back(phivals);


	}
    }

    if (!ok)
    {
        ATH_MSG_FATAL("Found error reading file at line: " << line);
        throw "FPGATrackSimHough1DShiftTool::readShifts read error";
    }

    if (m_shifts.size()==0) ATH_MSG_FATAL("FPGATrackSimHough1DShiftTool::readShifts no shifts read");
    ATH_MSG_INFO("Read " << m_shifts.size() << " patterns from " << filepath);
}

void FPGATrackSimHough1DShiftTool::readRadii(std::string const & filepath)
{
    // Open the file
    std::ifstream fin(filepath);
    if (!fin.is_open())
    {
        ATH_MSG_FATAL("Couldn't open " << filepath);
        throw ("FPGATrackSimHough1DShiftTool couldn't open " + filepath);
    }

    // Variables to fill
    std::string line;
    bool ok = true;

    // Parse the file
    while (getline(fin, line))
    {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream sline(line);
        std::vector<int> shifts;

	int subregion;
	ok = ok && (sline >> subregion);
	if (subregion==m_subRegion) {
	    for (unsigned layer = 0; layer < m_nLayers; layer++)
	    {
	        float r;
		ok = ok && (sline >> r);
		if (!ok) break;
		if (r<=0) {
		  ATH_MSG_WARNING("Radius in radiiFile is "<< r <<" for layer:"
				  << layer << " setting to dummy value!");
		  r = 500.0; // dummy value that won't cause a crash, but won't work anywhere.
		}
		m_r[layer]=r;

	   }
	}
	if (!ok) break;
    }

    if (!ok)
    {
        ATH_MSG_FATAL("Found error reading file at line: " << line);
        throw "FPGATrackSimHough1DShiftTool read error";
    }
}



StatusCode FPGATrackSimHough1DShiftTool::finalize()
{
    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimHough1DShiftTool::getRoads(const std::vector<const FPGATrackSimHit*> & hits, std::vector<FPGATrackSimRoad*> & roads)
{
    
    roads.clear();
    m_roads.clear();

    ATH_MSG_DEBUG("Input Hits Size:" << hits.size() << "\n");

    // Get hit masks
    std::vector<boost::dynamic_bitset<>> hitMasks(makeHitMasks(hits));
    if (m_event < 5) drawHitMasks(hitMasks, m_name + "_e" + std::to_string(m_event));

    // Iterate over shifts
    for (size_t iShift = 0; iShift < m_shifts.size(); iShift++)
    {
          // Track hit layers in each bin
        std::vector<boost::dynamic_bitset<>> binHits(m_phiBins, boost::dynamic_bitset<>(m_nLayers));


        // Add the counts
        for (unsigned i = 0; i < m_nLayers; i++)
            updateBinHits(binHits, i, lshift(hitMasks[i], m_shifts[iShift][i]));

        // Check the threshold
        for (unsigned bin = 0; bin < m_phiBins; bin++)
        {
	    // First check we're not missing hits in not allowed layers
	    if (m_applyDropable && !m_dropable.empty() && (~(binHits[bin] | m_dropable[iShift])).any()) continue;

	    m_currentcounts[bin]=binHits[bin].count();

            if (passThreshold(binHits,bin))
            {
	      FPGATrackSimRoad r = makeRoad(hits, bin, iShift);

	      if (m_phiRangeCut && (r.getX() > m_regionMax.phi || r.getX() < m_regionMin.phi)) continue;

	      m_roads.push_back(r);

	      ATH_MSG_DEBUG("New Road: "<< layersHit(r) <<" lyrs,  " << r.getNHits() << " hits :"
			   << to_string(r.getNHits_layer()) << " reg=" <<  r.getSubRegion()
			   << " bin=" << bin << " shift=" << iShift << " phi=" << r.getX() << " pti=" << r.getY() << " " << m_qpt[iShift]);


            }

        }

	// Add/Remove history
	m_vetolist.push_front(m_currentcounts);
	if (m_vetolist.size() > m_historyWindow) {
	    m_vetolist.pop_back();
	}
    }

    roads.reserve(m_roads.size());
    for (FPGATrackSimRoad & r : m_roads) roads.push_back(&r);

    if (roads.size()==0) drawHitMasks(hitMasks, m_name + "_fail_e" + std::to_string(m_event));

    m_event++;
    return StatusCode::SUCCESS;
}



bool FPGATrackSimHough1DShiftTool::passThreshold(std::vector<boost::dynamic_bitset<>>& binHits, int bin ) const
{
    // Check the threshold
    if (binHits[bin].count() <  m_threshold) return false;

    // Apply neighbor veto
    if (m_neighborWindow>0) {
	for (int i = -m_neighborWindow; i <= m_neighborWindow; i++) {
	    if ((bin+i < 0) or ((bin+i)>=int(m_phiBins))) continue; // ignore edges
	    if (binHits[bin+i].count()>binHits[bin].count()) {
		// neighbor has more layers
		return false;
	    }
	    if (i!=0 && (binHits[bin+i].count()==binHits[bin].count())) {
		// favor lower phi bin
		return false;
	    }
	    for (unsigned j = 0; j < m_vetolist.size(); j++) {
		if (m_vetolist[j][bin+i]>binHits[bin].count()) {
		    // past neighbor has more layers
		    return false;
		}
		if ((m_vetolist[j][bin+i]==binHits[bin].count()) and (i<0)) {
		    // favor previous shift and lower phi bin
		    return false;
		}
	    }
	}
    }

    return true; // passed threshold, wasn't vetoed by neighbors

}



std::vector<boost::dynamic_bitset<>> FPGATrackSimHough1DShiftTool::makeHitMasks(const std::vector<const FPGATrackSimHit*> & hits)
{
    std::vector<boost::dynamic_bitset<>> hitMasks(m_nLayers, boost::dynamic_bitset<>(m_phiBins));
    for (const FPGATrackSimHit* hit : hits)
    {
        if (m_subRegion >= 0 && !m_FPGATrackSimMapping->SubRegionMap()->isInRegion(m_subRegion, *hit)) continue;

        auto bins = getBins(hit);

        for (int i = std::max(bins.first, 0); i <= std::min(bins.second, (int)m_phiBins - 1); i++)
            hitMasks[hit->getLayer()][i] = true;
    }

    return hitMasks;
}


FPGATrackSimRoad FPGATrackSimHough1DShiftTool::makeRoad(std::vector<const FPGATrackSimHit*> hits, int bin_track, size_t iShift)
{
    std::vector<int> const & shifts = m_shifts[iShift];
    float qpT = m_qpt[iShift];

    std::vector<const FPGATrackSimHit*> road_hits;
    layer_bitmask_t hitLayers = 0;

    for (const FPGATrackSimHit* hit : hits)
    {
        if (m_subRegion >= 0 && !m_FPGATrackSimMapping->SubRegionMap()->isInRegion(m_subRegion, *hit)) continue;

        // Get the shifted bins of the hit
        auto bins = getBins(hit);
        bins.first += shifts[hit->getLayer()]; // note boost::dynamic_bitset stores [0] as the LSB, i.e. rightmost. So leftshift = +bin
        bins.second += shifts[hit->getLayer()];

        // Check if it's the same bin as the track
        if (bin_track >= bins.first && bin_track <= bins.second)
        {
            road_hits.push_back(hit);
            hitLayers |= 1 << hit->getLayer();
	 }
    }

    auto sorted_hits = ::sortByLayer(road_hits);
    sorted_hits.resize(m_nLayers); // If no hits in last layer, return from sortByLayer will be too short
    // Combination of last two addRoad() functions from FPGATrackSimHoughTransformTool.cxx are implemented above (up to here)
    // Below want to look at first addRoad()/matchIdealGeoSector() function in FPGATrackSimHoughTransformTool.cxx

    FPGATrackSimRoad r;
    r.setHitLayers(hitLayers);
    r.setHits(sorted_hits);
    r.setSubRegion(m_subRegion);
    if (m_fieldCorrection) {
      int inner_bin = static_cast<int>(bin_track)-static_cast<int>(shifts[0]);
      if ((inner_bin < 0) || (inner_bin >= static_cast<int>(m_phiBins))) {
	r.setX(-100000);
      } else {
	r.setX(m_bins[inner_bin] + 0.5*m_phiStep + deltaPhi(m_r[0],qpT));
      }
    } else {
      r.setX(phitrk(bin_track,shifts).first); // will be a large negative number if invalid
    }
    r.setY(qpT);

    matchIdealGeoSector(r);
    return r;
    // TODO sector, wildcard layers?
}

// Taken from "HoughTransformTool.cxx"
void FPGATrackSimHough1DShiftTool::matchIdealGeoSector(FPGATrackSimRoad & r) const
{

    // We now look up the binning information in the sector bank.
    const FPGATrackSimSectorBank* sectorbank = m_FPGATrackSimBankSvc->SectorBank_1st();

    // Look up q/pt (or |q/pt|) from the Hough road, convert to MeV.
    double qoverpt = r.getY()*0.001;
    if (sectorbank->isAbsQOverPtBinning()) {
       qoverpt = std::abs(qoverpt);
    }

    int sectorbin = 0;

    // Retrieve the bin boundaries from the sector bank; map this onto them.
    std::vector<double> qoverpt_bins = sectorbank->getQOverPtBins();
    auto bounds = std::equal_range(qoverpt_bins.begin(), qoverpt_bins.end(), qoverpt);

    sectorbin = fpgatracksim::QPT_SECTOR_OFFSET*(bounds.first - qoverpt_bins.begin() - 1);
    if (sectorbin < 0) sectorbin = 0;
    if ((sectorbin / 10) > static_cast<int>(qoverpt_bins.size() - 2))sectorbin = 10*(qoverpt_bins.size() - 2);

    if (m_doRegionalMapping){
      int subregion = r.getSubRegion();
      sectorbin += subregion*fpgatracksim::SUBREGION_SECTOR_OFFSET;
    }
    std::vector<module_t> modules;
    for (unsigned int il = 0; il < r.getNLayers(); il++) {
        if (r.getNHits_layer()[il] == 0) {
            modules.push_back(-1);

            layer_bitmask_t wc_layers = r.getWCLayers();
            wc_layers |= (0x1 << il);
            r.setWCLayers(wc_layers);

            FPGATrackSimHit *wcHit = new FPGATrackSimHit();
            wcHit->setHitType(HitType::wildcard);
            wcHit->setLayer(il);
            wcHit->setDetType(m_FPGATrackSimMapping->PlaneMap_1st()->getDetType(il));
            std::vector<const FPGATrackSimHit*> wcHits;
            wcHits.push_back(wcHit);
            r.setHits(il,wcHits);
        }
        else {
            modules.push_back(sectorbin);
        }
    }

    // If we are using eta patterns. We need to first run the roads through the road filter.
    // Then the filter will be responsible for setting the actual sector.
    // As a hack, we can store the sector bin ID in the road for now.
    // This is fragile! If we want to store a different ID for each layer, it will break.

    // Similarly, we do the same thing for spacepoints. this probably means we can't combine the two.
    // maybe better to store the module array instead of just a number?

    r.setSectorBin(sectorbin);
    if (!m_doEtaPatternConsts && !m_useSpacePoints) {
        r.setSector(sectorbank->findSector(modules));
    }
}


///////////////////////////////////////////////////////////////////////////////
// Helpers

// Given a relative shift between iterLayer and layer 0, returns the corresponding qpt.
// This does a linear approximation of the Hough transform equation.
float FPGATrackSimHough1DShiftTool::getPtFromShiftDiff(int shift) const
{
    if (m_iterLayer == 0) ATH_MSG_FATAL("getPtFromShiftDiff() iterLayer can't be 0");
    return (shift * m_phiStep / fpgatracksim::A) / (m_r[m_iterLayer] - m_r[0]);
}


// Returns the range of bins (inclusive) given a phi and an extension in number of bins
std::pair<int, int> FPGATrackSimHough1DShiftTool::getBins(const FPGATrackSimHit* hit) const
{
    float phi = hit->getGPhi();
    float bin_extend = m_hitExtend[hit->getLayer()];
    float center = (phi - m_phiMin) / m_phiStep;

    if (m_variableExtend){
      float r = hit->getR();
      float dphimax = deltaPhi(r,m_qptMax);
      float maxshift = +1*((std::sin(dphimax)*(r-m_r[hit->getLayer()]))/r)/m_phiStep;
      float dphimin = deltaPhi(r,m_qptMin);
      float minshift = +1*((std::sin(dphimin)*(r-m_r[hit->getLayer()]))/r)/m_phiStep;

      center += (maxshift+minshift)/2.0;
      bin_extend += std::abs((maxshift-minshift)/2.0);

    }

    int low = std::max(static_cast<int>(center - bin_extend),0);
    int high = std::min(static_cast<int>(center + bin_extend),static_cast<int>(m_phiBins-1));
    return { low, high };
}

float FPGATrackSimHough1DShiftTool::qPt(float r, float deltaPhi) const
{
    if (m_useDiff) {
      float r1=m_r[0];
      float r2=r;
      float sign = deltaPhi>0 ? 1 : -1;
      return sign*1/(2*fpgatracksim::A) * std::sqrt((4*std::sin(deltaPhi)*std::sin(deltaPhi))/( r1*r1 + r2*r2 -2*r2*r1*std::cos(deltaPhi)));
    }
    return std::sin(deltaPhi) / (fpgatracksim::A * r);
}

float FPGATrackSimHough1DShiftTool::phitrkDiff(float r1, float phi1,  float r2,  float phi2) const
{
   float phi_track = phi1+ std::atan2(r2-r1*std::cos(phi2-phi1),r1*std::sin(phi2-phi1)) - TMath::Pi()/2.0;
  return phi_track;
}


std::pair<float, bool> FPGATrackSimHough1DShiftTool::phitrk(int bin, std::vector<int> const &  shifts ) const
{
  int inner_bin = static_cast<int>(bin)-static_cast<int>(shifts[0]);
  int outer_bin = static_cast<int>(bin)-static_cast<int>(shifts[m_nLayers-1]);
  if ((inner_bin < 0) || (inner_bin >= static_cast<int>(m_phiBins))) return {-100000,false};
  if ((outer_bin < 0) || (outer_bin >= static_cast<int>(m_phiBins))) return {-100000,false};
  float phi1 = m_bins[inner_bin];
  float phi2 = m_bins[outer_bin];
  float phi_track = phitrkDiff(m_r[0],phi1,m_r[m_nLayers-1],phi2)+0.5*m_phiStep;
  return {phi_track,true};
}


float FPGATrackSimHough1DShiftTool::deltaPhi(float r, float qPt) const
{
   float dPhi = std::asin(fpgatracksim::A * r * qPt);
    if (m_fieldCorrection) dPhi += FPGATrackSimHoughTransformTool::fieldCorrection(m_EvtSel->getRegionID(), qPt, r);
    return dPhi;
}


static inline std::string to_string(std::vector<int> v)
{
    std::ostringstream oss;
    oss << "[";
    if (!v.empty())
    {
        std::copy(v.begin(), v.end()-1, std::ostream_iterator<int>(oss, ", "));
        oss << v.back();
    }
    oss << "]";
    return oss.str();
}


static inline std::string to_string(std::vector<boost::dynamic_bitset<>> & v)
{
    std::ostringstream oss;
    for (auto bs : v) {
	std::string s;
	boost::to_string(bs,s);
	oss << "[" << s <<  "]";
    }
    return oss.str();
}

static inline std::string to_string(std::vector<size_t> v)
{
    std::ostringstream oss;
    oss << "[";
    if (!v.empty())
    {
        std::copy(v.begin(), v.end()-1, std::ostream_iterator<size_t>(oss, ", "));
        oss << v.back();
    }
    oss << "]";
    return oss.str();
}


static inline std::string instance_name(std::string const & s)
{
    size_t pos = s.find_last_of(".");
    if (pos != std::string::npos)
        return s.substr(pos + 1);
    return s;
}

static inline boost::dynamic_bitset<> lshift(boost::dynamic_bitset<> const & b, int n)
{
    if (n < 0) return rshift(b, -n);
    return b << n;
}

static inline boost::dynamic_bitset<> rshift(boost::dynamic_bitset<> const & b, int n)
{
    if (n < 0) return lshift(b, -n);

    boost::dynamic_bitset<> out(b >> n);

    // Force 0-padding
    for (int i = 0; i < n; i++)
        out[out.size() - 1 - i] = false;

    return out;
}

static inline void updateBinHits(std::vector<boost::dynamic_bitset<>> & binHits, unsigned layer, boost::dynamic_bitset<> const & b)
{
    for (size_t i = 0; i < b.size(); i++)
        if (b[i]) binHits[i].set(layer);
}

void FPGATrackSimHough1DShiftTool::printHitMasks(std::vector<boost::dynamic_bitset<>> const & hitMasks) const
{
    std::stringstream ss;
    ss << "Hit Masks:\n";
    for (auto const & b : hitMasks)
    {
        ss << "\t";
        for (size_t i = 0; i < b.size(); i++)
        {
            if (b[i]) ss << 1;
            else ss << ".";
        }
        ss << "\n";
    }
    ATH_MSG_DEBUG(ss.str() << "\n\n");
}

void FPGATrackSimHough1DShiftTool::drawHitMasks(std::vector<boost::dynamic_bitset<>> const & hitMasks, std::string const & name)
{
    m_monitorFile.cd();

    TH2F h(name.c_str(), "Hough 1D Shift;phi;layer",
            m_phiBins, m_phiMin, m_phiMax,
            m_nLayers, 0, m_nLayers
    );

    for (size_t layer = 0; layer < m_nLayers; layer++)
        for (size_t i = 0; i < m_phiBins; i++)
            h.SetBinContent(i+1, layer+1, hitMasks[layer][i]); // +1 since root bins are 1-indexed

    h.Write();
}

void FPGATrackSimHough1DShiftTool::drawHitMasks(std::vector<boost::dynamic_bitset<>> const & hitMasks, std::string const & name, std::vector<int> const & shifts)
{
    std::vector<boost::dynamic_bitset<>> shiftedMasks;
    for (size_t layer = 0; layer < m_nLayers; layer++)
        shiftedMasks.push_back(lshift(hitMasks[layer], shifts[layer]));

    drawHitMasks(shiftedMasks, name);
}

void FPGATrackSimHough1DShiftTool::printShifts() const
{
    std::stringstream ss;
    ss << "Shifts:\n";
    for (size_t i = 0; i < m_shifts.size(); i++)
    {
        float qpt = qPt(m_r[m_iterLayer], m_phiStep * m_shifts[i][m_iterLayer]); // q/pT represented by this shift
        ss << "q/pT=" << qpt << "; ";
        for (int shift : m_shifts[i]) ss << shift << " ";
        if (!m_dropable.empty()) ss << m_dropable[i];
        ss << "\n";
    }
    ATH_MSG_VERBOSE(ss.str());
}


void FPGATrackSimHough1DShiftTool::calculated0Shifts()
{
    int steps=1;
    while (true) {
      float d0step = steps*m_phiStep*m_r[0];
      ATH_MSG_DEBUG("d0step = " << d0step);
      std::vector<int> d0shift;
      for (unsigned lyr = 0; lyr < m_nLayers; lyr++) {
         float phi_for_d0step = d0step/m_r[lyr];
         int shift = static_cast<int>(round(phi_for_d0step/ m_phiStep));
         d0shift.push_back(shift);
      }
      m_d0shifts.push_back(d0shift);
      if (d0step>m_d0spread) break;
      ++steps;
    }
    ATH_MSG_DEBUG("d0 Shifts Found = " << m_d0shifts.size());
}

std::vector<int> FPGATrackSimHough1DShiftTool::applyVariation(std::vector<int> base, std::vector<int> var, int sign) const
{
    std::vector<int> retv;

    if (base.size()!=var.size()) {
      ATH_MSG_ERROR("Base and Var lengths must match " << base.size() << " " << var.size() );
    }

    for (unsigned i = 0; i < base.size(); i++) {
      retv.push_back(base[i]+sign*var[i]);
    }
    return retv;
}

std::vector<int> FPGATrackSimHough1DShiftTool::shiftWithDrop(std::vector<int>& shift,unsigned droplayer) const
{
  std::vector<int> retv(m_nLayers-1);
  for (unsigned lyr = 0; lyr < m_nLayers; lyr++) {
    if (lyr !=droplayer) retv.push_back(shift[lyr]);
  }
  return retv;
}

void FPGATrackSimHough1DShiftTool::calculateDropable()
{

  // one set per layer with shift set
  std::vector< std::set< std::vector<int> > > reducedShifts(m_nLayers);

  m_dropable.resize(m_shifts.size());

  for (size_t iShift = 0; iShift < m_shifts.size(); iShift++)
  {
    boost::dynamic_bitset<> drops(m_nLayers);
    for (unsigned droplayer =0; droplayer < m_nLayers; droplayer++) {
       if (reducedShifts[droplayer].insert(shiftWithDrop(m_shifts[iShift],droplayer)).second) {
          // true if the is a new shift in the set
          drops.set(droplayer);
       }
    }
    
    m_dropable[iShift]=drops;

  }

}


static inline int layersHit(FPGATrackSimRoad& r) {
  int cnt =0;
  for (unsigned i = 0; i < r.getNLayers(); i++) {
    if (r.getHitLayers()  &  (1<<i)) cnt++;
  }
  return cnt;
}
