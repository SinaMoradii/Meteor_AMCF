#include "MeteorInterface.hpp"
#include "PrinterInterface.h"     // PiOpenPrinter, PiSetParamEx, PiGetParamEx, PiSendCommand, etc.
#include "Meteor.h"              // CPEX_TargetTemp, PCMD_IMAGE_BUFFER, MeteorConsts...
#include <cmath>

MeteorInterface::MeteorInterface() {}
MeteorInterface::~MeteorInterface() {}

bool MeteorInterface::initialize() {
    return PiOpenPrinter() == RVAL_OK;
}
bool MeteorInterface::shutdown() {
    return PiClosePrinter() == RVAL_OK;
}

// 1. Temperature
bool MeteorInterface::setHeadTemperature(uint8_t pcc, uint8_t hdc, uint8_t head, double tempC) {
    uint64_t val = static_cast<uint64_t>(std::round(tempC * 10.0)); // 0.1°C units
    uint32_t addr = makePEAddress(pcc, hdc, head);
    return PiSetParamEx(addr, CPEX_TargetTemp, val) == RVAL_OK;
}
double MeteorInterface::getHeadTemperature(uint8_t pcc, uint8_t hdc, uint8_t head) {
    uint32_t addr = makePEAddress(pcc, hdc, head);
    uint64_t raw = 0;
    if (!readParam(addr, CPEX_TargetTemp, raw)) return NAN;
    return raw / 10.0;
}

// 2. Power on/off
bool MeteorInterface::powerOn() {
    return PiSendCommand(reinterpret_cast<const uint32_t*>(
        std::array<uint32_t, 2>{ PCMD_POWER_ON, 1 }.data()
        )) == RVAL_OK;
}
bool MeteorInterface::powerOff() {
    return PiSendCommand(reinterpret_cast<const uint32_t*>(
        std::array<uint32_t, 2>{ PCMD_POWER_OFF, 1 }.data()
        )) == RVAL_OK;
}

// 3. Waveform
bool MeteorInterface::selectWaveform(WaveformType type) {
    uint32_t cmd[] = { PCMD_SELECT_WAVEFORM, 1, uint32_t(type) };
    return PiSendCommand(cmd) == RVAL_OK;
}
bool MeteorInterface::uploadWaveform(const std::string& filepath) {
    // assume same API as TIFF
    return PiAllocateTiffImageBufferA(filepath.c_str(), IMG_LOAD_BPP_ANY, nullptr, nullptr) == RVAL_OK;
}

// 4. Encoder config
bool MeteorInterface::setEncoderParams(uint32_t mul, uint32_t div) {
    bool ok1 = PiSetParamEx(0, CPEX_EncMultiplier, mul) == RVAL_OK;
    bool ok2 = PiSetParamEx(0, CPEX_EncDivider, div) == RVAL_OK;
    return ok1 && ok2;
}
bool MeteorInterface::uploadEncoderConfig() {
    uint32_t cmd[] = { PCMD_UPLOAD_ENCODER_CFG, 0 };
    return PiSendCommand(cmd) == RVAL_OK;
}

// 5. Home
bool MeteorInterface::homeAll() {
    uint32_t cmd[] = { PCMD_HOME_ALL, 0 };
    return PiSendCommand(cmd) == RVAL_OK;
}

// 6. Start
bool MeteorInterface::startPrint() {
    uint32_t cmd[] = { PCMD_START_PRINT, 0 };
    return PiSendCommand(cmd) == RVAL_OK;
}

// 7. Errors
std::vector<uint32_t> MeteorInterface::getErrors() {
    uint32_t cmd[] = { PCMD_QUERY_ERRORS, 0 };
    PiSendCommand(cmd);
    uint32_t count = cmd[1];
    std::vector<uint32_t> errs(count);
    for (uint32_t i = 0; i < count; ++i) {
        errs[i] = cmd[2 + i];
    }
    return errs;
}

// 8. Images (reuse earlier)
bool MeteorInterface::loadImage(const std::string& filepath, uint32_t& outBufId) {
    TIFFImageDetails det = {};
    det.StructureSizeBytes = sizeof(det);
    if (PiAllocateTiffImageBufferA(filepath.c_str(), IMG_LOAD_BPP_ANY, &det, nullptr) != RVAL_OK)
        return false;
    outBufId = det.ImageBufferID;
    return true;
}
bool MeteorInterface::sendImage(uint32_t bufferId, uint8_t plane, int x, int y) {
    uint32_t cmd[7] = {
        PCMD_IMAGE_BUFFER,
        7,
        (1u << 31) | (plane & 0xFF),
        uint32_t(x),
        uint32_t(y),
        0,
        bufferId
    };
    return PiSendCommand(cmd) == RVAL_OK;
}

// private helpers
bool MeteorInterface::readParam(uint32_t addr, uint32_t paramId, uint64_t& outVal) {
    return PiGetParamEx(addr, paramId, &outVal) == RVAL_OK;
}

uint32_t MeteorInterface::makePEAddress(uint8_t pcc, uint8_t hdc, uint8_t head, uint8_t ja) {
    return (uint32_t(pcc) << 24) | (uint32_t(hdc) << 16) | (uint32_t(head) << 8) | ja;
}

double MeteorInterface::calculateDpi(uint32_t multiplier,
    uint32_t divider,
    uint32_t ppi)
{
    if (divider == 0) {
        return NAN;
    }
    // convert to double so we don’t lose any fractional dpi
    return static_cast<double>(ppi) * multiplier / divider;
}
