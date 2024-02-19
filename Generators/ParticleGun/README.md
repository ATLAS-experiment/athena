# ParticleGun documentation

## Introduction

The ParticleGun generator is an event sampler from directly provided
kinematic distributions rather than microscopic physics, usually used
to generate single-particle events for performance testing, but also
capable of multiparticle events with possibly correlated distributions.

Unlike the previous ParticleGenerator package, ParticleGun allows
completely general kinematic and particle ID samplers to be passed to
the generator. The most common samplers, such as flat phi, const pT or
E, etc., are accessible in a much simpler way.

As the package is entirely written in Python, new samplers can be
written and used entirely within a job options script: it is
relatively simple to even do complex things like correlated sampling
of multiple particles this way, without the need to create a new
production tag of the ParticleGun package.


## Using ParticleGun

Here is an example job option steering fragment for constant-energy
particle production, sampled flat in an eta range:

```python
import ParticleGun as PG
pg = PG.ParticleGun()
pg.sampler.pid = 11
pg.sampler.mom = PG.EEtaMPhiSampler(energy=10000, eta=[-2,2])
topSeq += pg
```

Here a ParticleGun algorithm is created (first two lines), and is told
to make electrons on line 3 (default is geantinos, PID = 999). On line
4 the momentum sampler is chosen to be expressed in energy,
pseudorapidity, phi, and mass: the phi and mass are left at the
defaults of flat `0..2pi` and `0` respectively. The number given as
the energy argument is interpreted as a constant, the two-item list
given for eta is interpreted as a uniform sampler in that range.

Syntactic sugar is also provided for continuous sampling in a disjoint
range: `[0,1,2,4]` is interpreted as a flat sampler between 0..1 and
`2..4` (with twice as many events falling in the latter sub-range on
average, since it is twice as large) and with a unsampled gap between
`1..2`.


## Samplers

Explicit sampler objects can also be passed as arguments for full
control, in the same way as the `EEtaMPhiSampler` is passed above. The
set of samplers are defined and documented in
https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/python/samplers.py,
but for reference the main ones are listed here:

### Continuous samplers

- `UniformSampler`: flat sampling in a range
- `ModUniformSampler`: flat sampling in a range and its opposite sign
- `DisjointUniformSampler`: flat sampling in a set of disjoint ranges (e.g. eta crack regions)
- `LogSampler`: flat sampling "on a log scale", i.e. equal weight distributed over orders of magnitude
- `GaussianSampler`: sampling from a Gaussian distribution

### Discrete samplers

- `CyclicSeqSampler`: choose in increasing order from a discrete sequence of values, with cyclic repetition on the sequence length
- `RandomSeqSampler`: choose randomly from a discrete sequence of values
- `ConstSampler`: return a constant value

Several of these basic distribution samplers can be obtained via
Python literal forms:

- a Python `list` is treated as a `UniformSampler` if it has two entries, and a `DisjointUniformSampler` if it has more;
- a Python `tuple` is converted to the sequence of a `CyclicSeqSampler`; and
- a Python `set` is treated as a `RandomSeqSampler` sequence.

In Python 2.7 and 3.x these may all be provided as Python literals,
with syntaxes `[a,b,c]`, `(a,b,c)` and `{a,b,c}` respectively. This
conversion is implemented via the `mksampler(seq)` function, which is
automatically called inside the composite samplers now described,
making the configuration syntax in job-option scripts very clean.

### Composite samplers

- `PosSampler`: a position (and time) 4-vector sampler, with each x,y,z component sampled using the distribution samplers above NOTE: to be generalised to provide several coordinate basis representations;
- `MomSampler`: samplers of 4-momentum vectors, with the coordinates sampled via the distribution samplers above. There are versions for several coordinate bases:
  - `NullMomSampler`: return `(0,0,0,0)`
  - `MXYZSampler`
  - `EEtaMPhiSampler`
  - `ERapMPhiSampler`
  - `EThetaMPhiSample`
  - `MEtaMPhiSampler`
  - `MRapMPhiSampler`
  - `MThetaMPhiSample`
- `ParticleSampler`: a complete particle sampler, which uses a `MomSampler`
  for 4-momentum sampling, a discrete sampler for the particle-ID code and
  number of particles to sample, and a `PosSampler` for vertex positioning.
  This is the main configuration interface, via the sampler property of an
  Athena `ParticleGun` algorithm.

Any of these samplers may be subclassed or replaced with any Python
object possessing a no-argument `__call__` method which returns the
correct type. This can be useful for using existing functions, or
setting up complex configurations such as this correlated 2-particle
sampling example:
https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/share/examples/jobOption.ParticleGun_correlated.py


## Examples

More example job options may be found at
https://gitlab.cern.ch/atlas/athena/-/tree/main/Generators/ParticleGun/share/examples
including:

- the most common sort of configuration (cf. above): https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/share/examples/jobOption.ParticleGun_constenergy_flateta.py
- use of cyclic sequencers and a non-origin vertex positioning: https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/share/examples/jobOption.ParticleGun_fwd_sequence.py
- an example of a custom correlated 2-particle sampler: https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/share/examples/jobOption.ParticleGun_correlated.py
- a custom sampler using a ROOT histogram distribution: https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/share/examples/jobOption.ParticleGun_corrhist.py
- a custom sampler in 1/pt, with momentum / vertex correlation: https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/ParticleGun/share/examples/jobOption.ParticleGun_flatcurvature_flatip.py

Note that these are raw Athena job options. To convert them for use with the `Gen_tf` wrapper
"transform" script, `topAlg` needs to be replaced with `genSeq`, and the standard-production
`evgenConfig` metadata flags need to be set.


## Note on event structure

The events produced by ParticleGun have different structure with the
HepMC2 and HepMC3 backends.

For the HepMC2 case, each event contains at least one vertex with at
least one particle of interest attached to it. The spatial
distribution of those vertices of interest is regulated by the
ParticleGun settings. As there are no beams and at least some
particles are orphans, such an event record cannot be used with
some Athena algorithms might not work correctly on it. In particular,
the `TestHepMC` filter may need to be disabled in ParticleGun production
jobs.

For the HepMC3 case, the generated event is a valid Athena event.
Namely, it contains beams (by default 7 TeV protons), a primary
interaction point (always at (x=0,y=0,z=0) ) and no orphans, i.e. all
particles in the event are connected. Namely, the production vertex of
the particle of interest is connected to the interaction point with a
dummy particle with status 11 and the physical-particle PID.  This
dummy vertex and particle should be ignored in event analyses.  The
beam information can be potentially used for the analyses and probably
should be used for the booking purposes.  Other than that, the
kinematics of the particle of innterest and the spatial distribution
of the vertex of interest are exactly the same as for the HepMC2 case.


## To-do

- Revisit event structure in HepMC3: in particular the dummy vertex at (0,0,0,0)
  and the connecting dummy-particle PID codes can be misleading and should
  perhaps be changed to make the dummy nature clearer.
  