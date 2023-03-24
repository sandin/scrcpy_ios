#ifndef SCRCPY_IOS_QUICKTIME_PROTOCOL_H
#define SCRCPY_IOS_QUICKTIME_PROTOCOL_H

#include <cstdint>

namespace scrcpy_ios {

namespace quicktime_protocol {


class Packet {
public:
  virtual bool Serialize(char** buf, size_t* size) const = 0;
  virtual ~Packet() {};

}; // class Packet

class PingPacket : public Packet {
public:
	const static uint32_t kMagic = 0x70696E67;
	const static uint32_t kLength = 16;
	const static uint64_t kHeader = 0x0000000100000000;

	virtual bool Serialize(char** buf, size_t* size) const override;

}; // class PingPacket



} // namepsace quicktime_protocol

}  // namespace scrcpy_ios

#endif  // SCRCPY_IOS_QUICKTIME_PROTOCOL_H