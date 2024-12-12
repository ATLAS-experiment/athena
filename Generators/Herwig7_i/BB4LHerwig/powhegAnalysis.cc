// -*- C++ -*-
//
// powhegAnalysis.h - (c) Silvia Ferrario Ravasio and Tomas Jezo
// inspired by HepMCFile.h which is a part of ThePEG
//

#include "powhegAnalysis.h"
#include "ThePEG/Interface/ClassDocumentation.h"
#include "ThePEG/Interface/Parameter.h"
#include "ThePEG/Interface/Switch.h"
#include "ThePEG/EventRecord/Event.h"
#include "ThePEG/Repository/EventGenerator.h"
#include "ThePEG/Persistency/PersistentOStream.h"
#include "ThePEG/Persistency/PersistentIStream.h"

#include "herwig7_interface.h"

using namespace ThePEG;
extern int maxev;
bool vetoedEvent;
bool showerSuccess;
double extern powheg_weight;
powhegAnalysis::powhegAnalysis() 
  : m_runNumber(0), m_unitchoice(),
    m_geneventPrecision(16) {}

// Cannot copy streams. 
// Let doinitrun() take care of their initialization.
powhegAnalysis::powhegAnalysis(const powhegAnalysis & x) 
  : AnalysisHandler(x), m_runNumber(x.m_runNumber),
    m_unitchoice(x.m_unitchoice), m_geneventPrecision(x.m_geneventPrecision) {}

IBPtr powhegAnalysis::clone() const {
  return new_ptr(*this);
}

IBPtr powhegAnalysis::fullclone() const {
  return new_ptr(*this);
}

void powhegAnalysis::doinitrun() {
  AnalysisHandler::doinitrun();	  
  nevcounter_.nev=0;
}

void powhegAnalysis::dofinish() {
  AnalysisHandler::dofinish();
/*	return;	
  herwig7_end_(&(m_runNumber));*/
  cout << "\npowhegAnalysis: analysis finished.\n";
  cout << "Number of analyzed events: "<<nevcounter_.nev<<endl;
  cout.flush();
}

void powhegAnalysis::analyze(tEventPtr event, long, int, int) {

  showerSuccess=true;

  Energy eUnit;
  Length lUnit;
  switch (m_unitchoice) {
  default: eUnit = GeV; lUnit = millimeter; break;
  case 1:  eUnit = MeV; lUnit = millimeter; break;
  case 2:  eUnit = GeV; lUnit = centimeter; break;
  case 3:  eUnit = MeV; lUnit = centimeter; break;
  }


  vetoedEvent=false;
	
  if(!vetoedEvent){
    nevcounter_.nev++;
//    herwiganalysis_();
//    if( (nevcounter_.nev % 2000)==0) herwig7_end_(&(m_runNumber));
  } else {
    // should never be called, because vetoedEvent is set to false
    // to avoid warning of unused event object, which needs to be 
    // passed due to AnalysisHandler::analyze base class definition
    cout << "Event number: " <<  event->number() << endl;
  }
	

  if(nevcounter_.nev>=maxev)
    {
      powhegAnalysis::dofinish();          
    }
}


void powhegAnalysis::persistentOutput(PersistentOStream & os) const {
  os << m_runNumber << m_unitchoice << m_geneventPrecision;
}

void powhegAnalysis::persistentInput(PersistentIStream & is, int) {
  is >> m_runNumber >> m_unitchoice >> m_geneventPrecision;
}

const ClassDescription<powhegAnalysis> powhegAnalysis::m_initpowhegAnalysis;
// Definition of the static class description member.

void powhegAnalysis::Init() {

  static ClassDocumentation<powhegAnalysis> documentation
    ("This analysis handler will convert first into HepMC format then"
     "into hepevt common block and then run the analysis implemented in"
     "the file pwhg_analysis*.f (specified in the Makefile).");

  static Parameter<powhegAnalysis,int> interfaceRunNumber
    ("RunNumber",
     "The number identifying the run. The run number will be used in the filename of the"
     ".top file produced. For example, if run number is 2, `pwg-0002-POWHEG+HERWIG7-output.top`"
     "file containing the histograms will be produced. Values below 1 and above 9999 will be"
     "ignored.",
     &powhegAnalysis::m_runNumber,
     0, // the default value
     1, // the minimum
     9999, // the maximum
     true, // depsafe
     false, // readonly
     Interface::limited); //limits

  static Parameter<powhegAnalysis,unsigned int> interfacePrecision
    ("Precision",
     "Choice of output precision for the HepMC GenEvent format "
     " (as number of digits).",
     &powhegAnalysis::m_geneventPrecision, 16, 6, 16,
     false, false, Interface::limited);
  
  static Switch<powhegAnalysis,int> interfaceUnits
    ("Units",
     "Unit choice for energy and length",
     &powhegAnalysis::m_unitchoice, 0, false, false);
  static SwitchOption interfaceUnitsGeV_mm
    (interfaceUnits,
     "GeV_mm",
     "Use GeV and mm as units.",
     0);
  static SwitchOption interfaceUnitsMeV_mm
    (interfaceUnits,
     "MeV_mm",
     "Use MeV and mm as units.",
     1);
  static SwitchOption interfaceUnitsGeV_cm
    (interfaceUnits,
     "GeV_cm",
     "Use GeV and cm as units.",
     2);
  static SwitchOption interfaceUnitsMeV_cm
    (interfaceUnits,
     "MeV_cm",
     "Use MeV and cm as units.",
     3);
}
