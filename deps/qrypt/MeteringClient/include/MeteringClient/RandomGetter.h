
#pragma once

/**
 * This is the header file for the RandomGetter class. This is the abstract
 * base class for other classes defining how to get random.
*/

#include "ImplPtrTemplate_h.h"
#include <memory> // unique_ptr

namespace meteringclientlib
{
// Provide a version for the library programmatically. Version may
// not match package.
static const char *MeteringClientLibraryVersion() { return "1.2.1"; }

static const int B_TO_KB = 1024;

class RandomGetter
{
public:
	static const char *RANDOM_FILE_NAME;
	// number of bytes to get before logging
	static const uint64_t LOG_LIMIT = 1024 * meteringclientlib::B_TO_KB;
	static const int DEFAULT_FAIL_MAX = 3;
	static const int RANDOM_PACKET_SIZE = 1024;

public:
	explicit RandomGetter();

	virtual ~RandomGetter();

	virtual void getRandomFile(uint64_t randomAmount, const char *randomOutputLocation) = 0;
	virtual void getRandomBuffer(uint64_t randomAmount, uint8_t *inputBuffer) = 0;
	virtual void removeRandomFile();

private:
};
} // namespace meteringclientlib
