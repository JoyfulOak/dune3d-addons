# dune3d-addons Build Commands

```sh
cd /Users/justin/Documents/GitHub/dune3d-addons
```

```sh
export DUNE3D_ROOT="/Users/justin/dune3d"
export CXX="/opt/homebrew/opt/llvm/bin/clang++"
```

```sh
bash build_addons.sh
```

```sh
find addons -maxdepth 2 \( -name '*.dylib' -o -name 'addon.json' \) | sort
```
