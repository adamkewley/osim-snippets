This is a collection of various apps/scripts I occasionally use while
hacking on OpenSim.

There is no rhyme or reason to most of these scripts: they're mostly
just one-offs that I throw together while working on the project.

## Constituents

- `build-opensim`: top-level script I use in a typical
  [opensim-core](https://github.com/opensim-org/opensim-core) checkout
  to generate binaries on my platform
  
- `size_of_objects`: prints the size of some OpenSim top-level
  objects. Useful to get an idea of the memory overhead of those
  objects when accessed frequently by OpenSim (i.e. how big are the
  heap allocs, how much fragmentation is likely, how efficiently is it
  using L1)
  
- `study_*`: usually just applications I wrote when going through
  Simbody/OpenSim's user manual or docs
  
- Everything else: so irrelevant that I can't even be bothered
  documenting them
