#pragma once
#ifndef LAYOUT_VERTICAL_H
#define LAYOUT_VERTICAL_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A layout class which controls the position and sizes of its children, laying them out evenly in a column. */
class Layout_Vertical : public UI_Element
{
public:
	// (de)Constructors
	~Layout_Vertical() = default;
	Layout_Vertical(Engine * engine) {
		alignChildren();
	}


	// Interface Implementation
	virtual void update() override {
		alignChildren();

		UI_Element::update();
	}


	// Public Methods
	/** Set the margin distance between elements and the edge of this layout.
	@param	margin		the margin for this layout. */
	void setMargin(const float & margin) {
		m_margin = margin;
	}
	/** Get the margin distance between elements and the edge of this layout.
	@return the the margin for this layout. */
	float getMargin() const {
		return m_margin;
	}
	/** Set the spacing distance between elements in this layout.
	@param	spacing		the spacing distance between elements. */
	void setSpacing(const float & spacing) {
		m_spacing = spacing;
	}
	/** Get the spacing distance between elements in this layout.
	@return the spacing distance between elements. */
	float getSpacing() const {
		return m_spacing;
	}


protected:
	// Protected Methods
	void alignChildren() {
		const float innerRectSize = m_scale.y - m_margin;
		const float elementSize = (innerRectSize / float(m_children.size())) - m_spacing;
		const float position = innerRectSize - elementSize;
		for (size_t x = 0; x < m_children.size(); ++x) {
			m_children[x]->setScale(glm::vec2(m_scale.x - m_margin, elementSize));
			m_children[x]->setPosition(glm::vec2(0, -((2.0f * (float(x) / (float(m_children.size()) - 1.0f)) - 1.0f) * position)));
		}
	}


	// Protected Attributes
	float m_margin = 10.0f;
	float m_spacing = 10.0f;
};

#endif // LAYOUT_VERTICAL_H