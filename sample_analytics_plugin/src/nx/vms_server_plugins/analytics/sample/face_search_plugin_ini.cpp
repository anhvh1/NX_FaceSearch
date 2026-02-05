// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "face_search_plugin_ini.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

Ini& ini()
{
    static Ini ini;
    return ini;
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
