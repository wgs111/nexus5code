#include <stdint.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <hardware/sensors.h>
#include <utils/Timers.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define acctype 2
#define gyrtype 3
#define bartype 4
#define port 6789
#define maxlen 1024

char const* getSensorName(int type) {
    switch(type) {
        case SENSOR_TYPE_ACCELEROMETER:
            return "Acc";
        case SENSOR_TYPE_MAGNETIC_FIELD:
            return "Mag";
        case SENSOR_TYPE_ORIENTATION:
            return "Ori";
        case SENSOR_TYPE_GYROSCOPE:
            return "Gyr";
        case SENSOR_TYPE_LIGHT:
            return "Lux";
        case SENSOR_TYPE_PRESSURE:
            return "Bar";
        case SENSOR_TYPE_TEMPERATURE:
            return "Tmp";
        case SENSOR_TYPE_PROXIMITY:
            return "Prx";
        case SENSOR_TYPE_GRAVITY:
            return "Grv";
        case SENSOR_TYPE_LINEAR_ACCELERATION:
            return "Lac";
        case SENSOR_TYPE_ROTATION_VECTOR:
            return "Rot";
        case SENSOR_TYPE_RELATIVE_HUMIDITY:
            return "Hum";
        case SENSOR_TYPE_AMBIENT_TEMPERATURE:
            return "Tam";
    }
    return "ukn";
}

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
	//mytestsensor -o/-g waittime outputnum
    int err;
    struct sensors_poll_device_t* device;
    struct sensors_module_t* module;
    FILE *fp , *fp1;
	char para1[5]="-g";
	char para2[5]="-o";
	if(argc!=4)
	{
		printf("the parameter is wrong!\n");
		return -1;
	}

    //printf("the main argc=%d\n",argc);
    int waittime=wgs_atoi(argv[2]);
    //printf("the waittime=%d\n",waittime);
    int outputnum=wgs_atoi(argv[3]);
    //printf("the outputnum=%d\n",outputnum);
	sleep(1);
	int socket_descriptor; //套接口描述字
    	int iter=0;
    	char buf[maxlen];
    	struct sockaddr_in address;//处理网络通信的地址
	bzero(&address,sizeof(address));
    	address.sin_family=AF_INET;
    	address.sin_addr.s_addr=inet_addr("127.0.0.1");//这里不一样
    	address.sin_port=htons(port);

	socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）

    err = hw_get_module(SENSORS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err != 0) {
        printf("hw_get_module() failed (%s)\n", strerror(-err));
        return 0;
    }

    err = sensors_open(&module->common, &device);
    if (err != 0) {
        printf("sensors_open() failed (%s)\n", strerror(-err));
        return 0;
    }


    struct sensor_t const* list;
    int count = module->get_sensors_list(module, &list);
    printf("%d sensors found:\n", count);
    for (int i=0 ; i<count ; i++) {
        printf("%s\n"
                "\tvendor: %s\n"
                "\tversion: %d\n"
                "\thandle: %d\n"
                "\ttype: %d\n"
                "\tmaxRange: %f\n"
                "\tresolution: %f\n"
                "\tpower: %f mA\n",
                list[i].name,
                list[i].vendor,
                list[i].version,
                list[i].handle,
                list[i].type,
                list[i].maxRange,
                list[i].resolution,
                list[i].power);
    }

    static const size_t numEvents = 16;
    sensors_event_t buffer[numEvents];

    for (int i=0 ; i<count ; i++) {
        err = device->activate(device, list[i].handle, 0);
        if (err != 0) {
            printf("deactivate() for '%s'failed (%s)\n",
                    list[i].name, strerror(-err));
            return 0;
        }
    }

    for (int i=0 ; i<count ; i++) {
        err = device->activate(device, list[i].handle, 1);
        if (err != 0) {
            printf("activate() for '%s'failed (%s)\n",
                    list[i].name, strerror(-err));
            return 0;
        }
        device->setDelay(device, list[i].handle, ms2ns(10));
    }


    int j=0;

    do {
        int n = device->poll(device, buffer, numEvents);
        if (n < 0) {
            printf("poll() failed (%s)\n", strerror(-err));
            break;
        }

        //printf("read %d events:\n", n);
	//fprintf(fp,"read %d events:\n", n);
        for (int i=0 ; i<n ; i++) {
            const sensors_event_t& data = buffer[i];

            if (data.version != sizeof(sensors_event_t)) {
                printf("incorrect event version (version=%d, expected=%d",
                        data.version, sizeof(sensors_event_t));
                break;
            }
			switch(data.type) {
			 	case SENSOR_TYPE_ACCELEROMETER:
				sprintf(buf,"num=%d, time=%lld, type=%d, sensor=%s, value=<%5.1f,%5.1f,%5.1f>\n",
				j,
				data.timestamp,
				acctype,                            	
				getSensorName(data.type),
                            	data.data[0],
                            	data.data[1],
                            	data.data[2]);
		    		break;
				
				case SENSOR_TYPE_GYROSCOPE:
				sprintf(buf,"num=%d, time=%lld, type=%d, sensor=%s, value=<%5.1f,%5.1f,%5.1f>\n",
                            	j,
				data.timestamp,
				gyrtype,
				getSensorName(data.type),
                            	data.data[0],
                            	data.data[1],
                            	data.data[2]);
				break;
			
				case SENSOR_TYPE_PRESSURE:
				sprintf(buf,"num=%d, time=%lld, type=%d, sensor=%s, value=%f\n",
				j,
				data.timestamp,				
				bartype,
                            	getSensorName(data.type),
                            	data.data[0]);
				break;
			}
		sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
        }
	//usleep(400000);
        usleep(waittime);
	j++;
	if(outputnum==j) break;
    } while (1); // fix that

	 sprintf(buf,"stop");
    	sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));//发送stop 命令
    	close(socket_descriptor);
    	printf("sensor Messages Sent,terminating\n");

    //exit(0);

    for (int i=0 ; i<count ; i++) {
        err = device->activate(device, list[i].handle, 0);
        if (err != 0) {
            printf("deactivate() for '%s'failed (%s)\n",
                    list[i].name, strerror(-err));
            return 0;
        }
    }

    err = sensors_close(device);
    if (err != 0) {
        printf("sensors_close() failed (%s)\n", strerror(-err));
    }
    return 0;
}
