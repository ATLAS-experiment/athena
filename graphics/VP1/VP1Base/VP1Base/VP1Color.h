/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1COLOR_H
#define VP1COLOR_H

#include "Inventor/nodes/SoMaterial.h"

#include <iostream>

class VP1Color {

public:

static float getValFromRGB(const int rgb)
{
    return rgb/255.0;
}

static void setColorFromRGB(SoMaterial* mat, const std::string& type, const int r, const int g, const int b)
{
    const float fr = getValFromRGB(r);
    const float fg = getValFromRGB(g);
    const float fb = getValFromRGB(b);
    if (type == "ambient")
        mat->ambientColor.setValue(fr, fg, fb);
    else if (type == "diffuse")
        mat->diffuseColor.setValue(fr, fg, fb);
    else if (type == "specular")
        mat->specularColor.setValue(fr, fg, fb);
    else if (type == "emissive")
        mat->emissiveColor.setValue(fr, fg, fb);
    else 
        std::cout << "ERROR! Color type not supported ==> " << type << std::endl;
    
    // Debug Msg
    //std::cout << "Set color (" << r << "," << g << "," << b << ") to (" << fr << "," << fg << "," << fb << ")" << std::endl;
    return;
}

};

#endif