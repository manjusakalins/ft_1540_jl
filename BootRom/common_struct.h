#ifndef _COMMON_STRUCT_H_
#define _COMMON_STRUCT_H_

#include "type_define.h"

#define MAX_PARTITION_COUNT		128

/*-----------USER SETTINGS-------------------*/
#define POWER_BATTERY            (0)
#define POWER_USB                (1)
#define POWER_AUTO_DECT          (2)

#define RESET_KEY_DEFAULT         (0)
#define RESET_KEY_ONE             (0x50)
#define RESET_KEY_TWO             (0x68)

/*--------------------------------------------*/
#define CHKSUM_LEVEL_NONE        (0)
#define CHKSUM_LEVEL_USB         (1<<0)
#define CHKSUM_LEVEL_STORAGE     (1<<1)
#define CHKSUM_LEVEL_USB_STORAGE (CHKSUM_LEVEL_USB | CHKSUM_LEVEL_STORAGE)


/*---------------------------enum----------------------------*/
typedef enum shutdown_method
{
   DEV_REBOOT = 0x1,
   DEV_ONLY_SHUT_DOWN,
}shutdown_method_e;

typedef enum otp_action
{
   OTP_ZONE_READ = 0x1,
   OTP_ZONE_WRITE = 0x2,   
   OTP_ZONE_GET_ADDR = 0x4,
   OTP_ZONE_GET_SIZE = 0x8,
   //LOCK can be added to WRITE as addititional flag.
   //eg: (OTP_WRITE | OTP_LOCK)
   OTP_ZONE_LOCK = 0x10000,
}otp_action_e;

typedef enum storage_type
{
   STORAGE_UNKNOW = 0x0,
   STORAGE_NONE = STORAGE_UNKNOW,
   STORAGE_EMMC = 0x1,
   STORAGE_SDMMC,
   STORAGE_UFS,
   STORAGE_NAND = 0x10,
   STORAGE_NAND_SLC,
   STORAGE_NAND_MLC,
   STORAGE_NAND_TLC,
   STORAGE_NAND_SPI,
   STORAGE_NOR = 0x20,
   STORAGE_NOR_SERIAL,
   STORAGE_NOR_PARALLEL,
}storage_type_e;

typedef enum emmc_section{
   EMMC_UNKNOWN = 0
   ,EMMC_BOOT1
   ,EMMC_BOOT2
   ,EMMC_RPMB
   ,EMMC_GP1
   ,EMMC_GP2
   ,EMMC_GP3
   ,EMMC_GP4
   ,EMMC_USER
   ,EMMC_END
   ,EMMC_BOOT1_BOOT2
} emmc_section_e;

typedef enum ufs_section{
   UFS_SECTION_UNKONWN = 0
   ,UFS_SECTION_LUA1
   ,UFS_SECTION_LUA2
   ,UFS_SECTION_LUA3
   ,UFS_SECTION_LUA4
   ,UFS_SECTION_LUA5
   ,UFS_SECTION_LUA6
   ,UFS_SECTION_LUA7
   ,UFS_SECTION_LUA8
} ufs_section_e;

typedef enum normal_section{
   MONODY_SECTION = 0
}normal_section_e;

typedef enum nand_cell_usage
{
   CELL_UNI = 0,
   CELL_BINARY,
   CELL_TRI,
   CELL_QUAD,	
   CELL_PENTA,
   CELL_HEX,
   CELL_HEPT,
   CELL_OCT,
}nand_cell_usage_e;

typedef enum nand_format_level
{
   FORMAT_LEVEL0 = 0,  //force erase, create BMT
   FORMAT_LEVEL1,        //physical erase, create BMT
   FORMAT_LEVEL2,        //logical erase, if no BMT, create it.
   FORMAT_LEVEL_END
}nand_format_level;

typedef enum operation_type
{
   PAGE_SPARE = 0
   ,PAGE_ONLY
   ,SPARE_ONLY
   ,PAGE_WITH_ECC
   ,PAGE_FDM
   ,PAGE_SPARE_WITH_ECCDECODE
   ,VERIFY_AFTER_PROGRAM
   ,PAGE_SPARE_NORANDOM
   ,FLAG_END
}operation_type;

//memory test
typedef enum memory_type
{
   TEST_MEMORY
   ,TEST_STORAGE
}memory_type;

//RAM
typedef enum ram_type
{
   NONE = 0
  ,GENERAL_DRAM
  ,GENERAL_SRAM
}ram_type_e;
/*-------------------------parameter struct------------------------------*/
struct nand_extension
{
   enum nand_cell_usage  cell_usage;
   union
   {   
      enum operation_type type;
      enum nand_format_level level;
   };
};

struct emmc_extension
{
   //none.
};

struct nor_extension
{
   //none.
};

struct partition_extension_struct
{
   union
   {   
      uint32 stub[6];//size alignment.
      struct nand_extension nand_ext;
      struct emmc_extension emmc_ext;
   };
};

/*--------------device info------------------------------*/
#pragma pack(push, 4)
struct mmc_info_struct
{
   uint32 type; //emmc or sdmmc or none.
   uint32 padding; //for alignmeng.
   uint64 boot1_size;
   uint64 boot2_size;
   uint64 rpmb_size;
   uint64 gp1_size;
   uint64 gp2_size;
   uint64 gp3_size;
   uint64 gp4_size;
   uint64 user_size;
   uint32 cid[4];
   uint8 fwver[8];
};
#pragma pack(pop)

struct nand_info_struct
{
   uint32 type; //slc, mlc, spi, none
   uint32 page_size;
   uint32 block_size;
   uint32 spare_size;
   uint64 total_size;
   uint64 available_size;
   uint32 nand_bmt_exist;
   uint8 nand_id[12];
};

struct nor_info_struct
{
   uint32 type; //nor, none
   uint32 page_size;		
   uint64 available_size;	
};

struct ufs_info_struct
{
   uint32 type; //ufs or none.
   uint32 padding; //for alignmeng.
   uint64 lua1_size;
   uint64 lua2_size;
   uint64 lua3_size;
   uint64 lua4_size;
};

struct ram_item_struct
{
   uint32 type; //general_sram, general_dram, none
   uint32 padding;
   uint64 base_address;
   uint64 size;
};
struct ram_info_struct
{
   struct ram_item_struct sram;
   struct ram_item_struct dram;
};

struct chip_id_struct
{
   uint16 hw_code;
   uint16 hw_sub_code;
   uint16 hw_version;
   uint16 sw_version;
};

struct otp_parameter_struct
{
   uint32 offset_to_group; 
   uint32 length;
};

#endif
