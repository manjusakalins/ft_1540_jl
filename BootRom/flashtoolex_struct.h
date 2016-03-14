#ifndef __FLASHTOOL_EX_STRUCT__
#define __FLASHTOOL_EX_STRUCT__

#include "common_struct.h"

#if defined(_LINUX)  
   #define __stdcall
#endif

typedef enum logging_level
{
    ktrace = 0,
    kdebug,
    kinfo,
    kwarning,
    kerror,
    kfatal
}logging_level_e;

#define PARAM_BUF_SIZE 64
#define MAX_FILE_NAME_LEN 512
typedef struct partition_info_t
{
   char8 part_name[PARAM_BUF_SIZE];
   char8 file_name[MAX_FILE_NAME_LEN];
   uint64 start_address;
   uint64 size;
   uint32 storage;
   uint32 region;
   uint32 operation_type;
   BOOL enable;
}partition_info_t;

typedef struct partition_info_list_t
{
   uint32 count;
   partition_info_t part[MAX_PARTITION_COUNT];
}partition_info_list_t;

typedef struct scatter_file_info_t
{
   struct header_t
   {
      char8 version[PARAM_BUF_SIZE];
      char8 platform[PARAM_BUF_SIZE];
      uint32 partition_count;
   }hdr;
   partition_info_t part[MAX_PARTITION_COUNT]; //only visable partitions
}scatter_file_info_t;

typedef struct op_part_list_t
{
   char8 part_name[PARAM_BUF_SIZE];   
   char8 file_path[MAX_FILE_NAME_LEN];
}op_part_list_t;

typedef struct op_data_list_t
{
   enum storage_type storage;
   uint32 section;
   uint64 address;   
   uint64 length;
   char8 file_path[MAX_FILE_NAME_LEN];
   struct partition_extension_struct extension;
}op_data_list_t;

typedef struct format_params_struct
{
   enum storage_type storage;
   uint32 section;
   uint64 address;
   uint64 length;
   struct partition_extension_struct extension;
}format_params_struct;

typedef enum transfer_phase
{
    TPHASE_INIT = 0
   ,TPHASE_DA
   ,TPHASE_LOADER
   ,TPHASE_GENERIC
   ,TPHASE_CHECKSUM
   ,TPHASE_FORMAT
}transfer_stage_e;

typedef void (__stdcall *CB_OPERATION_PROGRESS)(pvoid _this, enum transfer_phase phase, 
                                                uint32 progress, 
                                                uint64 data_xferd);
typedef void (__stdcall *CB_STAGE_MESSAGE)(pvoid _this, char8* message);
typedef BOOL (__stdcall *CB_NOTIFY_STOP)(pvoid _this);
typedef status_t (__stdcall *CB_REMOTE_READ_FILE)(const char *filepath, unsigned int package_length, char8 *content);
typedef status_t (__stdcall *CB_REMOTE_GET_FILE_LENGTH)(const char *filepath, unsigned long long *file_size);

struct remote_file_callbacks
{
   CB_REMOTE_READ_FILE cb_read_file;
   CB_REMOTE_GET_FILE_LENGTH cb_get_file_length;
};

typedef struct callbacks_struct_t
{
   pvoid _this;
   CB_OPERATION_PROGRESS cb_op_progress;
   CB_STAGE_MESSAGE cb_stage_message;
   CB_NOTIFY_STOP cb_notify_stop;      
   remote_file_callbacks cb_remote;
}callbacks_struct_t;

typedef struct connect_params_struct_t
{
   uint32 checksum_level;
   uint32 battery_setting;
   uint32 reset_key_setting;
}connect_params_struct_t;

//The device info struct size is 8K.
//If want to add more info in this, add at the tail.
//Do NOT insert.
typedef struct device_info_t
{
   union
   {
      char8 STUB[8192];
      struct
      {
         struct mmc_info_struct mmc;
         struct nand_info_struct nand;
         struct nor_info_struct nor;
         struct ufs_info_struct ufs;
         struct ram_info_struct ram;
         struct chip_id_struct chip_id;
         uint32 random_id[4];
         //add more info here if needed.
      };
   };
}device_info_t;

typedef enum erase_type
{
    AUTO_ALL = 0
   ,AUTO_EXCEPT_BL
   ,AUTO_PHYSICAL_ALL
   ,AUTO_PHYSICAL_EXC_BL
   ,AUTO_FORCE_ALL
   ,AUTO_FORCE_EXC_BL
   ,MANUAL
   ,ERASE_UNKNOWN_TYPE
}erase_str_e;

typedef struct dll_info_t
{
   char8* version;
   char8* release_type;
   char8* build_data;
   char8* change_list;
   char8* builder;
}dll_info_t;

//brom & DA device control 
typedef enum device_control_code
{
    DEV_GET_CHIP_ID = 1,
    DEV_BROM_WRITE_MEM16,
    DEV_BROM_WRITE_MEM32,
    DEV_BROM_READ_MEM16,
    DEV_BROM_READ_MEM32,
    DEV_BROM_SEND_DATA_TO,
    DEV_BROM_JUMP_TO,
    DEV_DA_GET_USB_SPEED
}device_control_code_e;

struct brom_send_data_param
{	
   uint8*   data;
   uint32   length;
   uint32   at_address;
};

struct write_memory_parameter
{
   uint32 address;
   pvoid data;
   uint32 count;
};
//same as internal section_block_t. do not modify this struct alone.
struct execute_data_parameter
{	
   pvoid data;
   uint32 length;
   uint32 at_address;
   uint32 sig_offset;
   uint32 sig_length;
};

#endif
