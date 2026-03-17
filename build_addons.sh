#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
dune3d_root="${DUNE3D_ROOT:-/Users/justin/dune3d}"
cxx="${CXX:-/opt/homebrew/opt/llvm/bin/clang++}"

common_flags=(
  -std=c++20
  -dynamiclib
  -undefined
  dynamic_lookup
  -fno-autolink
  -DGLM_ENABLE_EXPERIMENTAL
  -I/opt/homebrew/include
  -I"$dune3d_root/src"
  -I"$dune3d_root"
  -I"$dune3d_root/3rd_party"
)

build_addon() {
  local folder="$1"
  local src="$repo_root/addons/$folder/addon.cpp"
  local manifest="$repo_root/addons/$folder/addon.json"
  local output

  output="$(ruby -rjson -e 'puts JSON.parse(File.read(ARGV[0])).fetch("entry")' "$manifest")"

  "$cxx" \
    "${common_flags[@]}" \
    "$src" \
    -o "$repo_root/addons/$folder/$output" \
    $(pkg-config --cflags gtkmm-4.0 epoxy eigen3 glm harfbuzz freetype2 libpng)
}

build_addon "constraint-context-delete"
build_addon "mac-quit-cmd-q"
build_addon "native-mac-window-buttons"
build_addon "sketch-tree"

echo "Built add-ons into $repo_root/addons"
