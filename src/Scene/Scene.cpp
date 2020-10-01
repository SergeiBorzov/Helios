
#include <cstdio>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>     // Post processing flags

#include "../Core/Globals.h"
#include "Scene.h"

using namespace glm;

namespace Helios {

    void Scene::Create() {
        m_Scene = rtcNewScene(g_Device);
        //rtcSetSceneFlags(m_Scene, RTC_SCENE_FLAG_ROBUST);
    }

    void Scene::PushEntity(const TriangleMesh& mesh, unsigned int material_id) {
        unsigned int geometry_id = rtcAttachGeometry(m_Scene, mesh.GetRTCGeometry());
        m_TriangleMeshes.push_back(mesh);
        m_MaterialMap.insert({geometry_id, material_id});
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
        record.primitive_id = rayhit.hit.primID;

        if (record.geometry_id == RTC_INVALID_GEOMETRY_ID) {
            return false;
        }

        record.distance = rayhit.ray.tfar;
        record.normal = normalize(vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
       
        vec3 shading_normal;

        rtcInterpolate0(rtcGetGeometry(m_Scene, rayhit.hit.geomID), rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &shading_normal.x, 3);
        record.shading_normal = normalize(shading_normal);

        record.hit_point = vec3(ray.org_x, ray.org_y, ray.org_z) + 
                           vec3(ray.dir_x, ray.dir_y, ray.dir_z)*record.distance +
                           shading_normal*0.01f;
        return true;
    }

    bool Scene::Occluded(RTCRay& ray) const {
        RTCIntersectContext context;
	    rtcInitIntersectContext(&context);

	    rtcOccluded1(m_Scene, &context, &ray);

        return ray.tfar < 0.0f;
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

            mat4 view = inverse(cam_to_world);
            view[3] = cam_to_world[3];
            
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

            mat3 light_to_world;
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

            // Get diffuse color:
            aiColor4D diffuse (1.0f, 1.0f, 1.0f, 1.0f);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);

            // Get diffuse texture:
            aiString texture_path_ai;
            if (material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path_ai) == AI_SUCCESS) {
                printf("Texture Path: %s\n", texture_path_ai.C_Str());
            }

            if (auto texture = scene_data->GetEmbeddedTexture(texture_path_ai.C_Str())) {
                // Note: Returned pointer is not null, texture is embedded we now have texture data in 'texture'
                // TODO
                assert(false);
            } else {
                // Note: Regular file, check if it exists and read
                std::filesystem::path texture_path = std::filesystem::path(texture_path_ai.C_Str());

                if (texture_path.is_relative()) {
                    // texture_path = path_from_launch + relative_path
                }

                const char* texture_path_c = texture_path.u8string().c_str();
                std::shared_ptr<Texture> helios_texture = std::make_shared<Texture>();
                if (helios_texture->LoadFromFile(texture_path_c)) {
                    helios_scene->PushTexture(texture_path_c, helios_texture);
                } else {
                    // Warning texture wasn't loaded
                    printf("Warning: Couldn't load texture '%s'\n", texture_path_c);
                }

                //regular file, check if it exists and read it
            }
            /*if (material->GetTexture(
                    aiTextureType_DIFFUSE, 0, 
                    &path_to_texture, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
            }*/

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

            // Create vertex buffer
            std::vector<vec3> vertices;
            vertices.resize(mesh->mNumVertices);
            // TODO: Instead of multiplying every vertex just set transform in embree
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                auto& vertex = mesh->mVertices[i];
                vertices[i] = vec3(local_to_world*vec4(vertex.x, vertex.y, vertex.z, 1.0f));
            }

            // Create index buffer
            std::vector<unsigned int> indices;
            indices.resize(mesh->mNumFaces*3);
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                const auto& face = mesh->mFaces[i];
                assert(face.mNumIndices == 3);
                indices[3*i + 0] = face.mIndices[0];
                indices[3*i + 1] = face.mIndices[1];
                indices[3*i + 2] = face.mIndices[2];
            }

            // Create normal buffer
            std::vector<vec3> normals;
            if (mesh->HasNormals()) {
                normals.resize(mesh->mNumVertices);
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    auto& normal = mesh->mNormals[i];
                    normals[i] = normalize((local_to_world*vec4(normal.x, normal.y, normal.z, 0.0f)));
                }
            }

            // Create uvs buffer
            std::vector<vec2> uvs;

            if (mesh->HasTextureCoords(0)) {
                // For now load only one set of basic texture coordinates
                assert(mesh->mNumUVComponents[0] == 2);
                uvs.resize(mesh->mNumVertices);
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    auto& uv = mesh->mTextureCoords[0][i];
                    uvs[i] = vec2(uv.x, uv.y);
                }
            }
           
            // Push entity
            TriangleMesh helios_mesh;
            helios_mesh.Create(std::move(vertices), std::move(indices), 
                               std::move(normals), std::move(uvs));

            helios_scene->PushEntity(helios_mesh, mesh->mMaterialIndex);
        }

        rtcCommitScene(helios_scene->GetRTCScene());
        return helios_scene;
    }

    Scene::~Scene() {
        if (m_Scene) {
            rtcReleaseScene(m_Scene);
        }

        for (unsigned int i = 0; i < m_TriangleMeshes.size(); i++) {
            m_TriangleMeshes[i].Release();
        }

        for (unsigned int i = 0; i < m_Lights.size(); i++) {
            delete m_Lights[i];
        }

        for (unsigned int i = 0; i < m_Materials.size(); i++) {
            delete m_Materials[i];
        }
    }
}