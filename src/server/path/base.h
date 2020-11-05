#pragma once
#include "../../share/util/string_utils.h"
#include "../../share/log/logger.h"
#include "../../share/notify/wx.h"
#include "../server/status_code.h"
#include "../server/http_server.h"
#include "../token/token_manager.h"
#include <jsoncpp/json/json.h>
#include <vector>
#include <string>

/**
 * 传入的参数都是 前后缀下划线
 * 隐含生成的变量 前缀下划线
 */


/******************************************************
 *
 *     return
 *
******************************************************/

#define RETURN() \
    res.set_body(root);\
    return

#define RETURN_CODE(_code_) \
    root["code"]=_code_;\
    root["msg"]=api::to_string(_code_);\
    LTrace("code:{}",_code_);\
    RETURN()
#define RETURN_CODE_MSG(_code_,_msg_) \
    LError("Return Error: {}, {}", _code_, _msg_);\
    root["code"]=_code_;\
    root["msg"]=_msg_;\
    LTrace("code:{}, msg:{}",_code_,_msg_);\
    RETURN()

#define RETURN_NOT_FOUND() \
    RETURN_CODE(api::StatusCode::NOT_FOUND)
#define RETURN_NOT_FOUND_MSG(_msg_) \
    RETURN_CODE_MSG(api::StatusCode::NOT_FOUND, _msg_)

#define RETURN_INTERNAL_SERVER_ERROR() \
    RETURN_CODE(api::INTERNAL_SERVER_ERROR)
#define RETURN_INTERNAL_SERVER_ERROR_MSG(_msg_) \
    RETURN_CODE_MSG(api::INTERNAL_SERVER_ERROR, _msg_)

#define RETURN_BILI_API_ERROR() \
    RETURN_CODE(api::StatusCode::BILI_API_ERROR)
#define RETURN_BILI_API_ERROR_MSG(_msg_) \
    RETURN_CODE_MSG(api::StatusCode::BILI_API_ERROR, _msg_)

#define RETURN_GENERAL_ERROR() \
    RETURN_CODE(api::StatusCode::GENERAL_ERROR)

#define RETURN_ERROR(_msg_) \
    RETURN_CODE_MSG(api::StatusCode::GENERAL_ERROR,_msg_)

#define RETURN_OK() \
    if (!data.isNull()){\
        root["data"]=data;\
    }\
    RETURN_CODE(api::StatusCode::NO_ERROR)
#define RETURN_OK_MSG(_msg_) \
    if (!data.isNull()){\
        root["data"]=data;\
    }\
    root["code"]=api::StatusCode::NO_ERROR;\
    root["msg"]=_msg_;RETURN()

#define RETURN_MISSING_PARAM(_param_) \
    RETURN_CODE_MSG(api::MISSING_PARAM, "Missing param:"#_param_)
#define RETURN_MISSING_PARAM_MSG(_param_,_msg_) \
    RETURN_CODE_MSG(api::MISSING_PARAM, _msg_)

#define RETURN_INVALID_PARAM(_param_) \
    RETURN_CODE_MSG(api::INVALID_PARAM, "Invalid param:"#_param_)
#define RETURN_INVALID_PARAM_MSG(_param_,_msg_) \
    RETURN_CODE_MSG(api::INVALID_PARAM, _msg_)


/******************************************************
 *
 *     Check param  (Must be exist)
 *
******************************************************/

#define CHECK_PARAM(_param_) \
    std::string _param_=req.get_param(#_param_);\
    if(_param_.empty()){\
        RETURN_MISSING_PARAM(_param_);\
    }

#define CHECK_PARAM_TYPE(_param_, type) \
    std::string _param_##temp=req.get_param(#_param_);\
    if(_param_##temp.empty()) {\
        RETURN_MISSING_PARAM(_param_);\
    }\
    type _param_;\
    if (!util::convert(_param_##temp, _param_)){\
        RETURN_INVALID_PARAM(_param_);\
    }

#define CHECK_PARAM_STR(_param_) \
    std::string _param_=req.get_param(#_param_);\
    if(_param_.empty()){\
        RETURN_MISSING_PARAM(_param_);\
    }
#define CHECK_PARAM_INT(_param_)     CHECK_PARAM_TYPE(_param_, int)
#define CHECK_PARAM_INT64(_param_)   CHECK_PARAM_TYPE(_param_, int64_t)
#define CHECK_PARAM_DOUBLE(_param_)  CHECK_PARAM_TYPE(_param_, double)


/************************************************************
 *
 *     Get param  (if not exist, return default value)
 *
************************************************************/

#define GET_PARAM(_param_) \
    std::string _param_=req.get_param(#_param_)

#define GET_PARAM_TYPE(_param_, type, _default_) \
    std::string _param_##temp=req.get_param(#_param_);\
    type _param_;\
    if(_param_##temp.empty()) {_param_=_default_;}\
    else if (!util::convert(_param_##temp, _param_)){RETURN_INVALID_PARAM(_param_);}

#define GET_PARAM_STR(_param_,_default_) \
    std::string _param_=req.get_param(#_param_);\
    if (_param_.empty()) { _param_=_default_; }
#define GET_PARAM_INT(_param_,_default_)     GET_PARAM_TYPE(_param_,int, _default_)
#define GET_PARAM_INT64(_param_,_default_)   GET_PARAM_TYPE(_param_,int64_t,_default_)
#define GET_PARAM_DOUBLE(_param_,_default_)  GET_PARAM_TYPE(_param_,double,_default_)


/************************************************************
 *
 *     Check others
 *
************************************************************/

#define CHECK_TOKEN(_level_) \
    std::string _token = getToken(req);\
    int _level = checkToken(_token,res,_level_,nullptr);\
    if (_level == LV_INVALID) return
#define CHECK_TOKEN_EX(_level_) \
    std::string _token = getToken(req);\
    std::string _user;\
    int _level = checkToken(_token,res,_level_,&_user);\
    if (_level == LV_INVALID) return


bool checkPassword(Request& req, Response& res);

std::string getToken(Request& req);
int checkToken(const std::string& token, Response& res, int level, std::string* user = nullptr);

