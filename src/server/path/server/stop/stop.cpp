#include "stop.h"
#include "../../base.h"
#include "../../../server/http_server.h"

extern HttpServer g_svr;

void cb_server_stop(Request& req, Response& res) {
    Json::Value root, data;
    CHECK_TOKEN(LV_ROOT);
    LInfo("Server stopped by http request");
    g_svr.stop();
    RETURN_OK();
}
