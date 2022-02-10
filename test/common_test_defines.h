#pragma once

#include "unity_fixture.h"

#include <errno.h>
#include <string.h>

#include "romfs.h"
#include "romfs-internal.h"

extern unsigned char empty_romfs[];
extern unsigned char basic_romfs[];
extern unsigned char advanced_romfs[];
extern unsigned int empty_romfs_len;
extern unsigned int basic_romfs_len;
extern unsigned int advanced_romfs_len;

#define ROOT_OFFSET           0x20
#define SECOND_ENTRY_ROOT_OFF 0x40
#define DIR_OFFSET            0x60
#define FIRST_ENTRY_IN_DIR    0x80
#define A_FILE_OFFSET         0xF0
#define B_FILE_OFFSET         0xA0
#define DIR_IN_DIR_OFFSET     0xD0
