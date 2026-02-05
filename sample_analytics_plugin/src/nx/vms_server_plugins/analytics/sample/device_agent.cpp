// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"
#include "manifest_device_agent_VI.h"
#include "common.h"
#include <chrono>
#include <pugixml.hpp>
#include <clocale>

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_track_best_shot_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/helpers/uuid_helper.h>

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

DeviceAgent::DeviceAgent(Engine* engine, const nx::sdk::IDeviceInfo* deviceInfo):
    ConsumingDeviceAgent(deviceInfo, NX_DEBUG_ENABLE_OUTPUT, engine->integration()->instanceId()),
    m_engine(engine)
{

}

DeviceAgent::~DeviceAgent()
{
}

std::string DeviceAgent::manifestString() const
{
    return kDeviceAgentManifest;
}

// bool DeviceAgent::pushCompressedVideoFrame(Ptr<const ICompressedVideoPacket> videoFrame)
// {
//     NX_PRINT << "pushCompressedVideoFrame START";
//     NX_PRINT << "pushCompressedVideoFrame END";
//     return true;
// }

struct FaceResult {
    std::string image_base64;
    float left, top, right, bottom;
    float age_estimated;
    std::string gender;
    std::string name;
};

std::vector<char> DeviceAgent::decodeBase64(const std::string& input)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[base64_chars[i]] = i;
    int val = 0, valb = -8;
    std::vector<char> out;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

bool DeviceAgent::pushCustomMetadataPacket(
    Ptr<const nx::sdk::analytics::ICustomMetadataPacket> customMetadataPacket)
{
    std::setlocale(LC_NUMERIC, "C");
    if (!ini().needMetadata)
    {
        NX_PRINT <<
            "ERROR: Received Custom Metadata packet, contrary to streamTypeFilter in Manifest.";
        return false;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(
        customMetadataPacket->data(), 
        customMetadataPacket->dataSize());

    if (!result)
    {
        NX_PRINT << "Error: Khong the parse XML metadata!";
        return false;
    }

    pugi::xpath_variable_set vars;
    pugi::xpath_variable* ttVar = vars.add("tt", pugi::xpath_type_string);
    ttVar->set("http://www.onvif.org/ver10/schema");

    pugi::xml_node videoAnalytics =
        doc.child("tt:MetadataStream")
        .child("tt:VideoAnalytics");

    if (!videoAnalytics)
        return true;

    for (pugi::xml_node frame = videoAnalytics.child("tt:Frame");
        frame;
        frame = frame.next_sibling("tt:Frame"))
    {
        const char* apl = frame.attribute("AplSource").value();
        if (std::strcmp(apl, "AIFaceDetection") != 0)
            continue;

        for (pugi::xml_node obj = frame.child("tt:Object");
            obj;
            obj = obj.next_sibling("tt:Object"))
        {
            pugi::xml_node imageNode = obj.select_node(".//tt:Image", &vars).node();
            if (!imageNode) {
                continue;
            } 
            pugi::xml_node humanFace = obj.select_node(".//HumanFace", nullptr).node();

            // ------------- score            
            pugi::xml_node bsScoreNode = humanFace.child("bs-score");
            std::string bsScoreStr = bsScoreNode.text().as_string();
            std::replace(bsScoreStr.begin(), bsScoreStr.end(), ',', '.');
            float bsScore = std::stof(bsScoreStr);
            if (bsScore < 0.4f) { 
                continue;
            }

            // ------------- object Id
            std::string objectIdStr = obj.attribute("ObjectId").as_string();
            uint8_t buffer[16];
            std::memset(buffer, 0, 16);
            size_t len = std::min((size_t) 16, objectIdStr.length());
            std::memcpy(buffer, objectIdStr.c_str(), len);
            // nx::sdk::Uuid trackId = nx::sdk::UuidHelper::fromRawData(buffer);
            nx::sdk::Uuid trackId = nx::sdk::UuidHelper::randomUuid();
            
            // ------------- timing
            int64_t currentTimestampUs = customMetadataPacket->timestampUs();

            // ------------- Mask
            bool mask = humanFace.child("Mask").text().as_bool();

            // ------------- Age Range
            pugi::xml_node ageNode = humanFace.child("Age");
            float maxAgeConfidence = -1.0f;
            std::string ageDisplay;
            pugi::xml_node bestAgeRangeNode;
            for (pugi::xml_node range = ageNode.child("Range"); range; range = range.next_sibling("Range"))
            {
                float confidence = range.text().as_float();
                NX_PRINT << "confidence: " << confidence;

                if (confidence > maxAgeConfidence)
                {
                    maxAgeConfidence = confidence;
                    bestAgeRangeNode = range;
                }
            }
            std::uint32_t a;
            // 3. Nếu tìm thấy khoảng tuổi có xác suất cao nhất
            if (bestAgeRangeNode && maxAgeConfidence > 0.0f)
            {
                std::string minAge = bestAgeRangeNode.attribute("min").as_string();
                std::string maxAge = bestAgeRangeNode.attribute("max").as_string();

                if (!maxAge.empty())
                    ageDisplay = minAge + " - " + maxAge;
                else
                    ageDisplay = "> " + minAge;
                // bestShotPacket->addAttribute(makePtr<Attribute>("Độ tin cậy tuổi", std::to_string(maxAgeConfidence)));
            }
            
            // ------------- Gender
            pugi::xml_node genderNode = humanFace.child("Gender");
            float male   = genderNode.child("Male").text().as_float();
            float female = genderNode.child("Female").text().as_float();
            std::string genderValue = (male >= female) ? "Nam" : "Nữ";

            // ------------- OBJECT packet
            // auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();
            // objectMetadataPacket->setTimestampUs(currentTimestampUs);
            // auto objMeta = makePtr<ObjectMetadata>();
            // objMeta->setTrackId(trackId);
            // objMeta->setTypeId("lightjsc.facesearch.face");
            // objMeta->addAttribute(makePtr<Attribute>("Khẩu trang", mask ? "Có đeo" : "Không đeo"));
            // objectMetadataPacket->addItem(objMeta);
            // pushMetadataPacket(objectMetadataPacket);

            // ------------- BESTSHOT packet
            std::vector<char> binaryBestshot = decodeBase64(imageNode.text().as_string());
            auto bestShotPacket = makePtr<ObjectTrackBestShotPacket>(trackId, currentTimestampUs);
            bestShotPacket->setImageDataFormat("image/jpeg");
            bestShotPacket->setImageData(binaryBestshot);
            bestShotPacket->addAttribute(makePtr<Attribute>("Độ tuổi", ageDisplay));
            bestShotPacket->addAttribute(makePtr<Attribute>("Giới tính", genderValue));
            pushMetadataPacket(bestShotPacket);
            // -------------
        }
    }
    
    //     // // Bounding box        
    //     // float l = 0, t = 0, r = 0, b = 0, w = 0, h = 0;
    //     // pugi::xml_node bboxNode = obj.select_node(".//tt:BoundingBox", &vars).node();
    //     // l = bboxNode.attribute("left").as_float();
    //     // t = bboxNode.attribute("top").as_float();
    //     // r = bboxNode.attribute("right").as_float();
    //     // b = bboxNode.attribute("bottom").as_float();
    //     // NX_PRINT << "l" << l << "t" << t << "r" << r << "b" << b;
    //     // auto norm = [](float v) { return (v + 1.0f) / 2.0f; };
    //     // l   = norm(l);
    //     // r  = norm(r);        
    //     // t    = 1.0f - norm(t);
    //     // b = 1.0f - norm(b);
    //     // l = std::min(l, r);
    //     // t = std::min(t, b);
    //     // w = std::abs(r - l);
    //     // h = std::abs(b - t);

    return true;
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
