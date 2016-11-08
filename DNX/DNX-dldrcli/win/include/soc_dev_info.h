#ifndef _SOC_DEV_INFO_H
#define _SOC_DEV_INFO_H

#define WD_PORT_NUM_LEN 5
#define LIBUSB_PORT_NUM_LEN 7
#define DRIVER_KEY_NAME_LEN 1024
typedef enum
{
   device_info_type_wd,
   device_info_type_libusb   
}device_info_type;


typedef struct _wd_info_t
{
	unsigned long dw_unique_id;
	unsigned long root_hub_number;
	unsigned long port_number[WD_PORT_NUM_LEN];
	wchar_t  driver_key_name[DRIVER_KEY_NAME_LEN];
} wd_info_t;

typedef struct _libusb_info_t
{
	unsigned char bus_number;
	unsigned char port_numbers[LIBUSB_PORT_NUM_LEN];
	int ports_count;
} libusb_info_t;


typedef struct _soc_device_info_t
{
   device_info_type dev_info_type;
   union
   {
   	wd_info_t wd_info;
	libusb_info_t libusb_info;
   }device_info;
}soc_device_info_t;

#endif //#ifndef _SOC_DEV_INFO_H
