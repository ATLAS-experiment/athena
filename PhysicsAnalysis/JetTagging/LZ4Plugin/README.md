LZ4 Plugin for HDF5
===================

While using LZ4 for HDF5 compression is pretty widespread, I wasn't able to find a common package. Instead there seem to be a lot of people copying the same underlying `H5LZ4.c` file. Inside the file is almost exactly the same.

Here I'm taking a version from [DiamondLightSource][1]. It's nearly identical to the one from [the HDF5 group][2], but had fewer macros. There's also [a version that is used for h5py][3], and [the NeXus version][4] which also look nearly identical.

[1]: https://github.com/DiamondLightSource/hdf5filters/blob/master/h5lzfilter/H5Zlz4.c
[2]: https://github.com/HDFGroup/hdf5_plugins/blob/master/LZ4/src/H5Zlz4.c
[3]: https://github.com/silx-kit/hdf5plugin/blob/main/lib/LZ4/H5Zlz4.c
[4]: https://github.com/nexusformat/HDF5-External-Filter-Plugins/blob/master/LZ4/src/H5Zlz4.c
