#pragma once

#include <memory>
#include <vector>
#include <ranges>

#include "scene_object.hpp"
#include "cube.hpp"

#include "material_factory.hpp"
#include "geometry_factory.hpp"
#include "simple_scene.hpp"

inline SimpleScene createCottageScene(MaterialFactory& aMaterialFactory, GeometryFactory& aGeometryFactory) {
	SimpleScene scene;
	{
		auto mask = std::make_shared<LoadedMeshObject>("./data/geometry/buddha.obj");
		mask->addMaterial(
			"solid",
			MaterialParameters(
				"material_deffered",
				RenderStyle::Solid,
				{
					{ "u_diffuseTexture", TextureInfo("buddha.png") },
				}
				)
		);
		/*
		auto mask = std::make_shared<LoadedMeshObject>("./data/geometry/mask.obj");
		mask->addMaterial(
			"solid",
			MaterialParameters(
				"solid_color",
				RenderStyle::Solid,
				{
					{ "u_solidColor", glm::vec4(0.0, 0.0, 1.0, 1.0)}
				})
		);
		*/
		mask->setScale(glm::vec3(100, 100, 100));
		mask->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		mask->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(mask);
	}
	return scene;
}
