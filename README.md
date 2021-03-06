**cupla** - C++ User interface for the Platform independent Library Alpaka
==========================================================================

Cupla is a simple user interface for the platform independent parallel kernel
acceleration library
[alpaka](https://github.com/ComputationalRadiationPhysics/alpaka).
It follows a similar concept as the
[Nvidia CUDA (TM) API](https://developer.nvidia.com/cuda-zone) by
providing a software layer to manage accelerator devices.
alpaka is used as backend for cupla.

Please keep in mind that a first, "find & replace" port from
**CUDA to cupla(x86)** will result in rather bad performance. To get decent
performance on x86 systems you just need to add the alpaka element level to
your kernels.

(Read: add some *tiling* to your CUDA kernels by letting the same thread
compute a fixed number of elements (N=4..16) instead of just computing one
*element* per thread. Also, make the number of elements in your tiling a
*compile-time constant* and your CUDA code (N=1) will just stay with the
very same performance while adding single-source performance portability for,
e.g., x86 targets).


Software License
----------------

**cupla** is licensed under **LGPLv3** or later.

For more information see [LICENSE.md](LICENSE.md).


Dependencies
------------

- **cmake 3.3.0**
- **alpaka** 
  - is loaded as `git submodule` within cupla (see [INSTALL.md](INSTALL.md))
  - for more information please read [README.md](https://github.com/ComputationalRadiationPhysics/alpaka/blob/master/README.md)

Usage
-----

- See our notes in [INSTALL.md](INSTALL.md).
- Checkout the [guide](doc/PortingGuide.md) how to port your project.
- Checkout the [tuning guide](doc/TuningGuide.md) for a step further to performance
  portable code.


Authors
-------

### Maintainers and core developers

- Rene Widera

### Former Members, Contributions and Thanks

- Axel Huebl
- Dr. Michael Bussmann
