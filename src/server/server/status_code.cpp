#include "status_code.h"

namespace api {

const std::string to_string(StatusCode status) {
    switch (status) {
        case NO_ERROR:              return "OK";
        case MISSING_PARAM:         return "Missing param";
        case WRONG_PASSWROD:        return "Wrong passowrd";       
        case INVALID_TOKEN:         return "Invalid token";
        case INVALID_PARAM:         return "Invalid param";
        case RISK_CONTROL:          return "Too may requests, trigger risk control stystem";
        case RISK_CONTROL_LV1:      return "Too may requests, trigger risk control stystem";
        case RISK_CONTROL_LV2:      return "Too may requests, trigger risk control stystem";
        case RISK_CONTROL_LV3:      return "Too may requests, trigger risk control stystem";
        case INTERNAL_SERVER_ERROR: return "Internal server error";
        case BILI_API_ERROR:        return "Bilibili API error";
        case NOT_FOUND:             return "NOT FOUND";
        /*case NOT_EXIST:           return "NOT EXIST";*/
        case NOT_START:             return "NOT START";
        case NOT_PUBLIC:            return "NOT PUBLIC";
        case GENERAL_ERROR:         return "Error";
        default:                    return "Error";
    }
}

}
