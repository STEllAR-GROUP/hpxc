<!-- Copyright (c) 2007-2022 Louisiana State University                             -->
<!--                                                                                -->
<!--   Distributed under the Boost Software License, Version 1.0. (See accompanying -->
<!--   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)        -->

# HPXC - A C interface to a subset of HPX

## Build
```sh
cmake -S . -DHPX_DIR=/path/to/hpx/lib/cmake/HPX -B cmake-build/
cmake --build cmake-build/ --parallel
```

> **NOTE:**
>
>HPX must be built with `HPX_WITH_DYNAMIC_HPX_MAIN=OFF`.
