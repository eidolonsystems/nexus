#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root=$(pwd)
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
for i in "$@"; do
  case $i in
    -DD=*)
      dependencies="${i#*=}"
      shift
      ;;
    *)
      config="$i"
      shift
      ;;
  esac
done
if [ "$config" = "" ]; then
  config="Release"
fi
if [ "$dependencies" = "" ]; then
  dependencies="$root/Dependencies"
fi
if [ ! -d "$dependencies" ]; then
  mkdir -p "$dependencies"
fi
pushd "$dependencies"
"$directory"/setup.sh
popd
if [ "$dependencies" != "$root/Dependencies" ] && [ ! -d Dependencies ]; then
  rm -rf Dependencies
  ln -s "$dependencies" Dependencies
fi
if [ -d "$directory/Include" ]; then
  include_hash=$(find $(pwd) $directory/Include -name "*.hpp" | grep "^/" | md5sum | cut -d" " -f1)
  if [ -f "CMakeFiles/hpp_hash.txt" ]; then
    hpp_hash=$(cat "CMakeFiles/hpp_hash.txt")
    if [ "$include_hash" != "$hpp_hash" ]; then
      run_cmake=1
    fi
  else
    run_cmake=1
  fi
  if [ "$run_cmake" = "1" ]; then
    if [ ! -d "CMakeFiles" ]; then
      mkdir CMakeFiles
    fi
    echo $include_hash > "CMakeFiles/hpp_hash.txt"
  fi
fi
if [ -d "$directory/Source" ]; then
  source_hash=$(find $(pwd) $directory/Source -name "*.cpp" | grep "^/" | md5sum | cut -d" " -f1)
  if [ -f "CMakeFiles/cpp_hash.txt" ]; then
    cpp_hash=$(cat "CMakeFiles/cpp_hash.txt")
    if [ "$source_hash" != "$cpp_hash" ]; then
      run_cmake=1
    fi
  else
    run_cmake=1
  fi
  if [ "$run_cmake" = "1" ]; then
    if [ ! -d "CMakeFiles" ]; then
      mkdir CMakeFiles
    fi
    echo $source_hash > "CMakeFiles/cpp_hash.txt"
  fi
fi
if [ "$run_cmake" = "1" ]; then
  cmake -S "$directory" -DCMAKE_BUILD_TYPE=$config -DD=$dependencies
fi
