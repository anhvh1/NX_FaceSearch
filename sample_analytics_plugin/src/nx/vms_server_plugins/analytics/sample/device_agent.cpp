// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"
#include "manifest_device_agent_VI.h"
#include "common.h"
#include <chrono>
#include "pugixml.hpp"
#include <clocale>
#include <algorithm>

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

                DeviceAgent::DeviceAgent(Engine* engine, const nx::sdk::IDeviceInfo* deviceInfo) :
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

                bool DeviceAgent::pushCompressedVideoFrame(Ptr<const ICompressedVideoPacket> videoFrame)
                {
                    return true;
                }

                struct FaceResult {
                    std::string image_base64;
                    float left, top, right, bottom;
                    float age_estimated;
                    std::string gender;
                    std::string name;
                };

                std::vector<char> DeviceAgent::decodeBase64(const std::string& data)
                {
                    static constexpr int8_t b64[] = { 
                        62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,
                        10,11, 12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,
                        34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51 };

                    std::vector<char> result;
                    if (data.empty()) return result;

                    result.reserve(data.size() * 0.8f);

                    uint32_t buffer = 0;
                    int state = 0;

                    for (unsigned char c : data)
                    {
                        if (c == '=') break;
                        int i = (int)c - 43;
                        if (i < 0 || i >= (int)sizeof(b64) || b64[i] < 0) continue;

                        buffer = (buffer << 6) | b64[i];
                        state++;

                        if (state == 4)
                        {
                            result.push_back((char)((buffer >> 16) & 0xFF));
                            result.push_back((char)((buffer >> 8) & 0xFF));
                            result.push_back((char)(buffer & 0xFF));
                            buffer = 0;
                            state = 0;
                        }
                    }

                    // Xử lý nốt phần dư (Padding logic) để ảnh không bị lỗi sọc ở đáy
                    if (state == 2)
                    {
                        result.push_back((char)((buffer >> 4) & 0xFF));
                    }
                    else if (state == 3)
                    {
                        result.push_back((char)((buffer >> 10) & 0xFF));
                        result.push_back((char)((buffer >> 2) & 0xFF));
                    }

                    return result;
                }

                nx::sdk::Result<const nx::sdk::ISettingsResponse*> DeviceAgent::settingsReceived()
                {
                    
                    scoreSetting = std::stof(settingValue("scoreSetting"));
                    intervalSetting = std::stoi(settingValue("intervalSetting"));
                    pushIntegrationDiagnosticEvent( IIntegrationDiagnosticEvent::Level::info,
                            "Đã lưu thiết lập cho camera",
                            "" );
                    return nullptr;
                }

                bool DeviceAgent::pushCustomMetadataPacket(
                    Ptr<const nx::sdk::analytics::ICustomMetadataPacket> customMetadataPacket)
                {
                    //timing
                    currentTimestampUs = customMetadataPacket->timestampUs();
                    int64_t cooldown = intervalSetting * kOneSecondUs;
                    if (currentTimestampUs - m_lastProcessFinishedUs < cooldown) {
                        return true; 
                    }

                    //mark sent
                    m_hasSentSomeMetadataPacket = false;

                    std::setlocale(LC_NUMERIC, "C");
                    if (!ini().needMetadata)
                    {
                        //NX_PRINT << "ERROR: Received Custom Metadata packet, contrary to streamTypeFilter in Manifest.";
                        return false;
                    }

                    pugi::xml_document doc;
                    pugi::xml_parse_result result = doc.load_buffer(
                        customMetadataPacket->data(),
                        customMetadataPacket->dataSize());

                    if (!result)
                    {
                        //NX_PRINT << "Error: Khong the parse XML metadata!";
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
                            if (bsScore < scoreSetting) {
                                continue;
                            }

                            // ------------- object Id
                            // std::string objectIdStr = obj.attribute("ObjectId").as_string();
                            // uint8_t buffer[16];
                            // std::memset(buffer, 0, 16);
                            // size_t len = std::min((size_t)16, objectIdStr.length());
                            // std::memcpy(buffer, objectIdStr.c_str(), len);
                            nx::sdk::Uuid trackId = nx::sdk::UuidHelper::randomUuid();

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

                                if (confidence > maxAgeConfidence)
                                {
                                    maxAgeConfidence = confidence;
                                    bestAgeRangeNode = range;
                                }
                            }
                            std::uint32_t a;
                            if (bestAgeRangeNode && maxAgeConfidence > 0.0f)
                            {
                                std::string minAge = bestAgeRangeNode.attribute("min").as_string();
                                std::string maxAge = bestAgeRangeNode.attribute("max").as_string();

                                if (!maxAge.empty())
                                    ageDisplay = minAge + " - " + maxAge;
                                else
                                    ageDisplay = "> " + minAge;
                            }

                            // ------------- Gender
                            pugi::xml_node genderNode = humanFace.child("Gender");
                            float male = genderNode.child("Male").text().as_float();
                            float female = genderNode.child("Female").text().as_float();
                            std::string genderValue = (male >= female) ? "Nam" : "Nữ";

                            // ------------- OBJECT packet
                            auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();
                            objectMetadataPacket->setTimestampUs(currentTimestampUs);
                            auto objMeta = makePtr<ObjectMetadata>();
                            objMeta->setTrackId(trackId);
                            objMeta->setTypeId("lightjsc.facesearch.face");
                            objMeta->addAttribute(makePtr<Attribute>("Khẩu trang", mask ? "Có đeo" : "Không đeo"));
                            objectMetadataPacket->addItem(objMeta);
                            pushMetadataPacket(objectMetadataPacket);

                            // ------------- BESTSHOT packet
                            std::vector<char> binaryBestshot = decodeBase64(imageNode.text().as_string());
                            auto bestShotPacket = makePtr<ObjectTrackBestShotPacket>(trackId, currentTimestampUs);
                            bestShotPacket->setImageDataFormat("image/jpeg");
                            bestShotPacket->setImageData(binaryBestshot);
                            bestShotPacket->addAttribute(makePtr<Attribute>("Độ tuổi", ageDisplay));
                            bestShotPacket->addAttribute(makePtr<Attribute>("Giới tính", genderValue));
                            pushMetadataPacket(bestShotPacket);

                            m_hasSentSomeMetadataPacket = true;
                        }
                    }

                        // // Bounding box        
                        // float l = 0, t = 0, r = 0, b = 0, w = 0, h = 0;
                        // pugi::xml_node bboxNode = obj.select_node(".//tt:BoundingBox", &vars).node();
                        // l = bboxNode.attribute("left").as_float();
                        // t = bboxNode.attribute("top").as_float();
                        // r = bboxNode.attribute("right").as_float();
                        // b = bboxNode.attribute("bottom").as_float();
                        // NX_PRINT << "l" << l << "t" << t << "r" << r << "b" << b;
                        // auto norm = [](float v) { return (v + 1.0f) / 2.0f; };
                        // l   = norm(l);
                        // r  = norm(r);        
                        // t    = 1.0f - norm(t);
                        // b = 1.0f - norm(b);
                        // l = std::min(l, r);
                        // t = std::min(t, b);
                        // w = std::abs(r - l);
                        // h = std::abs(b - t);

                    if (m_hasSentSomeMetadataPacket) {
                        m_lastProcessFinishedUs = currentTimestampUs;
                    }
                    return true;
                }

            } // namespace sample
        } // namespace analytics
    } // namespace vms_server_plugins
} // namespace nx
