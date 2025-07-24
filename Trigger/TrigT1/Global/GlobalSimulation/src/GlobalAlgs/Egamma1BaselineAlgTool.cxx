/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Egamma1BaselineAlgTool.h"
#include "../dump.h"
#include "../dump.icc"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

namespace GlobalSim {

  Egamma1BaselineAlgTool::Egamma1BaselineAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode Egamma1BaselineAlgTool::initialize() {
       
    CHECK(m_nbhdContainerReadKey.initialize());
    CHECK(m_eRatioKey.initialize());
    CHECK(m_eRatioSimpleKey.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode
  Egamma1BaselineAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in LArStrip neighborhoods from the event store
    // there is one neighborhood per EFex RoI
    auto in =
      SG::ReadHandle<LArStripNeighborhoodContainer>(m_nbhdContainerReadKey,
						    ctx);
    CHECK(in.isValid());

    ATH_MSG_DEBUG("read in " << (*in).size() << " neighborhoods");

    ap_int<16> peak = 0;
    ap_int<16> secondMax = 0;

    SG::WriteHandle<std::vector<float> > h_eRatio(m_eRatioKey, ctx);
    CHECK(h_eRatio.record(std::make_unique<std::vector<float> >()));
    SG::WriteHandle<std::vector<float> > h_eRatioSimple(m_eRatioSimpleKey, ctx);
    CHECK(h_eRatioSimple.record(std::make_unique<std::vector<float> >()));
	
    for (const auto& nbhd : *in) {
      auto c_phi = combine_phi(nbhd);
      if (msgLevel() <= MSG::DEBUG) {
        std::stringstream ss;
        ss << "Baseline input: ";
        for (const auto& i : c_phi) {ss << i << ' ';}
        ATH_MSG_DEBUG(ss.str());
      }
      //if (c_phi.empty()) {continue;}  // corner case: not all phi have len 17
      auto input = digitizer::digitize16(c_phi);
      //Do want to ap_int them?
      //ap_int<10>* c_input = &input[0];  // vector->array

      if (msgLevel() <= MSG::DEBUG) {
	std::stringstream ss;
	ss << "Baseline input: ";
	for (const auto& i : input) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

      //Central peak is easy, it's column 9 in the middle row, i.e. entry 25.
      peak = input.at(25);
      ATH_MSG_DEBUG("Peak " << peak);

      //Find the 6 secondary peaks, lets do it like the VHDL
      //holder of the current second peak (we will have 6)
      std::vector<ap_int<16>> secondPeak;
      secondPeak.resize(6);
      
      //Noise Margin: This should be 2sigma... set to 1 for now...
      int noiseMargin = 1;
      //Route one: <<< middle row, 24 down to 17
      secondPeak[0] = secondPeakSearch(input, peak, 24, 17, noiseMargin);
      //Route two: middle row >>>, 26 up to 33
      secondPeak[1] = secondPeakSearch(input, peak, 26, 33, noiseMargin);
      //Route three: <<< top row, 8 down to 0
      secondPeak[2] = secondPeakSearch(input, peak, 8, 0, noiseMargin);
      //Route four: top row >>>, 8 up to 16
      secondPeak[3] = secondPeakSearch(input, peak, 8, 16, noiseMargin);
      //Route five: <<< bottom row, 42 down to 34
      secondPeak[4] = secondPeakSearch(input, peak, 42, 34, noiseMargin);
      //Route five: bottom row >>>, 42 up to 50
      secondPeak[5] = secondPeakSearch(input, peak, 42, 50, noiseMargin);

      auto result = std::max_element(secondPeak.begin(), secondPeak.end());
      ATH_MSG_DEBUG("Max element found at index "
		    << std::distance(secondPeak.begin(), result)
		    << " has value " << *result);
      //tidy up.
      secondMax = *result;
      ATH_MSG_DEBUG("Peak " << peak << " second " << secondMax);
      
      //I am not confident on waht div_gen_0 does. So I am casting to floats for now.
      if(peak > 0 || secondMax > 0){
	auto eRatio = static_cast< float >(peak - secondMax)/static_cast< float >(peak + secondMax);
	h_eRatio->push_back(eRatio);
	ATH_MSG_DEBUG("eRatio (p-sp/p+sp) is " << eRatio);
	
	auto eRatioSimple = static_cast< float >(secondMax)/static_cast< float >(peak);
	h_eRatioSimple->push_back(eRatioSimple);
	ATH_MSG_DEBUG("eRatio (sp/p) is " << eRatio);
      } 
      
    }
    
    return StatusCode::SUCCESS;
  }

  ap_int<16> Egamma1BaselineAlgTool::secondPeakSearch(const std::vector<ap_int<16>>& input,
						      const ap_int<16> peak,
						      const int startCell,
						      const int endCell,
						      const ap_int<16> noiseMargin) const {
    //First set a series of counters to "descending" = 0                                                            
    int ascending = 0;

    //Setup holder of the last energy we checked... which starts at the peak...
    ap_int<16> lastEnergy = peak;
    ap_int<16> secondPeak = 0;

    //There must be a better way to do this?
    int direction = 0;
    if(startCell > endCell) {
      direction =-1;
    } else {
      direction =1;
    }
    
    //Route 1: go down in the middlle row
    for (auto itr = input.begin() + startCell; itr != input.begin() + endCell + direction; itr+=direction){
      //Going down hill... until we are not...
      ATH_MSG_DEBUG("Input is " << *itr << " last energy is " << lastEnergy);
      if(ascending==0 && *itr>lastEnergy && *itr-lastEnergy > noiseMargin){
	ATH_MSG_DEBUG("We are going up now " << *itr << "  is more then " << lastEnergy);
	ascending=1;
	lastEnergy=*itr;
	//Now check if we are at the peak as we are going uphill
      } else if(ascending==1 && lastEnergy>*itr && lastEnergy-*itr > noiseMargin){
	ATH_MSG_DEBUG("We are past the top " << *itr << "  is less than " << lastEnergy);
	if(secondPeak<*itr) secondPeak = lastEnergy;
	ATH_MSG_DEBUG("The peak was " << secondPeak);
	//I think we can break here... don't need to find a third peak
	break;
      } else {
	lastEnergy=*itr;
      }
    }
    
    return secondPeak;
  }
  
  std::vector<double>
  Egamma1BaselineAlgTool::combine_phi(const LArStripNeighborhood* nbhd) const  {
    auto result = std::vector<double>();

    const auto& phi_low = nbhd->phi_low();
    //if (phi_low.size() != s_required_phi_len) {return result;}

    const auto& phi_center = nbhd->phi_center();
    //if (phi_center.size() != s_required_phi_len) {return result;}
    
    const auto& phi_high = nbhd->phi_high();
    //if (phi_high.size() != s_required_phi_len) {return result;}

    result.reserve(s_combination_len);

    //Make a big vector as the VHDL does. Not strictly necessary, could just use the nbhd.
    std::transform(std::begin(phi_high), std::end(phi_high), std::back_inserter(result), [](const auto& high) {
      return high.m_e;
    });
    std::transform(std::begin(phi_center), std::end(phi_center), std::back_inserter(result), [](const auto& center) {
      return center.m_e;
    });
    std::transform(std::begin(phi_low), std::end(phi_low), std::back_inserter(result), [](const auto& low) {
      return low.m_e;
    });

    return result;
  }
  
  std::string Egamma1BaselineAlgTool::toString() const {

    std::stringstream ss;
    ss << "Egamma1BaselineAlgTool. name: " << name() << '\n'
       << m_nbhdContainerReadKey << '\n'
       << '\n';
    return ss.str();
  }
}

