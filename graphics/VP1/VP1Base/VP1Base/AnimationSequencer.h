/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1BASE_ANIMATIONSEQUENCER_H
#define VP1BASE_ANIMATIONSEQUENCER_H
//____________________________________________________________
//
// A sequencer for animating the camera...
//
// Joe Boudreau May 2008
//____________________________________________________________
#include "VP1Base/AnimationSequence.h"

#include <QObject>

class VP1ExaminerViewer;

class AnimationSequencer : public QObject {

  Q_OBJECT

 public:

  // Constructor
  AnimationSequencer(VP1ExaminerViewer *viewer);

  // Destructor
  ~AnimationSequencer();

  // Get the animation sequence (read/write)
  AnimationSequence & sequence();

  // Get the animation sequence (read only)
  const AnimationSequence & sequence() const;

  // Start animating:
  void startAnimating(bool skipFirstFrame = false);

  //For frame-by-frame output:
  void setMovie(bool);
  void setMovieParameters(const QString& outdir, const QString& frameFileNamePrefix, int fps, int width, int height);

signals:
  void animationFinishedSuccessfully();
  void clipVolumePercentOfATLAS(double);

private Q_SLOTS:

  // Next animation frame:
  void nextAnimationFrame();
  void abortAnimation();

private:

  class Clockwork;
  Clockwork *m_c;

};

#endif
