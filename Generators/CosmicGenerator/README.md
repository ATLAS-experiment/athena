# CosmicGenerator

Authors: W. Seligman, M. Shapiro, I. Hinchliffe, M. Zdrazil,

## General information

CosmicGenerator package is a generator used for the cosmic particle production.
The output will is stored in the transient event store so it can be passed to t
he simulation.

The CosmicGenerator is used e.g. by default by the G4 cosmic simulation as a so
urce of cosmic muons at ground level.

One of the features of the CosmicGenerator is the ability to filter primary muo
ns depending on their direction and energy.
If you look in jobOptions_ConfigCosmicProd.py, you will find that the following
 properties can be set:

   - CosmicGenerator.emin, CosmicGenerator.emax: energy range for the primary muon

   - CosmicGenerator.xvert_low, CosmicGenerator.xvert_high, CosmicGenerator.zvert_low, CosmicGenerator.zvert_high: the (x,z) surface at ground level in which the primary vertex has to be created

   - CosmicGenerator.yvert_val: the y quota at which the primary vertexes must be created (i.e. the "ground level")

   - CosmicGenerator.ctcut: angular cut (wrt to the vertical)

Another set of properties allows further optimization:

   - CosmicGenerator.OptimizeForCavern: if True, muons are passed to the simulation only if they are pointing towards the interaction point, within a given tolerance. In order for this to work, the CosmicGenerator must be informed on where the IP is exactly. This is what the next properties are for

   - CosmicGenerator.IPx, CosmicGenerator.IPy, CosmicGenerator.IPz: the (x,y,z) coordinates of the IP

   - CosmicGenerator.Radius: the tolerance of the direction filtering. Only muons pointing inside a sphere centered in the IP with the given radius will be accepted.
