// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/kit/ini_config.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

struct Ini: public nx::kit::IniConfig
{
    Ini(): IniConfig("face_search_plugin.ini") { reload(); }

    NX_INI_FLAG(0, enableOutput, "");
    
    NX_INI_FLAG(0, deviceDependent, "Respective capability in the manifest.");

    NX_INI_FLAG(1, needMetadata,
        "If set, Engine will declare the corresponding stream type filter in the manifest.");
};

Ini& ini();

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
