#pragma once

/**
 * This is the header file for the UdpRandomGetter class. This class can
 * not be copied, but can be moved. This class can be used to perform UDP
 * to get data into a file or a provided buffer.
*/
#include "RandomGetter.h"
#include "ImplPtrTemplate_h.h"
#include <memory> // unique_ptr
#include <algorithm> // std::max_element

namespace meteringclientlib
{
namespace detail
{
class UdpRandomGetterImpl;
} // namespace detail
class UdpRandomGetter : public RandomGetter
{
public:
    static const int RANDOM_UDP_MAX_SIZE = 1024;
    static const int UDP_FAIL_MAX = RandomGetter::DEFAULT_FAIL_MAX;

    enum PacketType
    {
        PACKET_TYPE_APPLIANCE = 0,
        PACKET_TYPE_CAPTURE_TRANSFORM,
        PACKET_TYPE_MAX
    };

    struct PacketFormat
    {
        int packetSize;
        int packetStart;
        int packetEnd;
        int payloadSize;  // payloadSize := packetEnd - packetStart
    };

    static constexpr PacketFormat PACKET_FORMAT[PACKET_TYPE_MAX] = {
        { // PACKET_TYPE_APPLIANCE
            1280, // packetSize
            16,   // packetStart
            1040, // packetEnd
            1024  // payloadSize
        },
        { // PACKET_TYPE_CAPTURE_TRANSFORM
            1024, // packetSize
            0,    // packetStart
            1024, // packetEnd
            1024  // payloadSize
        }
    };

public:
    explicit UdpRandomGetter(int inputPort);
    explicit UdpRandomGetter(const char *inputApi, int inputPort, const char *inputToken, const char *inputLogLocation);
    explicit UdpRandomGetter(const char *inputApi, int inputPort, const char *inputToken, const char *inputLogLocation, const char *inputCaPath);

    ~UdpRandomGetter();

    // movable
    UdpRandomGetter(UdpRandomGetter &&rhs) noexcept;
    UdpRandomGetter &operator=(UdpRandomGetter &&rhs) noexcept;

    // and not copyable
    UdpRandomGetter(const UdpRandomGetter &rhs) = delete;
    UdpRandomGetter &operator=(const UdpRandomGetter &rhs) = delete;

    void getRandomFile(uint64_t randomAmount, const char *randomOutputLocation);
    void getRandomBuffer(uint64_t randomAmount, uint8_t *inputBuffer);
    virtual uint64_t getLastWrittenRandAmount();
    // possible to use #include <iosfwd> for declaration of ostream
    // void getRandomOstream(int randomAmount, std::ostream &outputRandomFile);

private:
    ImplPtrTemplate<detail::UdpRandomGetterImpl> hiddenImpPtr;
};

static constexpr bool comp(const UdpRandomGetter::PacketFormat &a, const UdpRandomGetter::PacketFormat &b)
{
    return (a.packetSize < b.packetSize);
}

// Find the max packet size among UdpRandomGetter::PACKET_FORMAT[] at compile time
static constexpr UdpRandomGetter::PacketFormat MAX_PACKET_FORMAT = *std::max_element(std::begin(UdpRandomGetter::PACKET_FORMAT), std::end(UdpRandomGetter::PACKET_FORMAT), comp);
static constexpr int MAX_PACKET_SIZE = MAX_PACKET_FORMAT.packetSize;
} // namespace meteringclientlib
