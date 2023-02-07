#pragma once

/*******************************************************************************
EasyVsp System
Copyright (c) 2022 Jimmy Song

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

namespace Jimmy
{

enum ErrorCode
{
    ec_ok = 0,
    ec_ok_none,
    ec_error,
    ec_fatal_error,
    ec_network_malfunction,
    ec_invalid_datatype,
    ec_invalid_param,
    ec_project_exist,
};

inline const char* ToString(ErrorCode errorCode)
{
    switch (errorCode)
    {
    case ErrorCode::ec_ok: return "ok";
    case ErrorCode::ec_ok_none: return "ok none";
    case ErrorCode::ec_error:return "error";
    case ErrorCode::ec_fatal_error:return "fatal error";
    case ErrorCode::ec_network_malfunction:return "network malfunction";
    case ErrorCode::ec_invalid_datatype:return "invalid datatype";
    case ErrorCode::ec_invalid_param:return "invalid param";
    case ErrorCode::ec_project_exist:return "project has exist";

    default:return "unknown error";
    }
}

}

