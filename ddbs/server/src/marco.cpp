#include <server/marco.hpp>

namespace server {

// For ClusterService
DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port, 1231, "TCP Port of this server");
DEFINE_string(listen_addr, "", "Server listen address, may be IPV4/IPV6/UDS."
            " If this is set, the flag port will be ignored");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
             "(waiting for client to close connection before server stops)");

// For ClusterManager
DEFINE_string(site_name, "site-1", "Local site name");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 200000, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");

}