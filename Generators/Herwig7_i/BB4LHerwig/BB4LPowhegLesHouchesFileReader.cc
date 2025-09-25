// -*- C++ -*-
//
// LesHouchesFileReader.cc is a part of ThePEG - Toolkit for HEP Event Generation
// Copyright (C) 1999-2024 Leif Lonnblad
//
// ThePEG is licenced under version 3 of the GPL, see COPYING for details.
// Please respect the MCnet academic guidelines, see GUIDELINES for details.
//
//
// This is the implementation of the non-inlined, non-templated member
// functions of the BB4LPowhegLesHouchesFileReader class.
//

#include "BB4LPowhegLesHouchesFileReader.h"
#include "ThePEG/Interface/ClassDocumentation.h"
#include "ThePEG/Interface/Reference.h"
#include "ThePEG/Interface/Switch.h"
#include "ThePEG/Interface/Parameter.h"
#include "ThePEG/Utilities/Throw.h"
#include "ThePEG/PDT/DecayMode.h"
#include "ThePEG/Persistency/PersistentOStream.h"
#include "ThePEG/Persistency/PersistentIStream.h"
#include <sstream>
#include <iostream>
#include <regex>


using namespace ThePEG;
using namespace std;

HEPEUP hepeup_save;
string eventComments_save;
string outsideBlock_save;
map<string,double> optionalWeights_save;
map<string,double> optionalWeightsTemp_save;
map<string,string> eventAttributes_save;
int optionalnpLO_save, optionalnpNLO_save;
int maxev;
nevcounter nevcounter_;
const int maxShowerTries=10;
const int maxVetoTries=1000;
int showerTries;
int vetoTries;
extern bool vetoedEvent;
extern bool showerSuccess;

double ubcorrections[3];


BB4LPowhegLesHouchesFileReader::
BB4LPowhegLesHouchesFileReader(const BB4LPowhegLesHouchesFileReader & x)
  : LesHouchesReader(x), m_neve(x.m_neve), m_ieve(0),
    m_LHFVersion(x.m_LHFVersion), m_outsideBlock(x.m_outsideBlock),
    m_headerBlock(x.m_headerBlock), m_initComments(x.m_initComments),
    m_initAttributes(x.m_initAttributes), m_eventComments(x.m_eventComments),
    m_eventAttributes(x.m_eventAttributes),
    m_theFileName(x.m_theFileName), m_theQNumbers(x.m_theQNumbers),
    m_theIncludeFxFxTags(x.m_theIncludeFxFxTags),
    m_theIncludeCentral(x.m_theIncludeCentral),
    m_theDecayer(x.m_theDecayer) {}

BB4LPowhegLesHouchesFileReader::~BB4LPowhegLesHouchesFileReader() {}

IBPtr BB4LPowhegLesHouchesFileReader::clone() const {
  return new_ptr(*this);
}

IBPtr BB4LPowhegLesHouchesFileReader::fullclone() const {
  return new_ptr(*this);
}

bool BB4LPowhegLesHouchesFileReader::preInitialize() const {
  return true;
}

void BB4LPowhegLesHouchesFileReader::doinit  () {
  LesHouchesReader::doinit();

  nevcounter_.nev=0;  
  ubcorrections[0]=powheginput_("#ub_btilde_corr",15);
  if(ubcorrections[0]<=0) ubcorrections[0]=1.;
  ubcorrections[1]=powheginput_("#ub_remn_corr",13);
  if(ubcorrections[1]<=0) ubcorrections[1]=1.;
  ubcorrections[2]=powheginput_("#ub_reg_corr",12);
  if(ubcorrections[2]<=0) ubcorrections[2]=1.;

  
  // are we using QNUMBERS
  if(!m_theQNumbers) return;
  // parse the header block and create 
  // any new particles needed in QNUMBERS blocks
  string block = m_headerBlock;
  string line  = "";
  bool readingSLHA = false;
  int (*pf)(int) = tolower;
  unsigned int newNumber(0);
  do {
    line  = StringUtils::car(block,"\r\n");
    block = StringUtils::cdr(block,"\r\n");
    if(line[0]=='#') continue;
    // are we reading the SLHA block
    if(readingSLHA) {
      // reached the end of slha block
      if(line.find("</slha") != string::npos) {
	readingSLHA = false;
	break;
      }
      // remove trailing comment from line
      vector<string> split = StringUtils::split(line,"#");
      // check for a qnumbers block
      transform(split[0].begin(), split[0].end(), split[0].begin(), pf);
      // if not contine
      if(split[0].find("block qnumbers")==string::npos)
	continue;
      // get name from comment
      string name;
      if(split.size()>=2) {
	name = StringUtils::stripws(split[1]);
      }
      else {
	++newNumber;
	ostringstream tname;
	tname << "NP" << newNumber;
	name = tname.str();
      }
      // extract the PDG code
      split = StringUtils::split(split[0]," ");
      istringstream is(split[2]);
      long PDGCode(0);
      is >> PDGCode;
      // get the charge, spin, colour and whether an antiparticle
      int charge(0),spin(0),colour(0),anti(0);
      for(unsigned int ix=0;ix<4;++ix) {
	line = StringUtils::car(block,"\r\n");
	block = StringUtils::cdr(block,"\r\n");
	int dummy[2];
	istringstream is(line);
	is >> dummy[0] >> dummy[1];
	switch (dummy[0]) {
	case 1:
	  charge = dummy[1];
	  break;
	case 2:
	  spin   = dummy[1];
	  break;
	case 3:
	  colour = dummy[1];
	  break;
	case 4:
	  anti   = dummy[1];
	  break;
	default:
	  assert(false);
	}
      }
      // check if particles already exist
      PDPair newParticle;
      newParticle.first = getParticleData(PDGCode);
      if(newParticle.first) Throw<SetupException>()
			      << "Particle with PDG code " << PDGCode 
			      << " whose creation was requested in a QNUMBERS Block"
			      << " already exists. Retaining the original particle" 
			      << Exception::warning;
      if(anti) {
	newParticle.second = getParticleData(-PDGCode);
	if(newParticle.second) Throw<SetupException>()
				 << "Anti-particle with PDG code " << -PDGCode 
				 << " whose creation was requested in a QNUMBERS Block"
				 << " already exists. Retaining the original particle" 
				 << Exception::warning;
	if(( newParticle.first  && !newParticle.second ) ||
	   ( newParticle.second && !newParticle.first  ) )
	  Throw<SetupException>()
	    << "Either particle or anti-particle with PDG code " << PDGCode 
	    << " whose creation was requested in a QNUMBERS Block"
	    << " already exists, but not both the particle and antiparticle. "
	    << " Something dodgy here stopping" 
	    << Exception::runerror;
      }
      // already exists continue
      if(newParticle.first) continue;
      // create the particles
      // particle with no anti particle
      if( anti == 0 ) {
	// construct the name
	if(name=="") {
	  ostringstream temp;
	  temp << PDGCode;
	  name = temp.str(); 
	}
	// create the ParticleData object
	newParticle.first = ParticleData::Create(PDGCode,name);
      }
      // particle anti-particle pair
      else {
	// construct the names
	string nameAnti;
	if(name=="") {
	  ostringstream temp;
	  temp << PDGCode;
	  name = temp.str(); 
	  ostringstream temp2;
	  temp << -PDGCode;
	  nameAnti = temp2.str(); 
	}
	else {
	  nameAnti=name;
	  // cppcheck-suppress invalidContainer; replacing single characters
	  for(string::iterator it=nameAnti.begin();it!=nameAnti.end();++it) {
	    if(*it=='+')      nameAnti.replace(it,it+1,"-");
	    else if(*it=='-') nameAnti.replace(it,it+1,"+");
	  }
	  if(nameAnti==name) nameAnti += "bar";
	}
	// create the ParticleData objects
	newParticle = ParticleData::Create(PDGCode,name,nameAnti);
      }
      // set the particle properties
      if(colour==1) colour = 0;
      newParticle.first->iColour(PDT::Colour(colour));
      newParticle.first->iSpin  (PDT::Spin  (spin  ));
      newParticle.first->iCharge(PDT::Charge(charge));
      // register it
      generator()->preinitRegister(newParticle.first,
				   "/Herwig/Particles/"+newParticle.first->PDGName());
      // set the antiparticle properties
      if(newParticle.second) {
	if(colour==3||colour==6) colour *= -1;
	charge = -charge;
	newParticle.second->iColour(PDT::Colour(colour));
	newParticle.second->iSpin  (PDT::Spin  (spin  ));
	newParticle.second->iCharge(PDT::Charge(charge));
	// register it
	generator()->preinitRegister(newParticle.second,
				     "/Herwig/Particles/"+newParticle.second->PDGName());
      }
    }
    // start of SLHA block ?
    else if(line.find("<slha") != string::npos) {
      readingSLHA = true;
    }
  }
  while(line!="");
  // now set any masses/decay modes
  block = m_headerBlock;
  line="";
  readingSLHA=false;
  bool ok=true;
  do {
    line = StringUtils::car(block,"\r\n");
    block = StringUtils::cdr(block,"\r\n");
    // are we reading the SLHA block
    if(readingSLHA) {
      // reached the end?
      if(line.starts_with("</slha")) {
	readingSLHA = false;
	break;
      }
      // make lower case
      transform(line.begin(),line.end(),line.begin(), pf);
      // found the mass block ?
      if(line.find("block mass")!=string::npos) {
	// read it
	line = StringUtils::car(block,"\r\n");
	// check not at end
	while(line[0] != 'D' && line[0] != 'B' &&
	      line[0] != 'd' && line[0] != 'b' &&
	      line    != "") {
	  // skip comment lines
	  if(line[0] == '#') {
	    block = StringUtils::cdr(block,"\r\n");
	    line = StringUtils::car(block,"\r\n");
	    continue;
	  }
	  // get the mass and PGD code
	  istringstream temp(line);
	  long id;
	  double mass;
	  temp >> id >> mass;
	  // skip resetting masses on SM particles
	  // as it can cause problems later on in event generation
	  if(std::abs(id) <= 6 || (std::abs(id) >= 11 && std::abs(id) <= 16) ||
	     std::abs(id) == 23 || std::abs(id) == 24) {
	    block = StringUtils::cdr(block,"\r\n");
	    line = StringUtils::car(block,"\r\n");
	    continue;
	  }
	  // magnitude of mass for susy models
	  mass = abs(mass);
	  // set the mass
	  tPDPtr particle = getParticleData(id);
	  if(!particle) throw SetupException() 
			  << "BB4LPowhegLesHouchesFileReader::doinit() - Particle with PDG code not"
			  << id << " not found." << Exception::runerror;
	  const InterfaceBase * ifb = BaseRepository::FindInterface(particle,
								    "NominalMass");
	  ostringstream os;
	  os << mass;
	  ifb->exec(*particle, "set", os.str());
	  // read the next line
	  block = StringUtils::cdr(block,"\r\n");
	  line = StringUtils::car(block,"\r\n");
	}; 
      }
      // found a decay block
      else if(line.starts_with("decay")) {
	// get PGD code and width
	istringstream iss(line);
	string dummy;
	long parent(0);
	Energy width(ZERO);
	iss >> dummy >> parent >> iunit(width, GeV);
	// get the ParticleData object
	PDPtr inpart = getParticleData(parent);
	if(!inpart)  {
	  throw SetupException() 
	    << "BB4LPowhegLesHouchesFileReader::doinit() - A ParticleData object with the PDG code "
	    << parent << " does not exist. " << Exception::runerror;
	  return;
	}
	if ( std::abs(inpart->id()) == 6 || 
	     std::abs(inpart->id()) == 15 || 
	     std::abs(inpart->id()) == 23 || 
	     std::abs(inpart->id()) == 24 || 
	     std::abs(inpart->id()) == 25 ) {
	  Throw<SetupException>() << "\n"
	    "************************************************************************\n"
	    "* Your LHE file changes the width of " << inpart->PDGName() << ".\n"
	    "* This can cause serious problems in the event generation!\n"
	    "************************************************************************\n"
	    "\n" << Exception::warning;
	}
	else if (inpart->width() > ZERO && width <= ZERO) {
	  Throw<SetupException>() << "\n"
	    "************************************************************************\n"
	    "* Your LHE file zeroes the non-zero width of " << inpart->PDGName() << ".\n"
	    "* If " << inpart->PDGName() << " is a decaying SM particle,\n"
	    "*     this can cause serious problems in the event generation!\n"
	    "************************************************************************\n"
				  "\n" << Exception::warning;
	}
	// set the width
	inpart->width(width);
	if( width > ZERO ) {
	  inpart->cTau(hbarc/width);
	  inpart->widthCut(5.*width);
	  inpart->stable(false);
	}
	// construct prefix for DecayModes
	string prefix(inpart->name() + "->"), tag(prefix),line("");
	unsigned int nmode(0);
	// read any decay modes
	line = StringUtils::car(block,"\r\n");
	while(line[0] != 'D' && line[0] != 'B' &&
	      line[0] != 'd' && line[0] != 'b' && 
	      line[0] != '<' && line    != "") {
	  // skip comments
	  if(line[0] == '#') {
	    block = StringUtils::cdr(block,"\r\n");
	    line = StringUtils::car(block,"\r\n");
	    continue;
	  }
	  // read decay mode and construct the tag
	  istringstream is(line);
	  double brat(0.);
	  unsigned int nda(0),npr(0);
	  is >> brat >> nda;
	  while( true ) {
	    long t;
	    is >> t;
	    if( is.fail() ) break; 
	    if( t == std::abs(parent) )
	      throw SetupException() 
		<< "An error occurred while read a decay of the " 
		<< inpart->PDGName() << ". One of its products has the same PDG code "
		<< "as the parent particle in BB4LPowhegLesHouchesFileReader::doinit()."
		<< " Please check the Les Houches file.\n"
		<< Exception::runerror;
	    tcPDPtr p = getParticleData(t);
	    if( !p )
	      throw SetupException()
		<< "BB4LPowhegLesHouchesFileReader::doinit() -"
		<< " An unknown PDG code has been encounterd "
		<< "while reading a decay mode. ID: " << t
		<< Exception::runerror;
	    ++npr;
	    tag += p->name() + ",";
	  }
	  if( npr != nda )
	    throw SetupException()
	      << "BB4LPowhegLesHouchesFileReader::doinit() - While reading a decay of the " 
	      << inpart->PDGName() << " from an SLHA file, an inconsistency "
	      << "between the number of decay products and the value in "
	      << "the 'NDA' column was found. Please check if the spectrum "
	      << "file is correct.\n"
	      << Exception::warning;
	  // create the DecayMode
	  if( npr > 1 ) {
	    if( nmode==0 ) {
	      generator()->preinitInterface(inpart, "VariableRatio" , "set","false");
	      if(inpart->massGenerator()) {
		ok = false;
		Throw<SetupException>()
		  << inpart->PDGName() << " already has a MassGenerator set"
		  << " this is incompatible with using QNUMBERS "
		  << "Use\n"
		  << "set " << inpart->fullName() << ":Mass_generator NULL\n"
		  << "to fix this." << Exception::warning;
	      }
	      if(inpart->widthGenerator()) {
		ok = false;
		Throw<SetupException>()
		  << inpart->PDGName() << " already has a WidthGenerator set"
		  << " this is incompatible with using QNUMBERS "
		  << "Use\n"
		  << "set " << inpart->fullName() << ":Width_generator NULL\n"
		  << "to fix this." << Exception::warning;
	      }
	      unsigned int ntemp=0;
	      for(DecaySet::const_iterator dit = inpart->decayModes().begin();
		  dit != inpart->decayModes().end(); ++dit ) {
		if((**dit).on()) ++ntemp;
	      }
	      if(ntemp!=0) {
		ok = false;
		Throw<SetupException>()
		  << inpart->PDGName() << " already has DecayModes"
		  << " this is incompatible with using QNUMBERS "
		  << "Use\n"
		  << "do " << inpart->fullName() << ":SelectDecayModes none\n" 
		  << " to fix this." << Exception::warning;
	      }
	    }
	    inpart->stable(false);
	    tag.replace(tag.size() - 1, 1, ";");
	    DMPtr dm = generator()->findDecayMode(tag);
	    if(!m_theDecayer) Throw<SetupException>()
			      << "BB4LPowhegLesHouchesFileReader::doinit() Decayer must be set using the "
			      << "BB4LPowhegLesHouchesFileReader:Decayer"
			      << " must be set to allow the creation of new"
			      << " decay modes."
			      << Exception::runerror;
	    if(!dm) {
	      dm = generator()->preinitCreateDecayMode(tag);
	      if(!dm)
		Throw<SetupException>()  
		  << "BB4LPowhegLesHouchesFileReader::doinit() - Needed to create "
		  << "new decaymode but one could not be created for the tag " 
		  << tag << Exception::warning;
	    }
	    generator()->preinitInterface(dm, "Decayer", "set",
					  m_theDecayer->fullName());
	    ostringstream br;
	    br << setprecision(13) << brat;
	    generator()->preinitInterface(dm, "BranchingRatio", "set", br.str());
	    generator()->preinitInterface(dm, "Active", "set", "Yes");
	    if(dm->CC()) {
	      generator()->preinitInterface(dm->CC(), "BranchingRatio", "set", br.str());
	      generator()->preinitInterface(dm->CC(), "Active", "set", "Yes");
	    }
	    ++nmode;
	  }
	  tag=prefix;
	  // read the next line
	  block = StringUtils::cdr(block,"\r\n");
	  line = StringUtils::car(block,"\r\n");
	};
	if(nmode>0) {
	  inpart->update();
	  if(inpart->CC())
	    inpart->CC()->update();
	}
      }
    }
    // start of SLHA block ?
    else if(line.find("<slha") != string::npos) {
      readingSLHA = true;
    }
  }
  while(line!="");
  if(!ok)
    throw SetupException() << "Problem reading QNUMBERS blocks in BB4LPowhegLesHouchesFileReader::doinit()"
			   << Exception::runerror;
}

void BB4LPowhegLesHouchesFileReader::initialize(LesHouchesEventHandler & eh) {
  LesHouchesReader::initialize(eh);
  if ( m_LHFVersion.empty() )
    Throw<powhegLesHouchesFileError>()
      << "The file associated with '" << name() << "' does not contain a "
      << "proper formatted Les Houches event file. The events may not be "
      << "properly sampled." << Exception::warning;
}

vector<string> BB4LPowhegLesHouchesFileReader::optWeightsNamesFunc() { return optionalWeightsNames; }

void BB4LPowhegLesHouchesFileReader::open  () {
  vetoTries=0;
  vetoedEvent=false;
  showerTries=0;
  showerSuccess=true;

  ubcorrections[0]=powheginput_("#ub_btilde_corr",15);
  if(ubcorrections[0]<=0) ubcorrections[0]=1.;
  ubcorrections[1]=powheginput_("#ub_remn_corr",13);
  if(ubcorrections[1]<=0) ubcorrections[1]=1.;
  ubcorrections[2]=powheginput_("#ub_reg_corr",12);
  if(ubcorrections[2]<=0) ubcorrections[2]=1.;
  
  if ( filename().empty() )
    throw powhegLesHouchesFileError()
      << "No Les Houches file name. "
      << "Use 'set " << name() << ":FileName'."
      << Exception::runerror;
  m_cfile.open(filename());
  if ( !m_cfile )
    throw powhegLesHouchesFileError()
      << "The BB4LPowhegLesHouchesFileReader '" << name() << "' could not open the "
      << "event file called '" << m_theFileName << "'."
      << Exception::runerror;

  m_cfile.readline();
  if ( !m_cfile.find("<LesHouchesEvents") ) return;
  map<string,string> attributes =
    StringUtils::xmlAttributes("LesHouchesEvents", m_cfile.getline());
  m_LHFVersion = attributes["version"];
  if ( m_LHFVersion.empty() ) return;

  bool readingHeader = false;
  bool readingInit = false;
  m_headerBlock = "";
  
//  char (cwgtinfo_weights_info[250][15]);
  string hs;
//  int cwgtinfo_nn(0);

  // Loop over all lines until we hit the </init> tag.
  bool readingInitWeights = false, readingInitWeights_sc = false;
  string weightinfo;
  while ( m_cfile.readline() ) {
    // found the init block for multiple weights
    if(m_cfile.find("<initrwgt")) readingInitWeights = true;
    
    // found end of init block for multiple weights: end the while loop
    if(m_cfile.find("</initrwgt")) {
      readingInitWeights = false;
      readingInitWeights_sc = false;
      continue;
    }

    // found the end of init block
    if(m_cfile.find("</init")) {
      readingInit = false;
      break;
    } 

    /* read the weight information block 
     * optionalWeightNames will contain information about the weights
     * this will be appended to the weight information
     */ 
    if(readingInitWeights) {
      string scalename = "";
      if(m_cfile.find("<weightgroup")) {
	/* we found a weight group
	 * start reading the information 
	 * within it
	 */
	readingInitWeights_sc = true;
	weightinfo = m_cfile.getline();
	/* to make it shorter, erase some stuff
	 */
	string str_weightgroup = "<weightgroup";
	string str_arrow = ">";
	string str_newline = "\n";
	erase_substr(weightinfo, str_weightgroup);
	erase_substr(weightinfo, str_arrow);
	erase_substr(weightinfo, str_newline);
	continue;
      }
      /* if we are reading a new weightgroup, go on 
       * until we find the end of it
       */
      /* BEGIN MOD*/
      if(readingInitWeights_sc && !m_cfile.find("</weightgroup")) {
	hs = m_cfile.getline();
	hs.erase(std::remove(hs.begin(), hs.end(), ' '), hs.end());
	std::string IdLabel = hs.substr(0, hs.find(">", 0));
	IdLabel = std::regex_replace(IdLabel, std::regex(R"([\D])"), "");
	std::string name = hs;
	erase_substr(name, "<weightid='"+IdLabel+"'>");
	name.erase(name.find('<'));
	m_optionalWeightsLabel[IdLabel]=name;	
	optionalWeightsNames.push_back(std::move(name));
      }
      /*END MOD*/
    }
   
    if ( m_cfile.find("<header") ) {
      // We have hit the header block, so we should dump this and all
      // following lines to m_headerBlock until we hit the end of it.
      readingHeader = true;
      m_headerBlock = m_cfile.getline() + "\n";
    }
    if ( (m_cfile.find("<init ") && !m_cfile.find("<initrwgt")) || m_cfile.find("<init>") ) {
      // We have hit the init block, so we should expect to find the
      // standard information in the following. But first check for
      // attributes.
      m_initAttributes = StringUtils::xmlAttributes("init", m_cfile.getline());
      readingInit = true;
      m_cfile.readline();
      if ( !( m_cfile >> heprup.IDBMUP.first >> heprup.IDBMUP.second
		    >> heprup.EBMUP.first >> heprup.EBMUP.second
	            >> heprup.PDFGUP.first >> heprup.PDFGUP.second
	            >> heprup.PDFSUP.first >> heprup.PDFSUP.second
		    >> heprup.IDWTUP >> heprup.NPRUP ) ) {
	heprup.NPRUP = -42;
	m_LHFVersion = "";
	return;
      }
      heprup.resize();
      for ( int i = 0; i < heprup.NPRUP; ++i ) {
	m_cfile.readline();
	if ( !( m_cfile >> heprup.XSECUP[i] >> heprup.XERRUP[i]
	              >> heprup.XMAXUP[i] >> heprup.LPRUP[i] ) ) {
	  heprup.NPRUP = -42;
	  m_LHFVersion = "";
	  return;
	}
      }
    }
    if ( m_cfile.find("</header") ) {
      readingHeader = false;
      m_headerBlock += m_cfile.getline() + "\n";
    }
    if ( readingHeader ) {
      /* We are in the process of reading the header block. Dump the
	 line to m_headerBlock.*/
      m_headerBlock += m_cfile.getline() + "\n";
    }
    if ( readingInit ) {
      // Here we found a comment line. Dump it to m_initComments.
      m_initComments += m_cfile.getline() + "\n";
    }
  }
  string central = "central";
  if (m_theIncludeCentral) optionalWeightsNames.push_back(std::move(central));

  if ( !m_cfile ) {
    heprup.NPRUP = -42;
    m_LHFVersion = "";
    return;
  }

 //copy in the heprup fortran common block
  heprup_.idbmup[0]= heprup.IDBMUP.first;
  heprup_.idbmup[1]= heprup.IDBMUP.second;
  heprup_.ebmup[0] = heprup.EBMUP.first;
  heprup_.ebmup[1] = heprup.EBMUP.second;
  heprup_.pdfgup[0]= heprup.PDFGUP.first;
  heprup_.pdfgup[1]= heprup.PDFGUP.second;
  heprup_.pdfsup[0]= heprup.PDFSUP.first;
  heprup_.pdfsup[1]= heprup.PDFSUP.second;
  heprup_.idwtup   = heprup.IDWTUP;
  heprup_.nprup    = heprup.NPRUP;
  for ( int i = 0; i < heprup.NPRUP; ++i ) 
    {     
      heprup_.xsecup[i]= heprup.XSECUP[i];
      heprup_.xerrup[i]= heprup.XERRUP[i];
      heprup_.xmaxup[i]= heprup.XMAXUP[i];
      heprup_.lprup[i] = heprup.LPRUP[i];
    }

  // This call sets the herwigSettings_ common block.
  // In particular the weveto member is set to true if
  // vetoing (for resonances) is to be done by the fortran code,
  // false otherwise

  herwig7_init_(maxev,filename().c_str(), filename().size());
  for ( int i = 0; i < max_num_weights; ++i )
    {
      weights_.weight[i]=0;
    }
  weights_.radtype=0;
  
}

bool BB4LPowhegLesHouchesFileReader::doReadEvent  () {

  
  if ( !m_cfile ) return false;
  if ( m_LHFVersion.empty() ) return false;
  if ( heprup.NPRUP < 0 ) return false;

    // we have showered all the events
  if(nevcounter_.nev==maxev) return false; 
  bool repeatEvent;

  
  m_eventComments = "";
  m_outsideBlock = "";
  hepeup.NUP = 0;
  hepeup.XPDWUP.first = hepeup.XPDWUP.second = 0.0;
  optionalWeights.clear();
  m_optionalWeightsTemp.clear();

  //Increase the tries counters:
  //a)showerTries parametries the tries to shower an event
  //b)vetoTries parametrises the tries to have a showered event that passes the veto.
  if(!showerSuccess)
    {
      showerTries++;
    }
  else {
    // If we managed to shower:
    showerTries=1;
    if(vetoedEvent)
      vetoTries++;
    else
      // the event has passed the veto:
      vetoTries = 1;
  }

 //Herwig manage to shower: check if we have vetoed the previous event
  if(showerSuccess)
    {
      if(vetoedEvent)
	{
    std::cout << "vetoed events: vetoTries " << vetoTries << " maxVetoTries: " << maxVetoTries << std::endl;
	  if( vetoTries<maxVetoTries) repeatEvent=true;
	  else
	    {
	      repeatEvent=false;
	      std::cout<<"Herwig could not pass the weto for the event with nevhep="<<nevcounter_.nev<<" after "<< maxVetoTries<<" tries..."<<endl;
	      std::cout<<"top scale: "<<resonancevetos_.vetoscaletp<<"; anti-top scale: "<<resonancevetos_.vetoscaletm<<endl;
	      //If we have to skip this event and do a new one, increase the evt counter and set vetoTries=1 for the new event
	      nevcounter_.nev=nevcounter_.nev+1;
	      // we abandon this event:
	      vetoTries=1;
	      showerTries=1;
	    }
	}
      else
	{
	  repeatEvent=false;
	  vetoTries=1;
	  showerTries=1;
	}
    }
  else // not showerSuccess
    {
      if(showerTries<maxShowerTries) repeatEvent=true;
      else
	{
	  repeatEvent=false;
	  std::cout<<"Herwig could not shower event with nevhep="<<nevcounter_.nev<<" after "<< maxShowerTries<<" tries..."<<endl;
	  //If we have to skip this event and do a new one, increase the evt counter and set showerTries=1 for the new event
	  nevcounter_.nev=nevcounter_.nev+1;
	  showerTries=1;
	  vetoTries=1;
	}
    }

  if(repeatEvent){
    
    // Herwig might have messed up the Les Houches event;
    // copy it back here.
    hepeup=hepeup_save;
    m_eventComments=eventComments_save;
    m_outsideBlock=outsideBlock_save;
    optionalWeights=optionalWeights_save;
    m_optionalWeightsTemp=optionalWeightsTemp_save;
    m_eventAttributes=eventAttributes_save;
    optionalnpLO=optionalnpLO_save;
    optionalnpNLO=optionalnpNLO_save;
    showerSuccess=false;
    // Go back to Herwig with the same event loaded
    return true;
  }
  
  //Here we start reading a new event

  
  // Keep reading lines until we hit the next event or the end of
  // the event block. Save any inbetween lines. Exit if we didn't
  // find an event.
  while ( m_cfile.readline() && !m_cfile.find("<event") )
    m_outsideBlock += m_cfile.getline() + "\n";

  // We found an event. First scan for attributes.
  m_eventAttributes = StringUtils::xmlAttributes("event", m_cfile.getline());

  /* information necessary for FxFx merging:
   * the npLO and npNLO tags
   */
  istringstream ievat(m_cfile.getline());
  int we(0), npLO(-99), npNLO(-99);
  do {
    string sub; ievat >> sub;
    if(we==2) { npLO = atoi(sub.c_str()); }
    if(we==5) { npNLO = atoi(sub.c_str()); }
    ++we;
  } while (ievat);
  optionalnpLO = npLO;
  optionalnpNLO = npNLO;
  std::stringstream npstringstream;
  npstringstream << "np " << npLO << " " << npNLO;
  std::string npstrings = npstringstream.str();
  /* the FxFx merging information 
   * becomes part of the optionalWeights, labelled -999 
   * for future reference
   */

  if(m_theIncludeFxFxTags) optionalWeights[npstrings.c_str()] = -999;

  if ( !m_cfile.readline()  ) return false;

  // The first line determines how many subsequent particle lines we
  // have.
  if ( !( m_cfile >> hepeup.NUP >> hepeup.IDPRUP >> hepeup.XWGTUP
	        >> hepeup.SCALUP >> hepeup.AQEDUP >> hepeup.AQCDUP ) )
    return false;
  hepeup.resize();
  // Read all particle lines.
  for ( int i = 0; i < hepeup.NUP; ++i ) {
    if ( !m_cfile.readline() ) return false;
    if ( !( m_cfile >> hepeup.IDUP[i] >> hepeup.ISTUP[i]
	          >> hepeup.MOTHUP[i].first >> hepeup.MOTHUP[i].second
         	  >> hepeup.ICOLUP[i].first >> hepeup.ICOLUP[i].second
	          >> hepeup.PUP[i][0] >> hepeup.PUP[i][1] >> hepeup.PUP[i][2]
	          >> hepeup.PUP[i][3] >> hepeup.PUP[i][4]
        	  >> hepeup.VTIMUP[i] >> hepeup.SPINUP[i] ) )
      return false;
    //print momenta for debugging
    if(std::isnan(hepeup.PUP[i][0])||std::isnan(hepeup.PUP[i][1])||
       std::isnan(hepeup.PUP[i][2])||std::isnan(hepeup.PUP[i][3])||
       std::isnan(hepeup.PUP[i][4])) 
      throw Exception() 
	<< "nan's as momenta in Les Houches file "
	<< Exception::eventerror;
    if(hepeup.MOTHUP[i].first -1==i || 
       hepeup.MOTHUP[i].second-1==i) {
      throw Exception()
	<< "Particle has itself as a mother in Les Houches "
	<< "file, this is not allowed\n"
	<< Exception::eventerror;
    } 
  }



 /**
   * We copy the event in the fortran common block
   * and we compute the scale of the resonances
   */
  
  hepeup_.nup=hepeup.NUP;
  hepeup_.idprup=hepeup.IDPRUP;
  hepeup_.xwgtup= hepeup.XWGTUP;
  hepeup_.scalup=hepeup.SCALUP;
  hepeup_.aqedup=hepeup.AQEDUP;
  hepeup_.aqcdup=hepeup.AQCDUP;

  for ( int i = 0; i < hepeup.NUP; ++i ) 
    { 
      hepeup_.idup[i]     = hepeup.IDUP[i];
      hepeup_.istup[i]    = hepeup.ISTUP[i];
      hepeup_.getMother(i,0)= hepeup.MOTHUP[i].first;
      hepeup_.getMother(i,1)= hepeup.MOTHUP[i].second;
      hepeup_.getColor(i,0)= hepeup.ICOLUP[i].first;
      hepeup_.getColor(i,1)= hepeup.ICOLUP[i].second;
      hepeup_.getP(i,0)   = hepeup.PUP[i][0];
      hepeup_.getP(i,1)   = hepeup.PUP[i][1];
      hepeup_.getP(i,2)   = hepeup.PUP[i][2];
      hepeup_.getP(i,3)   = hepeup.PUP[i][3];
      hepeup_.getP(i,4)   = hepeup.PUP[i][4];
      hepeup_.vtimup[i]   = hepeup.VTIMUP[i]; 
      hepeup_.spinup[i]   = hepeup.SPINUP[i]; 
    }

  herwig7_calculatescales_();

  // Initial value for number of weights. 
  weights_.numweights=0;



  
  LHEeventnum = -1; // set the LHEeventnum to -1, this will be the default if the tag <event num> is not found
  
  // Now read any additional comments and named weights.
  // read until the end of rwgt is found
  bool readingWeights = false, readingaMCFast = false, readingMG5ClusInfo = false;
  while ( m_cfile.readline() && !m_cfile.find("</event>")) {


    //Reading POWHEG extra info
    if(m_cfile.find("#rwgt")){
      istringstream iss(m_cfile.getline());
      int wi=0;
      do {
	string sub; iss >> sub;
	if(wi==1) weights_.radtype = stoi(sub.c_str());
	wi++;
      }while(iss);
    }
    
    
    if(m_cfile.find("</applgrid")) { readingaMCFast=false; } //aMCFast weights end
    if(m_cfile.find("</rwgt")) { readingWeights=false; } //optional weights end
    if(m_cfile.find("</clustering")) { readingMG5ClusInfo=false; } // mg5 mclustering scale info end
    
    /* reading of optional weights
     */
    /* BEGIN MOD*/
    if(readingWeights) { 
      if(!m_cfile.find("<wgt")) { continue; }
      istringstream iss(m_cfile.getline());
      double weightValue(0);
      std::string hs=m_cfile.getline();
      hs.erase(std::remove(hs.begin(), hs.end(), ' '), hs.end());
      std::string IdLabel = hs.substr(0, hs.find(">", 0));
      IdLabel = std::regex_replace(IdLabel, std::regex(R"([\D])"), "");
      std::string weightName = m_optionalWeightsLabel[IdLabel];
      std::string value = std::move(hs);
      erase_substr(value, "<wgtid='"+IdLabel+"'>");
      erase_substr(value, "<wgtid=\""+IdLabel+"\">");
      erase_substr(value, "</wgt>");
      erase_substr(value, "\n");
      weightValue=std::stod(value);
      m_optionalWeightsTemp[weightName] = weightValue; 
      optionalWeights[weightName] = weightValue; 
    }
    /* END MOD */
    
    /* reading of aMCFast weights
     */
    if(readingaMCFast) {
      std::stringstream amcfstringstream;
      amcfstringstream << "aMCFast " << m_cfile.getline();
      std::string amcfstrings = amcfstringstream.str();
      string str_newline = "\n";
      erase_substr(amcfstrings,str_newline);
      optionalWeights[amcfstrings.c_str()] = -111; //for the aMCFast we give them a weight -111 for future identification
    }

    /* read additional MG5 Clustering information 
     * used in LO merging
     */
    if(readingMG5ClusInfo) {
      string hs = m_cfile.getline();
      string startDEL = "<clus scale="; //starting delimiter
      string stopDEL = "</clus>"; //end delimiter
      unsigned firstLim = hs.find(startDEL); //find start of delimiter
      string mg5clusinfo = hs.substr(firstLim); //define the information for the scale
      erase_substr(mg5clusinfo,stopDEL);
      erase_substr(mg5clusinfo,startDEL);
      string str_arrow = ">";
      erase_substr(mg5clusinfo,str_arrow);
      string str_quotation = "\"";
      erase_substr(mg5clusinfo,str_quotation);
      optionalWeights[mg5clusinfo.c_str()] = -222; //for the mg5 scale info weights we give them a weight -222 for future identification
    }
    
    //store MG5 clustering information 
    if(m_cfile.find("<scales")) {
      string hs = m_cfile.getline();
      string startDEL = "<scales"; //starting delimiter
      string stopDEL = "</scales>"; //end delimiter
      unsigned firstLim = hs.find(startDEL); //find start of delimiter
      string mg5scinfo = hs.substr(firstLim); //define the information for the scale
      erase_substr(mg5scinfo,stopDEL);
      erase_substr(mg5scinfo,startDEL);
      string str_arrow = ">";
      erase_substr(mg5scinfo,str_arrow);
      optionalWeights[mg5scinfo.c_str()] = -333; //for the mg5 scale info weights we give them a weight -333 for future identification
    }

    //the event num tag
    if(m_cfile.find("<event num")) {
      string hs = m_cfile.getline();
      string startDEL = "<event num"; //starting delimiter
      string stopDEL = "</event num>"; //end delimiter
      unsigned firstLim = hs.find(startDEL);
      string LHEeventnum_str = hs.substr(firstLim);
      erase_substr(LHEeventnum_str,stopDEL);
      erase_substr(LHEeventnum_str,startDEL);
      string str_arrow = ">";
      erase_substr(LHEeventnum_str,str_arrow);
      LHEeventnum =  std::stol(LHEeventnum_str, nullptr, 10); 
    }

    //determine start of aMCFast weights
    if(m_cfile.find("<applgrid")) { readingaMCFast = true;}
    //determine start of optional weights
    if(m_cfile.find("<rwgt")) { readingWeights = true; }
    //determine start of MG5 clustering scale information
    if(m_cfile.find("<clustering")) { readingMG5ClusInfo = true;}
  }
  string str_quote = "'";
  string str_doublequote = "\"";
  // loop over the optional weights and add the extra information as found in the init
  for (map<string,double>::const_iterator it=m_optionalWeightsTemp.begin(); it!=m_optionalWeightsTemp.end(); ++it){
    //to avoid issues with inconsistencies of defining the weight ids, remove "" and ''
    string id_1 = it->first;
    erase_substr(id_1, str_quote);
    erase_substr(id_1, str_doublequote);
    for (map<string,string>::const_iterator it2=m_scalemap.begin(); it2!=m_scalemap.end(); ++it2){
      //find the scale id in the scale information and add this information
      string id_2 = it2->first;
      erase_substr(id_2, str_quote);
      erase_substr(id_2, str_doublequote);
      if(id_1==id_2) {
        string info = it2->second;
	string str_newline = "\n";
	erase_substr(info, str_newline);
	//set the optional weights
	optionalWeights[info] = it->second;
      }
    }
  }

  /* additionally, we set the "central" scale
   * this is actually the default event weight 
   */
  string central = "central";
  optionalWeights[central] = hepeup.XWGTUP; 
 /* We copy the event in global variable, in order to be able to access them in the next iteration*/
  hepeup_save=hepeup;
  eventComments_save=m_eventComments;
  outsideBlock_save=m_outsideBlock;
  optionalWeights_save=optionalWeights;
  optionalWeightsTemp_save=m_optionalWeightsTemp;
  eventAttributes_save=m_eventAttributes;
  optionalnpLO_save=optionalnpLO;
  optionalnpNLO_save=optionalnpNLO;

  showerSuccess=false;

  
  if ( !m_cfile ) return false;
  return true;
  
}

void BB4LPowhegLesHouchesFileReader::close() {
  m_cfile.close();
}

void BB4LPowhegLesHouchesFileReader::persistentOutput(PersistentOStream & os) const {
  os << m_neve << m_LHFVersion << m_outsideBlock << m_headerBlock << m_initComments
     << m_initAttributes << m_eventComments << m_eventAttributes << m_theFileName
     << m_theQNumbers << m_theIncludeFxFxTags << m_theIncludeCentral << m_theDecayer ;
}

void BB4LPowhegLesHouchesFileReader::persistentInput(PersistentIStream & is, int) {
  is >> m_neve >> m_LHFVersion >> m_outsideBlock >> m_headerBlock >> m_initComments
     >> m_initAttributes >> m_eventComments >> m_eventAttributes >> m_theFileName
     >> m_theQNumbers >> m_theIncludeFxFxTags >> m_theIncludeCentral >> m_theDecayer;
  m_ieve = 0;
}

ClassDescription<BB4LPowhegLesHouchesFileReader>
BB4LPowhegLesHouchesFileReader::m_initBB4LPowhegLesHouchesFileReader;
// Definition of the static class description member.

void BB4LPowhegLesHouchesFileReader::Init () {

  static ClassDocumentation<BB4LPowhegLesHouchesFileReader> documentation
    ("ThePEG::BB4LPowhegLesHouchesFileReader is an base class to be used for objects "
     "which reads event files from matrix element generators. This class is "
     "able to read plain event files conforming to the Les Houches Event File "
     "accord.");

  static Parameter<BB4LPowhegLesHouchesFileReader,string> interfaceFileName
    ("FileName",
     "The name of a file containing events conforming to the Les Houches "
     "protocol to be read into ThePEG. A file name ending in "
     "<code>.gz</code> will be read from a pipe which uses "
     "<code>zcat</code>. If a file name ends in <code>|</code> the "
     "preceeding string is interpreted as a command, the output of which "
     "will be read through a pipe.",
     &BB4LPowhegLesHouchesFileReader::m_theFileName, "", false, false);

  interfaceFileName.fileType();
  interfaceFileName.rank(11);

  static Switch<BB4LPowhegLesHouchesFileReader,bool> interfaceQNumbers
    ("QNumbers",
     "Whether or not to read search for and read a QNUMBERS"
     " block in the header of the file.",
     &BB4LPowhegLesHouchesFileReader::m_theQNumbers, false, false, false);
  static SwitchOption interfaceQNumbersYes
    (interfaceQNumbers,
     "Yes",
     "Use QNUMBERS",
     true);
  static SwitchOption interfaceQNumbersNo
    (interfaceQNumbers,
     "No",
     "Don't use QNUMBERS",
     false);

    static Switch<BB4LPowhegLesHouchesFileReader,bool> interfaceIncludeFxFxTags
    ("IncludeFxFxTags",
     "Include FxFx tags",
     &BB4LPowhegLesHouchesFileReader::m_theIncludeFxFxTags, false, true, false);
  static SwitchOption interfaceIncludeFxFxTagsYes
    (interfaceIncludeFxFxTags,
     "Yes",
     "Use the FxFx tags",
     true);
  static SwitchOption interfaceIncludeFxFxTagsNo
    (interfaceIncludeFxFxTags,
     "No",
     "Don't use the FxFx tags",
     false);

  static Switch<BB4LPowhegLesHouchesFileReader,bool> interfaceIncludeCentral
    ("IncludeCentral",
     "Include definition of central weight",
     &BB4LPowhegLesHouchesFileReader::m_theIncludeCentral, false, true, false);
  static SwitchOption interfaceIncludeCentralYes
    (interfaceIncludeCentral,
     "Yes",
     "include definition of central weight",
     true);
  static SwitchOption interfaceIncludeCentralNo
    (interfaceIncludeCentral,
     "No",
     "Don't include definition of central weight",
     false);



  static Reference<BB4LPowhegLesHouchesFileReader,Decayer> interfaceDecayer
    ("Decayer",
     "Decayer to use for any decays read from the QNUMBERS Blocks",
     &BB4LPowhegLesHouchesFileReader::m_theDecayer, false, false, true, true, false);

}

void BB4LPowhegLesHouchesFileReader::erase_substr(std::string& subject, const std::string& search) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
      subject.erase( pos, search.length() );
    }
}
