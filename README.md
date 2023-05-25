# Linux keylogger module

## Steps to compile the module
- download linux kernel from kernel.org
- extract the tarball into the root of the project (such that there is a linux-VERSION/ directory)
- `cd` to that directory
- `make defconfig`
- `make modules_prepare`
- `cd ..`
- `make` to compile the module itself

## Made fully by
- [Lucas Eduardo Gulka Pulcinelli](https://github.com/lucasgpulcinelli)
