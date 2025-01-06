#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

#include "lvgl.h"
#include <SDFAT.h>
#include <SPI.h>
#include "modules/ETC/SDcard.h"

// Function declarations
void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);
lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
lv_fs_res_t fs_remove(lv_fs_drv_t * drv, const char * path);
lv_fs_res_t fs_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
void * fs_dir_open(lv_fs_drv_t * drv, const char * path);
lv_fs_res_t fs_dir_read(lv_fs_drv_t*, void*, char*, unsigned int);
lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p);

// Initialization function for LVGL file system driver
void lv_fs_if_init();

#endif // FILE_EXPLORER_H
