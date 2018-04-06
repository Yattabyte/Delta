#pragma once
#ifndef VECTORBUFFER_H
#define VECTORBUFFER_H
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "GL\glew.h"
#include <utility>

struct VB_Ptr {
	void * pointer;
};

template <typename T>
class VectorBuffer
{
public:
	// Public (de)Constructors
	~VectorBuffer() {
		if (m_bufferID != 0) {
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);
		}
		if (m_fence)
			glDeleteSync(m_fence);
	}
	/** Default. */
	VectorBuffer(const GLsizeiptr & sizeHint = 512) {
		m_count = 0;
		m_maxCapacity = sizeHint;
		m_bufferID = 0;
		constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glCreateBuffers(1, &m_bufferID);
		glNamedBufferStorage(m_bufferID, sizeHint, 0, GL_DYNAMIC_STORAGE_BIT | flags);
		m_ptrContainer.pointer = glMapNamedBufferRange(m_bufferID, 0, sizeHint, flags);
		m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
		

	// Public Methods
	/** Bind this buffer.
	* @param	target	the target type of this buffer */
	void bindBuffer(const GLenum & target) {
		glBindBuffer(target, m_bufferID);
	}
	/** Bind this buffer to a particular binding point for shaders.
	 * @param	target	the target type of this buffer
	 * @param	index	the binding point index to use */
	void bindBufferBase(const GLenum & target, const GLuint & index) {
		glBindBufferBase(target, index, m_bufferID);
	}	
	/** Add an element to this buffers list. 
	 * @param	uboIndex	the element to add to this list
	 * @return	container holding the current buffer pointer (underlying pointer subject to change) */
	VB_Ptr * const addElement(unsigned int * uboIndex) {
		const GLsizeiptr elementSize = sizeof(T);
		expandToFit(m_count * elementSize, elementSize);

		*uboIndex = m_count++;
		m_indexPointers.push_back(uboIndex);
		return &m_ptrContainer;
	}
	/** Remove an element from this buffers list. *
	 * @param	uboIndex	the element to remove from this list */
	void removeElement(const unsigned int * uboIndex) {
		replaceWithEnd(uboIndex);
	}


private:
	// Private Methods
	/** Cause a synchronization point if the sync fence hasn't been passed. */
	void checkFence() {
		// Check if we should cause a synchronization point
		if (m_fence != nullptr) {
			auto state = GL_UNSIGNALED;
			while (state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED || state == GL_WAIT_FAILED)
				state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			glDeleteSync(m_fence);
			m_fence = nullptr;
		}
	}
	/** Create a sync fence, use if this buffers data was just changed. */
	void placeFence() {
		if (m_fence)
			glDeleteSync(m_fence);
		m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	/* Expands this buffer's container if it can't fit the specified range to write into
	 * @note Technically creates a new a new buffer to replace the old one and copies the old data
	 * @param	offset	byte offset from the beginning
	 * @param	size	the size of the data to write */
	void expandToFit(const GLsizeiptr & offset, const GLsizeiptr & size) {
		if (offset + size > m_maxCapacity) {
			// Create new buffer large enough to fit old data + new data
			const GLsizeiptr oldSize = m_maxCapacity;
			m_maxCapacity += offset + (size * 1.5);

			// Create new buffer
			checkFence();
			constexpr GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
			GLuint newBuffer = 0;
			glCreateBuffers(1, &newBuffer);
			glNamedBufferStorage(newBuffer, m_maxCapacity, 0, GL_DYNAMIC_STORAGE_BIT | flags);

			// Copy old buffer
			if (oldSize)
				glCopyNamedBufferSubData(m_bufferID, newBuffer, 0, 0, oldSize);

			// Delete old buffer
			glUnmapNamedBuffer(m_bufferID);
			glDeleteBuffers(1, &m_bufferID);

			// Migrate new buffer
			m_bufferID = newBuffer;
			m_ptrContainer.pointer = glMapNamedBufferRange(m_bufferID, 0, m_maxCapacity, flags);
			placeFence();
		}
	}
	/** */
	void replaceWithEnd(const unsigned int * uboIndex) {
		if (*uboIndex < m_indexPointers.size() - 1) {
			// Move the pointer from the last element of the list to the spot we are deleting
			unsigned int * lastIndex = m_indexPointers.back();
			m_indexPointers.at(*uboIndex) = lastIndex;
			m_indexPointers.pop_back();

			// Move the memory from the last index to the old index
			T * oldData = &reinterpret_cast<T*>(m_ptrContainer.pointer)[*uboIndex];
			T * endData = &reinterpret_cast<T*>(m_ptrContainer.pointer)[*lastIndex];
			*oldData = *endData;

			// Ensure that the pointers hold the right VALUE for the index the represent
			*lastIndex = *uboIndex;
		}
		m_count--;
	}


	// Private Attributes
	unsigned int m_count;
	std::vector<unsigned int *> m_indexPointers;
	GLuint m_bufferID;
	VB_Ptr m_ptrContainer;
	GLsync m_fence;
	GLuint m_maxCapacity;
};

#endif // VECTORBUFFER_H