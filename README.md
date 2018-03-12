# vuecash-gui

## Requirements

1. Qt5 libraries</br>
To install all required packages on Ubuntu use the following command:
```
sudo apt install qt5-qmake qtbase5-dev qtbase5-dev-tools
```

## How to build binaries from source code

### Windows

TODO

### MacOS

TODO

### Linux
```
$ git clone https://github.com/bcndev/vuecash.git
$ cd vuecash
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j4 vuecash-crypto
$ cd ../..
$ git clone https://github.com/bcndev/vuecash-gui.git
$ cd vuecash-gui
$ mkdir -p build
$ cd build
$ cmake ..
$ make -j4
```
