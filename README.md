## VCPKG install
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh

export VCPKG_ROOT=/c/path/to/vcpkg
export PATH=$PATH:$VCPKG_ROOT
```

## Instal dependencies
```bash
vcpkg install
```

## Build & run
```bash
rm -rf build

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

cmake --build build

./build/icp
```

## Build & run Windows
```bash
Remove-Item -Recurse -Force build

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/Users/tomch/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build build

./build/Debug/icp.exe
```
