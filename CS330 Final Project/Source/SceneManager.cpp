///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  EDITED: By Oliver Flores - SNHU CS330 Student 
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/

	// White material for all objects to makle lighting work
	OBJECT_MATERIAL whiteMaterial;
	whiteMaterial.ambientStrength = 1.0f;
	whiteMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	whiteMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
	whiteMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	whiteMaterial.shininess = 2.0f;
	whiteMaterial.tag = "white";
	m_objectMaterials.push_back(whiteMaterial);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	/*** STUDENTS - add the code BELOW for setting up light sources ***/

	// Turn on lighting
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// LIGHT 1: Top front light - WARM YELLOW (like room light)
	m_pShaderManager->setVec3Value("lightSources[0].position", glm::vec3(0.0f, 8.0f, 5.0f));
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(0.9f, 0.7f, 0.4f));  // Warm yellow/orange
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(0.5f, 0.4f, 0.3f));
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 10.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.1f);

	// LIGHT 2: Top back light - WARM YELLOW (like room light)
	m_pShaderManager->setVec3Value("lightSources[1].position", glm::vec3(0.0f, 8.0f, -3.0f));
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", glm::vec3(25.9f, 25.7f, 0.4f));  // Warm yellow/orange
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", glm::vec3(0.5f, 0.4f, 0.3f));
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 10.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.1f);

	// LIGHT 3: Left side light - Cool white (for balance)
	m_pShaderManager->setVec3Value("lightSources[2].position", glm::vec3(-5.0f, 5.0f, 1.0f));
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", glm::vec3(0.6f, 0.6f, 0.6f));  // Cool white
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", glm::vec3(0.4f, 0.4f, 0.4f));
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.1f);

	// LIGHT 4: Right side light - Cool white (for balance)
	m_pShaderManager->setVec3Value("lightSources[3].position", glm::vec3(5.0f, 5.0f, 1.0f));
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", glm::vec3(0.6f, 0.6f, 0.6f));  // Cool white
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", glm::vec3(0.4f, 0.4f, 0.4f));
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.1f);
}


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	// Loading my textures for the pencil
	CreateGLTexture("textures/pencilyellowwood.jpg", "yellowWood");
	CreateGLTexture("textures/pencilwoodtip.jpg", "woodtip");
	CreateGLTexture("textures/pencileraser.jpg", "eraser");
	CreateGLTexture("textures/penciltip.jpg", "graphite");
	CreateGLTexture("textures/tablemarble.jpg", "tabletop");
	CreateGLTexture("textures/notebook.jpg", "notebook");
	CreateGLTexture("textures/tissuebox.jpg", "tissuebox");
	CreateGLTexture("textures/tissuepaper.jpg", "tissuepaper");

	BindGLTextures();

	DefineObjectMaterials();   // Define materials for lighting
	SetupSceneLights();        // Set up the light

	m_basicMeshes->LoadPlaneMesh();     // This will be my table
	m_basicMeshes->LoadBoxMesh();       // For notebook and tissue box
	m_basicMeshes->LoadCylinderMesh();  // For coffee mug, pencil body, and eraser
	m_basicMeshes->LoadConeMesh();      // For pencil tip
	m_basicMeshes->LoadTorusMesh();      // For pencil tip
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("tabletop");
	SetShaderMaterial("white");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	/******************************************************************/

	//Pencil
	

	// PENCIL BODY (main cylinder)
	scaleXYZ = glm::vec3(0.3f, 4.0f, 0.3f);		// Length is 4.0

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 90.0f;					// Lays flat

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);	// Center the body at X=0

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Using the yellow wood texture
	SetShaderTexture("yellowWood");
	SetShaderMaterial("white");  
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();
	/******************************************************************/

	// WOODEN TIP (cone)	
	scaleXYZ = glm::vec3(0.3f, 0.8f, 0.3f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 90.0f;					// Same rotation as body

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-4.0f, 0.0f, 0.0f);	// Position at the end of body

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Using the wood tip texture
	SetShaderTexture("woodtip");
	SetShaderMaterial("white"); 
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawConeMesh();
	/******************************************************************/

	// GRAPHITE TIP (small cone)	
	scaleXYZ = glm::vec3(0.15f, 0.3f, 0.15f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 90.0f;					// Same rotation as body

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-4.5f, 0.0f, 0.0f);	// Position at end of wooden tip

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Using the black graphite texture
	SetShaderTexture("graphite");
	SetShaderMaterial("white");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawConeMesh();
	/******************************************************************/

	// ERASER (cylinder)	
	scaleXYZ = glm::vec3(0.35f, 0.6f, 0.35f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 90.0f;					// Same rotation as body

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.4f, 0.0f, 0.0f);	// Position at negative end of body

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Using the pink eraser texture
	SetShaderTexture("eraser");
	SetShaderMaterial("white");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	/*** NOTEBOOK - Back left area ***/
	scaleXYZ = glm::vec3(4.0f, 0.3f, 3.0f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 20.0f; // Slight angle for visual interest
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-5.0f, 0.15f, 3.0f); // Back left

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("notebook");
	SetShaderMaterial("white");

	m_basicMeshes->DrawBoxMesh();

	/*** TISSUE - Simple folded look with two perpendicular planes ***/

	// First plane - standing up slightly
	scaleXYZ = glm::vec3(0.8f, 0.5f, 0.1f); // Make it vertical

	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-4.0f, 1.21f, -3.0f); // Standing up from box

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1);


	m_basicMeshes->DrawPlaneMesh();

	// Second plane - folded over (horizontal)
	scaleXYZ = glm::vec3(2.5f, 1.2f, 1.8f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-4.0f, 0.6f, -3.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("tissuebox");
	SetShaderMaterial("white");

	m_basicMeshes->DrawBoxMesh();


	// Second plane - the tissue

	scaleXYZ = glm::vec3(0.6f, 0.0f, 0.6f);


	XrotationDegrees = 90.0f; // Make it horizontal
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(-4.0f, 1.8f, -3.0f); // On top of vertical piece

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("tissuepaper");
	SetShaderMaterial("white");

	m_basicMeshes->DrawPlaneMesh();

	/*** COFFEE MUG - With torus handle ***/
	// Mug body
	scaleXYZ = glm::vec3(1.5f, 2.0f, 1.5f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	positionXYZ = glm::vec3(5.0f, 0.0f, -4.0f); // Front right

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1f, 0.1f, 0.1f, 1); 

	m_basicMeshes->DrawCylinderMesh();

	// MUG HANDLE - Simple torus
	scaleXYZ = glm::vec3(0.8f, 0.8f, 0.3f); // Flattened torus for handle shape

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f; // Orient the torus to stick out from the mug

	positionXYZ = glm::vec3(6.2f, 1.0f, -4.0f); // Positioned to the right side of mug

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1f, 0.1f, 0.1f, 1); 

	m_basicMeshes->DrawTorusMesh();

}
