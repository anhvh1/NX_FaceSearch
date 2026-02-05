#pragma once

#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

static const std::string instanceId = "lightjsc.facesearch_VI";

static const std::string kIntegrationManifest = /*suppress newline*/ 1 + (const char*) R"json(
{
    "id": ")json" + instanceId + R"json(",
    "name": "I-PRO Plugin Tìm Kiếm Khuôn Mặt",
    "description": "Plugin cung cấp tính năng phát hiện và tìm kiếm khuôn mặt với camera I-PRO.",
    "version": "1.0.2",
    "vendor": "Light JSC",
    "engineId": ")json" + instanceId + R"json(.engine"
}
)json";

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
