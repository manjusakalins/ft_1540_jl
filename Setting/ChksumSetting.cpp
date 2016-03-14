#include "ChksumSetting.h"

#include "../Logger/Log.h"
#include "../Host/Inc/RuntimeMemory.h"

namespace APCore{

ChksumSetting::ChksumSetting()
    :cb_checksum_init_(NULL),
     cb_checksum_progress_(NULL),
     chksum_value_(0),
     index_(0),
     stop_flag_(NULL)
{}

ChksumSetting::~ChksumSetting()
{}

BromChksumArg ChksumSetting::GetBromChksumArg() const
{
   BromChksumArg chksumArg;

   if(cb_checksum_init_ != NULL)
   {
       chksumArg.set_init_cb(cb_checksum_init_);
   }

   if(cb_checksum_progress_ != NULL)
   {
       chksumArg.set_chksum_cb(cb_checksum_progress_);
   }

   chksumArg.set_chksum_value(chksum_value_);
   chksumArg.set_rom_index(index_);
   chksumArg.set_stop_flag(stop_flag_);

   return chksumArg;
}

ChksumSetting* ChksumSetting::Clone() const
{
    return new ChksumSetting(*this);
}

}
