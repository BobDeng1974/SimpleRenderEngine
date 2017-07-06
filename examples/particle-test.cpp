#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/Material.hpp"
#include "sre/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <sre/SDLRenderer.hpp>
#include "sre/impl/GL.hpp"
#include "sre/imgui_sre.hpp"
#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif
using namespace sre;

class ParticlesExample {
public:
    ParticlesExample(){
        r.init();
        camera = new Camera();
        camera->lookAt({0,0,3},{0,0,0},{0,1,0});
        camera->setPerspectiveProjection(60,0.1,100);
        defaultMat  = Shader::getStandard()->createMaterial();
        particleMat = Shader::getStandardParticles()->createMaterial();
        defaultMat->setSpecularity(20.0f);
        particleMat->setTexture(Texture::getSphereTexture());

        particleMesh = createParticles();
        mesh = Mesh::create()
                .withCube().build();
        worldLights = new WorldLights();
        worldLights->addLight(Light::create().withPointLight({ 0, 1,0}).withColor({1,0,0}).withRange(2).build());
        worldLights->addLight(Light::create().withPointLight({ 1, 0,0}).withColor({0,1,0}).withRange(2).build());
        worldLights->addLight(Light::create().withPointLight({ 0,-1,0}).withColor({0,0,1}).withRange(2).build());
        worldLights->addLight(Light::create().withPointLight({-1, 0,0}).withColor({1,1,1}).withRange(2).build());

        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.frameRender = [&](Renderer* r){
            render(r);
        };

        r.startEventLoop();
    }

    void update(float deltaTime){
        int w,h;
        SDL_GetWindowSize(r.getSDLWindow(),&w,&h);
        camera->setViewport(0,0,w,h);
        float aspect = w/(float)h;
        if (ortho){
            camera->setOrthographicProjection(-2*aspect,2*aspect,-2,2,-2,100);
        } else {
            camera->setPerspectiveProjection(60,0.1,100);
        }
        i += deltaTime;
    }

    void render(Renderer* re){
        auto rp = re->createRenderPass()
                .withCamera(*camera)
                .withWorldLights(worldLights)
                .withClearColor(true,{0,0,0.3,1})
                .build();

        rp.draw(mesh, glm::eulerAngleY(-i)*glm::scale(glm::mat4(1),{0.3f,0.3f,0.3f}), defaultMat);
        rp.draw(particleMesh, glm::eulerAngleY(i), particleMat);

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            ImGui::Text("Particle sprite");
            ImGui::Checkbox("Orthographic proj",&ortho);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            auto& renderStats = re->getRenderStats();

            float bytesToMB = 1.0f/(1024*1024);
            ImGui::Text("sre draw-calls %i meshes %i (%.2fMB) textures %i (%.2fMB) shaders %i", renderStats.drawCalls,renderStats.meshCount, renderStats.meshBytes*bytesToMB, renderStats.textureCount, renderStats.textureBytes*bytesToMB, renderStats.shaderCount);
        }
    }

    std::shared_ptr<Mesh> createParticles(int size = 2500){
        std::vector<glm::vec3> positions;
        std::vector<glm::vec4> colors;
        std::vector<float> sizes;
        std::vector<glm::vec4> uvs;
        for (int i=0;i<size;i++){
            positions.push_back(glm::linearRand(glm::vec3(-1,-1,-1),glm::vec3(1,1,1)));
            colors.push_back(glm::linearRand(glm::vec4(0,0,0,0),glm::vec4(1,1,1,1)));
            sizes.push_back(glm::linearRand(1.0f,20.0f));
            uvs.push_back({0,0,1,1});
        }

        auto particleMesh = Mesh::create ()
                .withPositions(positions)
                .withColors(colors)
                .withParticleSizes(sizes)
                .withUVs(uvs)
                .withMeshTopology(MeshTopology::Points)
                .build();

        return particleMesh;
    }
private:
    SDLRenderer r;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Mesh> particleMesh;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> defaultMat;
    std::shared_ptr<Material> particleMat;
    Camera* camera;
    WorldLights* worldLights;
    float i=0;
    bool ortho = false;
};

int main() {
    new ParticlesExample();
}