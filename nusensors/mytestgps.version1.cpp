#include <string.h>
//#include <hardware/gps.h>
//#include <hardware/hardware.h>
#include "hardware/gps.h"
#include "hardware/hardware.h"
#include "hardware_legacy/power.h"
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <utils/misc.h>
//#include "hardware/gps.h"
#include <stdlib.h>

#define NMEA_MAX_SIZE 83
#define WAKE_LOCK_NAME  "GPS"

#define D(...) LOGD(__VA_ARGS__)

//extern const GpsInterface* get_gps_interface();
static const GpsInterface* sGpsInterface = NULL;
//static GpsInterface* sGpsInterface = NULL;
static const GpsNiInterface* sGpsNiInterface = NULL;

//GpsCallbacks sGpsCallbacks;
static GpsLocation sGpsLocation;
static GpsStatus sGpsStatus;
static GpsUtcTime sGpsUtcTime;
//char snmea[NMEA_MAX_SIZE+1];
static GpsSvStatus  sGpsSvStatus;
static const char* sNmeaString;
//static char* sNmeaString;
static int sNmeaStringLength;


static void location_callback(GpsLocation* location)
{
    //printf("in location_callback(printf)\n");
    ALOGD("the location_callback(LOGD)\n");
    //D("addGpsLocation");
    //printf("In callback:Gpslocation->speed=%f\nIncallback:GpsLocation->accuracy=%f\n",location->speed,location->accuracy);
    memcpy(&sGpsLocation, location, sizeof(sGpsLocation));
    //D("addGpsLocation");
    /*JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_reportLocation, location->flags,
            (jdouble)location->latitude, (jdouble)location->longitude,
            (jdouble)location->altitude,
            (jfloat)location->speed, (jfloat)location->bearing,
            (jfloat)location->accuracy, (jlong)location->timestamp);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);*/
}

static void status_callback(GpsStatus* status)
{
    //D("addGpsStatus");
    memcpy(&sGpsStatus, status, sizeof(sGpsStatus));
    /*JNIEnv* env = AndroidRuntime::getJNIEnv();
    env->CallVoidMethod(mCallbacksObj, method_reportStatus, status->status);
    checkAndClearExceptionFromCallback(env, __FUNCTION__);*/
}

static void sv_status_callback(GpsSvStatus* sv_status)
{
    //D("addGpsSvStatus");
    //JNIEnv* env = AndroidRuntime::getJNIEnv();
    memcpy(&sGpsSvStatus, sv_status, sizeof(sGpsSvStatus));
    //env->CallVoidMethod(mCallbacksObj, method_reportSvStatus);
    //checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
    //D("addnmea");
    //*/JNIEnv* env = AndroidRuntime::getJNIEnv();
    // The Java code will call back to read these values
    // We do this to avoid creating unnecessary String objects
    sNmeaString = nmea;
    sNmeaStringLength = length;
    sGpsUtcTime = timestamp;
    //env->CallVoidMethod(mCallbacksObj, method_reportNmea, timestamp);
    //checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void set_capabilities_callback(uint32_t capabilities)
{
    ALOGD("set_capabilities_callback: %d\n", capabilities);
    //JNIEnv* env = AndroidRuntime::getJNIEnv();
    //env->CallVoidMethod(mCallbacksObj, method_setEngineCapabilities, capabilities);
    //checkAndClearExceptionFromCallback(env, __FUNCTION__);
}

static void acquire_wakelock_callback()
{
    acquire_wake_lock(PARTIAL_WAKE_LOCK, WAKE_LOCK_NAME);
}

static void release_wakelock_callback()
{
    release_wake_lock(WAKE_LOCK_NAME);
}

static void request_utc_time_callback()
{
    //JNIEnv* env = AndroidRuntime::getJNIEnv();
    //env->CallVoidMethod(mCallbacksObj, method_requestUtcTime);
    //checkAndClearExceptionFromCallback(env, __FUNCTION__);
    printf("request_utc_time_callback\n");
    ALOGD("request_utc_time_callback\n");
}

typedef void (*wgstest)(void * end);

typedef struct {
    //conversion  void (*)(void *)  to  (void*)(*)(void *)
    wgstest start;
    void * arg;
} Conversion;

//static Conversion * sConversion;

void* start_rtn(void* arg)
{
    printf("this is the void * return function\n");
    Conversion * test = (Conversion *)malloc(sizeof(Conversion));
    //test->start = (Conversion *)arg->start;
    //test->arg = (Conversion *)arg->arg;
    test = (Conversion *)arg;
    test->start(test->arg);
    printf("conversion function pointer\n");
    return NULL;
}

static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    //D("willcreatethread");
    pthread_t tid;
    //start_rtn(start);
    //void * start_rtn =  (start);
    Conversion * sConversion;
    sConversion = (Conversion *)malloc(sizeof(Conversion));
    sConversion->start=start;
    sConversion->arg=arg;
    //start_rtn(sConversion);

    int err = pthread_create(&tid, NULL, start_rtn , sConversion);
    if (err != 0)
        printf("can't create thread: %s\n", strerror(err));
    return tid;
    //return (pthread_t)AndroidRuntime::createJavaThread(name, start, arg);
}


GpsCallbacks sGpsCallbacks = {
    sizeof(GpsCallbacks),
    location_callback,
    status_callback,
    sv_status_callback,
    nmea_callback,
    set_capabilities_callback,
    acquire_wakelock_callback,
    release_wakelock_callback,
    create_thread_callback,
    request_utc_time_callback,
};


/*const GpsInterface* gps__get_gps_interface(struct gps_device_t* dev)
{
    return get_gps_interface();
}

static int open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    struct gps_device_t *dev = (gps_device_t *)malloc(sizeof(struct gps_device_t));
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
*/

int wgs_atoi(char * str)
{
    int ret=0;
    while((*str)!='\0')
    {
        ret=((*str)-'0') + ret*10;
        str++;  
    }
    return ret;
}

int main(int argc, char** argv)
{
    printf("the main argc=%d\n",argc);
    int outputnum=wgs_atoi(argv[1]);
    printf("the outputnum=%d\n",outputnum);
    printf("the gps control mode: %s\n",argv[2]);
    int err;
    //GpsInterface* sGpsInterface = NULL;
    //gps_device_t *dev;
    //GpsInterface * mytest;
    hw_module_t* module;
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
    printf("just-get_gps_interface-wgs\n");
    //sleep(60);

    if (!sGpsInterface || sGpsInterface->init(&sGpsCallbacks) != 0) {
        printf("fail if the main interface fails to initialize-wgs\n");
        return false;
    }
    printf("just gpsinterfate->init-wgs\n");
    printf("wgs-look GpsLocation data:\nlatitude->%lf\n",sGpsLocation.latitude);
    printf("this is nmea data->%s\n",sNmeaString);
    //sleep(60);

    if (sGpsInterface) {
        printf("gps will start!\n");
        if(sGpsInterface->start() < 0) {
           printf("gps not start,flase\n");
           return false;
        }
    }
    else
        return false;
    printf("just gpsinterface->start-wgs\n");

    int mode = sGpsInterface->set_position_mode(0, 0, 0, 1, 1 );
    if(mode!=0) {
       printf("set_position_mode false\n");
       return false;
    }
    //sleep(60);   

    FILE * fp1;
    fp1=fopen("/data/GPSdata.txt","w+");

    for(int i=0;i<outputnum;i++) {
        //sGpsCallbacks.location_cb(&sGpsLocation);
	if((i%1000) == 0)printf("the %d data\n",i);
        /*printf("The %d data->\nGpslocation->speed=%f\n GpsLocation->accuracy=%f\n",i,sGpsLocation.speed,sGpsLocation.accuracy);
        printf("Gpslocation->altitude=%lf\n",sGpsLocation.altitude);
        printf("look the locationflags:Gpslocation->flags=%d\n",sGpsLocation.flags);
        printf("look the status:GpsStatus->status=%d\n",sGpsStatus.status);
        printf("==the <longitude,latitude> <%lf,%lf>   timestamp:%lld\n", sGpsLocation.longitude, sGpsLocation.latitude, sGpsLocation.timestamp);*/
        fprintf(fp1,"The %d data->\n",i);
        fprintf(fp1,"look the locationflags:Gpslocation->flags=%d\n",sGpsLocation.flags);
        fprintf(fp1,"look the status:GpsStatus->status=%d\n",sGpsStatus.status);
        fprintf(fp1,"the accuracy:%f\n", sGpsLocation.accuracy);
        fprintf(fp1,"==the <longitude,latitude,altitude> <%lf,%lf,%lf>   timestamp:%lld\n\n", sGpsLocation.longitude, sGpsLocation.latitude, sGpsLocation.altitude, sGpsLocation.timestamp);
        sleep(1);
    }

    fclose(fp1);

    if(strcmp("continue",argv[2])==0) return 0;

    sGpsInterface->stop();
    printf("stop the gps\n");
    printf("look the locationflags:Gpslocation->flags=%d\n",sGpsLocation.flags);
    printf("look the status:GpsStatus->status=%d\n",sGpsStatus.status);
    sGpsInterface->cleanup();
    printf("cleanup the gps\n");
    

    return 0;
}
