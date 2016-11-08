#ifndef _DLDRAPI_H
#define _DLDRAPI_H
#include <dnx_protocol_public.h>
#include <errors.h>
#include <soc_dev_info.h>
#include <string.h>

#ifdef __unix__
	#define DLDRAPICALL
	#define DLDRAPISHARED_EXPORT __attribute__ ((visibility("default")))
#else
	#define DLDRAPICALL __stdcall
	#if defined(DLDRAPI_LIBRARY)
	#define DLDRAPISHARED_EXPORT __declspec(dllexport)
	#else
	#define DLDRAPISHARED_EXPORT __declspec(dllimport)
   #endif
#endif

#define MAX_VERSION_STR_LENGTH 	256
#define MAX_USBSN_LENGTH 			128
#define MAX_SOC_DEVICES			24

typedef struct _version_t 
{ 
	char str[MAX_VERSION_STR_LENGTH];
	size_t length;	
} version_t;


typedef struct _sn_t 
{
    char str[MAX_USBSN_LENGTH];
} sn_t;


typedef struct _token_part_id_t
{
	char value[TPID_LENGTH];
} token_part_id_t;

typedef struct _oem_platform_id_t
{
	char value[OEM_PLATFORM_ID_LENGTH];
} oem_platform_id_t;

typedef struct _platform_id_t
{
	char value[PLATFORM_UNIQUE_ID_LENGTH];
} platform_id_t;

typedef unsigned int device_id_t;

typedef struct _spi_content_addr_t
{
	unsigned int  reserved;
	unsigned int  start_offset;
	unsigned int blocks;
} spi_content_addr_t;

typedef struct _ufs_content_addr_t
{
	unsigned int  partition_index : 8;
	unsigned int  reserved: 24;
	unsigned int  start_offset;
	unsigned int blocks;
} ufs_content_addr_t;

typedef struct _emmc_content_addr_t
{
	unsigned int  partition_index : 8;
	unsigned int  reserved: 24;
	unsigned int  start_offset;
	unsigned int blocks;
} emmc_content_addr_t;

typedef union _content_addr_t
{
	spi_content_addr_t spi;
	ufs_content_addr_t ufs;
	emmc_content_addr_t emmc;
	struct
	{
		unsigned int  partition_index;
		unsigned int  start_offset;
		unsigned int blocks;
	} general;

} content_addr_t;

typedef enum _id_dev_cmd_flag
{
	id_dev_cmd_flag_normal_boot    =  0,
	id_dev_cmd_flag_os_dnx,
	id_dev_cmd_flag_fw_dnx,
	id_dev_cmd_flag_dnx_trigger
} id_dev_cmd_flag;

typedef enum _dnx_trigger
{
	dnx_trigger_hw_strap    =  0,
	dnx_trigger_cable_detection,
	dnx_trigger_bad_nv_content,
	dnx_trigger_virgin_part,
	dnx_trigger_bad_ia_fw,
	dnx_trigger_os_recovery_required,
} dnx_trigger;

typedef union _id_dev_flags_t
{
	struct
	{
		unsigned int dnx_trigger : 4;
		unsigned int reserved : 28;
	} bits;
	unsigned int value;
} id_dev_flags_t;

typedef struct _image_error_values_t
{
	char value[ID_DEV_IMAGE_ERROR_LENGTH];
} image_error_values_t;


typedef struct _soc_device_t
{
	char usbsn[MAX_USBSN_LENGTH];
} soc_device_t;

typedef struct _soc_device_arr_t
{
    unsigned int count;
    soc_device_t device[MAX_SOC_DEVICES];
} soc_device_arr_t;

typedef struct _soc_device_info_arr_t
{
   unsigned int count;
   soc_device_t dev_sn[MAX_SOC_DEVICES];
   soc_device_info_t dev_info[MAX_SOC_DEVICES];
}soc_device_info_arr_t;

// Reset type values. bits[1:0]
#define START_OVER_CMD_FLAG_NO_RESET                               0
#define START_OVER_CMD_FLAG_DEEP_RESET_CLEAR_SECURITY_POLICY       1
#define START_OVER_CMD_FLAG_SHALLOW_RESET                          2
#define START_OVER_CMD_FLAG_DEEP_RESET_RETAIN_SECURITY_POLICY      3

// Post reset steps values. bits[3:2]
#define START_OVER_CMD_FLAG_NORMAL_BOOT_PATH                       0
#define START_OVER_CMD_FLAG_ENTER_OS_DNX                           1
#define START_OVER_CMD_FLAG_IGNORE_DNX_TRIGGERS                    2
#define START_OVER_CMD_FLAG_POST_RESET_STEPS_RESERVED              3

// DN_FW_DNX_MOD. bits[5]
#define START_OVER_CMD_FLAG_PREPARE_FOR_FW_DNX                     1





/*!
    \typedef dldrstatuspfn
    \brief Represents an optional log callback function that can be provided to API in order to obtain status information.
*/
typedef void(*dldrstatuspfn)(unsigned long severity, const char* msg, void * clientdata);

#define IN
#define OUT
#define INOUT

class IDldrAPI 
{
public:
	virtual void DLDRAPICALL getfullversion(version_t *pVersion) = 0;
	
	virtual error_code_t DLDRAPICALL settargetretrycount(IN unsigned int retries, OUT error_msg_t *error_msg = 0) = 0;
	
	virtual error_code_t DLDRAPICALL setusbreadwritedelay(IN unsigned long delay_ms, OUT error_msg_t *error_msg = 0) = 0;

	virtual error_code_t  DLDRAPICALL getavailabletargets(OUT soc_device_info_arr_t* device_arr, OUT error_msg_t *error_msg = 0) = 0;
   
	virtual error_code_t DLDRAPICALL deviceid(IN unsigned int flags_req, IN id_dev_ifp_emulation_t *ifp_emulation, OUT oem_platform_id_t *oem_platform_id, OUT platform_id_t *platform_id, OUT id_dev_flags_t *flags_resp, OUT image_error_values_t *image_errors,
						IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	
	virtual error_code_t DLDRAPICALL startover(IN startover_cmd_flags_t *flags, OUT unsigned int *current_op, OUT unsigned int *current_ctx,
									IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
 	virtual error_code_t DLDRAPICALL downloadfwrecovery(IN const char *fw_dnx_module_path, 
 										IN const char* usbsn, IN id_dev_ifp_emulation_t *ifp_emulation, OUT error_msg_t *error_msg = 0, OUT dldrstatuspfn statuspfn = 0, OUT void *clientdata = 0) = 0;

	virtual error_code_t DLDRAPICALL downloadfwos(IN const char *fw_dnx_module_path, IN const char *fw_image_path, IN prov_fw_image_cmd_flags_t *flags, 
										IN const char *os_dnx_module_path, IN const char* os_image_path, 										
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL writetoken(IN const char *fw_dnx_module_path, IN const char *token, IN size_t token_size, IN unsigned char slot_idx, 
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL readtoken(IN const char *fw_dnx_module_path, OUT char *token_buffer, INOUT size_t *size,  IN unsigned char slot_idx,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL erasetoken(IN const char *fw_dnx_module_path, unsigned char slot_idx, 
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL setdevicelifecyclestage(IN const char *fw_dnx_module_path, IN const char *token, IN size_t token_size,  
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL oemifpprov(IN const char *fw_dnx_module_path, IN const char *oem_ifp_block, IN size_t oem_ifp_block_size,  
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL configpart(IN const char *fw_dnx_module_path, IN const char *dev_config_path, IN unsigned char dev_type, IN unsigned char dev_index,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t  DLDRAPICALL gettokenpid(IN const char *fw_dnx_module_path, 
										IN unsigned int flags, OUT token_part_id_t *tpid, OUT unsigned int *nonce, OUT unsigned int *trusted_time, OUT device_id_t *device_id, OUT oem_platform_id_t *oem_platform_id,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	 virtual error_code_t DLDRAPICALL getcardinfo(IN const char *fw_dnx_module_path, INOUT unsigned char *dev_type, IN unsigned char dev_index, OUT char *info_buffer, INOUT size_t *size, 
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	 virtual error_code_t DLDRAPICALL readbootmedia(IN const char *fw_dnx_module_path, IN unsigned char dev_type, IN unsigned char dev_index, IN content_addr_t *address, OUT char *content_buffer, INOUT size_t *size, 
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL clearRPMB(IN const char *fw_dnx_module_path, IN unsigned char dev_type, IN unsigned char dev_index,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;			
// to be removed:
   	virtual error_code_t DLDRAPICALL deviceid2(IN unsigned int flags_in, IN const char *ifp_emulation_path, OUT oem_platform_id_t *oem_platform_id, OUT platform_id_t *platform_id, OUT id_dev_flags_t *flags_out, OUT image_error_values_t *image_errors,
						IN const char* usbsn,  OUT error_msg_t *error_msg, IN dldrstatuspfn statuspfn, IN void *clientdata) = 0;
	virtual error_code_t DLDRAPICALL writetoken2(IN const char *fw_dnx_module_path, IN const char *token_path, IN unsigned char slot_idx, 
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL readtoken2(IN const char *fw_dnx_module_path, IN  const char *token_path,   IN unsigned char slot_idx,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL setdevicelifecyclestage2(IN const char *fw_dnx_module_path, IN const char *token_path,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	virtual error_code_t DLDRAPICALL oemifpprov2(IN const char *fw_dnx_module_path, IN const char *oem_ifp_block_path,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	 virtual error_code_t DLDRAPICALL getcardinfo2(IN const char *fw_dnx_module_path, INOUT unsigned char *dev_type, IN unsigned char dev_index, IN const char *info_path,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;
	 virtual error_code_t DLDRAPICALL readbootmedia2(IN const char *fw_dnx_module_path, IN unsigned char dev_type, IN unsigned char dev_index, IN content_addr_t *address, IN const char *content_path,
										IN const char* usbsn,  OUT error_msg_t *error_msg = 0, IN dldrstatuspfn statuspfn = 0, IN void *clientdata = 0) = 0;

	 virtual void DLDRAPICALL setlogging(bool generatelog, std::string logpath="") = 0;

};

extern "C" 
{
	DLDRAPISHARED_EXPORT IDldrAPI* api_acquire();
	DLDRAPISHARED_EXPORT void  api_release();
	
}

typedef IDldrAPI* (DLDRAPICALL *api_acquire_rt)(); 
typedef void (DLDRAPICALL *api_release_rt)(); 

#endif  //#ifndef _DLDRAPI_H

