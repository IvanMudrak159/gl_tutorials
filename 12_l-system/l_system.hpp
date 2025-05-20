#pragma once

#include <vector>
#include "vertex.hpp"
#include "mesh_object.hpp"
#include "ogl_geometry_construction.hpp"
#include "ogl_geometry_factory.hpp"

#include "rule_set.hpp"
#include "l_system_generator.hpp"

inline IndexedBuffer
createVAOFromLines(const std::vector<LineSegment>& lineSegments) {
	IndexedBuffer buffers {
		createVertexArray(),
	};
	buffers.vbos.push_back(createBuffer());
	buffers.vbos.push_back(createBuffer());

	// Create buffers/arrays
	glBindVertexArray(buffers.vao.get());

	// Prepare vertex data and indices
	std::vector<GLuint> indices(lineSegments.size() * 2);
	for (size_t i = 0; i < lineSegments.size(); ++i) {
		indices[2 * i] = 2 * i;
		indices[2 * i + 1] = 2 * i + 1;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffers.vbos[0].get());
	glBufferData(GL_ARRAY_BUFFER, lineSegments.size() * sizeof(LineSegment), lineSegments.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.vbos[1].get());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	// Vertex positions and thickness for start
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineSegment)/2, (void*)offsetof(LineSegment, start));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(LineSegment)/2, (void*)offsetof(LineSegment, startThickness));
	glEnableVertexAttribArray(1);

	// // Vertex positions and thickness for end
	// glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LineSegment), (void*)offsetof(LineSegment, end));
	// glEnableVertexAttribArray(2);
	// glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(LineSegment), (void*)offsetof(LineSegment, endThickness));
	// glEnableVertexAttribArray(3);

	glBindVertexArray(0);

	buffers.indexCount = indices.size();
	buffers.mode = GL_LINES;
	return buffers;
}

inline float getValueOrDefault(const std::map<std::string, float>& myMap, const std::string& key, float defaultValue) {
	auto it = myMap.find(key);
	if (it != myMap.end()) {
		return it->second;
	}
	return defaultValue;
}



class LSystem: public MeshObject {
public:
	LSystem():
		mSeed(std::chrono::system_clock::now().time_since_epoch().count())
	{
		// mLines = {
		// 	LineSegment{ glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), 0.8 },
		// 	LineSegment{ glm::vec3(0.0f, 1.0f, 0.0f), 0.8f, glm::vec3(0.0f, 1.8f, 0.0f), 0.6 },
		// };
		// auto str = std::string("F[+F][-F]");
		// // auto str = std::string("F+F+F+F--FF");
		// mLines = generateLSystemGeometry(str, 60.0f, 0.5f, 0.2f, 0.8f);
	}

	LSystem(const RuleSet &aRuleSet)
		: mRuleSet(aRuleSet),
		mSeed(std::chrono::system_clock::now().time_since_epoch().count())
	{}

	void loadRuleSet(fs::path aRuleFile) {
		mRuleSet = RuleSet::loadFromFile(aRuleFile);
	}

	void setConfig(const std::map<std::string, float> &aConfig) {
		mConfig = aConfig;
	}

	void runGenerations(int aGenerationCount) {
		currentLSystemString = generateLSystemString(mRuleSet, aGenerationCount, mSeed);
		std::cout << "Current L-System string: " << currentLSystemString << "\n";
		mLines = generateLSystemGeometry(currentLSystemString,
				getValueOrDefault(mConfig, "angle", 60.0f),
				getValueOrDefault(mConfig, "step", 0.5f),
				getValueOrDefault(mConfig, "shortening", 0.8f));
	}

	virtual std::shared_ptr<AGeometry> getGeometry(GeometryFactory &aGeometryFactory, RenderStyle aRenderStyle) {
		return std::make_shared<OGLGeometry>(createVAOFromLines(mLines));
	}

	void prepareRenderData(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) override {
		for (auto &mode : mRenderInfos) {
			mode.second.shaderProgram = aMaterialFactory.getShaderProgram(mode.second.materialParams.mMaterialName);
			mode.second.geometry = getGeometry(aGeometryFactory, mode.second.materialParams.mRenderStyle);
		}
	}
protected:
	RuleSet mRuleSet;
	int mSeed;
	std::string currentLSystemString;
	std::vector<LineSegment> mLines;
	std::map<std::string, float> mConfig;
};


