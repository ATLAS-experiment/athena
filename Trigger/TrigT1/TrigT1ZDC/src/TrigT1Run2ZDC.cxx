/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <fstream>
#include <stdexcept>
#include <bitset>
#include "TrigT1Run2ZDC.h"
#include "AthContainers/ConstAccessor.h"

using json = nlohmann::json;

namespace LVL1 {

 //--------------------------------
 // Constructors and destructors
 //--------------------------------

 TrigT1Run2ZDC::TrigT1Run2ZDC (const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm( name, pSvcLocator ) {}

 //---------------------------------
 // initialise()
 //---------------------------------

 StatusCode TrigT1Run2ZDC::initialize()
 {
   ATH_CHECK(m_zdcCTPLocation.initialize());
   ATH_CHECK(m_zdcModuleKey.initialize());
   ATH_CHECK(m_zdcModuleCalibEnergyKey.initialize());
   // Find the full path to filename:
   std::string file = PathResolverFindCalibFile(m_lutFile);
   ATH_MSG_INFO("Reading file " << file);
   std::ifstream fin(file.c_str());
   if(!fin){
      ATH_MSG_ERROR("Can not read file: " << file);
      return StatusCode::FAILURE;
   }
   json data = json::parse(fin);

   // Conversion factor from Run2 Energies (GeV) to dynamic range of LUT (4096 entries)
   std::vector<float> convfact;
   // if EnergyToADCScaleFactor is 0, using Run3 Data 
   // this means we make trigger decisions based on real trigger amplitudes
   convfact.push_back((m_energyToADCScaleFactor) ? static_cast<float>(m_energyToADCScaleFactor) : static_cast<float>(1.0));

   // Will eventually obtain LUTs from COOL, for now obtain them from calibration area
   // A data member to hold the side A LUT values
   std::array<unsigned int, 4096> sideALUT = data["LucrodHighGain"]["LUTs"]["sideA"];
   // A data member to hold the side C LUT values
   std::array<unsigned int, 4096> sideCLUT = data["LucrodHighGain"]["LUTs"]["sideC"];
   // A data member to hold the Combined LUT values
   std::array<unsigned int, 256> combLUT = data["LucrodHighGain"]["LUTs"]["comb"];

   // Construct Simulation Objects
   m_modInputs_p = std::make_shared<ZDCTriggerSim::ModuleAmplInputsFloat>(ZDCTriggerSim::ModuleAmplInputsFloat(convfact));
   m_simTrig = std::make_shared<ZDCTriggerSimModuleAmpls>(ZDCTriggerSimModuleAmpls(sideALUT, sideCLUT, combLUT));

   ATH_MSG_DEBUG("TrigT1Run2ZDC initilized");
   return StatusCode::SUCCESS;
 }

 //----------------------------------------------
 // execute() method called once per event
 //----------------------------------------------

 StatusCode TrigT1Run2ZDC::execute(const EventContext &ctx) const
 {
   // access ZDC modules
   SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_zdcModuleKey, ctx);
   // access ZDC aux data 
   SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibEnergyHandle( m_zdcModuleCalibEnergyKey, ctx);
   // create vector to store module CalibEnergy
   std::vector<float> moduleEnergy = {0., 0., 0., 0., 0., 0., 0., 0.};

   // Read Single Modules
   if (zdcModules.isValid())
   {
     for (const auto zdcModule : *zdcModules)
     {
       if (zdcModule->zdcType() == 0)
       { // type = 0 are big modules, type = 1 the pixels
       
       ATH_MSG_DEBUG("ZDC Side " << zdcModule->zdcSide() << ", Module: " << zdcModule->zdcModule() << " and Energy: " << zdcModuleCalibEnergyHandle(*zdcModule));
       // Side A
       static const SG::ConstAccessor<uint16_t> lucrodTriggerAmpAcc("LucrodTriggerAmp");
       if (zdcModule->zdcSide() > 0)
        {
          moduleEnergy.at(zdcModule->zdcModule()) = (!m_energyToADCScaleFactor && lucrodTriggerAmpAcc.isAvailable(*zdcModule)) ? static_cast<float>(lucrodTriggerAmpAcc(*zdcModule)) : zdcModuleCalibEnergyHandle(*zdcModule);
        }

       // Side C
       if (zdcModule->zdcSide() < 0)
        {
          moduleEnergy.at(zdcModule->zdcModule() + 4) = (!m_energyToADCScaleFactor && lucrodTriggerAmpAcc.isAvailable(*zdcModule)) ? static_cast<float>(lucrodTriggerAmpAcc(*zdcModule)) : zdcModuleCalibEnergyHandle(*zdcModule);
        }
       }
     }
   }
   
   // Get Output as an integer (0-7)
   m_modInputs_p->setData(moduleEnergy);

   // call ZDCTriggerSim to actually get ZDC Bits
   unsigned int wordOut = m_simTrig->simLevel1Trig(ZDCTriggerSim::SimDataCPtr(m_modInputs_p));

   // convert int to bitset
   std::bitset<3> bin(wordOut);

   // load output into trigger word on correct bits
   unsigned int word0 = 0;
   word0 += (bin[0] << 25);
   word0 += (bin[1] << 26);
   word0 += (bin[2] << 27);

   // form CTP obejct
   SG::WriteHandle<ZdcCTP> zdcCTP = SG::makeHandle(m_zdcCTPLocation, ctx);

   //record CTP object
   ATH_CHECK(zdcCTP.record(std::make_unique<ZdcCTP>(word0)));
   ATH_MSG_DEBUG("Stored ZDC CTP object with words " << std::hex << (zdcCTP->cableWord0()) << " from LUTOutput " << std::dec << wordOut);
   return StatusCode::SUCCESS;
  }
 }
