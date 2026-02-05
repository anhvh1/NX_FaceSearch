#pragma once

#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

static const std::string instanceId = "lightjsc.facesearch_EN";

static const std::string kIntegrationManifest = /*suppress newline*/ 1 + (const char*) R"json(
{
    "id": ")json" + instanceId + R"json(",
    "name": "I-PRO Face Search Plugin",
    "description": "This plugin featuring face detection and searching with I-Pro Cameras.",
    "version": "1.0.0",
    "vendor": "Light JSC",
    "engineId": ")json" + instanceId + R"json(.engine"
}
)json";

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
