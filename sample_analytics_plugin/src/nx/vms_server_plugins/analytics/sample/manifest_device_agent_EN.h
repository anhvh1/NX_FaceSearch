#pragma once

#include <string>
#include "common.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

static const std::string kDeviceAgentManifest = /*suppress newline*/ 1 + (const char*) R"json(
{
    "supportedTypes": [
        {
            "objectTypeId": "lightjsc.facesearch.face",
            "attributes": ["Age", "Gender"]
        }
    ],
    "typeLibrary": {
        "objectTypes": [
            {
                "id": "lightjsc.facesearch.face",
                "name": "Face Search",
                "attributes": [
                    { "name": "Age", "type": "Number", "subtype": "integer" },
                    { 
                        "name": "Gender", 
                        "type": "Enum", 
                        "subtype": "lightjsc.facesearch.genderEnum" 
                    }
                ]
            }
        ],
        "enumTypes": [
            {
                "id": "lightjsc.facesearch.genderEnum",
                "name": "Gender",
                "items": ["Male", "Female"]
            }
        ]
    }
}
)json";

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
