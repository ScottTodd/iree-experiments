Setup
```
git submodule update --init --recursive
mkdir -p .\build\models\
```

developer tool with everything included
``` bash
cmake -B build/ -G Ninja . -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build/ --target iree-run-module

# RelWithDebInfo
λ stat .\build\third_party\iree\tools\iree-run-module.exe
  File: .\build\third_party\iree\tools\iree-run-module.exe
  Size: 1083904         Blocks: 1060       IO Block: 65536  regular file

# MinSizeRel
λ stat .\build\third_party\iree\tools\iree-run-module.exe
  File: .\build\third_party\iree\tools\iree-run-module.exe
  Size: 603648          Blocks: 592        IO Block: 65536  regular file
```

Default build
``` bash
cmake -B build/ -G Ninja . -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake -B build/ -G Ninja . -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build/ --target binarysize

# RelWithDebInfo (can open in SizeBench)
λ stat .\build\binarysize\binarysize.exe
  File: .\build\binarysize\binarysize.exe
  Size: 768512          Blocks: 752        IO Block: 65536  regular file

# MinSizeRel
λ stat .\build\binarysize\binarysize.exe
  File: .\build\binarysize\binarysize.exe
  Size: 536576          Blocks: 524        IO Block: 65536  regular file
```

Default compile
``` bash
iree-compile .\models\simple_mul.mlir \
  --iree-hal-target-backends=llvm-cpu \
  --iree-llvmcpu-target-triple=x86_64 \
  -o ./build/models/simple_mul_cpu_x86.vmfb

λ stat .\build\models\simple_mul_cpu_x86.vmfb
  File: .\build\models\simple_mul_cpu_x86.vmfb
  Size: 7875            Blocks: 8          IO Block: 65536  regular file

iree-dump-module.exe .\build\models\simple_mul_cpu_x86.vmfb > .\build\models\simple_mul_cpu_x86.json
```

Run
``` bash
λ .\build\binarysize\binarysize.exe local-sync .\build\models\simple_mul_cpu_x86.vmfb
4xf32=1 1.1 1.2 1.3
 *
4xf32=10 100 1000 10000
 =
4xf32=10 110 1200 13000
```

Build with only CPU HAL driver (local-task, threading)
``` bash
rm .\build\CMakeCache.txt
cmake -B build/ -G Ninja . \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DIREE_HAL_DRIVER_DEFAULTS=OFF \
  -DIREE_HAL_DRIVER_LOCAL_TASK=ON \
  -DIREE_HAL_EXECUTABLE_LOADER_DEFAULTS=OFF \
  -DIREE_HAL_EXECUTABLE_LOADER_EMBEDDED_ELF=ON
cmake --build build/ --target binarysize

# RelWithDebInfo, local-task
λ stat .\build\binarysize\binarysize.exe
  File: .\build\binarysize\binarysize.exe
  Size: 444416          Blocks: 436        IO Block: 65536  regular file
```

Build with only CPU HAL driver (local-sync, no threading)
``` bash
rm .\build\CMakeCache.txt
cmake -B build/ -G Ninja . \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DIREE_HAL_DRIVER_DEFAULTS=OFF \
  -DIREE_HAL_DRIVER_LOCAL_SYNC=ON \
  -DIREE_HAL_EXECUTABLE_LOADER_DEFAULTS=OFF \
  -DIREE_HAL_EXECUTABLE_LOADER_EMBEDDED_ELF=ON
cmake --build build/ --target binarysize

# RelWithDebInfo, local-sync
λ stat .\build\binarysize\binarysize.exe
  File: .\build\binarysize\binarysize.exe
  Size: 365568          Blocks: 360        IO Block: 65536  regular file
```

Size optimized flags
``` bash
rm .\build\CMakeCache.txt
cmake -B build/ -G Ninja . \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DIREE_SIZE_OPTIMIZED=ON \
  -DIREE_HAL_DRIVER_DEFAULTS=OFF \
  -DIREE_HAL_DRIVER_LOCAL_SYNC=ON \
  -DIREE_HAL_EXECUTABLE_LOADER_DEFAULTS=OFF \
  -DIREE_HAL_EXECUTABLE_LOADER_EMBEDDED_ELF=ON
cmake --build build/ --target binarysize

# MinSizeRel + IREE_SIZE_OPTIMIZED
λ stat .\build\binarysize\binarysize.exe
  File: .\build\binarysize\binarysize.exe
  Size: 138752          Blocks: 136        IO Block: 65536  regular file
```

## Static library

Static library compile
``` bash
iree-compile .\models\simple_mul.mlir \
  --iree-hal-target-backends=llvm-cpu \
  --iree-llvmcpu-link-embedded=false \
  --iree-llvmcpu-link-static \
  --iree-llvmcpu-static-library-output-path=.\build\models\simple_mul.o \
  -o .\build\models\simple_mul_cpu_static.vmfb

λ stat .\build\models\simple_mul_cpu_static.vmfb
  File: .\build\models\simple_mul_cpu_static.vmfb
  Size: 4277            Blocks: 8          IO Block: 65536  regular file

iree-dump-module.exe .\build\models\simple_mul_cpu_static.vmfb > .\build\models\simple_mul_cpu_static.json
```
