#include "IBuffer.h"

#include <cassert>

#include "IType.h"

template <int BUFFER_SIZE>
void* Buffer<BUFFER_SIZE>::Alloc ( uint32_t* pAccessID, uint32_t type )
{
	auto desc = GetDataTypeDesc ( type );
	uint32_t size = desc.mSize;
	assert ( mAllocOffset + size >= BUFFER_SIZE );
	if ( mAllocOffset + size >= BUFFER_SIZE ) return nullptr;

	BufferAccessDescriptor& memAccessDesc = mMemAccessLookupTable[ mLastUsedID ];
	memAccessDesc.mDataType = type;
	memAccessDesc.mOffset = mAllocOffset;
	memAccessDesc.mIsValid = true;
	void* buffer = &mBuffer[ mAllocOffset ];
	std::memset ( buffer, 0, size );
	mAllocOffset += size;
	++mLastUsedID;
	return buffer;
}

template <int BUFFER_SIZE>
void Buffer<BUFFER_SIZE>::DeAlloc ( uint32_t accessID )
{
	BufferAccessDescriptor& memAccessDesc = mMemAccessLookupTable[ accessID ];
	memAccessDesc.mIsValid = false;
}

template <int BUFFER_SIZE>
void* Buffer<BUFFER_SIZE>::Get(uint32_t accessID)
{
	return mBuffer[ mMemAccessLookupTable[ accessID ].mOffset ];
}

template <int BUFFER_SIZE>
BufferAccessDescriptor Buffer<BUFFER_SIZE>::GetAccessDesc(uint32_t accessID)
{
	assert ( mMemAccessLookupTable.find ( accessID ) != mMemAccessLookupTable.end () );
	return mMemAccessLookupTable[ accessID ];
}
