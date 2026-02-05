// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "integration.h"

#include "engine.h"

#include "manifest_integration_VI.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Integration::doObtainEngine()
{
    return new Engine(this);
}

std::string Integration::manifestString() const
{
    return kIntegrationManifest;
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
