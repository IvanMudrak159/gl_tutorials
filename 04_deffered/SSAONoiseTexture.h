#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <random>
#include <glad/glad.h>

class SSAONoiseTexture {
public:
    SSAONoiseTexture(unsigned int noiseSize = 16);
    GLuint getTexture() const;

private:
    unsigned int noiseSize;
    GLuint noiseTexture;
    std::vector<glm::vec3> noiseVectors;

    float randomFloats(std::default_random_engine& generator);
    void generateNoise();
    void createTexture();
};
