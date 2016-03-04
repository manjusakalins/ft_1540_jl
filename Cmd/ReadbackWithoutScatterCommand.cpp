#include "ReadbackWithoutScatterCommand.h"
#include "../Logger/Log.h"
#include "../Err/Exception.h"
#include "../BootRom/flashtoolex_api.h"
#include "../BootRom/type_define.h"

namespace APCore
{

ReadbackWithoutScatterCommand::ReadbackWithoutScatterCommand(APKey key):
    ICommand(key)
{
}

ReadbackWithoutScatterCommand::~ReadbackWithoutScatterCommand()
{

}

op_part_list_t* ReadbackWithoutScatterCommand::get_partion_list()
{
    int count = part_list_.size();

    op_part_list_t *list = new op_part_list_t[count];
    if (list){
        for(int i = 0; i < count; i++){
            op_part_list_t item = part_list_.at(i);

            strncpy(list[i].part_name, item.part_name, strlen(item.part_name) + 1);
            strncpy(list[i].file_path, item.file_path, strlen(item.file_path) + 1);
        }
    }

    return list;
}

void ReadbackWithoutScatterCommand::exec(const QSharedPointer<Connection> &conn){
    Q_UNUSED(conn);

    HSESSION hs;

    int status;

    op_part_list_t* flist = get_partion_list();

    callbacks_struct_t cbs = {0};

    cbs.cb_op_progress = cb_rb_operation_progress;

    cbs.cb_stage_message = cb_rb_stage_message;

    if(connect(&hs) != STATUS_OK){
        goto exit;
    }
    //conn->ConnectDA();

    LOGI("Begin executing readback without scatter command...");

    status = flashtool_upload(
                hs,
                flist,
                part_list_.size(),
                &cbs);

    LOGI("Readback result(%d)",status);

    if(status != S_DONE)
    {
        goto exit;
    }

    LOGI("Readback Succeeded.");

exit:
    delete flist;
    flashtool_destroy_session(hs);

    if(status != S_DONE)
    {
        THROW_BROM_EXCEPTION(status,0);
    }
}

void ReadbackWithoutScatterCommand::cb_rb_operation_progress(void* arg, enum transfer_phase phase,
                                                             unsigned int progress,
                                                             unsigned long long data_xferd)
{
    Q_UNUSED(arg);
    Q_UNUSED(phase);
    Q_UNUSED(data_xferd);

    LOGI("Readback percent: %d", progress);
}

void ReadbackWithoutScatterCommand::cb_rb_stage_message(void *usr_arg, char *message)
{
    Q_UNUSED(usr_arg);

    std::cout<< "Stage: " << message << std::endl;
}

}
