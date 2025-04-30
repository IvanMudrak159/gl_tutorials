#include "SSAONoiseTexture.h"

SSAONoiseTexture::SSAONoiseTexture(unsigned int noiseSize)
    : noiseSize(noiseSize)
{
    generateNoise();
    createTexture();
}

GLuint SSAONoiseTexture::getTexture() const {
    return noiseTexture;
}

float SSAONoiseTexture::randomFloats(std::default_random_engine& generator) {
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}

void SSAONoiseTexture::generateNoise() {
    std::default_random_engine generator;
    for (unsigned int i = 0; i < noiseSize; i++) {
        glm::vec3 noise(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f 
        );
        noiseVectors.push_back(noise);
    }
}

void SSAONoiseTexture::createTexture() {
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noiseVectors[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
