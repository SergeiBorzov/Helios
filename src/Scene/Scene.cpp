
#include <cstdio>
#include <filesystem>

#include <glm/gtc/constants.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Core/Globals.h"
#include "../Core/ColorSpaces.h"
#include "Scene.h"

using namespace glm;


static std::shared_ptr<Helios::Texture> 
LoadTextureToScene(Helios::Scene& helios_scene, const std::filesystem::path& path_to_scene,
                   const aiScene& scene_data, const aiMaterial& material, aiTextureType texture_type) {
    std::shared_ptr<Helios::Texture> helios_texture = nullptr; // result

    aiString texture_path_ai;
    if (material.Get(AI_MATKEY_TEXTURE(texture_type, 0), texture_path_ai) == AI_SUCCESS) {
        // Check the cache first
        if (helios_scene.HasTexture(texture_path_ai.C_Str())) {
            helios_texture = helios_scene.GetTexture(texture_path_ai.C_Str());
        } else {
            if (auto texture = scene_data.GetEmbeddedTexture(texture_path_ai.C_Str())) {
                    // Note: Returned pointer is not null, texture is embedded we now have texture data in 'texture'
                    // TODO:
                    assert(false);
            } else {
                // Note: Texture is not embedded, regular file work, check if it exists and read
                std::filesystem::path texture_path = std::filesystem::path(texture_path_ai.C_Str());
                // Create absolute path
                if (texture_path.is_relative()) {
                    texture_path = path_to_scene / texture_path;
                }

                helios_texture = std::make_shared<Helios::Texture>();

                bool load_succesful = false;
                switch(texture_type) {
                    case aiTextureType_DIFFUSE: {
                        load_succesful = helios_texture->LoadFromFile(texture_path.u8string().c_str(), 
                                                                      Helios::Texture::ColorSpace::sRGB);
                        break;
                    }
                    case aiTextureType_NORMALS: {
                        load_succesful = helios_texture->LoadFromFile(texture_path.u8string().c_str(), 
                                                                      Helios::Texture::ColorSpace::Linear);
                        break;
                    }
                    default:
                        break;
                }

                if (load_succesful) {
                    helios_scene.AddTexture(texture_path_ai.C_Str(), helios_texture);
                    printf("Info: Loaded texture %s\n", texture_path_ai.C_Str());
                } else {
                    // Some error during texture loading
                    helios_texture = nullptr;
                    fprintf(stderr, "Warning: Couldn't load texture %s\n", texture_path.u8string().c_str());
                }
            }
        }
    }   
    return helios_texture;
}


namespace Helios {

    void Scene::Create() {
        m_Scene = rtcNewScene(g_Device);
        //rtcSetSceneFlags(....);
    }

    void Scene::AddEntity(const TriangleMesh& mesh, unsigned int material_id) {
        unsigned int geometry_id = rtcAttachGeometry(m_Scene, mesh.GetRTCGeometry());
        m_TriangleMeshes.insert({geometry_id, mesh});
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
        record.shading_normal = record.normal;

        auto& hitted_geometry = m_TriangleMeshes.find(rayhit.hit.geomID)->second;

        if (hitted_geometry.HasNormals()) {
            vec3 shading_normal;
            rtcInterpolate0(hitted_geometry.GetRTCGeometry(), rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &shading_normal.x, 3);
            record.shading_normal = normalize(shading_normal);
        }

        if (hitted_geometry.HasUVs()) {
            vec2 uv;
            rtcInterpolate0(hitted_geometry.GetRTCGeometry(), rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &uv.x, 2);
            record.uv = uv;
        }

        // Note: Assimp will generate tangents for us if we have uvs and normals
        if (hitted_geometry.HasNormals() && hitted_geometry.HasUVs()) {
            vec3 tangent;
            rtcInterpolate0(hitted_geometry.GetRTCGeometry(), rayhit.hit.primID, rayhit.hit.u, rayhit.hit.v,
                            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, &tangent.x, 3);
            record.tangent = normalize(tangent);
        }
       
        record.hit_point = vec3(ray.org_x, ray.org_y, ray.org_z) + 
                           normalize(vec3(ray.dir_x, ray.dir_y, ray.dir_z))*record.distance +
                           record.normal*0.01f;
        return true;
    }

    bool Scene::Occluded(RTCRay& ray) const {
        RTCIntersectContext context;
	    rtcInitIntersectContext(&context);

	    rtcOccluded1(m_Scene, &context, &ray);

        return ray.tfar < 0.0f;
    }

    Scene* Scene::LoadFromFile(const char* path_to_file, int width, int height) {
        Assimp::Importer importer;
        const aiScene *scene_data = importer.ReadFile(path_to_file, 
                                                      aiProcess_Triangulate |
                                                      aiProcess_CalcTangentSpace 
                                                     );
        // Failed to load file
        if (!scene_data) {
            return nullptr;
        }

        std::filesystem::path path_to_scene = std::filesystem::path(path_to_file).parent_path();

        Scene* helios_scene = new Scene();
        helios_scene->Create();        

        aiNode* root_node = scene_data->mRootNode;

        // 1) Load cameras
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
            
            float aspect = static_cast<float>(width)/height;
            PerspectiveCamera helios_cam(atan(tan(camera->mHorizontalFOV)/aspect), 
                                         aspect, 
                                         camera->mClipPlaneNear, 
                                         camera->mClipPlaneFar);
            helios_cam.SetView(view);
            helios_scene->AddCamera(helios_cam);
        }
        printf("Info: Loaded %u cameras\n", scene_data->mNumCameras);

        // 2) Load lights
        for (unsigned int i = 0; i < scene_data->mNumLights; i++) {
            auto& light = scene_data->mLights[i];
            aiNode* light_node = root_node->FindNode(light->mName);
            assert(light_node);

            aiMatrix4x4 light_to_world_ai;
            while (light_node != root_node) {
                light_to_world_ai = light_node->mTransformation*light_to_world_ai;
                light_node = light_node->mParent;
            }

            mat4 light_to_world;
            light_to_world[0] = vec4(light_to_world_ai.a1, light_to_world_ai.b1, light_to_world_ai.c1, 0.0f);
            light_to_world[1] = vec4(light_to_world_ai.a2, light_to_world_ai.b2, light_to_world_ai.c2, 0.0f);
            light_to_world[2] = vec4(light_to_world_ai.a3, light_to_world_ai.b3, light_to_world_ai.c3, 0.0f);
            light_to_world[3] = vec4(light_to_world_ai.a4, light_to_world_ai.b4, light_to_world_ai.c4, 1.0f);

            
            switch(light->mType) {
                case aiLightSource_DIRECTIONAL: {
                    glm::vec3 tmp = vec3(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b);
                    Spectrum intensity = { tmp.r, tmp.g, tmp.b }; // Something is wrong here, result differs from Cycles
                    vec3 dir = normalize(mat3(light_to_world)*vec3(light->mDirection.x, light->mDirection.y, light->mDirection.z));
                    helios_scene->AddLight(std::make_shared<DirectionalLight>(-dir, intensity));
                    break;
                }
                case aiLightSource_POINT: {
                    glm::vec3 tmp = vec3(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b)/(4.0f * glm::pi<float>());
                    Spectrum intensity = { tmp.r, tmp.g, tmp.b };
                    vec3 position = vec3(light_to_world[3].x, light_to_world[3].y, light_to_world[3].z);
                    helios_scene->AddLight(std::make_shared<PointLight>(position, intensity));
                    break;
                }
                case aiLightSource_SPOT: {
                    // Note: Assimp's bug with spot light angle
                    // light->mAngleOuterCone is always 45 degrees

                    glm::vec3 tmp = vec3(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b);
                    float cos_falloff = glm::cos(light->mAngleInnerCone);
                    tmp = tmp/(2.0f*glm::pi<float>()*(1 - .5f * cos_falloff));
                    Spectrum intensity = { tmp.r, tmp.g, tmp.b };
                    vec3 dir = normalize(mat3(light_to_world)*vec3(light->mDirection.x, light->mDirection.y, light->mDirection.z));
                    vec3 position = vec3(light_to_world[3].x, light_to_world[3].y, light_to_world[3].z);
                    helios_scene->AddLight(std::make_shared<SpotLight>(position, -dir, light->mAngleInnerCone, intensity));
                    break;
                }
                default: {
                    fprintf(stderr, "Warning: Light '%s' was found but it's type is not supported by Helios\n", light->mName.C_Str());
                    break;
                }
            }
        }
        printf("Info: Loaded %u lights\n", scene_data->mNumLights);


        // 3) Load materials
        for (unsigned int i = 0; i < scene_data->mNumMaterials; i++) {
            // Load materials
            auto& material = scene_data->mMaterials[i];

            // Get diffuse color:
            aiColor4D diffuse (1.0f, 1.0f, 1.0f, 1.0f);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);

            // Get diffuse texture:
            std::shared_ptr<Texture> helios_diffuse_texture = 
                LoadTextureToScene(*helios_scene, path_to_scene, *scene_data, *material, aiTextureType_DIFFUSE);
            
            // Get normal texture:
            std::shared_ptr<Texture> helios_normal_map = 
                LoadTextureToScene(*helios_scene, path_to_scene, *scene_data, *material, aiTextureType_NORMALS);

            helios_scene->AddMaterial(std::make_shared<Matte>(Spectrum(diffuse.r, diffuse.g, diffuse.b), 
                                                              helios_diffuse_texture, 
                                                              helios_normal_map));
        }
        printf("Info: Loaded %u materials\n", scene_data->mNumMaterials);


        // 4) Load geometry
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

            // Create tangent buffer
            std::vector<vec3> tangents;
            if (mesh->HasTangentsAndBitangents()) {
                tangents.resize(mesh->mNumVertices);
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    auto& tangent = mesh->mTangents[i];
                    tangents[i] = vec3(tangent.x, tangent.y, tangent.z);
                }
            }
           
            // Add entity
            TriangleMesh helios_mesh;
            helios_mesh.Create(std::move(vertices), std::move(indices), 
                               std::move(normals), std::move(uvs),
                               std::move(tangents));

            helios_scene->AddEntity(helios_mesh, mesh->mMaterialIndex);
        }
        printf("Info: Loaded %u meshes\n", scene_data->mNumMeshes);


        rtcCommitScene(helios_scene->GetRTCScene());
        return helios_scene;
    }

    Scene::~Scene() {
        if (m_Scene) {
            rtcReleaseScene(m_Scene);
        }

        for (auto& item: m_TriangleMeshes) {
            item.second.Release();
        }
    }
}