# Hijing 

Author: Georgios Stavropoulos (George.Stavropoulos@cern.ch)

Documentation for the Hijing MC can be found here:
http://www-nsdth.lbl.gov/~xnwang/hijing/index.html

The Hijing interface documentation and explanations how to use Hijing in Athena framework is described here:

## Notes on Hijing in Athena

This package runs Hijing from within Athena, puts the events into the
transient store in HepMC format. See the documentation on `GenModule` for
general information. The note refers only to Hijing specific
material. 
The module is activated from the jobOptions service. See the example
in `Hijing_i/share/jobOptions.hijing.py`

The Hijing parameters are set from the job options service. The
default parameters initialize Hijing for pp colisions at c.m. energy of 14 TeV.

Note that all parameters passed to Hijing are in the units specified
in the Hijing manual. In particular, energies and masses are in GeV,
not the standard atlas units of MeV.


Each quoted string sets one parameter. You can have as many as you like
seperated by commas. 
A variable must be one of the following variable names and
must be in lower case.

- `efrm`
- `frame`
- `proj`
- `targ`
- `iap`
- `izp`
- `iat`
- `izt`
- `bmin`
- `bmax`
- `nseed`
- `hipr1`
- `ihpr2`
- `hint1`
- `ihnt2`

An error message is returned if the specified variable is not in the above list. The job continues
to run but the behaviour may not be what you expect.
 The variables efrm to izt are the input parameters to Hijing initialization routine hijset
(see http://www-nsdth.lbl.gov/~xnwang/hijing/). Their default values are:
```
efrm=14000., frame='CMS', proj='P', targ='P', iap=izp=iat=izt=1.
```
The variables `bmin` and `bmax` are the input parameters to hijing routine
and their default value is `bmin`=`bmax`=0.
The variables `hipr1` to `ihnt2` are the arrays in the hiparnt common block. A detailed explanation
of these variables can be found in http://www-nsdth.lbl.gov/~xnwang/hijing/

`index` is the index in the arrays `hipr1`, `ihpr2`, `hint1` and `ihnt2`. For the other variables
has no meaning and SHOULD be omitted.

`value` is the parameter's value.

Example:
The following generates events for Au+Au collisions at 200 GeV c.m energy, switches off jet quenching,
switches on triggered jet production and sets the pt range of the triggered jets.
```
Hijing.Initialize = ["efrm 200", "frame CMS", "proj A", "targ A", 
                           "iap 197", "izp 79", "iat 197", "izt 79",
                           "ihpr2 4 0", "ihpr2 3 1", "hipr1 10 -20"]
```

### Random Numbers

`Hijing_i` is using the `AtRndmGenSvc` Athena Service to provide to Hijing the necessary random numbers.
The user can set the initial seeds of each stream via an option in the jobOption file.

The seeds of the random number service are saved for each event in the HepMC Event record and they are printed
on screen by `DumpMC`. In this way an event can be reproduced easily. The user has to rerun the job by simply seting
the seeds of the `HIJING` stream (the seeds of the `HIJING_INIT` stream should stay the same) to the seeds of that event.
