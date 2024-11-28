/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class VP1AvailEvtsLocalDir                //
//                                                            //
//  Description: Implementation of VP1AvailEvents which       //
//               monitors events as they appear in a local    //
//               directory.                                   //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2008                                 //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef VP1AVAILEVTSLOCALDIR_H
#define VP1AVAILEVTSLOCALDIR_H

#include "VP1Gui/VP1AvailEvents.h"

class VP1AvailEvtsLocalDir : public VP1AvailEvents {

  Q_OBJECT

public:

  friend class VP1LocalEvtRetriever;

  VP1AvailEvtsLocalDir( int timeCutForNew,
			const QString& sourcedir,
			const QString& tmpcopydir,
			int maxLocalFilesToKeep = -1,
			QObject * parent = 0 );
  void setSourceDir(const QString&);
  const QString& currentSourceDir() const;

  //Just to let this class transport the info around:
  void setAvailableSourceDirectories(const QStringList&);
  const QStringList& availableSourceDirectories() const;

  virtual ~VP1AvailEvtsLocalDir();

  virtual void init();

private:
  class Imp;
  Imp * m_d;
};

#endif
