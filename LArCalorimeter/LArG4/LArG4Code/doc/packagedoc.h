/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**


@page LArG4Code_page 

This package contains utility and helper classes used by other client packages
involved in LAr G4 simulation.

The standard and calibration sensitive detectors write into event-owned builder
containers stored in the `HitCollectionMap`.  The builders keep hit-merging
state for the full Athena event so that one Athena event can be transported by
multiple Geant4 events without depending on the lifetime or thread affinity of
Geant4 sensitive-detector instances.  Regular SDs register ordered partitions
inside the builders, preserving the historical behavior that hits merge within
one SD but not across different SDs.  Direct contributors such as frozen-shower
fast simulation use the unpartitioned bucket, which is finalized after regular
SD partitions.  These per-SD partitions are only needed to preserve that legacy
partitioned output contract, including SD ordering.  If the requirement for
identical historical output is dropped, the builders can be simplified to a
single event-wide merge bucket.

--------------------------------
  REQUIREMENTS 
--------------------------------


*/
