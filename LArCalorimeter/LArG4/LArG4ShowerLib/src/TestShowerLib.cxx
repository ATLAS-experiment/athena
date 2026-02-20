/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// this header file
#include "LArG4ShowerLib/TestShowerLib.h"

// CLHEP incldues
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

//#include <algorithm>
//#include <functional>
#include <sstream>
#include <fstream>

#include <iostream>

// local includes
//#include "LArG4ShowerLib/PositionBin.h"

// G4 includes
#include "G4Track.hh"

#include "LArG4Code/EnergySpot.h"
#include "LArG4ShowerLib/ShowerEnergySpot.h"

#include "TTree.h"
#include "TFile.h"
#include "TParameter.h"

#define LIB_VERSION 10

namespace ShowerLib {

  TestShowerLib::~TestShowerLib()
  {
  }

  IShowerLib* TestShowerLib::readFromROOTFile(TFile* source)
  {
      TParameter<int>* ver;
      ver = (TParameter<int>*)source->Get("version");

      if ((ver == nullptr) || (ver->GetVal() != LIB_VERSION)) return nullptr; //Test library header = 10

      TTree* TTreeMeta = (TTree*)source->Get("meta");
      TTree* TTreeLib  = (TTree*)source->Get("library");

      if ((TTreeMeta == nullptr) || (TTreeLib == nullptr)) return nullptr;

      std::cout << "TestShowerLib header found." << std::endl;

	  TestShowerLib* newlib = new TestShowerLib();

	  if (!(newlib->readMeta(TTreeMeta)) || !(newlib->read(TTreeLib))) {
		  delete newlib;
	      std::cout << "TestShowerLib read unsuccessful." << std::endl;
		  return nullptr;
	  }

	  return newlib;

  }

  IShowerLib* TestShowerLib::createEmptyLib(const std::string& inputFile)
  {
	  /*
	   * Test library Structure format:
	   *
	   * VER PART DET
	   * COMMENT
	   *
	   * where
	   *
	   * VER == 10
	   */
	  std::ifstream filestr(inputFile.c_str(),std::ios::in);

	  if (!filestr.is_open()) {
		  std::cout << "TestShowerLib         " << inputFile << ": bad file!" << std::endl;
		  return nullptr;
	  }

	  std::string instr;
	  std::getline(filestr,instr);
	  std::stringstream ss(instr);

	  int ver;

	  ss >> ver;

	  if (ver != LIB_VERSION) {
		  return nullptr;
	  }


	  int part;
	  std::string det;

	  ss >> part >> det;


	  TestShowerLib* newlib = new TestShowerLib();

	  newlib->m_detector = std::move(det);
	  newlib->m_particle = part;
	  newlib->m_filled = false;

	  std::getline(filestr,instr);
	  newlib->m_comment = std::move(instr);

	  return newlib;
  }


  std::vector<EnergySpot>* TestShowerLib::getShower(const G4Track* , ShowerLibStatistics* , int ) const
  {
	  if (!m_filled) {
		  std::cout << "Library is not created for production use" << std::endl;
		  return nullptr;
	  }

	  std::cout << "Library is only for testing, not for production use" << std::endl;
	  return nullptr;
  }

  double TestShowerLib::getContainmentZ(const G4Track* ) const
  {
	  if (!m_filled) {
		  std::cout << "Library is not created for production use" << std::endl;
		  return 0.0;
	  }

	  std::cout << "Library is only for testing, not for production use" << std::endl;
	  return 0.0;
  }

  double TestShowerLib::getContainmentR(const G4Track* ) const
  {
	  if (!m_filled) {
		  std::cout << "Library is not created for production use" << std::endl;
		  return 0.0;
	  }

	  std::cout << "Library is only for testing, not for production use" << std::endl;
	  return 0.0;
  }

bool TestShowerLib::storeShower(HepMC::ConstGenParticlePtr genParticle, const Shower* shower)
  {
	  if (m_filled) {
		  std::cout << "ERROR: filled" << std::endl;
		  return false;
	  }

	  genInfo theinfo{};
	  theinfo.vertex = std::make_unique<HepMC::FourVector>(genParticle->production_vertex()->position());
	  theinfo.momentum = std::make_unique<HepMC::FourVector>(genParticle->momentum());

	  m_libData.emplace_back(std::move(theinfo), *shower);

	  return true;
  }

  bool TestShowerLib::writeToROOT(TFile* dest)
  {
	  if (m_libData.empty()) return false;
	  TParameter<int> ver("version",LIB_VERSION);

      dest->WriteObject(&ver,"version");

      TTree TTreeMeta;
      TTree TTreeLib;

      write(&TTreeLib);
      writeMeta(&TTreeMeta);

      dest->WriteObject(&TTreeLib,"library");
      dest->WriteObject(&TTreeMeta,"meta");

	  return true;
  }


  bool TestShowerLib::read(TTree* source)
  {
	  /*
	   * Eta Energy library format:
	   * |       x      |       y      |       z      |    e     |  time  |  - name of branch in TTree
	   * ------------------------------------------------------------------
	   * |    vertex    |    vertex    |    vertex    |  num of  |  cont  |  - shower header
	   * |      X       |      Y       |      Z       |  hits    |    Z   |
	   * ------------------------------------------------------------------
	   * |   momentum   |   momentum   |   momentum   |  truth   |  cont  |  - shower header
	   * |      X       |      Y       |      Z       |  energy  |    R   |
	   * ------------------------------------------------------------------
	   * |x-coord of hit|y-coord of hit|z-coord of hit|dep.energy|hit time|  - hit
	   */
	  int nentr = source->GetEntriesFast();
	  if (nentr < 3) return false;
	  Float_t x,y,z,e,time;
	  source->SetBranchAddress("x",&x);
	  source->SetBranchAddress("y",&y);
	  source->SetBranchAddress("z",&z);
	  source->SetBranchAddress("e",&e);
	  source->SetBranchAddress("time",&time);
	  int entr = 0;

	  do {
		  //read eta bin header
		  source->GetEntry(entr++);
		  int nhits = (int)(e+0.1); // +0.1 just in case - c++ has low round
		  Shower shower;
		  shower.setZSize(time);
		  genInfo theinfo{};
		  theinfo.vertex = std::make_unique<HepMC::FourVector>(x,y,z,0);
		  source->GetEntry(entr++);
		  shower.setRSize(time);
		  theinfo.momentum = std::make_unique<HepMC::FourVector>(x,y,z,e);
		  for(int i = 0; i < nhits; i++) {
			  source->GetEntry(entr++); //variables mean what the name suggests
			  shower.push_back(new ShowerEnergySpot(G4ThreeVector(x,y,z),e,time));
		  }
		  m_libData.emplace_back(std::move(theinfo),shower);
	  } while (entr < nentr);

	  m_filled = true;
	  return true;
  }

  bool TestShowerLib::write(TTree* dest) const
  {
	  /*
	   * Eta Energy library format:
	   * |       x      |       y      |       z      |    e     |  time  |  - name of branch in TTree
	   * ------------------------------------------------------------------
	   * |    vertex    |    vertex    |    vertex    |  num of  |  cont  |  - shower header
	   * |      X       |      Y       |      Z       |   hits   |    Z   |
	   * ------------------------------------------------------------------
	   * |   momentum   |   momentum   |   momentum   |  truth   |  cont  |  - shower header
	   * |      X       |      Y       |      Z       |  energy  |    R   |
	   * ------------------------------------------------------------------
	   * |x-coord of hit|y-coord of hit|z-coord of hit|dep.energy|hit time|  - hit
	   */
	  Float_t x,y,z,e,time;
	  dest->Branch("x",&x);
	  dest->Branch("y",&y);
	  dest->Branch("z",&z);
	  dest->Branch("e",&e);
	  dest->Branch("time",&time);
          for (const storedShower& lib : m_libData) {
		  HepMC::FourVector vertex = *lib.first.vertex;
		  HepMC::FourVector momentum = *lib.first.momentum;
		  x = vertex.x();
		  y = vertex.y();
		  z = vertex.z();
		  e = lib.second.size();
		  time = lib.second.getZSize();
		  dest->Fill(); //eta bin header
		  x = momentum.px();
		  y = momentum.py();
		  z = momentum.pz();
		  e = momentum.e();
		  time = lib.second.getRSize();
		  dest->Fill(); //eta bin header
                  for (const ShowerEnergySpot* spot : lib.second) {
			  x = spot->GetPosition().x();
			  y = spot->GetPosition().y();
			  z = spot->GetPosition().z();
			  e = spot->GetEnergy();
			  time = spot->GetTime();
			  dest->Fill();
		  }
	  }
	  //dest->Write();
	  return true;
  }

  ShowerLibStatistics* TestShowerLib::createStatistics() const
  {
	  return nullptr;
  }

} // namespace ShowerLib
