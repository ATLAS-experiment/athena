/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILESWITCHES_h
#define TILESWITCHES_h

/* This is a very simple struct for holding parameters
   which are stored in TileSwitches table in GeoModel DB
   and which can be overwritten from jobOptions
   Default value -1 means that value will be taken from GeoModel DB later on
*/

class TileSwitches
{
 public:

  /** Setup defaults */
  TileSwitches(bool tb=false, bool pl=true) :
          testBeam(tb),
          addPlatesToCell(pl),
          uShape(-1),
          glue(-1),
          pvt(-1),
          steel(-1),
          csTube(-1),
          crackOption(0)
      {}

  /** setting up testbeam geometry or ATLAS geometry **/
  bool testBeam;

  /** calculate cell volumes with or without front-plates and end-plates **/
  bool addPlatesToCell;

  /** 0: simulation without U-shape
      1: simulation with U-shape **/
  int  uShape;

  /** 0: glue layer is removed and replaced by iron,
      1: simulation with glue,
      2: glue is replaced by iron + width of iron is modified in order to get the same sampling fraction      */
  int glue;

  /** 0: all scintillators are polystyrene
      1: crack scrintillators are PVT, others - polystyrene **/
  int pvt;

  /** 0: Absorber is pure Iron
      1: Absorber is tile::Steel defined in DB **/
  int steel;

  /** 0: without Cesium tubes
      1: with cesium tubes **/
  int csTube;

  /** 0: crack scintillators in ext.barrel top-level envelopes
      1: crack scintillators in separate TileCal top-level envelopes
      2: crack scintillators in top-level enevelopes defined via jobOptions
      3: crack scintillators in top-level enevelopes passed by pointer in constructor
      4: positive crack scintillator in top-level enevelope passed by pointer in create method
      5: negative crack scintillator in top-level enevelope passed by pointer in create method
      9: crack scintillators are not created by TileAtlasFactory
     x0: where x=1,2,3,4,5 the same as 0, but skip creation of Barrel/ExtBar/ITC/Gap/Crack
     x1: where x=1,2,3,4,5 the same as 1, but skip creation of Barrel/ExtBar/ITC/Gap/Crack
     x2: where x=1,2,3,4,5 the same as 2, but skip creation of Barrel/ExtBar/ITC/Gap/Crack
     x3: where x=1,2,3,4,5 the same as 3, but skip creation of Barrel/ExtBar/ITC/Gap/Crack
     x9: where x=1,2,3,4,5 the same as 9, but skip creation of Barrel/ExtBar/ITC/Gap/Crack
     59: nothing is created **/
  int crackOption;

};

#endif
