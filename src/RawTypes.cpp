#include "RawTypes.h"

namespace {
	//std::map<uint32_t, const char*> gSupportedRawDataTypes;
	//std::map<const char*, uint32_t> gSupportRawDataTypeSizeAccessTable;
	//std::map<uint32_t, RawDataTypeFlag> gSupportRawDataTypeFlagTable;

	//std::map< DataTypeID, RawDataTypeDesc> gDataTypeLookupTable;
}

//uint32_t RawType::GetTypeSize()
//{
//	return gSupportRawDataTypeSizeAccessTable[ mTypeID ];
//}
//
//void* Buffer::Alloc(uint32_t accessID, uint32_t type)
//{
//	//uint32_t size = 
//	assert ( accessID + size >= MAX_BUFFER_SIZE );
//	if ( accessID + size >= MAX_BUFFER_SIZE ) return nullptr;
//	mOffsetTable[ mLastUsedID++ ] = mAllocOffset;
//	void* buffer = &mBuffer[ mAllocOffset ];
//	std::memset ( buffer, 0, size );
//	mAllocOffset += size;
//	return buffer;
//}
