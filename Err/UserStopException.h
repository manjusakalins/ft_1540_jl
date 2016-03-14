#ifndef USERSTOPEXCEPTION_H
#define USERSTOPEXCEPTION_H

#include "BaseException.h"

class UserStopException: public BaseException
{
public:
    UserStopException(const char* func="", const char* file="", int line=0);
    virtual ~UserStopException() throw();

    virtual std::string err_msg() const { return "DUMMY"; }
};

#endif // USERSTOPEXCEPTION_H
