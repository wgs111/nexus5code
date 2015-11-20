#include <string.h>
#include <hardware/gps.h>
#include <hardware/hardware.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <cutils/log.h>
//#include "hardware/gps.h"
#include <stdlib.h>

extern const GpsInterface* get_gps_interface();
static const GpsInterface* sGpsInterface = NULL;

const GpsInterface* gps__get_gps_interface(struct gps_device_t* dev)
{
    return get_gps_interface();
}

static int open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->get_gps_interface = gps__get_gps_interface;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t gps_module_methods = {
    .open = open_gps
};


const struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "loc_api GPS Module",
    .author = "Qualcomm USA, Inc.",
    .methods = &gps_module_methods,
};

int main(int argc, char** argv)
{
    int err;
    //GpsInterface* sGpsInterface = NULL;
    //gps_device_t *dev;
    //GpsInterface * mytest;
    hw_module_t * module;
    //mytest=gps__get_gps_interface(dev);
    err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err == 0) {
        hw_device_t* device;
        err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            gps_device_t* gps_device = (gps_device_t *)device;
            sGpsInterface = gps_device->get_gps_interface(gps_device);
        }
    }
    return 0;
}
