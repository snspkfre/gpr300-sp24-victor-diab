#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/procGen.h>

#include <vd/animation.h>
#include <vd/kinematics.h>
#include <queue>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void animationControls();
void kinematicsControls(vd::Joint* joint);


//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Transform monkeyTransform;
ew::Camera camera;
ew::CameraController cameraController;

vd::Animator animator;
vd::Joint root;

struct Material
{
	float Ka = 1.0f;
	float Kd = 0.5f;
	float Ks = 0.5f;
	float Shininess = 128;
}material;

bool useBlur = false;
bool useGamma = false;

int bluriness = 5.0f;
float gamma = 2.2f;

glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, -0.2f);
float biasValue = 0.03f;

unsigned int depthMap;

vd::Joint* selJoint = nullptr;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader simpleDepthShader = ew::Shader("assets/simpleDepthShader.vert", "assets/simpleDepthShader.frag");
	ew::Shader postProcessShader = ew::Shader("assets/frameBufferScreen.vert", "assets/postProcessing.frag");

	ew::Model monkeyModel = ew::Model("assets/suzanne.obj");

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	ew::MeshData planeMeshData = ew::createPlane(10.0f, 10.0f, 1);
	ew::Mesh plane(planeMeshData);
	ew::Transform planeTransform;
	planeTransform.position = glm::vec3(0.0f, -5.0f, 0.0f);

	
	animator.clip = new vd::AnimationClip();
	animator.clip->duration = 5.0f;

	animator.clip->positionKeys.push_back(vd::KeyFrame<glm::vec3>(0, glm::vec3(0.0f, 0.0f, 0.0f)));
	animator.clip->positionKeys.push_back(vd::KeyFrame<glm::vec3>(5, glm::vec3(5.0f, 0.0f, 0.0f)));

	animator.isPlaying = true;
	animator.isLooping = true;

	//forward kinematics stuff
	root.m_name = "Root Node";
	root.m_children = std::vector<vd::Joint*>();
	root.m_localPose.m_translation = glm::vec3(0, 2.0f, 0);
	root.m_localPose.m_scale = glm::vec3(0.5f);

	vd::Joint lShoulder;
	lShoulder.m_parent = &root;
	lShoulder.m_localPose.m_translation = glm::vec3(1, 0, 0) * -2.0f;
	lShoulder.m_localPose.m_scale = glm::vec3(0.5f);
	lShoulder.m_name = "Left Shoulder";
	root.m_children.push_back(&lShoulder);

	vd::Joint rShoulder;
	rShoulder.m_parent = &root;
	rShoulder.m_localPose.m_translation = glm::vec3(1, 0, 0) * 2.0f;
	rShoulder.m_localPose.m_scale = glm::vec3(0.5f);
	rShoulder.m_name = "Right Shoulder";
	root.m_children.push_back(&rShoulder);

	vd::Joint lArm;
	lArm.m_parent = &lShoulder;
	lArm.m_localPose.m_translation = glm::vec3(1, 0, 0) * -2.0f;
	lArm.m_localPose.m_scale = glm::vec3(1);
	lArm.m_name = "Left Arm";
	root.m_children.push_back(&lArm);

	vd::Joint rArm;
	rArm.m_parent = &rShoulder;
	rArm.m_localPose.m_translation = glm::vec3(1, 0, 0) * 2.0f;
	rArm.m_localPose.m_scale = glm::vec3(1);
	rArm.m_name = "Left Arm";
	root.m_children.push_back(&rArm);

	vd::Joint lElbow;
	lElbow.m_parent = &lArm;
	lElbow.m_localPose.m_translation = glm::vec3(1, 0, 0) * -1.5f;
	lElbow.m_localPose.m_scale = glm::vec3(1);
	lElbow.m_name = "Left Elbow";
	lArm.m_children.push_back(&lElbow);

	vd::Joint lWrist;
	lWrist.m_parent = &lElbow;
	lWrist.m_localPose.m_translation = glm::vec3(1, 0, 0) * -1.0f;
	lWrist.m_localPose.m_scale = glm::vec3(0.5f);
	lWrist.m_name = "Left Wrist";
	lElbow.m_children.push_back(&lWrist);

	vd::Joint lHand;
	lHand.m_parent = &lWrist;
	lHand.m_localPose.m_translation = glm::vec3(1, 0, 0) * -1.0f;
	lHand.m_localPose.m_scale = glm::vec3(0.5f);
	lHand.m_name = "Left Hand";
	lWrist.m_children.push_back(&lHand);

	vd::Joint rElbow;
	rElbow.m_parent = &rArm;
	rElbow.m_localPose.m_translation = glm::vec3(1, 0, 0) * 1.5f;
	rElbow.m_localPose.m_scale = glm::vec3(1);
	rElbow.m_name = "Right Elbow";
	rArm.m_children.push_back(&rElbow);

	vd::Joint rWrist;
	rWrist.m_parent = &rElbow;
	rWrist.m_localPose.m_translation = glm::vec3(1, 0, 0) * 1.0f;
	rWrist.m_localPose.m_scale = glm::vec3(0.5f);
	rWrist.m_name = "Right Wrist";
	rElbow.m_children.push_back(&rWrist);

	vd::Joint rHand;
	rHand.m_parent = &rWrist;
	rHand.m_localPose.m_translation = glm::vec3(1, 0, 0) * 1.0f;
	rHand.m_localPose.m_scale = glm::vec3(0.5f);
	rHand.m_name = "Right Hand";
	rWrist.m_children.push_back(&rHand);

	vd::Joint head;
	head.m_parent = &root;
	head.m_localPose.m_translation = glm::vec3(0, 2.5f, 0);
	head.m_localPose.m_scale = glm::vec3(0.6f);
	head.m_name = "Head";
	root.m_children.push_back(&head);

	vd::Joint lFoot;
	lFoot.m_parent = &root;
	lFoot.m_localPose.m_translation = glm::vec3(1, 0, 0) * -1.0f;
	lFoot.m_localPose.m_translation.y = -1.5f;
	lFoot.m_localPose.m_scale = glm::vec3(0.6f);
	lFoot.m_name = "Left Foot";
	root.m_children.push_back(&lFoot);

	vd::Joint rFoot;
	rFoot.m_parent = &root;
	rFoot.m_localPose.m_translation = glm::vec3(1, 0, 0) * 1.0f;
	rFoot.m_localPose.m_translation.y = -1.5f;
	rFoot.m_localPose.m_scale = glm::vec3(0.6f);
	rFoot.m_name = "Right Foot";
	root.m_children.push_back(&rFoot);

	float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("It's so Joever!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
		animator.Update(deltaTime);
		monkeyTransform.position = animator.GetValue(animator.clip->positionKeys, glm::vec3(0.0f, 0.0f, 0.0f));
		monkeyTransform.rotation = animator.GetValue(animator.clip->rotationKeys, glm::vec3(0.0f, 0.0f, 0.0f)) / 180.0f * 3.14159265358979323846f;
		monkeyTransform.scale = animator.GetValue(animator.clip->scaleKeys, glm::vec3(1.0f));

		cameraController.move(window, &camera, deltaTime);

		//fk updates
		vd::SolveFK(&root);


		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		float near_plane = -15.0f, far_plane = 15.0f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
			lightDir,
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		
		{//configure shader and matrices
			simpleDepthShader.use();
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			glEnable(GL_CULL_FACE);//to avoid peter panning
			glCullFace(GL_FRONT);

			simpleDepthShader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);
		}

		{//render depth
			/*simpleDepthShader.setMat4("_Model", monkeyTransform.modelMatrix());
			monkeyModel.draw();*/

			std::queue<vd::Joint*> fkQueue;
			fkQueue.push(&root);

			while (!fkQueue.empty())
			{
				simpleDepthShader.setMat4("_Model", fkQueue.front()->m_globalPose);
				monkeyModel.draw();

				for (vd::Joint* child : fkQueue.front()->m_children)
				{
					fkQueue.push(child);
				}
				fkQueue.pop();
			}
			
			simpleDepthShader.setMat4("_Model", planeTransform.modelMatrix());
			plane.draw();
		}
		
		glCullFace(GL_BACK);

		// First Pass
		glViewport(0, 0, screenWidth, screenHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		
		shader.use();
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());	
		shader.setInt("_MainTex", 0);
		shader.setInt("_ShadowMap", 1);
		shader.setVec3("_EyePos", camera.position);
		shader.setMat4("_LightSpaceMatrix", lightSpaceMatrix);

		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		shader.setVec3("_LightDirection", lightDir);
		shader.setFloat("_BiasValue", biasValue);

		std::queue<vd::Joint*> fkQueue;
		fkQueue.push(&root);

		while (!fkQueue.empty())
		{
			shader.setMat4("_Model", fkQueue.front()->m_globalPose);
			monkeyModel.draw();

			for (vd::Joint* child : fkQueue.front()->m_children)
			{
				fkQueue.push(child);
			}
			fkQueue.pop();
		}

		shader.setMat4("_Model", planeTransform.modelMatrix());
		plane.draw();

		// Second Pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		postProcessShader.use();
		postProcessShader.setBool("useBlur", useBlur);
		postProcessShader.setBool("useGamma", useGamma);
		postProcessShader.setInt("bluriness", bluriness);
		postProcessShader.setFloat("gamma", gamma);

		glBindVertexArray(quadVAO);
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		

		drawUI();

		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &fbo);

	printf("Shutting down...");
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	ImGui::Text("Add Controls Here!");

	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	if (ImGui::CollapsingHeader("Post Processing Effects"))
	{
		ImGui::Checkbox("Use Blur", &useBlur);
		if (ImGui::SliderInt("Bluriness", &bluriness, 1, 15))
		{
			if (bluriness % 2 == 0)
			{
				bluriness++;
			}
		}
		ImGui::Checkbox("Use Gamma Correction", &useGamma);
		ImGui::SliderFloat("Gamma", &gamma, 0.0f, 10.0f);
	}
	if (ImGui::CollapsingHeader("Shadow Settings")) {
		ImGui::SliderFloat3("Light Direction", &lightDir.x, -1.0f, 1.0f);
		ImGui::SliderFloat("Bias Value", &biasValue, 0.0f, 0.5f);
	}

	/*ImGui::Begin("Shadow Map");
		//Using a Child allow to fill all the space of the window.
		ImGui::BeginChild("Shadow Map");
		//Stretch image to be window size
		ImVec2 windowSize = ImGui::GetWindowSize();
		//Invert 0-1 V to flip vertically for ImGui display
		//shadowMap is the texture2D handle
		ImGui::Image((ImTextureID)depthMap, windowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndChild();
	ImGui::End();*/

	if (selJoint != nullptr)
	{
		glm::vec3 euler = glm::eulerAngles(selJoint->m_localPose.m_rotation);
		ImGui::DragFloat3("Position", &selJoint->m_localPose.m_translation.x, 0.1f);
		ImGui::DragFloat3("Rotation", &euler.x, 0.1f);
		ImGui::DragFloat3("Scale", &selJoint->m_localPose.m_scale.x, 0.1f);
		selJoint->m_localPose.m_rotation = glm::quat(euler);
	}

	ImGui::End();

	//animationControls();
	kinematicsControls(&root);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void animationControls()
{
	const char* itemNames[4] = {
	  "Linear",
	  "Cubic",
	  "Cosine",
	  "Exponential"
	};

	ImGui::Begin("Animation Controls");
	{
		ImGui::Checkbox("Playing", &animator.isPlaying);
		ImGui::Checkbox("Looping", &animator.isLooping);
		ImGui::DragFloat("Playback Speed", &animator.playbackSpeed);
		ImGui::SliderFloat("Playback Time", &animator.playbackTime, 0.0f, animator.clip->duration);
		ImGui::DragFloat("Playback Duration", &animator.clip->duration);

		int pushID = 0;

		if (ImGui::CollapsingHeader("Position Keys")) {
			for (int i = 0; i < animator.clip->positionKeys.size(); i++)
			{
				ImGui::PushID(pushID++);
				{
					int curItem = 1;
					ImGui::DragFloat("Time", &animator.clip->positionKeys[i].time);
					ImGui::DragFloat3("Value", &animator.clip->positionKeys[i].value.x);
					ImGui::Combo("Interpolation Method", &animator.clip->positionKeys[i].method, itemNames, 4);
				}
				ImGui::PopID();
			}

			ImGui::PushID(pushID++);
			{
				if (ImGui::Button("Add Keyframe"))
				{
					if (animator.clip->positionKeys.size() == 0)
						animator.clip->positionKeys.push_back(vd::KeyFrame<glm::vec3>(0, glm::vec3(0.0f, 0.0f, 0.0f)));
					else
						animator.clip->positionKeys.push_back(vd::KeyFrame<glm::vec3>(animator.clip->duration, animator.clip->positionKeys[animator.clip->positionKeys.size() - 1].value));
				}
				if (ImGui::Button("Remove Keyframe") && animator.clip->positionKeys.size() >= 1)
					animator.clip->positionKeys.pop_back();
			}
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Rotation Keys")) {
			for (int i = 0; i < animator.clip->rotationKeys.size(); i++)
			{
				ImGui::PushID(pushID++);
				{
					ImGui::DragFloat("Time", &animator.clip->rotationKeys[i].time);
					ImGui::DragFloat3("Value", &animator.clip->rotationKeys[i].value.x);
					ImGui::Combo("Interpolation Method", &animator.clip->rotationKeys[i].method, itemNames, 4);
				}
				ImGui::PopID();
			}

			ImGui::PushID(pushID++);
			{
				if (ImGui::Button("Add Keyframe"))
				{
					if (animator.clip->rotationKeys.size() == 0)
						animator.clip->rotationKeys.push_back(vd::KeyFrame<glm::vec3>(0, glm::vec3(0.0f, 0.0f, 0.0f)));
					else
						animator.clip->rotationKeys.push_back(vd::KeyFrame<glm::vec3>(animator.clip->duration, animator.clip->rotationKeys[animator.clip->rotationKeys.size() - 1].value));
				}
				if (ImGui::Button("Remove Keyframe") && animator.clip->rotationKeys.size() >= 1)
					animator.clip->rotationKeys.pop_back();
			}
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Scale Keys")) {
			for (int i = 0; i < animator.clip->scaleKeys.size(); i++)
			{
				ImGui::PushID(pushID++);
				{
					ImGui::DragFloat("Time", &animator.clip->scaleKeys[i].time);
					ImGui::DragFloat3("Value", &animator.clip->scaleKeys[i].value.x);
					ImGui::Combo("Interpolation Method", &animator.clip->scaleKeys[i].method, itemNames, 4);
				}
				ImGui::PopID();
			}

			ImGui::PushID(pushID++);
			{
				if (ImGui::Button("Add Keyframe"))
				{
					if (animator.clip->scaleKeys.size() == 0)
						animator.clip->scaleKeys.push_back(vd::KeyFrame<glm::vec3>(0, glm::vec3(1.0f, 1.0f, 1.0f)));
					else
						animator.clip->scaleKeys.push_back(vd::KeyFrame<glm::vec3>(animator.clip->duration, animator.clip->scaleKeys[animator.clip->scaleKeys.size() - 1].value));
				}
				if (ImGui::Button("Remove Keyframe") && animator.clip->scaleKeys.size() >= 1)
					animator.clip->scaleKeys.pop_back();
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}

void kinematicsControls(vd::Joint* joint)
{
	ImGui::Begin("Kinematics Controls");
	{
		ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
		if (ImGui::TreeNodeEx(joint->m_name.c_str(), flag))
		{
			if (ImGui::IsItemClicked())
			{
				//joint->m_clicked = true;
				selJoint = joint;
			}

			for (int i = 0; i < joint->m_children.size(); i++)
			{
				kinematicsControls(joint->m_children[i]);
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

