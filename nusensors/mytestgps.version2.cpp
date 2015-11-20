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
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define NMEA_MAX_SIZE 83
#define maxlen 4096
#define gpstype 1
#define port 6789
#define WAKE_LOCK_NAME  "GPS"

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
}

static void status_callback(GpsStatus* status)
{
    memcpy(&sGpsStatus, status, sizeof(sGpsStatus));
}

static void sv_status_callback(GpsSvStatus* sv_status)
{
    memcpy(&sGpsSvStatus, sv_status, sizeof(sGpsSvStatus));
}

static void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length)
{
    // The Java code will call back to read these values
    // We do this to avoid creating unnecessary String objects
    sNmeaString = nmea;
    sNmeaStringLength = length;
    sGpsUtcTime = timestamp;
}

static void set_capabilities_callback(uint32_t capabilities)
{
    ALOGD("set_capabilities_callback: %d\n", capabilities);
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
    //printf("request_utc_time_callback\n");
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
    //printf("this is the void * return function\n");
    Conversion * test = (Conversion *)malloc(sizeof(Conversion));
    test = (Conversion *)arg;
    test->start(test->arg);
    //printf("conversion function pointer\n");
    return NULL;
}


static pthread_t create_thread_callback(const char* name, void (*start)(void *), void* arg)
{
    pthread_t tid;
    Conversion * sConversion;
    sConversion = (Conversion *)malloc(sizeof(Conversion));
    sConversion->start=start;
    sConversion->arg=arg;

    int err = pthread_create(&tid, NULL, start_rtn , sConversion);
    if (err != 0)
        printf("can't create thread: %s\n", strerror(err));
    return tid;
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
	//mytestgps -o/-g num stop
    	//printf("the main argc=%d\n",argc);
    	int outputnum=wgs_atoi(argv[2]);
    	//printf("the outputnum=%d\n",outputnum);
    	//printf("the gps control mode: %s\n",argv[2]);
    	int err;
	FILE * fp1;
	char para1[5]="-g";
	char para2[5]="-o";
	if(argc!=4)
	{
		printf("the parameter is wrong!\n");
		return -1;
	}

	int socket_descriptor; //套接口描述字
    	int iter=0;
    	char buf[maxlen];
    	struct sockaddr_in address;//处理网络通信的地址
	bzero(&address,sizeof(address));
    	address.sin_family=AF_INET;
    	address.sin_addr.s_addr=inet_addr("127.0.0.1");//这里不一样
    	address.sin_port=htons(port);
	if(strcmp(argv[1],para1)==0)
	{
		socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）
	}
	
	hw_module_t* module;

    	err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    	if (err == 0) {
		hw_device_t* device;
        	err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
        	if (err == 0) {
            		gps_device_t* gps_device = (gps_device_t *)device;
            		sGpsInterface = gps_device->get_gps_interface(gps_device);
        	}
    	}
    	//printf("just-get_gps_interface-wgs\n");
    	//sleep(60);

    	if (!sGpsInterface || sGpsInterface->init(&sGpsCallbacks) != 0) {
        	printf("fail if the main interface fails to initialize\n");
        	return false;
    	}
    	//printf("just gpsinterfate->init-wgs\n");
    	//printf("wgs-look GpsLocation data:\nlatitude->%lf\n",sGpsLocation.latitude);
    	//printf("this is nmea data->%s\n",sNmeaString);
    	//sleep(60);

    	if (sGpsInterface) {
        	//printf("gps will start!\n");
        	if(sGpsInterface->start() < 0) {
           		printf("gps not start,flase\n");
           		return false;
        	}
    	}
   	else
	{
		return false;
	}
    	//printf("just gpsinterface->start-wgs\n");

    	int mode = sGpsInterface->set_position_mode(0, 0, 0, 1, 1 );
   	if(mode!=0) {
       		printf("set_position_mode false\n");
       		return false;
    	}
    	//sleep(60);   

	if(strcmp(argv[1],para2)==0)
	{
    		fp1=fopen("/data/GPSdata.txt","w+");
	}

    	for(int i=0;i<outputnum;i++) {
		if(strcmp(argv[1],para1)==0)
		{
			sprintf(buf,"time=%lld, type=%d, sensor=GPS, value=<%lf,%lf,%lf>, accuracy=%f, status=%d\n",sGpsLocation.timestamp,gpstype,sGpsLocation.longitude, sGpsLocation.latitude, sGpsLocation.altitude,sGpsLocation.accuracy,sGpsStatus.status);
			sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
		}
        	//sGpsCallbacks.location_cb(&sGpsLocation);
		//if((i%1000) == 0)printf("the %d data\n",i);
        	/*printf("The %d data->\nGpslocation->speed=%f\n GpsLocation->accuracy=%f\n",i,sGpsLocation.speed,sGpsLocation.accuracy);
        	printf("Gpslocation->altitude=%lf\n",sGpsLocation.altitude);
        	printf("look the locationflags:Gpslocation->flags=%d\n",sGpsLocation.flags);
        	printf("look the status:GpsStatus->status=%d\n",sGpsStatus.status);
        	printf("==the <longitude,latitude> <%lf,%lf>   timestamp:%lld\n", sGpsLocation.longitude, sGpsLocation.latitude, sGpsLocation.timestamp);*/
		if(strcmp(argv[1],para2)==0)
		{        	
			fprintf(fp1,"The %d data->\n",i);
        		fprintf(fp1,"the locationflags:Gpslocation->flags=%d ",sGpsLocation.flags);
        		fprintf(fp1,"the status:GpsStatus->status=%d ",sGpsStatus.status);
        		fprintf(fp1,"the accuracy:%f\n", sGpsLocation.accuracy);
        		fprintf(fp1,"the <longitude,latitude,altitude> <%lf,%lf,%lf>   timestamp:%lld\n\n", sGpsLocation.longitude, sGpsLocation.latitude, sGpsLocation.altitude, sGpsLocation.timestamp);
		}
        	sleep(1);
    	}

	if(strcmp(argv[1],para2)==0)
	{
    		fclose(fp1);
	}

	if(strcmp(argv[1],para1)==0)
	{
		sprintf(buf,"stop");
		sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));//发送stop 命令
    		close(socket_descriptor);
	}
    	if(strcmp("continue",argv[3])==0) return 0;

    	sGpsInterface->stop();
    	printf("stop the gps\n");
    	printf("look the locationflags:Gpslocation->flags=%d\n",sGpsLocation.flags);
    	printf("look the status:GpsStatus->status=%d\n",sGpsStatus.status);
    	sGpsInterface->cleanup();
    	printf("cleanup the gps\n");
    

    	return 0;
}

