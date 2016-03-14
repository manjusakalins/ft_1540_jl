/*
 * BromException.cpp
 *
 *  Created on: Sep 2, 2011
 *      Author: MTK81019
 */

#include "BromException.h"
#include "ErrorLookup.h"

BromException::BromException(int err_code,unsigned int msp_code,
        const char* func, const char* file, int line)
        :BaseException(func,file,line), err_code_(err_code),msp_code_(msp_code)
{
}

BromException::~BromException() throw()
{
}

std::string BromException::err_msg() const
{
    return ErrorLookup::BromErrorMessage(static_cast<STATUS_E>(err_code_),msp_code_);
}



