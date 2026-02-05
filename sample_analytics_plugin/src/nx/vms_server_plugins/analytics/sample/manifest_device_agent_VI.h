#pragma once

#include <string>
#include "common.h"

namespace nx {
    namespace vms_server_plugins {
        namespace analytics {
            namespace sample {

                static const std::string kDeviceAgentManifest = /*suppress newline*/ 1 + (const char*)R"json(
{
    "supportedTypes": [
        {
            "objectTypeId": "lightjsc.facesearch.face",
            "attributes": ["Độ tuổi", "Giới tính", "Khẩu trang"]
        }
    ],
    "typeLibrary": {
        "objectTypes": [
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

            } // namespace sample
        } // namespace analytics
    } // namespace vms_server_plugins
} // namespace nx
