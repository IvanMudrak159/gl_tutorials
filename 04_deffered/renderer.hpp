#pragma once

#include <vector>

#include "camera.hpp"
#include "spotlight.hpp"
#include "framebuffer.hpp"
#include "shadowmap_framebuffer.hpp"
#include "ogl_material_factory.hpp"
#include "ogl_geometry_factory.hpp"
#include "SSAONoiseTexture.h"

class QuadRenderer {
public:
	QuadRenderer()
		: mQuad(generateQuadTex())
	{}

	void render(const OGLShaderProgram &aShaderProgram, MaterialParameterValues &aParameters) const {
		aShaderProgram.use();
		aShaderProgram.setMaterialParameters(aParameters, MaterialParameterValues());
		GL_CHECK(glBindVertexArray(mQuad.vao.get()));
  		GL_CHECK(glDrawElements(mQuad.mode, mQuad.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0)));
	}
protected:

	IndexedBuffer mQuad;
};

inline std::vector<CADescription> getColorNormalPositionAttachments() {
	return {
		{ GL_RGBA, GL_FLOAT, GL_RGBA },
		// To store values outside the range [0,1] we need different internal format then normal GL_RGBA
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
	};
}

inline std::vector<CADescription> getSingleColorAttachment() {
	return {
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
	};
}


class Renderer {
public:
	Renderer(OGLMaterialFactory &aMaterialFactory)
		: mMaterialFactory(aMaterialFactory)
	{
		mCompositingShader = std::static_pointer_cast<OGLShaderProgram>(
				mMaterialFactory.getShaderProgram("compositing"));
		// mShadowMapShader = std::static_pointer_cast<OGLShaderProgram>(
		// 	mMaterialFactory.getShaderProgram("solid_color"));
		mShadowMapShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("shadowmap"));
		mSSAOShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("ssao"));
		mDebugShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("debug"));
		mBlurShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("blur"));
	}

	void initialize(int aWidth, int aHeight) {
		mWidth = aWidth;
		mHeight = aHeight;
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

		mSSAOFrameBuffer = std::make_unique<Framebuffer>(aWidth, aHeight, getColorNormalPositionAttachments());
		mBlurFrameBuffer = std::make_unique<Framebuffer>(aWidth, aHeight, getColorNormalPositionAttachments());
		mFramebuffer = std::make_unique<Framebuffer>(aWidth, aHeight, getColorNormalPositionAttachments());
		// mShadowmapFramebuffer = std::make_unique<Framebuffer>(600, 600, getSingleColorAttachment());
		mShadowmapFramebuffer = std::make_unique<ShadowmapFramebuffer>(600, 600);
		mCompositingParameters = {
			{ "u_diffuse", TextureInfo("diffuse", mFramebuffer->getColorAttachment(0)) },
			{ "u_normal", TextureInfo("diffuse", mFramebuffer->getColorAttachment(1)) },
			{ "u_position", TextureInfo("diffuse", mFramebuffer->getColorAttachment(2)) },
			{ "u_shadowMap", TextureInfo("shadowMap", mShadowmapFramebuffer->getDepthMap()) },
			{ "u_ssaoMap", TextureInfo("ssaoMap", mBlurFrameBuffer->getColorAttachment(0)) },
			// { "u_shadowMap", TextureInfo("shadowMap", mShadowmapFramebuffer->getColorAttachment(0)) },
		};
		mDebugParameters = {
			{"u_ssaoMap", TextureInfo("ssaoMap", mBlurFrameBuffer->getColorAttachment(0)) }
		};
	}

	void clear() {
		mFramebuffer->bind();
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		mSSAOFrameBuffer->bind();
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	template<typename TScene, typename TCamera>
	void geometryPass(const TScene &aScene, const TCamera &aCamera, RenderOptions aRenderOptions) {
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		mFramebuffer->bind();
		mFramebuffer->setDrawBuffers();
		auto projection = aCamera.getProjectionMatrix();
		auto view = aCamera.getViewMatrix();

		std::vector<RenderData> renderData;
		for (const auto &object : aScene.getObjects()) {
			auto data = object.getRenderData(aRenderOptions);
			if (data) {
				renderData.push_back(data.value());
			}
		}

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_solidColor"] = glm::vec4(0,0,0,1);
		fallbackParameters["u_viewPos"] = aCamera.getPosition();
		fallbackParameters["u_near"] = aCamera.near();
		fallbackParameters["u_far"] = aCamera.far();
		for (const auto &data: renderData) {
			const glm::mat4 modelMat = data.modelMat;
			const MaterialParameters &params = data.mMaterialParams;
			const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(data.mShaderProgram);
			const OGLGeometry &geometry = static_cast<const OGLGeometry&>(data.mGeometry);
				
			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			shaderProgram.use();
			shaderProgram.setMaterialParameters(params.mParameterValues, fallbackParameters);

			geometry.bind();
			geometry.draw();
		}
		mFramebuffer->unbind();
	}

	std::shared_ptr<OGLTexture> createSSAONoiseTexture() {
		// Create noise vectors

		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;

		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++) {
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		// Create OpenGL texture using your existing factory function
		auto noiseTexture = createTexture();

		// Set up the texture
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, noiseTexture.get()));
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

		// Create and return OGLTexture wrapper
		return std::make_shared<OGLTexture>(std::move(noiseTexture), GL_TEXTURE_2D);
	}

	template<typename TScene, typename TCamera>
	void ssaoPass(const TScene& aScene, const TCamera& aCamera, RenderOptions aRenderOptions)
	{
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		mSSAOFrameBuffer->bind();
		mSSAOFrameBuffer->setDrawBuffers();

		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		std::vector<glm::vec3> samples;

		for (unsigned int i = 0; i < 64; i++)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = normalize(sample);
			sample *= randomFloats(generator);
			float scale = (float)i / 64.0;
			scale = std::lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			samples.push_back(sample);
		}

		std::shared_ptr<ATexture> noiseTexture = createSSAONoiseTexture();

		MaterialParameterValues fallbackParameters;
		fallbackParameters["texNoise"] = TextureInfo("noiseTexture", noiseTexture);
		fallbackParameters["view"] = aCamera.getViewMatrix();
		fallbackParameters["projection"] = aCamera.getProjectionMatrix();
		fallbackParameters["gPosition"] = TextureInfo("diffuse", mFramebuffer->getColorAttachment(2));
		fallbackParameters["gNormal"] = TextureInfo("diffuse", mFramebuffer->getColorAttachment(1));
		fallbackParameters["radius"] = 0.5f;
		fallbackParameters["bias"] = 0.025f;
		fallbackParameters["kernelSize"] = 128;
		fallbackParameters["samples[0]"] = samples;
		mSSAOShader->use();
		mSSAOShader->setMaterialParameters(fallbackParameters);
		mQuadRenderer.render(*mSSAOShader, fallbackParameters);
		mSSAOFrameBuffer->unbind();



		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		mBlurFrameBuffer->bind();
		mBlurFrameBuffer->setDrawBuffers();


		MaterialParameterValues blurParameters;
		blurParameters["ssaoInput"] = TextureInfo("ssaoInput", mSSAOFrameBuffer->getColorAttachment(0));

		mBlurShader->use();
		mBlurShader->setMaterialParameters(blurParameters);
		//mQuadRenderer.render(*mBlurShader, blurParameters);

		mBlurFrameBuffer->unbind();
	}

	template<typename TLight>
	void compositingPass(const TLight &aLight) {
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		mCompositingParameters["u_lightPos"] = aLight.getPosition();
		mCompositingParameters["u_lightMat"] = aLight.getViewMatrix();
		mCompositingParameters["u_lightProjMat"] = aLight.getProjectionMatrix();
		//mQuadRenderer.render(*mCompositingShader, mCompositingParameters);
		mQuadRenderer.render(*mDebugShader, mDebugParameters);
	}

	template<typename TScene, typename TLight>
	void shadowMapPass(const TScene &aScene, const TLight &aLight) {
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		mShadowmapFramebuffer->bind();
		GL_CHECK(glViewport(0, 0, 600, 600));
		GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		mShadowmapFramebuffer->setDrawBuffers();
		auto projection = aLight.getProjectionMatrix();
		auto view = aLight.getViewMatrix();

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_viewPos"] = aLight.getPosition();

		std::vector<RenderData> renderData;
		RenderOptions renderOptions = {"solid"};
		for (const auto &object : aScene.getObjects()) {
			auto data = object.getRenderData(renderOptions);
			if (data) {
				renderData.push_back(data.value());
			}
		}
		mShadowMapShader->use();
		for (const auto &data: renderData) {
			const glm::mat4 modelMat = data.modelMat;
			const MaterialParameters &params = data.mMaterialParams;
			const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(data.mShaderProgram);
			const OGLGeometry &geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			mShadowMapShader->setMaterialParameters(fallbackParameters, {});

			geometry.bind();
			geometry.draw();
		}



		mShadowmapFramebuffer->unbind();
	}

protected:
	int mWidth = 100;
	int mHeight = 100;
	std::unique_ptr<Framebuffer> mFramebuffer;
	//std::unique_ptr<Framebuffer> mShadowmapFramebuffer;
	std::unique_ptr<ShadowmapFramebuffer> mShadowmapFramebuffer;
	std::unique_ptr<Framebuffer> mSSAOFrameBuffer;
	std::unique_ptr<Framebuffer> mBlurFrameBuffer;
	MaterialParameterValues mCompositingParameters;
	MaterialParameterValues mDebugParameters;
	QuadRenderer mQuadRenderer;
	std::shared_ptr<OGLShaderProgram> mCompositingShader;
	std::shared_ptr<OGLShaderProgram> mDebugShader;
	std::shared_ptr<OGLShaderProgram> mShadowMapShader;
	std::shared_ptr<OGLShaderProgram> mSSAOShader;
	std::shared_ptr<OGLShaderProgram> mBlurShader;
	OGLMaterialFactory &mMaterialFactory;
};
