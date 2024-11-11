#include "lvgl.h"
#include "lv_fs_if.h"
#include "modules/ETC/SDcard.h"

// SD card singleton instance
SDcard& SD_FE = SDcard::getInstance();

// Callback function declarations
void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);
lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
void * fs_dir_open(lv_fs_drv_t * drv, const char * path);
lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len);
lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p);

// Helper function to normalize path
String normalizePath(const String& path) {
    String normalized = path;
    normalized.replace("//", "/");
    normalized.replace("///", "/");
    normalized.replace("///", "/");
    return path;
}

// Open a file
void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode) {
    String fullPath = normalizePath(String(path) + "/");
    uint8_t openMode;
    if (mode == LV_FS_MODE_WR) {
        openMode = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (mode == LV_FS_MODE_RD) {
        openMode = O_RDONLY;
    } else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) {
        openMode = O_RDWR | O_CREAT;
    } else {
        Serial.println(F("Invalid file mode."));
        return NULL;
    }

    File32* file = SD_FE.createOrOpenFile(fullPath.c_str(), O_RDWR | O_CREAT | O_TRUNC);
    return static_cast<void*>(file);
}

// Close a file
lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p) {
    File32* file = static_cast<File32*>(file_p);
    if (SD_FE.closeFile(file)) {
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Read from a file
lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br) {
    File32* file = static_cast<File32*>(file_p);
    if (file && file->isOpen()) {
        *br = SD_FE.readFile(file, buf, btr);
        return LV_FS_RES_OK;
    }
    Serial.println(F("Failed to read from file."));
    return LV_FS_RES_FS_ERR;
}

// Write to a file
lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw) {
    File32* file = static_cast<File32*>(file_p);
    if (file && file->isOpen()) {
        *bw = file->write(static_cast<const uint8_t*>(buf), btw);
        return (*bw == btw) ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
    }
    Serial.println(F("Failed to write to file."));
    return LV_FS_RES_FS_ERR;
}

// Seek within a file
lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence) {
    File32* file = static_cast<File32*>(file_p);
    if (file && file->isOpen()) {
        bool result;
        switch (whence) {
            case LV_FS_SEEK_SET:
                result = file->seekSet(pos);
                break;
            case LV_FS_SEEK_CUR:
                result = file->seekCur(pos);
                break;
            case LV_FS_SEEK_END:
                result = file->seekEnd(-static_cast<int32_t>(pos));
                break;
            default:
                Serial.println(F("Invalid seek parameter."));
                return LV_FS_RES_INV_PARAM;
        }
        return result ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
    }
    Serial.println(F("Failed to seek in file."));
    return LV_FS_RES_FS_ERR;
}

// Tell the current position in a file
lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p) {
    File32* file = static_cast<File32*>(file_p);
    if (file && file->isOpen()) {
        *pos_p = file->curPosition();
        return LV_FS_RES_OK;
    }
    Serial.println(F("Failed to get file position."));
    return LV_FS_RES_FS_ERR;
}

void * fs_dir_open(lv_fs_drv_t * drv, const char * path) {
    Serial.print(path);
    File32* dir = SD_FE.getByPath(path);
    if (!dir || !dir->isDirectory()) {
        return NULL;  
    }
    return dir;
}

// Read the next entry in a directory
lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn, uint32_t fn_len) {
    File32* dir = static_cast<File32*>(rddir_p);
    File32 entry = dir->openNextFile();
    if (!entry) {
        fn[0] = '\0';
        return LV_FS_RES_OK;
    }

    char name[64];  
        

        entry.getName(name, sizeof(name)); 

    if (entry.isDirectory()) {
        snprintf(fn, fn_len, "/%s", name);  
    } else {
        snprintf(fn, fn_len, "%s", name);
    }
    entry.close();
    return LV_FS_RES_OK;
    
}



// Close a directory
lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p) {
    File32* dir = static_cast<File32*>(dir_p);
    if (SD_FE.closeFile(dir)) {
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_FS_ERR;
}

// Initialize the LVGL filesystem interface
void lv_fs_if_init() {
    // Initialize the filesystem driver
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
    Serial.print(F("Filesystem interface initialized with drive letter '"));
    Serial.print(fs_drv.letter);
    Serial.println(F("'"));
}
