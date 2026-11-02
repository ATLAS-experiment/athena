/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1GEOMETRYSYSTEMS_GEOVISATTRIBUTES_H
#define VP1GEOMETRYSYSTEMS_GEOVISATTRIBUTES_H

#include <string>
#include <optional>
#include <Inventor/SbColor.h>
#include <QByteArray>
#include <QMap>
#include <QString>
class SoMaterial;

class VisAttributes {

 public:

  // Constructor:
  VisAttributes();
  //delete copy
  VisAttributes(const VisAttributes &) = delete;
  //delete assignment
  VisAttributes & operator=(const VisAttributes &)=delete;
  void init();//Call at end of constructor in derived classes.
  // Destructor:
  virtual ~VisAttributes();

  // Fetch a material
  SoMaterial *get(const std::string & name) const;

  // Optional outline colour; absence preserves the inherited material colour.
  std::optional<SbColor> outlineColour(const std::string& name) const;

  //Override transparencies of materials (will multiply current transparencies):
  void overrideTransparencies(float transpfact);

  QByteArray getState(bool onlyChangedMaterials=true);
  void applyState(QByteArray);

 protected:

  // Add a material
  void add(const std::string & name, SoMaterial *);

  void setOutlineColour(const std::string& name, const SbColor&);

  // set SoMaterial color from RGB values
  void setColorFromRGB(SoMaterial* mat, const std::string& type, const int r, const int g, const int b);

 private:

  class Imp;
  Imp * m_d;

  // convert the usual range RGB [0,255] to Coin [0,1]
  float getValFromRGB(const int rgb);

};


// A list of Vis attributes for Detectors:
class DetVisAttributes : public VisAttributes {
  public:
  DetVisAttributes();
};


// A list of Vis attributes for Materials:
class MatVisAttributes : public VisAttributes {
  public:
  MatVisAttributes();
};


// A list of Vis attributes for Volumes:
class VolVisAttributes : public VisAttributes {
  public:
  VolVisAttributes();
};

#endif
