// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "engine.h"
#include "common.h"
#include "device_agent.h"

#include <nx/kit/utils.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/analytics/helpers/pixel_format.h>

#include "face_search_plugin_ini.h"

#undef NX_PRINT_PREFIX
#define NX_PRINT_PREFIX (this->logUtils.printPrefix)
#include <nx/kit/debug.h>

namespace nx {
    namespace vms_server_plugins {
        namespace analytics {
            namespace sample {

                using namespace nx::sdk;
                using namespace nx::sdk::analytics;

                Engine::Engine(Integration* integration) :
                    nx::sdk::analytics::Engine(NX_DEBUG_ENABLE_OUTPUT, integration->instanceId()),
                    m_integration(integration)
                {
                }

                Engine::~Engine()
                {
                }

                void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
                {
                    *outResult = new DeviceAgent(this, deviceInfo);
                }

                static std::string buildCapabilities()
                {
                    if (ini().deviceDependent)
                        return "deviceDependent";

                    return "";
                }


                std::string Engine::manifestString() const
                {
                    return /*suppress newline*/ 1 + (const char*)R"json(
{
    "id": "lightjsc.facesearch.engine",
    "streamTypeFilter": "compressedVideo|metadata",
    "capabilities": ")json" + buildCapabilities() + R"json(",
    "deviceAgentSettingsModel": {
        "type": "Settings",
        "items": [
            {
                "type": "DoubleSpinBox",
                "name": "scoreSetting",
                "caption":"Tùy chỉnh độ tin cậy",
                "description": "Độ tin cậy tối thiểu để phát hiện đối tượng (0.0 - 1.0)",
                "defaultValue": 0.0,
                "minValue": 0.0,
                "maxValue": 1.0,
                "step": 0.05,
                "decimals": 3
            },
            {
                "type": "SpinBox",
                "name": "intervalSetting",
                "caption":"Tùy chỉnh độ trễ",
                "description": "Khoảng thời gian giữa những lần phát hiện ( 1 - 3600 giây )",
                "defaultValue": 3,
                "minValue": 0,
                "maxValue": 3600,
                "step": 1
            }
        ]
    },
    "typeLibrary":
    {
        "objectTypes":
        [
            {
                "id": "lightjsc.facesearch.face",
                "name": "Tìm kiếm khuôn mặt",
                "attributes": [
                    { "name": "Độ tuổi", "type": "Enum", "subtype": "lightjsc.facesearch.ageEnum"},
                    { 
                        "name": "Giới tính", 
                        "type": "Enum", 
                        "subtype": "lightjsc.facesearch.genderEnum" 
                    },
                    { "name": "Khẩu trang", "type": "Enum", "subtype": "lightjsc.facesearch.maskEnum"}
                ]
            }
        ],
        "enumTypes": [
            {
                "id": "lightjsc.facesearch.genderEnum",
                "name": "Giới tính",
                "items": ["Nam", "Nữ"]
            },
            { "id": "lightjsc.facesearch.maskEnum", "name": "Khẩu trang", "items": ["Có đeo", "Không đeo"]},
            { "id": "lightjsc.facesearch.ageEnum",
              "name": "Độ tuổi", 
              "items": ["0 - 10", "11 - 20", "21 - 30", "31 - 40", "41 - 50", "51 - 60", "> 61"]
            }
        ]
    }
}
)json";
                }


            } // namespace sample
        } // namespace analytics
    } // namespace vms_server_plugins
} // namespace nx
