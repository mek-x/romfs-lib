# romfs-lib

[![Build and test](https://github.com/mek-x/romfs-lib/actions/workflows/cmake.yml/badge.svg)](https://github.com/mek-x/romfs-lib/actions/workflows/cmake.yml)
[![CodeQL](https://github.com/mek-x/romfs-lib/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/mek-x/romfs-lib/actions/workflows/codeql-analysis.yml)

Simple library for reading the romfs images. Inteded to be used for WASI syscalls.

## ToDo

- [ ] get file mode - m3_wasi_generic_fd_fdstat_get
- [ ] open - m3_wasi_generic_path_open
- [ ] read dir - m3_wasi_generic_fd_readdir
- [ ] read file - m3_wasi_generic_fd_read
