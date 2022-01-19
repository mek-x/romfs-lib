# romfs-lib

[![Build and test](https://github.com/mek-x/romfs-lib/actions/workflows/cmake.yml/badge.svg)](https://github.com/mek-x/romfs-lib/actions/workflows/cmake.yml)
[![CodeQL](https://github.com/mek-x/romfs-lib/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/mek-x/romfs-lib/actions/workflows/codeql-analysis.yml)

Simple library for reading the romfs images. Only using static memory allocation. For use in embedded environments.

## ToDo

- [x] get file mode
- [x] fstatat
- [x] open
- [x] close
- [x] read
- [x] seek
- [x] read dir
- [x] map file
- [ ] ReadNodeHdr - check for bad offset, maybe count checksum?
- [ ] Checksum checking
- [ ] Documentation!
- [ ] compile options (max path length, max filename length, etc.)

