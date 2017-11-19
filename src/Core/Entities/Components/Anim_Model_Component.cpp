#include "Entities\Components\Anim_Model_Component.h"
#include "Systems\ECS\ECSMessage.h"
#include "Systems\ECS\ECSMessages.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"

Anim_Model_Component::~Anim_Model_Component()
{
	glDeleteBuffers(1, &m_uboID);
}

Anim_Model_Component::Anim_Model_Component(const ECSHandle &id, const ECSHandle &pid) : Geometry_Component(id, pid)
{
	m_uboID = 0;
	Asset_Manager::load_asset(m_model, "");
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Transform_Buffer), &m_uboData, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &m_uboData, sizeof(Transform_Buffer));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Anim_Model_Component::Update()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform_Buffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Anim_Model_Component::Draw()
{
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);

		glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
		glBindVertexArray(m_model->gl_vao_ID);
		glDrawArrays(GL_TRIANGLES, 0, m_model->mesh_size);
		glBindVertexArray(0);
	}
}

bool Anim_Model_Component::IsVisible(const mat4 & PVMatrix)
{
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		Frustum frustum(PVMatrix * m_uboData.mMatrix);

		if (frustum.AABBInFrustom(m_model->bbox_min, m_model->bbox_max))
			return true;
	}

	return false;	
}

void Anim_Model_Component::ReceiveMessage(ECSMessage * message)
{
	switch (message->GetTypeID()) {
		case SET_MODEL_DIR:
		{
			auto msg = (MSG_Set_Model_Dir*)message;
			Asset_Manager::load_asset(m_model, msg->GetPayload());
			break;
		}
		case SET_TRANSFORM:
		{
			auto msg = (MSG_Set_Transform*)message;
			m_uboData.mMatrix = msg->GetPayload().modelMatrix;
			Update();
			break;
		}
	}
}