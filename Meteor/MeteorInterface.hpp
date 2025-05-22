#pragma once
#include <string>
#include <vector>
#include <cstdint>

class MeteorInterface {
public:
    MeteorInterface();
    ~MeteorInterface();

    /// Connection
    bool initialize();
    bool shutdown();

    /// 1. Temperature: set and get
    bool setHeadTemperature(uint8_t pcc, uint8_t hdc, uint8_t head, double tempC);
    double getHeadTemperature(uint8_t pcc, uint8_t hdc, uint8_t head);

    /// 2. On/off (power control)
    bool powerOn();
    bool powerOff();

    /// 3. Select waveform type and upload from file
    enum class WaveformType : uint8_t { TypeA = 0, TypeB, TypeC };
    bool selectWaveform(WaveformType type);
    bool uploadWaveform(const std::string& filepath);

    /// 4. External encoder: set multiplier & divider, and upload & apply config
    bool setEncoderParams(uint32_t multiplier, uint32_t divider);
    bool uploadEncoderConfig();

    /// 5. Home axes/heads
    bool homeAll();

    /// 6. Start (begin printing job)
    bool startPrint();

    /// 7. Show current errors
    std::vector<uint32_t> getErrors();

    /// 8. Load & send image
    bool loadImage(const std::string& filepath, uint32_t& outBufId);
    bool sendImage(uint32_t bufferId, uint8_t plane, int x, int y);

    static double calculateDpi(uint32_t multiplier,
        uint32_t divider);

private:
    static uint32_t makePEAddress(uint8_t pcc, uint8_t hdc, uint8_t head, uint8_t ja = 0);

    // helper for reading a 32-bit param
    bool readParam(uint32_t addr, uint32_t paramId, uint64_t& outVal);
};
