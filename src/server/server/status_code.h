#pragma once
#include <string>

namespace api {

enum StatusCode {
    NO_ERROR               = 0,

    CODE_RESULT            = 10,

    MISSING_PARAM          = 100,
    WRONG_PASSWROD         = 101,
    INVALID_TOKEN          = 102,
    INVALID_PARAM          = 103,

    RISK_CONTROL           = 300,
    RISK_CONTROL_LV1       = 301,
    RISK_CONTROL_LV2       = 302,
    RISK_CONTROL_LV3       = 303,

    INTERNAL_SERVER_ERROR  = 400,
    BILI_API_ERROR         = 402,

    END                    = 403,
    NOT_FOUND              = 404,
    NOT_EXIST              = 404,
    NOT_START              = 405,
    NOT_PUBLIC             = 406,

    GENERAL_ERROR          = 500
};

const std::string to_string(StatusCode status);

}

