#ifndef _DNX_PROTOCOL_PUBLIC_H
#define _DNX_PROTOCOL_PUBLIC_H
#include <stdint.h>

#define ID_DEV_IFP_EMULATION_SIZE 44
#define OEM_PLATFORM_ID_LENGTH 		4
#define PLATFORM_UNIQUE_ID_LENGTH       16
#define ID_DEV_IMAGE_ERROR_LENGTH       12
#define TPID_LENGTH 12

#define UFS_LUN_NUM 8

typedef uint32_t protocol_version_t;
typedef enum _PROTOCOL_VER
{
	PROTOCOL_VER_BASE = 0,
	PROTOCOL_VER_RCR1304143477 = 1,
		
} PROTOCOL_VER;
#define SUPPORTED_PROTOCOL_VERSIONS PROTOCOL_VER_BASE, PROTOCOL_VER_RCR1304143477
	

typedef enum _DEV_TYPE
{
	DEV_TYPE_DEFAULT = 0x0,
	DEV_TYPE_SPI = 0x01,
	DEV_TYPE_EMMC = 0x02,
	DEV_TYPE_UFS = 0x03,
	DEV_TYPE_PCIE_NAND = 0x04,
	DEV_TYPE_SXP = 0x05,
} DEV_TYPE;


typedef enum _DEVICE_CTX
{
	DEVICE_CTX_ROM = 0,
	DEVICE_CTX_CSE_DNX = 0x1,
} DEVICE_CTX;

//Bit [1:0]: RESET_TYPE*
//"	00: Reset DnX protocol (no CSE /device reset) by cancelling currently active command (if any) and wait for the next command
//"	01: Perform Deep reset  of the SoC after sending response. After deep reset, the security policy (SoC unlock state) must be reset to default
//"	10: Perform Shallow reset  after sending response
//"	11: Perform Deep reset  of the SoC after sending response. After deep reset, the current security policy (SoC unlock state) must be retained 
//Bit [3:2]: POST_RESET_STEPS
//"	00: After reset, take normal boot path (including honoring the DnX triggers etc.)
//"	01: After reset, enter IA FW OS DNX flow 
//"	10: After reset, ignore optional DnX triggers such as HW strap, cable detect etc. and perform a full host boot 
//"	11: Reserved
//Bits [4]: Reserved (0)
//Bits [5]: DN_FW_DNX_MOD
//"	0: Device will behave according to the RESET_TYPE field
//"	1: Device will prepare to download FW DnX module (RESET_TYPE and NEXT STEPS parameters will be ignored) 
//Bits [31:6]: Reserved (0)

typedef union _startover_cmd_flags_t
{
	struct
	{
		uint32_t reset_type 		: 2;
		uint32_t post_reset_steps	: 2;
		uint32_t rsvd0 			: 1;
		uint32_t dn_fw_dnx_mod	: 1;
		uint32_t rsvd1 			: 26;
	} bits;
	uint32_t value;

} startover_cmd_flags_t;

//"	Bit [0]: FOSR - (used by BIOS DnX)
//Force OS recovery - BIOS enters OS Recovery regardless of the state of the User partition being Virgin or not
//"	Bit [1]: FWRDR_SPD (used by CSE DnX)
//FW recovery Deep reset  with security policies reset to default - module will perform a Deep reset after FW is updated
//"	Bit [2]: OSRCR (used by BIOS DnX)
//OS recovery global reset - BIOS DnX module will perform a global reset after OS is updated 
//"	Bit [3]: MM (used by BIOS DnX)
//Manufacturing mode - Boot using IA-32 firmware build-time defaults
//"	Bit [4]: NOREn (used by BIOS DnX)
//SPI NOR Flash Enable - Directs IA-32 firmware to fetch OSImage/s from SPI NOR Flash and bypass eMMC
//"	Bit [5]: DnXOS (used by BIOS DnX)
//Directs IA-32 firmware to boot to OS Image loaded into DDR during OS Recovery
//"	Bit [6]: FWRCR (used by CSE DnX)
//FW recovery shallow reset  -  module will perform a shallow reset after FW is updated
//"	Bit [7]: FWRDR_SPR (used by CSE DnX)
//FW recovery Deep reset  with security policies retained - module will perform a Deep reset after FW is updated such that security unlock policies are retained (not reset)
//"	Others: reserved (0)

typedef union _prov_fw_image_cmd_flags_t
{
	struct
	{
		uint32_t FOSR 		: 1;
		uint32_t FWRDR_SPD	: 1;
		uint32_t OSRCR 		: 1;
		uint32_t MM			: 1;
		uint32_t NOREn 		: 1;
		uint32_t DnXOS		: 1;
		uint32_t FWRCR 		: 1;
		uint32_t FWRDR_SPR	: 1;
		uint32_t rsvd			: 24;
	} bits;
	uint32_t value;
} prov_fw_image_cmd_flags_t;

#pragma pack(1)

#define PNM_LENGTH 6
#define PSN_LENGTH 4

typedef struct _emmc_card_info_cid_t
{
	uint8_t manufactureID;   // MID
	uint8_t cardBGA;         // CBX -- Only lower 2 bit valid
	uint8_t oemAppID;        // OID
	uint8_t productName[PNM_LENGTH];  // PNM
	uint8_t productVer;      // PRV
	uint8_t productSN[PSN_LENGTH];    // PSN
	uint8_t manufactureDate; // MDT
	uint8_t crcChecksum;     // CRC --Only top 7 bit
} emmc_card_info_cid_t;

#define EMMC_CARD_INFO_CSD_LENGTH 16
#define EMMC_CARD_INFO_EXT_CSD_LENGTH 512

typedef struct _card_info_emmc_t
{
	uint32_t  ocr;           // Info read from OCR (operations Conditions Register) register. Tools doesn't make any use of this info, provided here to assist OEM /technician
	emmc_card_info_cid_t cid;
	uint8_t csd[EMMC_CARD_INFO_CSD_LENGTH];         		// Card Specific Data - info about card operation conditions
	uint8_t  ext_csd[EMMC_CARD_INFO_EXT_CSD_LENGTH];  	// Extended Card Specific Data. Contains information about the card capabilities and selected modes.
} card_info_emmc_t;

typedef union _ufs_attr_enables_t
{
	struct
	{
		uint32_t reserved0 			: 4;
		uint32_t cfg_ooo_data_en 		: 1;
		uint32_t reserved1 			: 5;
		uint32_t cfg_ref_clck_freq 	: 1;
		uint32_t cfg_descr_lock	 	: 1;
		uint32_t reserved3 			: 20;		
	} bits;
	uint32_t value;
	
} ufs_attr_enables_t;

typedef struct _ufs_dev_attrib_cfg
{
	ufs_attr_enables_t AttributesEnables;  // Which attribute to configure – tool may decide to only configure specific attributes.
	                            //   If tool wants to configure an attribute, it EN bit must be set to 1 as well as corresponding substructure must contain the correct value.
	                            //   If tool doesn’t wants to configure an attribute, it EN bit must be set to 0 as well as the corresponding substructure below must contain 0s.
	                            //
	                            //   Bit [0]: Configure bBootLunEn
	                            //   	1: Configure this attribute
	                            //   	0: Skip configuration of this attribute
	                            //   Bit [1]: Reserved (0b)
	                            //   Bit [2]: Reserved (0b)
	                            //   Bit [3]: Config bActiveCCLevel (see Bit [0])
	                            //   Bit [4]: Configure bOutOfOrderDataEn (see Bit [0])
	                            //   Bit [5]: Reserved (0b)
	                            //   Bit [6]: Reserved (0b)
	                            //   Bit [7]: Configure bMaxDataInSize (see Bit [0])
	                            //   Bit [8]: Configure bMaxDataOutSize (see Bit [0])
	                            //   Bit [9]: Reserved (0b)
	                            //   Bit [10]: Configure bRefClkFreq (see Bit [0])
	                            //   Bit [11]: Configure bConfigDescrLock (see Bit [0])
	                            //   Bit [12]: Configure bMaxNumOfRTT (see Bit [0])
	                            //   Bit [13]: Configure wExceptionEventControl (see Bit [0])
	                            //   Bit [14]: Reserved (0b)
	                            //   Bit [15]: Configure dSecondsPassed (see Bit [0])
	                            //   Bit [16]: Reserved  (Please see footnote below)
	                            //   Bit [17]: Reserved (0b)
	                            //   Bits [31:18]: Reserved (0s)
	uint8_t reserved0[4];
	uint8_t bBootLunEn;          //   Which boot LUN is active / enabled for boot (and mapped to well-known boot LUN W-LUN=30h):
	                            //      00h: Boot LUN A/B disabled
	                            //      01h: Boot LUN A enabled / boot LUN B disabled
	                            //      10h: Boot LUN A disabled / boot LUN B enabled
	                            //      Others: reserved
	uint8_t reserved1[3];
	uint8_t bCurrentPowerMode;   // Current Power Mode:
	                            //   	00h: Idle mode
	                            //   	10h: Pre-Active mode
	                            //   	11h: Active mode
	                            //   	20h: Pre-Sleep mode
	                            //   	22h: UFS-Sleep mode
	                            //   	30h: Pre-PowerDown mode
	                            //   	33h: UFS-PowerDown mode
	                            //   	Others: Reserved
	uint8_t reserved2[3];
	uint8_t bActiveICCLevel;      // Active ICC Level:
	                            //   bActiveICCLevel defines the maximum current
	                            //   consumption allowed during Active Mode.
	                            //   	00h: Lowest Active ICC level
	                            //   	…
	                            //   	0Fh: Highest Active ICC level
	                            //   	Others: Reserved
	                            //   Valid range from 00h to 0Fh
	uint8_t reserved3[3];
	uint8_t bOutOfOrderDataEn;   // Out of Order Data transfer Enable
	                            //  	00h: Out-of-order data transfer is disabled.
	                            //  	01h: Out-of-order data transfer is enabled.
	                            //  	Others: Reserved
	                            //   This bit shall have effect only when bDataOrdering = 01h
	uint8_t reserved4[3];
	uint8_t bMaxDataInSize;      // Maximum Data In Size
	                            // Maximum data size in a DATA IN UPIU.
	                            //	Value expressed in number of 512 Byte units.
	                            //	bMaxDataInSize shall not exceed the bMaxInBufferSize parameter.
	                            //	bMaxDataInSize = bMaxInBufferSize when the UFS device is shipped.
	                            //	This parameter can be written by the host only when all LU task queues are empty
	uint8_t reserved5[3];
	uint8_t bMaxDataOutSize;     // Maximum Data-Out Size
	                            // Maximum data-size in a DATA-OUT UPIU.
	                            //   	Value expressed in number of 512 Byte units.
	                            //   	bMaxDataOutSize shall not  exceed the
	                            //   	bMaxOutBufferSize parameter.
	                            //   	bMaxDataOutSize = bMaxOutBufferSize when the UFS device is shipped.
	                            //   	This parameter can be written by the host only when all LU task queues are empty
	uint8_t reserved6[3];
	uint8_t bRefClkFreq;         // Reference Clock Frequency value
	                           //	0h:19.2MHz
	                           //	1h: 26MHz
	                           //	2h: 38.4MHz
	                           //	3h: 52MHz
	                           //	Others: Reserved
	uint8_t reserved7[3];
	uint8_t bConfigDescrLock;    // This field defines various attributes:
	                            //   Bit [0]: LOCK the configuration
	                            //   	1: Lock the device config
	                            //   	0: don’t config the device. It will be lost upon power cycle
	                            //   Bit [7:1]: reserved (0)
	uint8_t reserved8[3];
	uint8_t bMaxNumOfRTT;        // Maximum current number of outstanding RTTs in device that is allowed.
	                            //      bMaxNumOfRTT shall not exceed the bDeviceRTTCap parameter.
	                            //   	This parameter can be written by the host only when all LU task queues are empty
	uint8_t reserved9[3];
	uint16_t wExceptionEventControl;  // Each bit, if set to ‘1’ by the host, enables the assertion of the relevant exception event bit, allowing the device to raise the EVENT_ALERT bit in the Device Information field in the Response UPIU:
	                                //   	Bit 0: DYNCAP_EVENT_EN
	                                //   	Bit 1: SYSPOOL_EVENT_EN
	                                //   	Bit 2: URGENT_BKOPS_EN
	                                //   	Bit 3-15: Reserved
	uint8_t reserved10[2];
	uint32_t dSecondsPassed;         // Bits[31:0]: Seconds passed from TIME BASELINE
	uint8_t reserved11[8];
} ufs_dev_attrib_cfg;

typedef struct _ufs_lun_cfg
{
	uint8_t bLUEnable;              // LUN ID 0  Logical Unit Enable
	uint8_t bBootLunID;             // LUN ID 0 Boot LUN ID
	uint8_t bLUWriteProtect;        // LUN ID 0 Logical Unit Write Protect
	uint8_t bMemoryType;            // LUN ID Memory type
	uint32_t dNumAllocUnits;         // LUN ID 0 Number of allocation units assigned to the logical unit.
	uint8_t bDataReliability;       // LUN ID 0 Data reliability
	uint8_t bLogicalBlockSize;      // LUN ID 0 Logical block size
	uint8_t bProvisioningType;      // LUN ID 0Provisioning Type
	uint16_t wContextCapabilities;   // LUN ID 0 Context capabilities
   	uint8_t reserved[3];
} ufs_lun_cfg;

#define UFS_DEV_CFG_PARAM_DESC_OFFSET_DW  2 //((sizeof(ufs_dev_cfg_param) -sizeof(ufs_dev_attrib_cfg) - (sizeof(ufs_lun_cfg)*UFS_LUN_NUM)  - sizeof(ufs_dev_desc_cfg))/4)
#define UFS_DEV_CFG_PARAM_ATTRIB_OFFSET_DW ((sizeof(ufs_dev_cfg_param) -sizeof(ufs_dev_attrib_cfg))/4)

typedef struct _ufs_dev_desc_cfg
{
	uint8_t reserved0[3];
	uint8_t bBootEnable;     // Enables boot feature
	uint8_t bDescAccessEn;   // Enables access to the Device Descriptor after the partial initialization phase of the boot sequence
	uint8_t bInitPowerMode;  // Configures the power mode after device initialization or hardware reset
	uint8_t bHighPriorityLUN;    // Configures the high priority logical unit
	uint8_t bSecureRemovalType;  // Configures the secure removal type
	uint8_t bInitActiveICCLevel; // Configures the ICC level in Active mode after device initialization or hardware reset
	uint16_t wPeriodicRTCUpdate; // Frequency and method of Real-Time Clock update (see Device Descriptor)
	uint8_t reserved1[5];
} ufs_dev_desc_cfg;

typedef struct _ufs_geo_desc
{
	uint8_t 	bLength;
	uint8_t 	bDescriptorType;
	uint8_t 	bMediaTechnology;
	uint8_t 	reserved0;
	uint64_t	qTotalRawDeviceCapacity;
	uint8_t 	reserved1;
	uint32_t 	dSegmentSize;
	uint8_t 	bAllocationUnitSize;
	uint8_t 	bMinAddrBlockSize;
	uint8_t 	bOptimalReadBlockSize;
	uint8_t 	bOptimalWriteBlockSize;
	uint8_t 	bMaxInBufferSize;
	uint8_t 	bMaxOutBufferSize;
	uint8_t 	bRPMBReadWriteSize;
	uint8_t 	reserved2;
	uint8_t 	bDataOrdering;
	uint8_t 	bMaxContextIDNumber;
	uint8_t 	bSysDataTagUnitSize;
	uint8_t 	bSysDataTagResSize;
	uint8_t 	bSupportedSecRTypes;
	uint16_t 	wSupportedMemoryTypes;
	uint32_t 	dSystemCodeMaxNAllocU;
	uint16_t 	wSystemCodeCapAdjFac;
	uint32_t 	dNonPersistMaxNAllocU;
	uint16_t 	wNonPersistCapAdjFac;
	uint32_t 	dEnhanced1MaxNAllocU;
	uint16_t 	wEnhanced1CapAdjFac;
	uint32_t 	dEnhanced2MaxNAllocU;
	uint16_t 	wEnhanced2CapAdjFac;
	uint32_t 	dEnhanced3MaxNAllocU;
	uint16_t 	wEnhanced3CapAdjFac;
	uint32_t 	dEnhanced4MaxNAllocU;
	uint16_t 	wEnhanced4CapAdjFac;
} ufs_geo_desc;

typedef struct _card_info_ufs_t
{
	ufs_dev_desc_cfg	desc;
	ufs_lun_cfg lun[UFS_LUN_NUM];       // Unit Descriptor Configurable Parameters following structure repeated for LUN ID 0 - 7
	ufs_geo_desc geoDesc;
} card_info_ufs_t;

typedef struct _card_info_ufs_v2_t
{
	ufs_dev_desc_cfg	desc;
	ufs_lun_cfg lun[UFS_LUN_NUM];       // Unit Descriptor Configurable Parameters following structure repeated for LUN ID 0 - 7
	ufs_dev_attrib_cfg attrib;
	ufs_geo_desc geoDesc;
} card_info_ufs_v2_t;

#define SPI_FLASH_COMPONENTS_MAX 2
typedef struct _card_info_spi_t
{
	uint8_t 	bFlashCompNum;
	uint8_t 	bFlashCompDensity[SPI_FLASH_COMPONENTS_MAX];
} card_info_spi_t;

#define UFS_INFO_SIZE_MAX  (sizeof(card_info_ufs_v2_t))
#define UFS_EMMC_INFO_SIZE_MAX  (sizeof(card_info_emmc_t) > (UFS_INFO_SIZE_MAX) ? sizeof(card_info_emmc_t) : (UFS_INFO_SIZE_MAX))
#define CARD_INFO_SIZE_MAX  (sizeof(card_info_spi_t) > (UFS_EMMC_INFO_SIZE_MAX) ? sizeof(card_info_spi_t) : (UFS_EMMC_INFO_SIZE_MAX))

typedef   struct  _id_dev_ifp_emulation_t
{
	unsigned int rom_ifps;
	unsigned char rbe_ifps[12];
	char oem_plat_id_emu[OEM_PLATFORM_ID_LENGTH];
	unsigned char reserved [24];
} id_dev_ifp_emulation_t;

#pragma pack()

#endif //#ifndef _DNX_PROTOCOL_PUBLIC_H
