#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sane/sane.h"
#include "sane/saneopts.h"



#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize SANE
 **/
void init();
// Get all devices
SANE_Status get_devices(const SANE_Device ***device_list);
// Open a device
SANE_Status open_device(SANE_Device *device, SANE_Handle *sane_handle);
// Start scanning
SANE_Status start_scan(SANE_Handle sane_handle, SANE_String_Const fileName);
// Cancel scanning
void cancle_scan(SANE_Handle sane_handle);
// Close SANE device
void close_device(SANE_Handle sane_handle);
// Release SANE resources
void my_sane_exit();

#ifdef __cplusplus
}
#endif
