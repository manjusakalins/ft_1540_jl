#ifndef ICOMMAND_H
#define ICOMMAND_H

#include "../Resource/SharedPtr.h"
#include "../Public/Host.h"

namespace APCore
{

class Connection;
class ICommand
{
public:
    ICommand(APKey key):key_(key){}
    virtual ~ICommand(){}

    virtual void exec(const SharedPtr<APCore::Connection> &conn) = 0;

protected:
    APKey key_;
};

}

#endif // ICOMMAND_H
