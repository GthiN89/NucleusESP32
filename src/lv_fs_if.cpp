
#include "lvgl.h"
#include <SD.h>
#include <SPI.h>
#include "lv_fs_if.h"
#include "modules/ETC/SDcard.h"
// Define file and directory types for LVGL

extern bool SDInit();

// Callback function declarations
void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);
lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
lv_fs_res_t fs_remove(lv_fs_drv_t * drv, const char * path);
lv_fs_res_t fs_size(lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
void * fs_dir_open(lv_fs_drv_t * drv, const char * path);
lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len);
lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p);


// Open a file on the SD card
void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    const char* flags = (mode == LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ;
    String fullPath = "/" + String(path);

    File * file = new File(SD.open(fullPath.c_str(), flags));
    if (!file || !*file) {
        delete file;
        return NULL;
    }
    return static_cast<void *>(file);
}

// Close an open file
lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p)
{
    File * file = static_cast<File *>(file_p);
    if (file) {
        file->close();
        delete file;
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Read from an open file
lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    File * file = static_cast<File *>(file_p);
    if (file) {
        *br = file->read(static_cast<uint8_t *>(buf), btr);
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Write to an open file
lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    File * file = static_cast<File *>(file_p);
    if (file) {
        *bw = file->write(static_cast<const uint8_t *>(buf), btw);
        return (*bw == btw) ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
    }
    return LV_FS_RES_FS_ERR;
}

// Move the file pointer to a specific position
lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    File * file = static_cast<File *>(file_p);
    if (file) {
        switch (whence) {
            case LV_FS_SEEK_SET:
                file->seek(pos, SeekSet);
                break;
            case LV_FS_SEEK_CUR:
                file->seek(file->position() + pos, SeekCur);
                break;
            case LV_FS_SEEK_END:
                file->seek(file->size() - pos, SeekEnd);
                break;
            default:
                return LV_FS_RES_INV_PARAM;
        }
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Get the current position in the file
lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    File * file = static_cast<File *>(file_p);
    if (file) {
        *pos_p = file->position();
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Open a directory on the SD card
void * fs_dir_open(lv_fs_drv_t * drv, const char * path)
{
    String fullPath = "/" + String(path);
    // if (fullPath.startsWith("/sd///")) {
    // fullPath.replace("/sd///", "/sd/");
    // } else if (fullPath.startsWith("/sd//")) {
    // fullPath.replace("/sd//", "/sd/");
    // }
    File * dir = new File(SD.open(String(path).c_str()));
    if (dir && dir->isDirectory()) {
        return static_cast<void *>(dir);
    }
    delete dir;
    return NULL;
}

// Read the next file in the directory
lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len)
{
    File * dir = static_cast<File *>(rddir_p);
    if (!dir) {
        return LV_FS_RES_FS_ERR;
    }

    File entry = dir->openNextFile();
    if (!entry) {
        fn[0] = '\0';  // No more files, end of directory
        return LV_FS_RES_OK;
    }

    // Copy the name to fn, and handle directory prefix
    const char * name = entry.name();
    if (entry.isDirectory()) {
        snprintf(fn, fn_len, "/%s", name);  // Prefix directories with '/'
    } else {
        snprintf(fn, fn_len, "%s", name);
    }
    entry.close();
    return LV_FS_RES_OK;
}

// Close an open directory
lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p)
{
    File * dir = static_cast<File *>(dir_p);
    if (dir) {
        dir->close();
        delete dir;
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Initialization function to register the SD card as a filesystem in LVGL
void lv_fs_if_init()
{
    if (!SDInit()) {
        LV_LOG_USER("SD Card initialization failed!");
        return;
    }

    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    fs_drv.letter = 'S';  // Drive letter to use in LVGL file paths
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;
    fs_drv.dir_close_cb = fs_dir_close;

    lv_fs_drv_register(&fs_drv);
    LV_LOG_USER("Filesystem interface initialized with drive letter '%c'", fs_drv.letter);
}
