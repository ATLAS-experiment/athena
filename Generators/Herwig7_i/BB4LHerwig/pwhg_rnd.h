c -*- fortran -*-
      
      integer rnd_numseeds,rnd_initialseed,rnd_iwhichseed,rnd_i1,rnd_i2
      character * 8 rnd_cwhichseed
      common/pwhg_rnd/rnd_numseeds,rnd_initialseed,rnd_iwhichseed,
     1 rnd_i1,rnd_i2,rnd_cwhichseed
      save /pwhg_rnd/
