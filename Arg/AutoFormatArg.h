#ifndef AUTOFORMATARG_H
#define AUTOFORMATARG_H

#include "../Setting/FormatSetting.h"

namespace APCore
{
class Connection;
class AutoFormatArg
{
public:
    AutoFormatArg();
    AutoFormatArg(const AutoFormatArg& ref);
    AutoFormatArg& operator = (const AutoFormatArg& ref);
    virtual ~AutoFormatArg();

    virtual void GenAutoFormatRange(const QSharedPointer<Connection> &conn,
                                    const FormatSetting::AutoFormatFlag fmt_flag,
                                    bool does_physical_format,
                                    U64 &fmt_addr,
                                    U64 &fmt_len) = 0;

protected:

    bool QueryPartLayout(const QSharedPointer<Connection> &conn,const std::string part_name,
                          unsigned int &addr, unsigned int &len);


private:
    U64 format_start_address_;
    U64 format_length_;

};

}

#endif // AUTOFORMATARG_H
