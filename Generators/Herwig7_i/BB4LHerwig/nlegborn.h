c -*- Fortran -*-
      integer nlegborn,nlegreal
      parameter (nlegborn=12 )

c     with resonances, nlegreal is no longer bound to be nlegborn+1
      parameter (nlegreal=13)
      
      integer ndiminteg
      parameter (ndiminteg=23)
      
      integer maxprocborn,maxprocreal
      parameter (maxprocborn=22,maxprocreal=90)
      
      integer maxalr
c      parameter (maxalr=maxprocreal*nlegreal*(nlegreal-1)/2)
      parameter (maxalr=159)


      integer maxreshists
      parameter (maxreshists=500)

c      integer maxresgroups
c      parameter (maxresgroups=2)

c these are the external (i.e. not including intermediate
c resonances) particles in the born process
      integer nlegbornexternal
      parameter (nlegbornexternal=8)
