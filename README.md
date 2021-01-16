# v2

## dependencies
- cppcheck
- clangtidy

For example, on ubuntu/debian:

```bash
sudo apt install cppcheck clangtidy -y
```

## compilation

```bash
git clone --recursive https://github.com/turtlecoin/turtlecoin-v2
cd turtlecoin-v2
mkdir build
cd build
cmake ..
make
```

## updating submodules

```bash
git submodule update --init --recursive
```
