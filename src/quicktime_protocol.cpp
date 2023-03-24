#include "scrcpy_ios/quicktime_protocol.h"

#include <memory> // malloc

using namespace scrcpy_ios;
using namespace scrcpy_ios::quicktime_protocol;


bool PingPacket::Serialize(char** buf, size_t* size) const {
	char* buffer = static_cast<char*>(malloc(kLength));
	if (buffer) {
		*(uint32_t*)buffer = kLength;
		*(uint32_t*)(buffer + 4) = kMagic;
		*(uint64_t*)(buffer + 4 + 4) = kHeader; // TODO: wrap it
		*buf = buffer;
		*size = kLength;
		return true;
	}
	return false;
}