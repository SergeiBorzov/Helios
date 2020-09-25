
#include <cstdio>

#include <Eigen/Dense>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>     // Post processing flags

#include "globals.h"
#include "scene.h"

using Eigen::Vector3f;

namespace Helios {

    void Scene::Create() {
        m_Scene = rtcNewScene(g_Device);
    }

    Scene* Scene::LoadFromFile(const char* path_to_file) {
        Assimp::Importer importer;
        const aiScene *scene_data = importer.ReadFile(path_to_file, aiProcess_Triangulate | aiProcess_FlipUVs);

        Scene* helios_scene = new Scene();
        helios_scene->Create();

        // Failed to load file
        if (!scene_data) {
            return nullptr;
        }

        // 1) Load cameras
        for (unsigned int i = 0; i < scene_data->mNumCameras; i++) {
            auto& camera = scene_data->mCameras[i];
            helios_scene->PushCamera(PerspectiveCamera(
                                        camera->mHorizontalFOV, 
                                        camera->mAspect, 
                                        camera->mClipPlaneNear, 
                                        camera->mClipPlaneFar)
                                    );
            printf("Camera fov: %f\n", camera->mHorizontalFOV);
            printf("Camera aspect: %f\n", camera->mAspect);
        }

        // 2) Load lights
        for (unsigned int i = 0; i < scene_data->mNumLights; i++) {
            auto& light = scene_data->mLights[i];
            
            switch(light->mType) {
                case aiLightSource_DIRECTIONAL: {
                    Vector3f dir = Vector3f(light->mDirection.x, light->mDirection.y, light->mDirection.z);
                    dir.normalize();
                    Spectrum intensity = { light->mColorDiffuse.r, light->mColorDiffuse.b, light->mColorDiffuse.b };
                    helios_scene->PushLight(DirectionalLight(dir, intensity));
                    break;
                }
                default: {
                    fprintf(stderr, "Warning: Light '%s' was found but it's type is not supported by Helios\n", light->mName.C_Str());
                    break;
                }
            }
        }

        // 3) Load geometry
        printf("Found %d meshes\n", scene_data->mNumMeshes);

        for (unsigned int i = 0; i < scene_data->mNumMeshes; i++) {
            const auto& mesh = scene_data->mMeshes[i];
            Helios::Mesh helios_mesh;
            helios_scene->PushGeometryID(helios_mesh.CreateAndAttach(*helios_scene, mesh));
        }

        return helios_scene;
    }

    Scene::~Scene() {
        if (m_Scene) {
            rtcReleaseScene(m_Scene);
        }
    }
}