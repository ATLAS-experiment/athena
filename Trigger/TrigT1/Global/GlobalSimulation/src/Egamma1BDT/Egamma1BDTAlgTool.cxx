/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Egamma1BDTAlgTool.h"
#include "../Utilities/dump.h"
#include "../Utilities/dump.icc"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "./Egamma1BDT/parameters.h"

#include "../IO/eEmEg1BDTTOB.h"


namespace GlobalSim {

  using eEmEg1BDTTOB = GlobalSim::IOBitwise::eEmEg1BDTTOB;
  
  Egamma1BDTAlgTool::Egamma1BDTAlgTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode Egamma1BDTAlgTool::initialize() {

    //Input keys
    CHECK(m_nbhdTOBContainerReadKey.initialize());
    CHECK(m_BDTScoreKey.initialize());
    //Output keys
    CHECK(m_eEmEg1BDTTOBContainerKey.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode
  Egamma1BDTAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in LArStrip neighborhood TOBs from the event store
    auto in =
      SG::ReadHandle<IOBitwise::eEmNbhoodTOBContainer>(m_nbhdTOBContainerReadKey,
						       ctx);
    CHECK(in.isValid());

    ATH_MSG_DEBUG("read in " << (*in).size() << " neighborhoods");

    SG::WriteHandle<std::vector<float> > h_BDTScore(m_BDTScoreKey, ctx);
    CHECK(h_BDTScore.record(std::make_unique<std::vector<float> >()));

    //Setup a container of TOBs with associated GlobalLArCell windows
    auto eEmEg1BDTTOBs = std::make_unique<IOBitwise::eEmEg1BDTTOBContainer>();
    
    for (const auto nbhdTOB : *in) {
      auto c_phi = combine_phi(nbhdTOB);
      if (c_phi.empty()) {continue;}  // corner case: not all phi have len 17
      auto input = digitizer::digitize10(c_phi);

      assert(input.size() == n_features);
      ap_int<BDT_ouput_width>* c_input = &input[0];  // vector->array

      score_t scores[GlobalSim::BDT::fn_classes(n_classes)];

      // the bdt variable is already set up 
      bdt.decision_function(c_input, scores);
      if (msgLevel() <= MSG::DEBUG) {
	std::stringstream ss;
	ss << "BDT input: ";
	for (const auto& i : input) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

      if (msgLevel() <= MSG::DEBUG) {
	std::stringstream ss;
	ss << "C BDT output: ";
	for (const auto& i : scores) {ss << i << ' ';}
	ATH_MSG_DEBUG(ss.str());
      }

      //Extract the bits from the ap_fixed<10,5> object.
      std::bitset<BDT_ouput_width> BDT_bits;
      for (int i=0;i<scores[0].length();i++){
	BDT_bits[i] = scores[0][i];
      }
      ATH_MSG_DEBUG("BDT bits " << BDT_bits);
 
      //Shift to an unsigned range, stored in a Bitset<8> as the hardware will.
      std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width> result;
      //First, interpret the ap_fixed<10,5> as an integer.
      int BDT_int = bitSetToInt(BDT_bits);
      ATH_MSG_DEBUG("BDT int " << BDT_int);
      //We are going to restrict the range, but keep the resolution.
      if(BDT_int >= 128) {
	//So, cut off at 2^8/2-1 as the maximum possible value.
	result = std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width>{0xFF};
      } else if (BDT_int < -128) {
	//So, cut off at and -2^8/2 as the minimum possible value.
	result = std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width>{0x00};
      } else {
	//Then we take the remaining bits, flipping the maximum remaining bit.
	//Due to the above logic, this is only 1 if -ive.
	result[eEmEg1BDTTOB::s_eGamma1BDT_width-1] = ~BDT_bits[eEmEg1BDTTOB::s_eGamma1BDT_width-1];
	for(uint i = 0; i <= eEmEg1BDTTOB::s_eGamma1BDT_width-2;i++){
	  result[i] = BDT_bits[i];
	}
      }

      ATH_MSG_DEBUG("Result bits " << result);
      ATH_MSG_DEBUG("Result int " << result.to_ulong());
      
      //Outpout the ulong for debug, and store the result in the output TOB
      h_BDTScore->push_back(result.to_ulong());
      eEmEg1BDTTOBs->push_back(std::make_unique<IOBitwise::eEmEg1BDTTOB>(*nbhdTOB, result));
    }

    //Setup the write out of the resultant TOBs
    SG::WriteHandle<GlobalSim::IOBitwise::eEmEg1BDTTOBContainer> h_eEmEg1BDTTOBs(m_eEmEg1BDTTOBContainerKey, ctx);
    CHECK(h_eEmEg1BDTTOBs.record(std::move(eEmEg1BDTTOBs)));

    return StatusCode::SUCCESS;
  }

  int Egamma1BDTAlgTool::bitSetToInt(std::bitset<BDT_ouput_width> bitSet) const {
    if (!bitSet[BDT_ouput_width - 1]) return bitSet.to_ulong();
    bitSet.flip();
    return -(bitSet.to_ulong() + 1);
  }
  
  std::vector<double>
  Egamma1BDTAlgTool::combine_phi(const IOBitwise::eEmNbhoodTOB* nbhdTOB) const  {
    auto result = std::vector<double>();

    const auto& phi_low = nbhdTOB->Neighbourhood().phi_low();
    if (phi_low.size() != s_required_phi_len) {return result;}

    const auto& phi_center = nbhdTOB->Neighbourhood().phi_center();
    if (phi_center.size() != s_required_phi_len) {return result;}

    
    const auto& phi_high = nbhdTOB->Neighbourhood().phi_high();
    if (phi_high.size() != s_required_phi_len) {return result;}

    result.resize(s_combination_len);

    constexpr int c{8};

    result.at(0) = phi_center.at(c).m_e;

    result.at(1) = std::max(phi_low.at(c).m_e, phi_high.at(c).m_e);

    int ri{2};
    for (int diff = 1; diff != 9; ++diff) { 
      result.at(ri) =
	std::max({phi_center.at(c-diff).m_e,
	    phi_center.at(c+diff).m_e});

      result.at(ri+1) =
	std::max({phi_low.at(c-diff).m_e,
	  phi_low.at(c+diff).m_e,
	  phi_high.at(c-diff).m_e,
	  phi_high.at(c+diff).m_e});
      
      ri += 2;
    }
    
    return result;
  }

  std::string Egamma1BDTAlgTool::toString() const {

    std::stringstream ss;
    ss << "Egamma1BDTAlgTool. name: " << name() << '\n'
       << m_nbhdTOBContainerReadKey << '\n'
       << '\n';
    return ss.str();
  }

}

