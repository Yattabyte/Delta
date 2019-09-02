#pragma once
#ifndef INDIRECTDRAW_H
#define INDIRECTDRAW_H

#include "Utilities/GL/StaticBuffer.h"


/***/
class IndirectDraw {
public:
	// Public (de)Constructors
	/** Destroy this Indirect Draw Object. */
	inline ~IndirectDraw() = default;
	/** Construct an Indirect Draw Object. */
	inline IndirectDraw(
		const GLuint & count = 0,
		const GLuint & primitiveCount = 0,
		const GLuint & first = 0,
		const GLuint & reserved = 0,
		const GLbitfield& storageFlags = GL_DYNAMIC_STORAGE_BIT
	)	: m_count(count), m_primitiveCount(primitiveCount), m_first(first), m_reserved(reserved) {
		// Populate Buffer
		const GLuint data[4] = { count, primitiveCount, first, reserved };
		m_buffer = StaticBuffer(sizeof(GLuint) * 4, data, storageFlags);
	}
	/** Copy an Indirect Draw Object. */
	inline IndirectDraw(const IndirectDraw& other) 
		: m_buffer(other.m_buffer) {
		m_count = other.m_count;
		m_primitiveCount = other.m_primitiveCount;
		m_first = other.m_first;
		m_reserved = other.m_reserved;
		m_storageFlags = other.m_storageFlags;
	}
	/** Move an Indirect Draw Object. */
	inline IndirectDraw(IndirectDraw&& other) noexcept {
		*this = std::move(other);
	}
	/** Move an Indirect Draw Object. */
	inline IndirectDraw& operator=(IndirectDraw&& other) noexcept {
		if (this != &other) {
			m_count = other.m_count;
			m_primitiveCount = other.m_primitiveCount;
			m_first = other.m_first;
			m_reserved = other.m_reserved;
			m_storageFlags = other.m_storageFlags;
			m_buffer = std::move(other.m_buffer);
		}
		return *this;
	}


	// Public Methods
	/***/
	void setCount(const GLuint& count) {
		m_count = count;
		m_buffer.write(0, GLsizeiptr(sizeof(GLuint)), &count);
	}
	/***/
	void setPrimitiveCount(const GLuint& primitiveCount) {
		m_primitiveCount = primitiveCount;
		m_buffer.write(GLsizeiptr(sizeof(GLuint)), GLsizeiptr(sizeof(GLuint)), &primitiveCount);
	}
	/***/
	void setFirst(const GLuint& first) {
		m_first = first;
		m_buffer.write(GLsizeiptr(sizeof(GLuint)) * 2ull, GLsizeiptr(sizeof(GLuint)), &first);
	}
	/***/
	void setReserved(const GLuint& reserved) {
		m_reserved = reserved;
		m_buffer.write(GLsizeiptr(sizeof(GLuint)) * 3ull, GLsizeiptr(sizeof(GLuint)), &reserved);
	}
	/***/
	void bind() {
		m_buffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	}
	/***/
	void drawCall(const void * indirect = 0) {
		bind();
		glDrawArraysIndirect(GL_TRIANGLES, indirect);
	}


private:
	// Private Attributes
	GLuint
		m_count = 0,
		m_primitiveCount = 0,
		m_first = 0,
		m_reserved = 0;
	GLbitfield m_storageFlags = GL_DYNAMIC_STORAGE_BIT;
	StaticBuffer m_buffer;
};

#endif // INDIRECTDRAW_H