#pragma once

#include <memory>
#include <vector>
#include <ranges>

#include "vertex.hpp"
#include "scene_object.hpp"
#include "cube.hpp"
#include "l_system.hpp"

#include "material_factory.hpp"
#include "geometry_factory.hpp"
#include "simple_scene.hpp"

constexpr unsigned int DIFFUSE = 1;
constexpr unsigned int SPECULAR = 1 << 1;
constexpr unsigned int BUMP = 1 << 2;
constexpr unsigned int PARALLAX = 1 << 3;
constexpr unsigned int AMBIENT_OCC = 1 << 4;
constexpr unsigned int SHADOW = 1 << 5;
constexpr unsigned int DEBUG = 1 << 7;


inline SimpleScene createLSystemScene1(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) {
	SimpleScene scene;
	{
		auto tree = std::make_shared<LSystem>();
		tree->loadRuleSet("./rules/tree_01.rule");
		tree->setConfig({
				{"step", 0.4}
				});
		tree->runGenerations(7);

		tree->setName("TREE1");
		tree->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		tree->addMaterial(
			"solid",
			MaterialParameters(
				"branches",
				RenderStyle::Solid,
				{
					{"u_solidColor", glm::vec4(0.3, 0.2, 0.0, 1)},
					{"u_shrinkageFactor", 0.7f},
					{"u_initialThickness", 0.2f}
				}
				)
			);
		tree->addMaterial(
			"wireframe",
			MaterialParameters(
				"solid_color",
				RenderStyle::Wireframe,
				{}
				)
			);
		tree->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(tree);
	}
	return scene;
}

inline SimpleScene createLSystemScene2(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) {
	SimpleScene scene;
	{
		auto tree = std::make_shared<LSystem>();
		tree->loadRuleSet("./rules/tree_02.rule");
		tree->setConfig({
				{"angle", 90.0f}
				});
		tree->runGenerations(7);

		tree->setName("TREE1");
		tree->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		tree->addMaterial(
			"solid",
			MaterialParameters(
				"branches",
				RenderStyle::Solid,
				{
					{"u_solidColor", glm::vec4(0.3, 0.2, 0.0, 1)},
					{"u_shrinkageFactor", 0.7f},
					{"u_initialThickness", 0.2f}
				}
				)
			);
		tree->addMaterial(
			"wireframe",
			MaterialParameters(
				"solid_color",
				RenderStyle::Wireframe,
				{}
				)
			);
		tree->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(tree);
	}
	return scene;
}

