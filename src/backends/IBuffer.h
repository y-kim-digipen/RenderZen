#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include "IType.h"

using MemAccessOffset = uint32_t;
struct BufferAccessDescriptor
{
	MemAccessOffset mOffset;
	uint32_t mDataType;
	bool mIsValid;
};

template<int BUFFER_SIZE = 1024>
class Buffer
{
public:
	void* Alloc ( uint32_t* pAccessID, uint32_t type );
	void DeAlloc ( uint32_t accessID );
	void* Get ( uint32_t accessID );
	BufferAccessDescriptor GetAccessDesc ( uint32_t accessID );
private:
	MemAccessOffset  mAllocOffset = 0;
	uint32_t mLastUsedID = 0;
	std::map< uint32_t , BufferAccessDescriptor> mMemAccessLookupTable;
	char mBuffer[ BUFFER_SIZE ] {};
};

template <int BUFFER_SIZE>
void* Buffer<BUFFER_SIZE>::Alloc ( uint32_t* pAccessID, uint32_t type ) {
	auto desc = GetDataTypeDesc ( type );
	uint32_t size = desc.mSize;
	assert ( mAllocOffset + size < BUFFER_SIZE );
	if ( mAllocOffset + size >= BUFFER_SIZE ) return nullptr;

	BufferAccessDescriptor& memAccessDesc = mMemAccessLookupTable[ mLastUsedID ];
	memAccessDesc.mDataType = type;
	memAccessDesc.mOffset = mAllocOffset;
	memAccessDesc.mIsValid = true;
	void* buffer = &mBuffer[ mAllocOffset ];
	std::memset ( buffer, 0, size );
	mAllocOffset += size;

	*pAccessID = mLastUsedID;
	++mLastUsedID;
	return buffer;
}

template <int BUFFER_SIZE>
void Buffer<BUFFER_SIZE>::DeAlloc ( uint32_t accessID ) {
	BufferAccessDescriptor& memAccessDesc = mMemAccessLookupTable[ accessID ];
	memAccessDesc.mIsValid = false;
}

template <int BUFFER_SIZE>
void* Buffer<BUFFER_SIZE>::Get ( uint32_t accessID ) {
	return &mBuffer[ mMemAccessLookupTable[ accessID ].mOffset ];
}

template <int BUFFER_SIZE>
BufferAccessDescriptor Buffer<BUFFER_SIZE>::GetAccessDesc ( uint32_t accessID ) {
	assert ( mMemAccessLookupTable.find ( accessID ) != mMemAccessLookupTable.end () );
	return mMemAccessLookupTable[ accessID ];
}
