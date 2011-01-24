#include "Renderer.h"
#include "Camera.h"
#include "RendererInitializer.h"
#include "Material.h"
#include "App.h"
#include "Scene.h"
#include "Exception.h"
#include "ModelNode.h"
#include "Model.h"
#include "Mesh.h"



//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Renderer::Renderer():
	ms(*this),
	is(*this),
	pps(*this),
	bs(*this),
	width(640),
	height(480)
{}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// set from the initializer
	width = initializer.width;
	height = initializer.height;

	aspectRatio = float(width)/height;
	framesNum = 0;

	// a few sanity checks
	if(width < 10 || height < 10)
	{
		throw EXCEPTION("Incorrect sizes");
	}

	// init the stages. Careful with the order!!!!!!!!!!
	ms.init(initializer);
	is.init(initializer);
	pps.init(initializer);
	bs.init(initializer);

	// quad VBOs and VAO
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords), quadVertCoords, GL_STATIC_DRAW);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	quadVertIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces), quadVertIndeces, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, NULL);
	quadVao.attachElementArrayBufferVbo(quadVertIndecesVbo);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void Renderer::render(Camera& cam_)
{
	cam = &cam_;

	viewProjectionMat = cam->getProjectionMatrix() * cam->getViewMatrix();

	ms.run();
	is.run();
	pps.runPrePass();
	bs.run();
	pps.runPostPass();

	++framesNum;
}


//======================================================================================================================
// drawQuad                                                                                                            =
//======================================================================================================================
void Renderer::drawQuad()
{
	quadVao.bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	quadVao.unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// setupShaderProg                                                                                                     =
//======================================================================================================================
void Renderer::setupShaderProg(const Material& mtl, const ModelNode& modelNode, const Camera& cam) const
{
	mtl.getShaderProg().bind();
	uint textureUnit = 0;

	//
	// FFP stuff
	//
	if(mtl.isBlendingEnabled())
	{
		glEnable(GL_BLEND);
		//glDisable(GL_BLEND);
		glBlendFunc(mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}
	else
	{
		glDisable(GL_BLEND);
	}


	if(mtl.isDepthTestingEnabled())
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if(mtl.isWireframeEnabled())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	//
	// calc needed matrices
	//
	Mat4 modelMat(modelNode.getWorldTransform());
	const Mat4& projectionMat = cam.getProjectionMatrix();
	const Mat4& viewMat = cam.getViewMatrix();
	Mat4 modelViewMat;
	Mat3 normalMat;
	Mat4 modelViewProjectionMat;

	// should I calculate the modelViewMat ?
	if(mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT) ||
	   mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT) ||
	   mtl.getStdUniVar(Material::SUV_NORMAL_MAT))
	{
		modelViewMat = Mat4::combineTransformations(viewMat, modelMat);
	}

	// set matrices
	if(mtl.getStdUniVar(Material::SUV_MODEL_MAT))
	{
		mtl.getStdUniVar(Material::SUV_MODEL_MAT)->setMat4(&modelMat);
	}

	if(mtl.getStdUniVar(Material::SUV_VIEW_MAT))
	{
		mtl.getStdUniVar(Material::SUV_VIEW_MAT)->setMat4(&viewMat);
	}

	if(mtl.getStdUniVar(Material::SUV_PROJECTION_MAT))
	{
		mtl.getStdUniVar(Material::SUV_PROJECTION_MAT)->setMat4(&projectionMat);
	}

	if(mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT))
	{
		mtl.getStdUniVar(Material::SUV_MODELVIEW_MAT)->setMat4(&modelViewMat);
	}

	if(mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT))
	{
		mtl.getStdUniVar(Material::SUV_VIEWPROJECTION_MAT)->setMat4(&viewProjectionMat);
	}

	if(mtl.getStdUniVar(Material::SUV_NORMAL_MAT))
	{
		normalMat = modelViewMat.getRotationPart();
		mtl.getStdUniVar(Material::SUV_NORMAL_MAT)->setMat3(&normalMat);
	}

	if(mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT))
	{
		modelViewProjectionMat = projectionMat * modelViewMat;
		mtl.getStdUniVar(Material::SUV_MODELVIEWPROJECTION_MAT)->setMat4(&modelViewProjectionMat);
	}


	//
	// FAis
	//
	if(mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_NORMAL_FAI)->setTexture(ms.getNormalFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DIFFUSE_FAI)->setTexture(ms.getDiffuseFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_SPECULAR_FAI)->setTexture(ms.getSpecularFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI))
	{
		mtl.getStdUniVar(Material::SUV_MS_DEPTH_FAI)->setTexture(ms.getDepthFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_IS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_IS_FAI)->setTexture(is.getFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_PRE_PASS_FAI)->setTexture(pps.getPrePassFai(), textureUnit++);
	}

	if(mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI))
	{
		mtl.getStdUniVar(Material::SUV_PPS_POST_PASS_FAI)->setTexture(pps.getPostPassFai(), textureUnit++);
	}


	//
	// Other
	//
	if(mtl.getStdUniVar(Material::SUV_RENDERER_SIZE))
	{
		Vec2 v(width, height);
		mtl.getStdUniVar(Material::SUV_RENDERER_SIZE)->setVec2(&v);
	}

	if(mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR))
	{
		Vec3 col(SceneSingleton::getInstance().getAmbientCol());
		mtl.getStdUniVar(Material::SUV_SCENE_AMBIENT_COLOR)->setVec3(&col);
	}


	//
	// set user defined vars
	//
	boost::ptr_vector<MtlUserDefinedVar>::const_iterator it = mtl.getUserDefinedVars().begin();
	for(; it !=  mtl.getUserDefinedVars().end(); it++)
	{
		const MtlUserDefinedVar& udv = *it;

		switch(udv.getUniVar().getGlDataType())
		{
			// texture or FAI
			case GL_SAMPLER_2D:
				if(udv.getTexture() != NULL)
				{
					udv.getUniVar().setTexture(*udv.getTexture(), textureUnit);
				}
				else
				{
					switch(udv.getFai())
					{
						case MtlUserDefinedVar::MS_DEPTH_FAI:
							udv.getUniVar().setTexture(ms.getDepthFai(), textureUnit);
							break;
						case MtlUserDefinedVar::IS_FAI:
							udv.getUniVar().setTexture(is.getFai(), textureUnit);
							break;
						case MtlUserDefinedVar::PPS_PRE_PASS_FAI:
							udv.getUniVar().setTexture(pps.getPrePassFai(), textureUnit);
							break;
						case MtlUserDefinedVar::PPS_POST_PASS_FAI:
							udv.getUniVar().setTexture(pps.getPostPassFai(), textureUnit);
							break;
						default:
							RASSERT_THROW_EXCEPTION("WTF?");
					}
				}
				++textureUnit;
				break;
			// float
			case GL_FLOAT:
				udv.getUniVar().setFloat(udv.getFloat());
				break;
			// vec2
			case GL_FLOAT_VEC2:
				udv.getUniVar().setVec2(&udv.getVec2());
				break;
			// vec3
			case GL_FLOAT_VEC3:
				udv.getUniVar().setVec3(&udv.getVec3());
				break;
			// vec4
			case GL_FLOAT_VEC4:
				udv.getUniVar().setVec4(&udv.getVec4());
				break;
		}
	}

	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// renderModelNode                                                                                                     =
//======================================================================================================================
void Renderer::renderModelNode(const ModelNode& modelNode, const Camera& cam, ModelNodeRenderType type) const
{
	boost::ptr_vector<ModelNodePatch>::const_iterator it = modelNode.getModelNodePatches().begin();
	for(; it != modelNode.getModelNodePatches().end(); it++)
	{
		const ModelNodePatch& modelNodePatch = *it;

		if((type == MNRT_MS && modelNodePatch.getCpMtl().renderInBlendingStage()) ||
		   (type == MNRT_BS && !modelNodePatch.getCpMtl().renderInBlendingStage()) ||
		   (type == MNRT_DP && modelNodePatch.getDpMtl().renderInBlendingStage()))
		{
			continue;
		}

		const Material* mtl;
		const Vao* vao;
		if(type == MNRT_MS || type == MNRT_BS)
		{
			mtl = &modelNodePatch.getCpMtl();
			vao = &modelNodePatch.getCpVao();
		}
		else
		{
			mtl = &modelNodePatch.getDpMtl();
			vao = &modelNodePatch.getDpVao();
		}

		// Shader
		setupShaderProg(*mtl, modelNode, cam);

		vao->bind();
		glDrawElements(GL_TRIANGLES, modelNodePatch.getModelPatchRsrc().getMesh().getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
		vao->unbind();
	}
}


//======================================================================================================================
// renderAllModelNodes                                                                                                 =
//======================================================================================================================
void Renderer::renderAllModelNodes(const Camera& cam, ModelNodeRenderType type) const
{
	Vec<ModelNode*>::const_iterator it = SceneSingleton::getInstance().modelNodes.begin();
	for(; it != SceneSingleton::getInstance().modelNodes.end(); ++it)
	{
		const ModelNode& md = *(*it);
		renderModelNode(md, cam, type);
	}

	// the rendering above fucks the polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


//======================================================================================================================
// unproject                                                                                                           =
//======================================================================================================================
Vec3 Renderer::unproject(const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat,
                         const int view[4])
{
	Mat4 invPm = projectionMat * modelViewMat;
	invPm.invert();

	// the vec is in NDC space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	Vec4 vec;
	vec.x() = (2.0 * (windowCoords.x() - view[0])) / view[2] - 1.0;
	vec.y() = (2.0 * (windowCoords.y() - view[1])) / view[3] - 1.0;
	vec.z() = 2.0 * windowCoords.z() - 1.0;
	vec.w() = 1.0;

	Vec4 final = invPm * vec;
	final /= final.w();
	return Vec3(final);
}


//======================================================================================================================
// ortho                                                                                                               =
//======================================================================================================================
Mat4 Renderer::ortho(float left, float right, float bottom, float top, float near, float far)
{
	float difx = right-left;
	float dify = top-bottom;
	float difz = far-near;
	float tx = -(right+left) / difx;
	float ty = -(top+bottom) / dify;
	float tz = -(far+near) / difz;
	Mat4 m;

	m(0, 0) = 2.0 / difx;
	m(0, 1) = 0.0;
	m(0, 2) = 0.0;
	m(0, 3) = tx;
	m(1, 0) = 0.0;
	m(1, 1) = 2.0 / dify;
	m(1, 2) = 0.0;
	m(1, 3) = ty;
	m(2, 0) = 0.0;
	m(2, 1) = 0.0;
	m(2, 2) = -2.0 / difz;
	m(2, 3) = tz;
	m(3, 0) = 0.0;
	m(3, 1) = 0.0;
	m(3, 2) = 0.0;
	m(3, 3) = 1.0;

	return m;
}

