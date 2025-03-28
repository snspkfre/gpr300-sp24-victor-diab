//#include <glm/glm.hpp>
//#include <imgui.h>
#ifndef KINEMATICS_H
#define KINEMATICS_H


#include <algorithm>
#include <vector>
#include "../ew/transform.h"

namespace vd
{
	
	//Represents the local transformation of a single joint
	class JointPose {
	public:
		glm::vec3 m_translation; //Translation
		glm::quat m_rotation; //Euler angle
		glm::vec3 m_scale; //Scale

		JointPose() : m_rotation(glm::quat(1, 0, 0, 0)), m_translation(0.0f), m_scale(1.0f) {}
	};

	//Joint and Skeleton represent the hierarchy structure, but do not hold poses directly. 
	class Joint {
	public:
		std::string m_name; //Human-readable joint name
		Joint* m_parent = nullptr; //Parent index (null if root)
		std::vector<Joint*> m_children;
		bool m_clicked = false;

		JointPose m_localPose;
		glm::mat4x4 m_globalPose;
	};

	void SolveFK(Joint* joint)
	{
		if (joint->m_parent == nullptr)
		{
			//global transform = local transform
			ew::Transform globalPose;
			globalPose.position = joint->m_localPose.m_translation;
			globalPose.rotation = joint->m_localPose.m_rotation;
			globalPose.scale = joint->m_localPose.m_scale;

			joint->m_globalPose = globalPose.modelMatrix();
		}
		else
		{
			ew::Transform localTransform;
			localTransform.position = joint->m_localPose.m_translation;
			localTransform.rotation = joint->m_localPose.m_rotation;
			localTransform.scale = joint->m_localPose.m_scale;
			joint->m_globalPose = joint->m_parent->m_globalPose * localTransform.modelMatrix();
		}

		for (int i = 0; i < joint->m_children.size(); i++)
		{
			SolveFK(joint->m_children[i]);
		}
	}
}

#endif // KINEMATICS_H