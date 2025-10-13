/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**

@page xAODForward_page xAODForward package

@section xAODForward_rcRetrieve Retrieving object in Root Core

   @subsection rcContainer Retrieving ALFADataContainer

   In the xAOD file an ALFADataContainer object is stored. It has to be
   read first with the help of an xAOD::TEvent object. When EventLoop
   is used, the code will be the following

   @code{.cpp}
   xAOD::TEvent* event = wk()->xaodEvent();
   const xAOD::ALFADataContainer* alfaContainer = NULL;
   if( ! event->retrieve( alfaContainer, "ALFADataContainer").isSuccess() ){
     Error("execute", "Failed to retrieve ALFADataContainer. Exiting." );
     return EL::StatusCode::FAILURE;
   }
   @endcode

   In the code above it is checked if reading of the
   ALFADataContainer was successful. Without it RootCore will issue
   a warning.

   @note
   Objects read from xAOD::TEvent are const pointers.

   The retrieved xAOD::ALFADataContainer has an interface similar to
   std::vector and contains two ALFAData objects. *The first* object
   is filled with the *TrackingData* tree and _the second_ with the _EventHeader_
   tree. The trees are described in the twiki page
   https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ALFAntupleAlg

   @warning Both ALFAData objects contain all the variables, but the
   variables that are not present in the corresponding tree contain
   default values.

   @subsection rcData Retrieving an ALFAData object from ALFADataContainer

   ALFAData objects can be read from the ALFADataContainer similarly
   to std::vector. It means that
   e.g. the at() method can be used.

   In order to read the first ALFAData object which contains
   TrackingData information the following code can be used:

   @code{.cpp}
   const xAOD::ALFAData* trackingData = alfaContainer->at(0);
   @endcode

   In order to read the first ALFAData object which contains
   event data information the following code can be used:

   @code{.cpp}
   const xAOD::ALFAData* eventData = alfaContainer->at(1);
   @endcode

   Detailed descriptions of the functions providing access to the specific
   variables contain information in which tree the variable is
   stored.

@section xAODForward_athena Retrieving data in Athena

   In the xAOD file an ALFADataContainer object is stored and has to
   be retrieved first. For example,

   @code{.cpp}
   const xAOD::ALFADataContainer* alfaContainer = 0;
   CHECK( evtStore()->retrieve( alfaContainer, "ALFADataContainer" ) );
   @endcode

   Retrieving ALFAData objects from the container object is common
   between Athena and RootCore, so please consult subsection @ref rcData

@section xAODForward_array2D Decoding vector indicies from serialised 2D arrays for tracks

   All arrays stored in the xAOD are one dimensional, so 2D arrays
   are transformed into 1D arrays in the xAOD.

   In the case of the 2D arrays storing information about tracks, the
   index of a 1D array is constructed in the following way:

   @verbatim
   potIndex*maxTrackCnt() + trackIndex
   @endverbatim

   where:
    - @c potIndex is the index of the pot ranging from 0 to 7,
    - maxTrackCnt() is the number of maximal number of tracks in one station,
    - @c trackIndex is the index of the track in a station.

   @warning
   The number of entries in the vector does not correspond to the number of
   tracks. To say if there is a track or not it has to be checked
   that the value of the variable is different from default one
   (usually -9999).

   In order to extract pot number from the array element index the
   following formula can be used

   @code{.cpp}
   int potIndex = index/maxTrackCnt();
   @endcode

   In order to extract the track index from the array element index, the
   following formula can be used

   @code{.cpp}
   int trackIndex = index%maxTrackCnt();
   @endcode

@section xAODForward_array2DFibers Decoding vector indices from serialised 2D arrays for plates

   All arrays stored in the xAOD are one dimensional, so 2D arrays
   are transformed into 1D arrays in the xAOD.

   In the case of the 2D arrays storing information about plates, the
   index of a 1D array is constructed in the following way:

   @verbatim
   potIndex*numberOfPlates + plateIndex
   @endverbatim

   where:
    - @c potIndex is the index of the pot ranging from 0 to 7,
    - @c numberOfPlates is the number of plates in a detector (20 for main detector and 3 for overlap detector),
    - @c plateIndex is the index of considered plate

   In order to extract the pot number from the array element index, the
   following formula can be used

   @code{.cpp}
   int potIndex = index/numberOfPlates;
   @endcode

   In order to extract plate index from the array element index the
   following formula can be used

   @code{.cpp}
   int trackIndex = index%numberOfPlates;
   @endcode

@section xAODForward_array3Dtracks Decoding vector indices with track information from serialised 3D arrays

   All arrays stored in the xAOD are one dimensional, so 3D arrays
   are transformed into 1D arrays in the xAOD.

   In the case of the 3D arrays storing information about tracks and
   plates, the index of a 1D array is constructed in the following
   way:

   @verbatim
   potIndex*maxTrackCnt()*numberOfPlates + trackIndex*numberOfPlates + plateIndex
   @endverbatim

   where:
    - @c potIndex is the index of the pot ranging from 0 to 7, 
    - @c maxTrackCnt() is the number of maximal number of tracks in one station,
    - @c numberOfPlates is the maximal number of plates that can be used in track reconstruction - for main detector tracks it is 20, for overlap detectors it is 3,
    - @c trackIndex is the index of the track in a station,
    - @c plateIndex index of considered plate (from 0 to 19 for main detectors or from 0 to 2 for overlap detectors)\

   @warning
   The number of entries in the vector does not correspond to
   the number of tracks. To say if there is a track or not it has to be
   checked that the track for this pot and this index was
   reconstructed in one of the detectors using getDetectorPartID().

   In order to extract the pot number from the array element index, the
   following formula can be used

   @code{.cpp}
   int potIndex = index/(maxTrackCnt()*numberOfPlates);
   @endcode

   In order to extract the track index from the array element index, the
   following formula can be used

   @code{.cpp}
   int trackIndex = (index/numberOfPlates)%maxTrackCnt();
   @endcode

   In order to extract the plate index from the array element index, the
   following formula can be used

   @code{.cpp}
   int plateIndex = index%numberOfPlates;
   @endcode

@section xAODForward_array3DFibers Decrypting vector index with fibers information from serialised 3D array

   All arrays stored in the xAOD are one dimensional, so 3D arrays
   are transformed into 1D arrays in the xAOD.
 
   In the case of the 3D arrays storing information about fibers, the
   index of a 1D array is constructed in the following way:

   @verbatim
   potIndex*numberOfPlates*numberOfLayers + plateIndex*numberOfLayers + layerIndex
   @endverbatim

   where:
    - @c potIndex is the index of the pot ranging from 0 to 7, 
    - @c numberOfPlates is the maximal number of plates that can be used in track reconstruction - for main detector tracks it is 20, for overlap detectors it is 3,
    - @c numberLayers is the number of fiber layers in a plate, which is equal to 64 for main detectors and 30 for overlap detectors,
    - @c plateIndex is the index of the considered plate of fibers (runs from 0 to 20 for main detectors and from 0 to 2 for overlap detectors)
    - @c layerIndex is the index of considered layer (runs from 0 to 63 for main detectors and from 0 to 29 for overlap detectors)

   In order to extract the pot number from the array element index, the
   following formula can be used

   @code{.cpp}
   int potIndex = index/(numberOfPlates*numberOfLayers);
   @endcode

   In order to extract the plate index from the array element index, the
   following formula can be used

   @code{.cpp}
   int plateIndex = (index/numberOfLayers)%numberOfPlates;
   @endcode

   In order to extract the layer index from the array element index, the
   following formula can be used

   @code{.cpp}
   int layerIndex = index%numberOfLayers;
   @endcode


*/
