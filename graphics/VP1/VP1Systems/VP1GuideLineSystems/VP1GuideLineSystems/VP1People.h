/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


//////////////////////////////////////////////////////////////////////
//                                                                  //
//  Header file for class VP1People                                 //
//                                                                  //
//  Description: Helper class providing people figures at scale.    //
//                                                                  //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch> //
//  Initial version: November 2021                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#ifndef VP1PEOPLE_H
#define VP1PEOPLE_H

#include "VP1Base/VP1HelperClassBase.h"
#include <QObject>
#include <Inventor/SbColor4f.h>

class SoMaterial;
class SoSeparator;

class VP1People : public QObject, public VP1HelperClassBase {

  Q_OBJECT

public:

  VP1People( SoMaterial * mat,
	      SoSeparator * attachsep,//where the letters separator will attach itself when visible
	      IVP1System * sys,QObject * parent = 0);
  virtual ~VP1People();

public Q_SLOTS:
  void setShown(bool);//will attach/detach itself from attachsep depending on this
  void setZPos(const double&);
  void setVerticalPosition(const double&);
  void setColourAndTransp(const SbColor4f&);

private:
  class Imp;
  Imp * m_d;

};

#endif
