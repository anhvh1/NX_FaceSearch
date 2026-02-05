// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/analytics/i_object_metadata_packet.h>
#include <nx/sdk/analytics/i_object_track_best_shot_packet.h>
#include <nx/sdk/helpers/uuid_helper.h>

#include "engine.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{
public:
    DeviceAgent(Engine* engine, const nx::sdk::IDeviceInfo* deviceInfo);
    virtual ~DeviceAgent() override;

protected:
    virtual std::string manifestString() const override;

    virtual bool pushCompressedVideoFrame(
        nx::sdk::Ptr<const nx::sdk::analytics::ICompressedVideoPacket> videoFrame) override;

    virtual bool pushCustomMetadataPacket(
        nx::sdk::Ptr<const nx::sdk::analytics::ICustomMetadataPacket> customMetadataPacket
    ) override;

private:

    int64_t m_lastVideoFrameTimestampUs = 0;

    Engine* const m_engine;
    std::vector<char> decodeBase64(const std::string& input);

};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
