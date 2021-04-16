#include "Renderer.h"
#include <glad/glad.h>
#include <iostream>

Renderer::Renderer()
{

}

void Renderer::Init()
{
	GLPrepare();
	SetupImgui();

	mCamera = std::make_unique<Camera>(glm::vec3(-10.f, 5.f, 0.f), 45, 16.f / 9.f, 0.1f, 100.f);

	BuildFrameBuffers();
	BuildTextures();
	BuildSmokes();
	BuildLights();
	BuildGeometries();
	BuildMaterials();
	BuildShaders();
	BuildRenderItems();
}

void Renderer::BuildFrameBuffers()
{
	mShadowMap = std::make_unique<ShadowMap>(2048, 2048);
	mShadowMap->BuildFrameBuffer();
	mSceneFB = std::make_unique<GameFrameBufferObject>(1920, 1080);
	mSceneFB->BuildFrameBuffer();
}

void Renderer::BuildLights()
{
	auto dirLight = std::make_unique<Light>();
	dirLight->type = 0;
	dirLight->strength = glm::vec3(4.0f, 4.0f, 4.0f);
	dirLight->position = glm::vec3(0.f, 15.f, 10.f);
	dirLight->focalPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	mLights.push_back(std::move(dirLight));
	//auto pointLight = std::make_unique<Light>();
	//pointLight->type = 1;
	//pointLight->strength = glm::vec3(0.8f, 0.4f, 0.5f);
	//pointLight->position = glm::vec3(1.f, 1.f, 0.f);
	//pointLight->falloffStart = 1.f;
	//pointLight->falloffEnd = 4.f;
	//mLight.push_back(std::move(pointLight));
}

void Renderer::BuildSmokes()
{
	auto smoke = std::make_unique<Smoke>();
	smoke->name = "smoke";
	smoke->BuildResource();
	smokes[smoke->name] = std::move(smoke);
}

void Renderer::BuildTextures()
{
	//std::vector<std::string> ddsTexNames =
	//{
	//	"bricksDiffuseMap",
	//	"bricksNormalMap",
	//	"tileDiffuseMap",
	//	"tileNormalMap"
	//};

	//std::vector<std::string> ddsTexFilenames =
	//{
	//	"Texture/bricks2.dds",
	//	"Texture/bricks2_nmap.dds",
	//	"Texture/tile.dds",
	//	"Texture/tile_nmap.dds"
	//};

	std::vector<std::string> stbTexNames =
	{
		"brickWallDiffuseMap",
		"brickWallNormalMap"
	};

	std::vector<std::string> stbTexFilenames =
	{
		"Texture/brickwall.jpg",
		"Texture/brickwall_normal.jpg"
	};

	std::vector<std::string> stbCubeNames =
	{
		"skyBox"
	};

	std::vector<std::vector<std::string>> stbCubeFilenames =
	{
		{
			"Texture/right.jpg",
			"Texture/left.jpg",
			"Texture/top.jpg",
			"Texture/bottom.jpg",
			"Texture/front.jpg",
			"Texture/back.jpg"
		}
	};

	// DDS
	//for (int i = 0; i < ddsTexNames.size(); ++i)
	//{
	//	auto texMap = std::make_unique<Texture>();
	//	texMap->name = ddsTexNames[i];
	//	texMap->path = ddsTexFilenames[i];
	//	auto ret = DDSLoader.Load(texMap->path.c_str());
	//	if (ret != TinyddsLoader::Result::Success)
	//	{
	//		std::cout << "Failed to load.[" << texMap->path << "]\n";
	//		std::cout << "Result : " << int(ret) << "\n";
	//	}
	//	texMap->BuildResource(DDSLoader);

	//	mTextures[texMap->name] = std::move(texMap);
	//}

	// PNG, JPG, BMP
	for (int i = 0; i < stbTexNames.size(); ++i)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->name = stbTexNames[i];
		texMap->path = stbTexFilenames[i];

		texMap->BuildResource(texMap->path.c_str());

		mTextures[texMap->name] = std::move(texMap);
	}
	// Cubemap
	for (int i = 0; i < stbCubeNames.size(); ++i)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->name = stbCubeNames[i];
		texMap->cubePath = stbCubeFilenames[i];

		texMap->BuildResource(texMap->cubePath);

		mTextures[texMap->name] = std::move(texMap);
	}
}

void Renderer::BuildGeometries()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData cube = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 0);
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);

	MeshGeometry cubeMesh;
	cubeMesh.name = "cube";
	cubeMesh.indexCount = (unsigned int)cube.Indices32.size();

	MeshGeometry boxMesh;
	boxMesh.name = "box";
	boxMesh.indexCount = (unsigned int)box.Indices32.size();

	MeshGeometry gridMesh;
	gridMesh.name = "grid";
	gridMesh.indexCount = (unsigned int)grid.Indices32.size();

	MeshGeometry sphereMesh;
	sphereMesh.name = "sphere";
	sphereMesh.indexCount = (unsigned int)sphere.Indices32.size();
	
	std::vector<Vertex> cubeVertices(cube.Vertices.size());
	std::vector<Vertex> boxVertices(box.Vertices.size());
	std::vector<Vertex> gridVertices(grid.Vertices.size());
	std::vector<Vertex> sphereVertices(sphere.Vertices.size());

	for (size_t i = 0; i < cube.Vertices.size(); ++i)
	{
		cubeVertices[i].pos = cube.Vertices[i].Position;
		cubeVertices[i].normal = cube.Vertices[i].Normal;
		cubeVertices[i].tangentU = cube.Vertices[i].TangentU;
		cubeVertices[i].texC = cube.Vertices[i].TexC;
	}
	
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		boxVertices[i].pos = box.Vertices[i].Position;
		boxVertices[i].normal = box.Vertices[i].Normal;
		boxVertices[i].tangentU = box.Vertices[i].TangentU;
		boxVertices[i].texC = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		gridVertices[i].pos = grid.Vertices[i].Position;
		gridVertices[i].normal = grid.Vertices[i].Normal;
		gridVertices[i].tangentU = grid.Vertices[i].TangentU;
		gridVertices[i].texC = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		sphereVertices[i].pos = sphere.Vertices[i].Position;
		sphereVertices[i].normal = sphere.Vertices[i].Normal;
		sphereVertices[i].tangentU = sphere.Vertices[i].TangentU;
		sphereVertices[i].texC = sphere.Vertices[i].TexC;
	}

	cubeMesh.BuildResources(cubeVertices, cube.Indices32);
	boxMesh.BuildResources(boxVertices, box.Indices32);
	gridMesh.BuildResources(gridVertices, grid.Indices32);
	sphereMesh.BuildResources(sphereVertices, sphere.Indices32);

	mGeometries["cube"] = std::move(std::make_unique<MeshGeometry>(cubeMesh));
	mGeometries["box"] = std::move(std::make_unique<MeshGeometry>(boxMesh));
	mGeometries["grid"] = std::move(std::make_unique<MeshGeometry>(gridMesh));
	mGeometries["sphere"] = std::move(std::make_unique<MeshGeometry>(sphereMesh));
}

void Renderer::BuildShaders()
{
	// Skybox
	auto skyShader = std::make_unique<Shader>();
	skyShader->CreateVS("Shader/SkyBox.vert");
	skyShader->CreatePS("Shader/SkyBox.frag");
	skyShader->Attach();
	skyShader->Link();
	skyShader->Use();
	skyShader->SetInt("skybox", 0);
	mShaders["Sky"] = std::move(skyShader);

	// Shadows
	auto shadowShader = std::make_unique<Shader>();
	shadowShader->CreateVS("Shader/Shadow.vert");
	shadowShader->CreatePS("Shader/Shadow.frag");
	shadowShader->Attach();
	shadowShader->Link();
	shadowShader->Use();
		// Main light: directional
	glm::mat4 lightView = glm::lookAt(mLights[0]->position, mLights[0]->focalPoint, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.f);
	shadowShader->SetMat4("lightProjView", lightProj * lightView);
	mShaders["Shadow"] = std::move(shadowShader);

	// Opaque objects
	auto shapeShader = std::make_unique<Shader>();
	shapeShader->CreateVS("Shader/Shape.vert");
	shapeShader->CreatePS("Shader/Shape.frag");
	shapeShader->Attach();
	shapeShader->Link();
	shapeShader->Use();
	for (int i = 0; i < mLights.size(); ++i)
	{
		shapeShader->SetInt("light[" + std::to_string(i) + "].type", mLights[i]->type);
		shapeShader->SetVec3("light[" + std::to_string(i) + "].pos", mLights[i]->position);
		shapeShader->SetVec3("light[" + std::to_string(i) + "].strength", mLights[i]->strength);
		shapeShader->SetVec3("light[" + std::to_string(i) + "].dir", mLights[i]->focalPoint - mLights[i]->position);
		shapeShader->SetFloat("light[" + std::to_string(i) + "].fallStart", mLights[i]->falloffStart);
		shapeShader->SetFloat("light[" + std::to_string(i) + "].fallEnd", mLights[i]->falloffEnd);
		shapeShader->SetFloat("light[" + std::to_string(i) + "].spotPower", mLights[i]->spotPower);
	}
	shapeShader->SetInt("diffuseMap", 0);
	shapeShader->SetInt("normalMap", 1);
	shapeShader->SetInt("shadowMap", 2);
	shapeShader->SetMat4("lightProjView", lightProj * lightView);
	mShaders["Shape"] = std::move(shapeShader);

	auto lightShader = std::make_unique<Shader>();
	lightShader->CreateVS("Shader/Light.vert");
	lightShader->CreatePS("Shader/Light.frag");
	lightShader->Attach();
	lightShader->Link();
	lightShader->Use();
	mShaders["Light"] = std::move(lightShader);

	// Smoke
	auto smokeShader = std::make_unique<Shader>();
	smokeShader->CreateVS("Shader/Smoke.vert");
	smokeShader->CreatePS("Shader/Smoke.frag");
	smokeShader->Attach();
	smokeShader->Link();
	smokeShader->Use();
	for (int i = 0; i < mLights.size(); ++i)
	{
		smokeShader->SetVec3("light.pos", mLights[i]->position);
		smokeShader->SetVec3("light.strength", mLights[i]->strength);
		smokeShader->SetVec3("light.dir", mLights[i]->focalPoint - mLights[i]->position);
	}
	mShaders["Smoke"] = std::move(smokeShader);
}

void Renderer::BuildMaterials()
{
	auto sky = std::make_unique<Material>();
	sky->name = "sky";
	sky->diffuseID = mTextures["skyBox"]->textureID;

	auto defaultMat = std::make_unique<Material>();
	defaultMat->name = "default";
	defaultMat->albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	defaultMat->metallic = 0.1f;
	defaultMat->roughness = 1.f;
	defaultMat->ambientOcclusion = 0.f;

	auto brick0 = std::make_unique<Material>();
	brick0->name = "brick0";
	brick0->albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	brick0->metallic = 0.1f;
	brick0->roughness = 0.8f;
	brick0->ambientOcclusion = 0.f;
	brick0->useDiffuse = true;
	brick0->useNormal = true;
	brick0->diffuseID = mTextures["brickWallDiffuseMap"]->textureID;
	brick0->normalID = mTextures["brickWallNormalMap"]->textureID;

	auto tile = std::make_unique<Material>();
	tile->name = "tile";
	tile->albedo = glm::vec3(1.0f, 1.0f, 1.0f);
	tile->metallic = 0.1f;
	tile->roughness = 0.8f;
	tile->ambientOcclusion = 0.f;
	tile->useDiffuse = true;
	tile->useNormal = true;
	tile->diffuseID = mTextures["brickWallDiffuseMap"]->textureID;
	tile->normalID = mTextures["brickWallNormalMap"]->textureID;

	auto light0 = std::make_unique<Material>();
	light0->name = "light0";
	light0->emmisive = mLights[0]->strength;

	auto smoke = std::make_unique<Material>();
	smoke->name = "smoke";
	smoke->densityID = smokes["smoke"]->densityFieldID;

	mMaterials["sky"] = std::move(sky);
	mMaterials["default"] = std::move(defaultMat);
	mMaterials["brick0"] = std::move(brick0);
	mMaterials["tile"] = std::move(tile);
	mMaterials["light0"] = std::move(light0);
}

void Renderer::BuildRenderItems()
{
	auto skyItem = std::make_unique<RenderItem>();
	skyItem->name = "Sky";
	skyItem->UpdateWorld();
	skyItem->mat = mMaterials["sky"].get();
	skyItem->geo = mGeometries["cube"].get();
	mSkyItem = std::move(skyItem);

	auto boxItem = std::make_unique<RenderItem>();
	boxItem->name = "Box";
	boxItem->position = glm::vec3(0.f, 2.f, 0.f);
	boxItem->UpdateWorld();
	boxItem->mat = mMaterials["brick0"].get();
	boxItem->geo = mGeometries["box"].get();
	boxItem->textureScale = 1;
	mOpaqueItems.push_back(boxItem.get());
	mRenderItems.push_back(std::move(boxItem));

	auto sphereItem = std::make_unique<RenderItem>();
	sphereItem->name = "Sphere";
	sphereItem->position = glm::vec3(3.f, 2.f, 0.f);
	sphereItem->UpdateWorld();
	sphereItem->mat = mMaterials["brick0"].get();
	sphereItem->geo = mGeometries["sphere"].get();
	sphereItem->textureScale = 1;
	mOpaqueItems.push_back(sphereItem.get());
	mRenderItems.push_back(std::move(sphereItem));

	auto sphereItem2 = std::make_unique<RenderItem>();
	sphereItem2->name = "Sphere2";
	sphereItem2->position = glm::vec3(0.f, 2.f, -2.f);
	sphereItem2->UpdateWorld();
	sphereItem2->mat = mMaterials["default"].get();
	sphereItem2->geo = mGeometries["sphere"].get();
	sphereItem2->textureScale = 1;
	mOpaqueItems.push_back(sphereItem2.get());
	mRenderItems.push_back(std::move(sphereItem2));

	auto gridItem = std::make_unique<RenderItem>();
	gridItem->name = "Floor";
	gridItem->UpdateWorld();
	gridItem->mat = mMaterials["tile"].get();
	gridItem->geo = mGeometries["grid"].get();
	gridItem->textureScale = 5;
	mOpaqueItems.push_back(gridItem.get());
	mRenderItems.push_back(std::move(gridItem));

	auto smokeItem = std::make_unique<RenderItem>();
	smokeItem->name = "Smoke";
	smokeItem->position = glm::vec3(2.f, 5.f, 4.f);
	smokeItem->scale = glm::vec3(2.f, 8.f, 2.f);
	smokeItem->UpdateWorld();
	smokeItem->mat = mMaterials["brick0"].get();
	smokeItem->geo = mGeometries["cube"].get();
	smokeItem->textureScale = 1;
	mSmokeItems.push_back(smokeItem.get());
	mRenderItems.push_back(std::move(smokeItem));

	auto lightItem = std::make_unique<RenderItem>();
	lightItem->name = "DirectionalLight";
	lightItem->position = mLights[0]->position;
	lightItem->scale = glm::vec3(0.3f);
	lightItem->UpdateWorld();
	lightItem->mat = mMaterials["light0"].get();
	lightItem->geo = mGeometries["cube"].get();
	mLightItems.push_back(lightItem.get());
	mRenderItems.push_back(std::move(lightItem));
}

void Renderer::UpdatePassCb()
{
	mPassCb.view = mCamera->GetViewMatrix();
	mPassCb.proj = mCamera->GetProjMatrix();
	mPassCb.eyePos = mCamera->position;
	mPassCb.lightNum = mLights.size();
}

void Renderer::UpdateObjectCb()
{
	for (int i = 0; i < mRenderItems.size(); ++i)
	{
		mRenderItems[i]->UpdateWorld();
	}
}

void Renderer::UpdateSmoke()
{
	phyxMutex.lock();
	glBindTexture(GL_TEXTURE_3D, smokes["smoke"]->densityFieldID);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, 32, 128, 32, 0, GL_RED, GL_FLOAT, &smokes["smoke"]->density[0]);
	phyxMutex.unlock();
}

void Renderer::Draw()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		// Process input
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		ProcessInput(mWindow);

		// Update info
		UpdatePassCb();
		UpdateObjectCb();
		UpdateSmoke();

		glBindFramebuffer(GL_FRAMEBUFFER, mSceneFB->GetFrameBufferID());
		// Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, screenWidth, screenHeight);

		glEnable(GL_DEPTH_TEST);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		DrawLight();
		DrawShadow();
		DrawOpaque();
		DrawSkyBox();
		DrawSmoke();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawImgui();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	}
	GLFinish();
}

void Renderer::DrawLight()
{
	auto light = mShaders["Light"].get();
	light->Use();
	light->SetMat4("passCb.view", mPassCb.view);
	light->SetMat4("passCb.proj", mPassCb.proj);
	for (auto item : mLightItems)
	{
		light->SetMat4("model", item->world);
		light->SetVec3("emmisive", item->mat->emmisive);
		item->Draw(light);
	}
}

void Renderer::DrawShadow()
{
	glViewport(0, 0, mShadowMap->GetWidth(), mShadowMap->GetHeight());
	auto shadow = mShaders["Shadow"].get();
	shadow->Use();
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap->GetFrameBufferID());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto item : mOpaqueItems)
	{
		shadow->SetMat4("model", item->world);
		item->Draw(shadow);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, mSceneFB->GetFrameBufferID());
	glViewport(0, 0, screenWidth, screenHeight);
}

void Renderer::DrawOpaque()
{
	auto shape = mShaders["Shape"].get();
	shape->Use();
	shape->SetMat4("passCb.view", mPassCb.view);
	shape->SetMat4("passCb.proj", mPassCb.proj);
	shape->SetVec3("passCb.eyePos", mPassCb.eyePos);
	shape->SetVec4("passCb.ambientLight", mPassCb.ambientLight);
	shape->SetInt("passCb.lightNum", mPassCb.lightNum);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mShadowMap.get()->GetFrameBufferID());
	for (auto item : mOpaqueItems)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, item->mat->diffuseID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, item->mat->normalID);
		shape->SetMat4("model", item->world);
		shape->SetInt("textureScale", item->textureScale);
		shape->SetVec3("material.albedo", item->mat->albedo);
		shape->SetFloat("material.metallic", item->mat->metallic);
		shape->SetFloat("material.roughness", item->mat->roughness);
		shape->SetFloat("material.ao", item->mat->ambientOcclusion);
		shape->SetFloat("material.useDiffuseMap", item->mat->useDiffuse);
		shape->SetFloat("material.useNormalMap", item->mat->useNormal);
		item->Draw(shape);
	}
}

void Renderer::DrawSmoke()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	auto smoke = mShaders["Smoke"].get();
	smoke->Use();
	smoke->SetMat4("passCb.view", mPassCb.view);
	smoke->SetMat4("passCb.proj", mPassCb.proj);
	smoke->SetVec3("passCb.eyePos", mPassCb.eyePos);
	smoke->SetVec4("passCb.ambientLight", mPassCb.ambientLight);
	smoke->SetInt("passCb.lightNum", mPassCb.lightNum);
	glEnable(GL_TEXTURE_3D);
	for (auto item : mSmokeItems)
	{
		glBindTexture(GL_TEXTURE_3D, item->mat->densityID);
		smoke->SetMat4("model", item->world);
		smoke->SetVec3("bbMin", item->geo->bounds.bbMin);
		smoke->SetVec3("bbMax", item->geo->bounds.bbMax);
		item->Draw(smoke);
	}
	glDisable(GL_BLEND);
}

void Renderer::DrawSkyBox()
{
	glDepthFunc(GL_LEQUAL);
	auto sky = mShaders["Sky"].get();
	sky->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyItem->mat->densityID);
	sky->SetMat4("view", glm::mat4(glm::mat3(mCamera->GetViewMatrix())));
	sky->SetMat4("proj", mPassCb.proj);
	mSkyItem->Draw(sky);
	glDepthFunc(GL_LESS);
}

void Renderer::SetupImgui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	mSceneImGuiCtx = ImGui::CreateContext();
	mUICtx = ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void Renderer::DrawImgui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static int selected = 0;

	{	// Scene
		ImGui::Begin("Scene");
		{
			ImGui::BeginChild("SceneRender");
			ImVec2 wsize = ImGui::GetWindowSize();
			// Invert the V from the UV.
			ImGui::Image((ImTextureID)mSceneFB->GetColorTextureID(), wsize, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndChild();
		}
		ImGui::End();
	}

	{	// RenderItems
		ImGui::Begin("Objects");
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			
			{
				ImGui::BeginChild("left pane", ImVec2(150, 0), true);
				for (int i = 0; i < mRenderItems.size(); i++)
				{
					if (ImGui::Selectable(mRenderItems[i]->name.c_str(), selected == i))
						selected = i;
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();
	}

	{	// Details
		ImGui::Begin("Details");
		{
			ImGui::Text(mRenderItems[selected]->name.c_str());
			if (ImGui::CollapsingHeader("Transform"))
			{
				ImGui::LabelText("label", "Value");
				{
					ImGui::DragFloat3("translate", reinterpret_cast<float*>(&mRenderItems[selected]->position));
					ImGui::DragFloat3("scale", reinterpret_cast<float*>(&mRenderItems[selected]->scale));
				}
			}
			if (ImGui::CollapsingHeader("Matrial"))
			{
				ImGui::LabelText("label", "Value");
				{
					ImGui::DragFloat3("albedo", reinterpret_cast<float*>(&mRenderItems[selected]->mat->albedo), 0.01f, 0.f, 1.f);
					ImGui::DragFloat("metallic", &mRenderItems[selected]->mat->metallic, 0.01f, 0.f, 1.f);
					ImGui::DragFloat("roughness", &mRenderItems[selected]->mat->roughness, 0.01f, 0.f, 1.f);
					ImGui::DragFloat("ao", &mRenderItems[selected]->mat->ambientOcclusion, 0.01f, 0.f, 1.f);
				}
			}
		}
		ImGui::End();
	}

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(mWindow, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::GLPrepare()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	mWindow = glfwCreateWindow(screenWidth, screenHeight, "MantaProject", NULL, NULL);
	if (mWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(mWindow);
	typedef void* (*FUNC)(GLFWwindow*, int, int);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}
}

void Renderer::GLFinish()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
}

void Renderer::ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		exit = 1;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		mCamera->ProcessKeyboard(ECameraMovement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		mCamera->ProcessKeyboard(ECameraMovement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		mCamera->ProcessKeyboard(ECameraMovement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		mCamera->ProcessKeyboard(ECameraMovement::RIGHT, deltaTime);

	double x, y;
	glfwGetCursorPos(mWindow, &x, &y);

	mCamera->ProcessMouseMovement(x, y, GetPressedButton(mWindow));
}

EInputButton Renderer::GetPressedButton(GLFWwindow* window)
{
	EInputButton result = EInputButton::NONE;

	if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
		return EInputButton::LEFT;
	else if (glfwGetMouseButton(window, 1) == GLFW_PRESS)
		return EInputButton::RIGHT;
	else if (glfwGetMouseButton(window, 2) == GLFW_PRESS)
		return EInputButton::MIDDLE;

	return EInputButton::NONE;
}