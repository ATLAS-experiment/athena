// -*- C++ -*-
//
// This is the implementation of the non-inlined, non-templated member
// functions of the bb4lFullShowerVeto class.
// Copyright (C) 2017, 2024 Silvia Ferrario Ravasio, Tomas Jezo
// inspired by Contrib/ShowrVeto/NonBShowerVeto.cc file
//

#include "bb4lFullShowerVeto.h"
#include "ThePEG/Interface/ClassDocumentation.h"
#include "ThePEG/EventRecord/Particle.h"
#include "ThePEG/Repository/UseRandom.h"
#include "ThePEG/Repository/EventGenerator.h"
#include "ThePEG/Utilities/DescribeClass.h"


using namespace Herwig;

IBPtr bb4lFullShowerVeto::clone() const {
  return new_ptr(*this);
}

IBPtr bb4lFullShowerVeto::fullclone() const {
  return new_ptr(*this);
}

// The following static variable is needed for the type
// description system in ThePEG. 
DescribeNoPIOClass<bb4lFullShowerVeto,FullShowerVeto>
  describeHerwigbb4lFullShowerVeto("Herwig::bb4lFullShowerVeto", "libpowhegHerwigBB4L.so");

void bb4lFullShowerVeto::Init() {

  static ClassDocumentation<bb4lFullShowerVeto> documentation
    ("The bb4lFullShowerVeto class vetos the parton-shower arising from top/anti-top decay if any branching in the shower has a scale harder than the one of the hardest radiation from top/anti-top decay in the Les Houches event");
  srand(1); //should be the same as not calling srand at all (init to 1 as default)

}

//In Herwig7.1, Lorentz5Momentum is in "Energy" unit, to compare a dot product with a double we need to remove two energy unit
double dotp(Lorentz5Momentum p1,Lorentz5Momentum p2)
{
  //double dotp=p1*p2*UnitRemoval::InvE*UnitRemoval::InvE/1000.0/1000.0;
  double dotp=p1*p2/1_GeV/1_GeV;
  return dotp;
}

	    
bool bb4lFullShowerVeto::vetoShower () {


   // Remnant events, in the bb4l default case, arise only from ISR.
   // Thus resonance decays shall not be vetoed.
  if((weights_.radtype==2) && (c_innlodec_.innlodec==0) )
    return false;
  

  /**
   * Incoming decayed particle 
   */
  vector<tPPtr>::const_iterator resPointer = incoming().begin();
  double resScale;
  std::string str_resonancetype="";
  if((**resPointer).id() == 6)
    {resScale=resonancevetos_.vetoscaletp; str_resonancetype = "(top resonance veto)";}
  else if((**resPointer).id() == - 6)
    {resScale=resonancevetos_.vetoscaletm; str_resonancetype = "(anti-top resonance veto)"; }
  else
  {
    //std::cout << "Not from top or anti-top resonance!" << std::endl; 
    return false;
  }


  //Top momentum
  Lorentz5Momentum ptop = (**resPointer).momentum();
  double bx, by, bz;  
  //Boost parameters to reach the top frame
  bx = - ptop.x()/ptop.t();
  by = - ptop.y()/ptop.t();
  bz = - ptop.z()/ptop.t();


  /**
   * Resonance direct sons
   */
  tPPtr bottom, gluon, gluon2, Wboson;
  bool gluonFound=false;
  for(vector<tPPtr>::const_iterator it = outgoing().begin(); it != outgoing().end(); ++it) {
    if(abs((**it).id()) == 5)
      bottom = *it;
    else if(abs((**it).id())== 24)
      Wboson = *it;
    else if((**it).id() == 21)
      {
  	gluon = *it;
  	gluonFound = true;
      }    
  }

  // before vetoing emissions, check that the invariant mass of the original bg pair
  // did not increase too much
  if(gluonFound){
    Lorentz5Momentum pW, pb, pg;
    pW = Wboson -> momentum();
    pb = bottom -> momentum();
    pg = gluon -> momentum();
    
    double mt2 = pow(ptop.m()/1_GeV, 2); 
    double mw2 = pow(pW.m()/1_GeV, 2);
    double mbg2 = ( pow((pb.e()+pg.e())/1_GeV, 2)-pow((pb.x()+pg.x())/1_GeV, 2)-pow((pb.y()+pg.y())/1_GeV, 2)-pow((pb.z()+pg.z())/1_GeV, 2));
    double mb2 = pow(bottom->nominalMass()/1_GeV,2); //pow(4.95,2);
    
    double Pfinal = sqrt(1.0 -2*(mw2+mbg2)/mt2+pow((mw2-mbg2)/mt2,2))/(mbg2-mb2);

    double pbg[4];
    for ( int i = 0; i <4; i++) pbg[i]=0.0;
    int itop=0;
    for ( int i = 2; i < hepeup_.nup; ++i )
      {
	if(hepeup_.idup[i]==(**resPointer).id()) itop=i+1; 
      }

    for ( int i = 2; i < hepeup_.nup; ++i )
    {
      if(hepeup_.getMother(i,0) == itop && abs(hepeup_.idup[i])!=24){
	for (int j=0; j<4; ++j) pbg[j]+=hepeup_.getP(i,j);
      }
    }

    mbg2 = pow(pbg[3],2);
    for (int i=0; i<3; ++i) mbg2-=pow(pbg[i],2);
    double Pinitial = sqrt(1.0 -2*(mw2+mbg2)/mt2+pow((mw2-mbg2)/mt2,2))/(mbg2-mb2);
    //random number function not being used for crypto, so safe here
    //coverity[dont_call]
    double r = ((double) rand() / (RAND_MAX));

    if(r > Pfinal/Pinitial)
    {
      return true;  
    } 
    
  
  }

  /**
   * Veto emission from bottom
   */
  Lorentz5Momentum pb, pg;
  double bScale = 0.0;
  //Follow the bottom line to find the hardest emission
  while( bottom->children().size() > 0 )
    {      
      //Emission from bottom
      if( bottom->children().size() == 2 )
  	{

  	  //Find the bottom and the gluon
  	  if(abs( bottom->children()[0]->id() ) == 5)
  	    {
	      gluon2 = bottom->children()[1];
	      bottom = bottom->children()[0];
  	    }
  	  else
  	    {
	      gluon2 = bottom->children()[0];
	      bottom = bottom->children()[1];
	      }

	  pb = bottom->momentum();
	  pg = gluon2->momentum();


	  //Veto only QCD radiation
	  if(gluon2->id() == 21){
	    //Boost everything in top frame and then compute the scale
	    pb = pb.boost(bx, by, bz);
	    pg = pg.boost(bx, by, bz);
	    bScale = sqrt(2.0*pg.t()/pb.t()*dotp(pb,pg));
	    if(bScale > resScale){       
        return true;  
	    }
	  }
  	}
      //Reshuffling
      else
  	{
  	  bottom = bottom->children()[0];
  	}
    }


  
  /**
   * Veto emission from gluon
   */
  //bW emission
  if( !gluonFound )
    {
      return false;
    }


  
  Lorentz5Momentum p0, p1;
  double gScale = 0.0;
  //Follow the hardest line to find the hardest emission as long as we have a g->gg splitting
  //while( (gluon->children().size()>0) && (gluon->children()[0]->id()==21) )
  while( (gluon->children().size()>0))
    {
      //Emission from gluon
      if(gluon->children().size() == 2)
  	{
  	  p0 = gluon->children()[0]->momentum();
  	  p1 = gluon->children()[1]->momentum();
  	  p0 = p0.boost(bx, by, bz);
  	  p1 = p1.boost(bx, by, bz);
  	  //Next gluon is the hardest (in the top frame)
  	  if( p0.t() > p1.t() )
  	    gluon = gluon->children()[0];
  	  else
  	     gluon = gluon->children()[1];

	  gScale = sqrt(2.0*p0.t()*p1.t()/((p0.t()+p1.t())*(p0.t()+p1.t()))*dotp(p0,p1));
	  if(  gScale > resScale )
	    {
        return true;  
	    }
	  //If the gluon has done g->qq splitting, we do not procede any further
	  if (gluon->id() != 21) return false;
  	}
      //Reshuffling
      else
  	{
  	  gluon = gluon->children()[0];
  	}
    }
  
  /**
   * Events passed the veto
  */
  return false;
}
