

//Tham khảo

// #include <unordered_set>

// // Khai báo tập hợp để lưu các TrackId đã gửi Best Shot (đặt ở cấp độ Class của DeviceAgent)
// // std::unordered_set<std::string> m_sentTracks; 

// bool DeviceAgent::parseMetadata(const ICustomMetadataPacket* customMetadataPacket)
// {
//     std::setlocale(LC_NUMERIC, "C");
    
//     if (!ini().needMetadata)
//         return false;

//     pugi::xml_document doc;
//     pugi::xml_parse_result result = doc.load_buffer(
//         customMetadataPacket->data(), 
//         customMetadataPacket->dataSize());

//     if (!result) return false;

//     // Truy cập thẳng vào VideoAnalytics để tiết kiệm CPU
//     pugi::xml_node videoAnalytics = doc.child("tt:MetadataStream").child("tt:VideoAnalytics");
//     if (!videoAnalytics) return true;

//     for (pugi::xml_node frame = videoAnalytics.child("tt:Frame"); frame; frame = frame.next_sibling("tt:Frame"))
//     {
//         // 1. Lọc nhanh AplSource
//         const char* apl = frame.attribute("AplSource").value();
//         if (std::strcmp(apl, "AIFaceDetection") != 0)
//             continue; 

//         for (pugi::xml_node obj = frame.child("tt:Object"); obj; obj = obj.next_sibling("tt:Object"))
//         {
//             // 2. Lấy ObjectId và kiểm tra xem đã gửi chưa
//             std::string objectIdStr = obj.attribute("ObjectId").as_string();
//             if (objectIdStr.empty()) continue;

//             // KIỂM TRA DUPLICATE: Nếu đã gửi ảnh cho Track này rồi thì bỏ qua
//             // if (m_sentTracks.find(objectIdStr) != m_sentTracks.end()) continue;

//             // 3. Truy cập sâu vào Extension để lấy bs-score và Image
//             pugi::xml_node appearance = obj.child("tt:Appearance");
//             pugi::xml_node extension = appearance.child("tt:Extension");
            
//             // i-PRO thường để bs-score trong HumanFace (phụ thuộc vào version firmware)
//             pugi::xml_node humanFace = extension.child("HumanFace"); 
//             if (!humanFace) humanFace = obj.child("HumanFace"); // Fallback nếu nằm ngoài

//             float bsScore = humanFace.child("bs-score").text().as_float();
            
//             // Lọc chất lượng ảnh (Ngưỡng 0.4 như bạn chọn)
//             if (bsScore < 0.4f) continue;

//             pugi::xml_node imageNode = extension.child("tt:Image");
//             if (!imageNode) imageNode = appearance.child("tt:Image"); // Fallback
//             if (!imageNode) continue;

//             // 4. Xử lý dữ liệu ảnh
//             std::string bestshotBase64 = imageNode.text().as_string();
//             std::vector<char> binaryBestshot = decodeBase64(bestshotBase64);
//             if (binaryBestshot.empty()) continue;

//             // 5. Tạo Track ID (Uuid)
//             uint8_t buffer[16];
//             std::memset(buffer, 0, 16);
//             std::memcpy(buffer, objectIdStr.c_str(), std::min((size_t)16, objectIdStr.length()));
//             nx::sdk::Uuid trackId = nx::sdk::UuidHelper::fromRawData(buffer);
//             int64_t timestampUs = customMetadataPacket->timestampUs();

//             // 6. Gửi ObjectMetadataPacket (Để báo cho NX biết loại đối tượng)
//             auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();
//             objectMetadataPacket->setTimestampUs(timestampUs);
//             auto objMeta = makePtr<ObjectMetadata>();
//             objMeta->setTrackId(trackId);
//             objMeta->setTypeId("lightjsc.facesearch.face");
//             objectMetadataPacket->addItem(objMeta);
//             pushMetadataPacket(objectMetadataPacket);

//             // 7. Gửi BestShotPacket
//             auto bestShotPacket = makePtr<ObjectTrackBestShotPacket>(trackId, timestampUs);
//             bestShotPacket->setImageDataFormat("image/jpeg");
//             bestShotPacket->setImageData(binaryBestshot);

//             // Phân tích giới tính
//             pugi::xml_node genderNode = humanFace.child("Gender");
//             float male = genderNode.child("Male").text().as_float();
//             float female = genderNode.child("Female").text().as_float();
//             std::string genderValue = (male >= female) ? "Nam" : "Nữ";
//             bestShotPacket->addAttribute(makePtr<Attribute>("Giới tính", genderValue));

//             pushMetadataPacket(bestShotPacket);

//             // Đánh dấu đã gửi để không gửi lại cùng 1 trackId ở các gói metadata sau
//             // m_sentTracks.insert(objectIdStr); 
//         }
//     }
//     return true;
// }

//-------------------------------------------------------------------------------------------------------------------

    // auto objects = doc.select_nodes("//tt:Object", &vars);

    // for (auto& objWrap : objects)
    // {
    //     pugi::xml_node obj = objWrap.node();
        
    //     pugi::xml_node imageNode = obj.select_node(".//tt:Image", &vars).node();
    //     if (!imageNode) {
    //         continue;
    //     } 
    //     pugi::xml_node bsScoreNode = obj.select_node(".//HumanFace/bs-score", nullptr).node();
    //     std::string bsScoreStr = bsScoreNode.text().as_string();
    //     std::replace(bsScoreStr.begin(), bsScoreStr.end(), ',', '.');
    //     float bsScore = std::stof(bsScoreStr);
        
    //     if (bsScore < 0.4f) {
    //         continue;
    //     }

    //     std::string objectIdStr = obj.attribute("ObjectId").as_string();
    //     uint8_t buffer[16];
    //     std::memset(buffer, 0, 16);
    //     size_t len = std::min((size_t) 16, objectIdStr.length());
    //     std::memcpy(buffer, objectIdStr.c_str(), len);
    //     nx::sdk::Uuid trackId = nx::sdk::UuidHelper::fromRawData(buffer);
    //     int64_t currentTimestampUs = customMetadataPacket->timestampUs();
    //     std::string bestshotBase64 = imageNode.text().as_string();
    //     std::vector<char> binaryBestshot = decodeBase64(bestshotBase64);
    //     pugi::xml_node genderNode = obj.select_node(".//HumanFace/Gender", nullptr).node();

    //     // -------------
    //     auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();
    //     objectMetadataPacket->setTimestampUs(currentTimestampUs);
    //     auto objMeta = makePtr<ObjectMetadata>();
    //     objMeta->setTrackId(trackId);
    //     objMeta->setTypeId("lightjsc.facesearch.face");

    //     // -------------
    //     objectMetadataPacket->addItem(objMeta);
    //     pushMetadataPacket(objectMetadataPacket);
    //     // -------------
    //     auto bestShotPacket = makePtr<ObjectTrackBestShotPacket>(trackId, currentTimestampUs);
    //     bestShotPacket->setImageDataFormat("image/jpeg");
    //     bestShotPacket->setImageData(binaryBestshot);
    //     float male   = genderNode.child("Male").text().as_float();
    //     float female = genderNode.child("Female").text().as_float();
    //     std::string genderValue = (male >= female) ? "Nam" : "Nữ";
    //     bestShotPacket->addAttribute(makePtr<Attribute>("Gender", genderValue));
    //     pushMetadataPacket(bestShotPacket);
    //     // -------------
    //-------------------------------------------------------------------------------------------------------------------

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

        
    //     // std::string age  = "unknown";
    // }