#pragma once
#include <cassert>
#include <cstdint>
#include <map>
#include <glad/glad.h>

#include "backends/IType.h"
#include "backends/IBuffer.h"

#include <glad/glad.h>
#include <iostream>
#include "backends/Defsr.h"

template<uint32_t MAX_SHADER = 10, uint32_t MAX_BINDING = 20, uint32_t BUFFER_SIZE_PER_SHADER = 256>
class ShaderUniformManager
{
public:
	struct UniformAccessDesc
	{
		int32_t mGPUBindLocations[ MAX_BINDING ];
		uint32_t mUniformLocations[ MAX_BINDING ];
		uint32_t mBufferAccessIDs[ MAX_BINDING ];
		uint32_t numActiveBindings;
	};

	bool LoadBuffer ( uint32_t shaderHandle );
	void* GetData ( uint32_t shaderIndex, uint32_t cpuBindIndex );

	const UniformAccessDesc& GetUniformAccessDesc ( uint32_t shaderIndex );
	BufferAccessDescriptor GetBufferAccessDesc ( uint32_t shaderIndex, uint32_t cpuBindIndex );
	void SetDirtyFlag ( uint32_t shaderIndex, uint32_t cpuBindIndex, bool status );
	//todo should be private someday..
private:

	UniformAccessDesc mUniformBufferAccessDesc[ MAX_SHADER ] {};
	uint32_t mShaderHandles[ MAX_SHADER ]{};
	uint32_t mNumBufferLoadedShader = 0;
	Buffer<BUFFER_SIZE_PER_SHADER * MAX_SHADER> mBuffer;
	bool mDirtyFlags[MAX_SHADER][ MAX_BINDING ]{};
};


template <uint32_t MAX_SHADER, uint32_t MAX_BINDING, uint32_t BUFFER_SIZE_PER_SHADER>
bool ShaderUniformManager<MAX_SHADER, MAX_BINDING, BUFFER_SIZE_PER_SHADER>::LoadBuffer ( uint32_t shaderHandle ) {
	char* pName = gSharedReadBuffer;
	int32_t numActiveUniform;
	glGetProgramiv ( shaderHandle, GL_ACTIVE_UNIFORMS, &numActiveUniform );

	int32_t length, size;
	uint32_t glDataType;

	uint32_t shaderIndex = mNumBufferLoadedShader;
	mShaderHandles[ shaderIndex ] = shaderHandle;

	auto& desc = mUniformBufferAccessDesc[ shaderIndex ];
	//todo might not be needed
	for ( int32_t index = 0; index < MAX_BINDING; ++index ) {
		desc.mGPUBindLocations[index] = -1;
	}
	
	for ( int32_t uniformIndex = 0, cpuBindLocation = 0; uniformIndex < numActiveUniform; ++uniformIndex ) {
		glGetActiveUniform ( shaderHandle, uniformIndex,
							 READ_BUFFER_SIZE, &length,
							 &size, &glDataType, pName );


		int32_t uniformLocation = glGetUniformLocation ( shaderHandle, pName );
		if ( uniformLocation >= 0 ) {
			DataTypeDesc dataTypeDesc = GetDataTypeDesc ( glDataType );

			int32_t len;
			glGetActiveUniformName ( shaderHandle, uniformLocation, BUFSIZ, &len, gSharedReadBuffer );
			const char* dataTypeStr = dataTypeDesc.mName;
			std::cout << gSharedReadBuffer << "\t";
			std::cout << dataTypeStr << "\t" << pName << "\t\t"  << uniformIndex << "\t" << uniformLocation << std::endl;
			
			desc.mGPUBindLocations[ cpuBindLocation ] = uniformIndex;
			desc.mUniformLocations[ cpuBindLocation ] = uniformLocation;
			uint32_t bufferAccessID;
			mBuffer.Alloc ( &bufferAccessID, glDataType );
			desc.mBufferAccessIDs[ cpuBindLocation ] = bufferAccessID;
			++cpuBindLocation;
			++desc.numActiveBindings;
		}
		else
		{
			DataTypeDesc dataTypeDesc = GetDataTypeDesc ( glDataType );

			const char* dataTypeStr = dataTypeDesc.mName;
			std::cout << dataTypeStr << "\t" << pName << "\t\t" << uniformIndex << "\t" << uniformLocation << std::endl;
		}
	}
	++mNumBufferLoadedShader;
	//todo handle uniform block objects
	//glGetProgramiv ( shaderHandle, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniform );

	//for ( int32_t uniformBlockIndex = 0; uniformBlockIndex < numActiveUniform; ++uniformBlockIndex ) {
	//	glGetActiveUniformBlockName ( shaderHandle, uniformBlockIndex,
	//								  MAX_UNIFORM_NAME_LEN, &length, name );

	//	std::cout << name << std::endl;
	//}

	return true;
}

template <uint32_t MAX_SHADER, uint32_t MAX_BINDING, uint32_t BUFFER_SIZE_PER_SHADER>
void* ShaderUniformManager<MAX_SHADER, MAX_BINDING, BUFFER_SIZE_PER_SHADER>::GetData ( uint32_t shaderIndex,
																					   uint32_t cpuBindIndex ) {
	const auto& desc = mUniformBufferAccessDesc[ shaderIndex ];
	uint32_t bufferAccessID = desc.mBufferAccessIDs[ cpuBindIndex ];
	return mBuffer.Get ( bufferAccessID );
}

template <uint32_t MAX_SHADER, uint32_t MAX_BINDING, uint32_t BUFFER_SIZE_PER_SHADER>
const typename ShaderUniformManager<MAX_SHADER, MAX_BINDING, BUFFER_SIZE_PER_SHADER>::UniformAccessDesc& ShaderUniformManager<
	MAX_SHADER, MAX_BINDING, BUFFER_SIZE_PER_SHADER>::GetUniformAccessDesc ( uint32_t shaderIndex ) {
	return mUniformBufferAccessDesc[ shaderIndex ];
}

template <uint32_t MAX_SHADER, uint32_t MAX_BINDING, uint32_t BUFFER_SIZE_PER_SHADER>
BufferAccessDescriptor ShaderUniformManager<MAX_SHADER, MAX_BINDING, BUFFER_SIZE_PER_SHADER>::GetBufferAccessDesc (
	uint32_t shaderIndex, uint32_t cpuBindIndex ) {
	uint32_t bufferAccessID =  mUniformBufferAccessDesc[ shaderIndex ].mBufferAccessIDs[ cpuBindIndex ];
	return mBuffer.GetAccessDesc ( bufferAccessID );
}

template <uint32_t MAX_SHADER, uint32_t MAX_BINDING, uint32_t BUFFER_SIZE_PER_SHADER>
void ShaderUniformManager<MAX_SHADER, MAX_BINDING, BUFFER_SIZE_PER_SHADER>::SetDirtyFlag ( uint32_t shaderIndex,
																						   uint32_t cpuBindIndex, bool status ) {
	mDirtyFlags[ shaderIndex ][ cpuBindIndex ] = status;
}

