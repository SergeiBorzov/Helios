
#include <cstdio>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>     // Post processing flags

#include "globals.h"
#include "scene.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;


namespace Helios {

    void Scene::Create() {
        m_Scene = rtcNewScene(g_Device);
        rtcSetSceneFlags(m_Scene, RTC_SCENE_FLAG_ROBUST);
    }

    bool Scene::Intersect(const RTCRay& ray, RayHitRecord& record) const {
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);

        RTCRayHit rayhit;
        rayhit.ray = ray;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(m_Scene, &context, &rayhit);

        record.geometry_id = rayhit.hit.geomID;

        if (record.geometry_id == RTC_INVALID_GEOMETRY_ID) {
            return false;
        }

        record.distance = rayhit.ray.tfar;
        record.normal = glm::normalize(vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
        record.hit_point = vec3(ray.org_x, ray.org_y, ray.org_z) + 
                           vec3(ray.dir_x, ray.dir_y, ray.dir_z)*record.distance;
        return true;
    }

    Scene* Scene::LoadFromFile(const char* path_to_file) {
        Assimp::Importer importer;
        const aiScene *scene_data = importer.ReadFile(path_to_file, 
                                                      aiProcess_Triangulate | 
                                                      aiProcess_FlipUVs);

        Scene* helios_scene = new Scene();
        helios_scene->Create();

        // Failed to load file
        if (!scene_data) {
            return nullptr;
        }

        // 1) Load cameras
        printf("Found %d cameras\n", scene_data->mNumCameras);
        aiNode* root_node = scene_data->mRootNode;

        for (unsigned int i = 0; i < scene_data->mNumCameras; i++) {
            auto& camera = scene_data->mCameras[i];
            aiNode* camera_node = root_node->FindNode(camera->mName);
            assert(camera_node);

            // Apply all parent transforms, we need world coordinates
            aiMatrix4x4 cam_to_world_ai;
            while (camera_node != root_node) {
                cam_to_world_ai = camera_node->mTransformation*cam_to_world_ai;
                camera_node = camera_node->mParent;
            }

            mat4 cam_to_world;
            cam_to_world[0] = vec4(cam_to_world_ai.a1, cam_to_world_ai.b1, cam_to_world_ai.c1, cam_to_world_ai.d1);
            cam_to_world[1] = vec4(cam_to_world_ai.a2, cam_to_world_ai.b2, cam_to_world_ai.c2, cam_to_world_ai.d2);
            cam_to_world[2] = vec4(cam_to_world_ai.a3, cam_to_world_ai.b3, cam_to_world_ai.c3, cam_to_world_ai.d3);
            cam_to_world[3] = vec4(cam_to_world_ai.a4, cam_to_world_ai.b4, cam_to_world_ai.c4, cam_to_world_ai.d4);

            mat4 view = glm::inverse(cam_to_world);
            view[3] = cam_to_world[3];

            /*
            printf("Cam to World:\n");
            printf("%f %f %f %f\n", cam_to_world[0][0], cam_to_world[0][1], cam_to_world[0][2], cam_to_world[0][3]);
            printf("%f %f %f %f\n", cam_to_world[1][0], cam_to_world[1][1], cam_to_world[1][2], cam_to_world[1][3]);
            printf("%f %f %f %f\n", cam_to_world[2][0], cam_to_world[2][1], cam_to_world[2][2], cam_to_world[2][3]);
            printf("%f %f %f %f\n", cam_to_world[3][0], cam_to_world[3][1], cam_to_world[3][2], cam_to_world[3][3]);

            printf("World to Cam:\n");
            printf("%f %f %f %f\n", view[0][0], view[0][1], view[0][2], view[0][3]);
            printf("%f %f %f %f\n", view[1][0], view[1][1], view[1][2], view[1][3]);
            printf("%f %f %f %f\n", view[2][0], view[2][1], view[2][2], view[2][3]);
            printf("%f %f %f %f\n", view[3][0], view[3][1], view[3][2], view[3][3]);
            */
            
            
            PerspectiveCamera helios_cam(atan(tan(camera->mHorizontalFOV)/camera->mAspect), 
                                         camera->mAspect, 
                                         camera->mClipPlaneNear, 
                                         camera->mClipPlaneFar);
            helios_cam.SetView(view);
            helios_scene->PushCamera(helios_cam);
        }

        // 2) Load lights
        printf("Found %d lights\n", scene_data->mNumLights);

        for (unsigned int i = 0; i < scene_data->mNumLights; i++) {
            auto& light = scene_data->mLights[i];
            aiNode* light_node = root_node->FindNode(light->mName);
            assert(light_node);

            aiMatrix4x4 light_to_world_ai;
            while (light_node != root_node) {
                light_to_world_ai = light_node->mTransformation*light_to_world_ai;
                light_node = light_node->mParent;
            }

            glm::mat3 light_to_world;
            light_to_world[0] = vec3(light_to_world_ai.a1, light_to_world_ai.b1, light_to_world_ai.c1);
            light_to_world[1] = vec3(light_to_world_ai.a2, light_to_world_ai.b2, light_to_world_ai.c2);
            light_to_world[2] = vec3(light_to_world_ai.a3, light_to_world_ai.b3, light_to_world_ai.c3);

            switch(light->mType) {
                case aiLightSource_DIRECTIONAL: {
                    vec3 dir = normalize(light_to_world*vec3(light->mDirection.x, light->mDirection.y, light->mDirection.z));
                    Spectrum intensity = { light->mColorDiffuse.r, light->mColorDiffuse.b, light->mColorDiffuse.b };
                    helios_scene->PushLight(new DirectionalLight(-dir, intensity));
                    break;
                }
                default: {
                    fprintf(stderr, "Warning: Light '%s' was found but it's type is not supported by Helios\n", light->mName.C_Str());
                    break;
                }
            }
        }

        // 3) Load materials
        printf("Found %d materials\n", scene_data->mNumMaterials);

        for (unsigned int i = 0; i < scene_data->mNumMaterials; i++) {
            // Load materials
            auto& material = scene_data->mMaterials[i];

            aiColor4D diffuse (1.0f, 1.0f, 1.0f, 1.0f);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);

            helios_scene->PushMaterial(new Matte({diffuse.r, diffuse.g, diffuse.b}));
        }

        // 4) Load geometry
        printf("Found %d meshes\n", scene_data->mNumMeshes);

        for (unsigned int i = 0; i < scene_data->mNumMeshes; i++) {
            const auto& mesh = scene_data->mMeshes[i];
            aiNode* mesh_node = root_node->FindNode(mesh->mName);

            if (!mesh_node){
                printf("%s wasn't found\n", mesh->mName.C_Str());
                continue;
            }

            // Apply all parent transforms, we need world coordinates
            aiMatrix4x4 local_to_world_ai;
            while (mesh_node != root_node) {
                local_to_world_ai = mesh_node->mTransformation*local_to_world_ai;
                mesh_node = mesh_node->mParent;
            }

            mat4 local_to_world;
            local_to_world[0] = vec4(local_to_world_ai.a1, local_to_world_ai.b1, local_to_world_ai.c1, local_to_world_ai.d1);
            local_to_world[1] = vec4(local_to_world_ai.a2, local_to_world_ai.b2, local_to_world_ai.c2, local_to_world_ai.d2);
            local_to_world[2] = vec4(local_to_world_ai.a3, local_to_world_ai.b3, local_to_world_ai.c3, local_to_world_ai.d3);
            local_to_world[3] = vec4(local_to_world_ai.a4, local_to_world_ai.b4, local_to_world_ai.c4, local_to_world_ai.d4);


            std::vector<float> vertices;
            vertices.resize(3*mesh->mNumVertices);
            // TODO: Instead of multiplying every vertex just set transform in embree
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                auto& vertex = mesh->mVertices[i];
                vec3 v = vec3(local_to_world*vec4(vertex.x, vertex.y, vertex.z, 1.0f));
                vertices[3*i + 0] = v.x;
                vertices[3*i + 1] = v.y;
                vertices[3*i + 2] = v.z;
            }


            std::vector<unsigned int> indices;
            indices.resize(mesh->mNumFaces*3);
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                const auto& face = mesh->mFaces[i];
                assert(face.mNumIndices == 3);
                indices[3*i + 0] = face.mIndices[0];
                indices[3*i + 1] = face.mIndices[1];
                indices[3*i + 2] = face.mIndices[2];
            }

            // Push entity
            helios_scene->PushEntity(CreateTriangleMesh(*helios_scene, std::move(vertices), std::move(indices)), 
                                     mesh->mMaterialIndex);
        }

        rtcCommitScene(helios_scene->GetRTCScene());
        return helios_scene;
    }

    Scene::~Scene() {
        if (m_Scene) {
            rtcReleaseScene(m_Scene);
        }

        for (unsigned int i = 0; i < m_Lights.size(); i++) {
            delete m_Lights[i];
        }
        for (unsigned int i = 0; i < m_Materials.size(); i++) {
            delete m_Materials[i];
        }
    }
}