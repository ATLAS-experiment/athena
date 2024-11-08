
/***********************************************************************
* Copyright 1998-2024 CERN for the benefit of the EvtGen authors       *
*                                                                      *
* This file is part of EvtGen.                                         *
*                                                                      *
* EvtGen is free software: you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* EvtGen is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with EvtGen.  If not, see <https://www.gnu.org/licenses/>.     *
***********************************************************************/

#ifndef EVT_I_EXTERNALGENFACTORY_HH
#define EVT_I_EXTERNALGENFACTORY_HH

#include "CxxUtils/checker_macros.h"
#include "EvtGenModels/EvtAbsExternalGen.hh"

#include <map>

// Description: A factory type method to create engines for external physics
// generators like Pythia.

class EvtExternalGenFactory {
  public:
    enum genId
    {
        PythiaGenId = 0,
        PhotosGenId,
        TauolaGenId
    };

    static const EvtExternalGenFactory* getInstance();

    static EvtExternalGenFactory* getInstance_mutable ATLAS_NOT_THREAD_SAFE ();

    EvtAbsExternalGen* getGenerator( int genId = 0 ) const;

    void initialiseAllGenerators();

    void definePythiaGenerator( const std::string& xmlDir, bool convertPhysCodes,
                                bool useEvtGenRandom = true );
    void definePhotosGenerator( const std::string& photonType = "gamma",
                                bool useEvtGenRandom = true );
    void defineTauolaGenerator( bool useEvtGenRandom = true );

    //methods to add configuration commands to the pythia generators
    //void addPythiaCommand( std::string generator, std::string module, std::string param, std::string value);
    //void addPythia6Command(std::string generator, std::string module, std::string param, std::string value);

  protected:
    EvtExternalGenFactory();
    ~EvtExternalGenFactory();

    typedef std::map<int, EvtAbsExternalGen*> ExtGenMap;
    typedef std::map<int, std::map<std::string, std::vector<std::string>>> ExtGenCommandMap;

  private:
    EvtExternalGenFactory( const EvtExternalGenFactory& ){};

    ExtGenMap m_extGenMap;
    ExtGenCommandMap m_extGenCommandMap;
};

#endif
