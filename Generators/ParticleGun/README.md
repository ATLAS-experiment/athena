ParticleGun documentation
-------------------------
See https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ParticleGunForAtlas
for some coherent documentation that should be kept up to date.

Note on the produced events.
The events produced by the Particle Gun with the HepMC2 backend and 
HepMC3 backend have different structure.

For the HepMC2 case, each event contains at least one vertex  with at least one 
particle of interest attahced to it. The spatial distribution of those vertices of interest is 
regulated by the settings of the particle gun. As there are no beams and at least 
some particles are orphans, such an event record is not a valid Athena event and 
some algorithms might not work correctly on it.

For the HepMC3 case, the generated event is a valid Athena event.
Namely, it contains beams (by default 7 TeV protons), primary interaction point 
(always at (x=0,y=0,z=0) ) and no orphants, i.e. all particles in the event are connected.
Namely, the production vertex of the particle of interest is connected to the 
interaction point with a dummy particle with status 11.
As of now this particle should be ignored in the analyses.
The beam information can be potentially used for the analyses and probably should be used for the booking purposes.
Other than that, the kinematics of the particle of innterest and the spatial 
distribution of the vertex of interest are exactly the same as for the HePMC2 case.