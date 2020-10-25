
#include <cstdio>
#include <filesystem>

#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include "../Core/Globals.h"
#include "../Core/ColorSpaces.h"
#include "Scene.h"

using namespace glm;
namespace fs = std::filesystem;

namespace Helios {

    void Scene::Create() {
        m_Scene = rtcNewScene(g_Device);
        //rtcSetSceneFlags(....);
    }

    void Scene::AddEntity(const TriangleMesh& mesh, u32 material_id) {
        u32 geometry_id = rtcAttachGeometry(m_Scene, mesh.GetRTCGeometry());
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

        // Note: It is possible to generate tangents if uvs and normals are known
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

    void LoadCamera(const tinygltf::Node& node, const dmat4& node_transform,
                    const tinygltf::Model& model, Helios::Scene& helios_scene) {
        if (node.camera >= 0 && node.camera < model.cameras.size()) {
            const auto& camera = model.cameras[node.camera];

            // Get View matrix
            mat4 view = inverse(node_transform);
            view[3] = node_transform[3];

            if (camera.type == "perspective") {
                const auto& p_cam = camera.perspective;
                std::shared_ptr<PerspectiveCamera> helios_cam = 
                    std::make_shared<PerspectiveCamera>(p_cam.yfov, p_cam.aspectRatio, p_cam.znear, p_cam.zfar);
                helios_cam->SetView(view);
                helios_scene.AddCamera(helios_cam);
            }
            else {
                fprintf(stderr, "Error: Orthographic camera is not supported yet\n");
            }
        }
    }

    void LoadLight(const tinygltf::Node& node, const dmat4& node_transform_ws,
                   const tinygltf::Model& model, Helios::Scene& helios_scene) {
        const auto& light_obj = node.extensions.find("KHR_lights_punctual");
        if (light_obj->second.IsObject()) {
            const auto& light_value = light_obj->second.Get("light");
            if (light_value.IsInt()) {
                const auto& light = model.lights[light_value.GetNumberAsInt()];
                // Note light.intensity is power in Watts
                
                if (light.type == "directional") {
                    // Note: local direction is (0, 0, 1) in case of Blender
                    vec3 dir = normalize(vec3(node_transform_ws*vec4(0.0, 0.0f, 1.0f, 0.0f)));
                    vec3 tmp = make_vec3(light.color.data());
                    Spectrum intensity = {tmp.x, tmp.y, tmp.z};
                    helios_scene.AddLight(std::make_shared<DirectionalLight>(dir, intensity));
                }
                else if (light.type == "point") {
                    vec3 pos = vec3(node_transform_ws[3]);
                    vec3 tmp = make_vec3(light.color.data())*(light.intensity/(4.0f * glm::pi<f32>()));
                    Spectrum intensity = {tmp.x, tmp.y, tmp.z};
                    helios_scene.AddLight(std::make_shared<PointLight>(pos, intensity));
                }
                else if (light.type == "spot") {
                    f32 cos_outer = cos(light.spot.outerConeAngle);
                    f32 cos_inner = cos(light.spot.innerConeAngle);
                    vec3 pos = vec3(node_transform_ws[3]);
                    vec3 dir = normalize(vec3(node_transform_ws*vec4(0.0, 0.0f, 1.0f, 0.0f)));
                    vec3 tmp = make_vec3(light.color.data())*(light.intensity/(2.0f*glm::pi<f32>()*(1.0f - 0.5f * (cos_inner + cos_outer))));
                    Spectrum intensity = {tmp.x, tmp.y, tmp.z};
                    helios_scene.AddLight(std::make_shared<SpotLight>(pos, dir, light.spot.outerConeAngle, light.spot.innerConeAngle, intensity));
                }
                else {
                    fprintf(stderr, "%s type of light is not supported\n", light.type.c_str());
                }
            }
        }
    }

    static void LoadMaterial(const fs::path& path_to_scene, const tinygltf::Primitive& primitive, 
                             const tinygltf::Model& model, Helios::Scene& helios_scene) {
        const auto& material = model.materials[primitive.material];

        // Color info                
        const auto& pbr_info = material.pbrMetallicRoughness;
        vec4 color = make_vec4(pbr_info.baseColorFactor.data());
        Spectrum spectrum = {color.x, color.y, color.z};

        std::shared_ptr<Helios::Texture> helios_albedo = nullptr; 
        if (pbr_info.baseColorTexture.index >= 0 && pbr_info.baseColorTexture.index < model.textures.size()) {
            const auto& albedo = model.images[model.textures[pbr_info.baseColorTexture.index].source];
            
            if (helios_scene.HasTexture(albedo.uri.c_str())) {
                helios_albedo = helios_scene.GetTexture(albedo.uri.c_str());
            }
            else {
                fs::path texture_path = std::filesystem::path(albedo.uri.c_str());

                // Create absolute path
                if (texture_path.is_relative()) {
                    texture_path = path_to_scene / texture_path;
                }
                helios_albedo = Helios::Texture::LoadFromFile(texture_path.u8string().c_str(), Helios::Texture::ColorSpace::sRGB);
                if (helios_albedo) {
                    helios_scene.AddTexture(albedo.uri.c_str(), helios_albedo);
                }
                else {
                    fprintf(stderr, "Failed to load texture %s\n", texture_path.u8string().c_str());
                }
            }
        }

        // Bump map
        // if (material.normalTexture...)

        std::shared_ptr<Helios::Texture> helios_bump = nullptr; 
        if (material.normalTexture.index >= 0) {
            const auto& bump = model.images[model.textures[material.normalTexture.index].source];
            
            if (helios_scene.HasTexture(bump.uri.c_str())) {
                helios_bump = helios_scene.GetTexture(bump.uri.c_str());
            }
            else {
                fs::path texture_path = std::filesystem::path(bump.uri.c_str());

                // Create absolute path
                if (texture_path.is_relative()) {
                    texture_path = path_to_scene / texture_path;
                }
                helios_bump = Helios::Texture::LoadFromFile(texture_path.u8string().c_str(), Helios::Texture::ColorSpace::Linear);
                if (helios_bump) {
                    helios_scene.AddTexture(bump.uri.c_str(), helios_bump);
                }
                else {
                    fprintf(stderr, "Failed to load texture %s\n", texture_path.u8string().c_str());
                }
            }
        }
        helios_scene.AddMaterial(primitive.material, 
                                 std::make_shared<Matte>(spectrum, helios_albedo, helios_bump));
    }

    static void LoadMesh(const fs::path& path_to_scene,
                         const tinygltf::Node& node, const dmat4& node_transform_ws, 
                         const tinygltf::Model& model, Helios::Scene& helios_scene) {
        if (node.mesh >= 0 && node.mesh < model.meshes.size()) {        
            const auto& mesh = model.meshes[node.mesh];

            std::vector<vec3> vertices;
            std::vector<vec3> normals;
            std::vector<vec3> tangents;
            std::vector<vec2> uvs;
            std::vector<u32> indices;
           
            for (const auto& primitive: mesh.primitives) {
                LoadMaterial(path_to_scene, primitive, model, helios_scene);

                for (auto &attrib : primitive.attributes) {
                    const auto& accessor = model.accessors[attrib.second];
                    const auto& buffer_view = model.bufferViews[accessor.bufferView];
                    const auto& buffer = model.buffers[buffer_view.buffer];

                    int byte_stride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                    int byte_offset = buffer_view.byteOffset;
                    int byte_length = buffer_view.byteLength;

                    if (attrib.first == "POSITION") {
                        const vec3* start = (const vec3*)(&buffer.data.at(0) + byte_offset);
                        vertices = std::vector<vec3>(start, start + byte_length/sizeof(vec3));

                        for (auto& vertex: vertices) {
                            vertex = vec3(node_transform_ws*vec4(vertex, 1.0f));
                        }
                    }
                    else if (attrib.first == "NORMAL") {
                        const vec3* start = (const vec3*)(&buffer.data.at(0) + byte_offset);
                        normals = std::vector<vec3>(start, start + byte_length/sizeof(vec3));

                        for (auto& normal: normals) {
                            normal = normalize(vec3(node_transform_ws*vec4(normal, 0.0f)));
                        }
                    }
                    else if (attrib.first == "TANGENT") {
                        const vec3* start = (const vec3*)(&buffer.data.at(0) + byte_offset);
                        tangents = std::vector<vec3>(start, start + byte_length/sizeof(vec3));

                        for (auto& tangent: tangents) {
                            tangent = normalize(vec3(node_transform_ws*vec4(tangent, 0.0f)));
                        }
                    }
                    else if (attrib.first == "TEXCOORD_0") {
                        const vec2* start = (const vec2*)(&buffer.data.at(0) + byte_offset);
                        uvs = std::vector<vec2>(start, start + byte_length/sizeof(vec2));

                        //for (auto& uv: uvs) {
                            //uv = scale*uv;
                        //}
                    }
                }

                // Indices
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                    fprintf(stderr, "Warning: %s not loaded. Helios doesn't support non-triangle meshes yet\n", mesh.name.c_str());
                    return;
                }
                const auto& accessor = model.accessors[primitive.indices];
                const auto& buffer_view = model.bufferViews[primitive.indices];
                const auto& buffer = model.buffers[buffer_view.buffer];

                int byte_stride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                int byte_offset = buffer_view.byteOffset;
                int byte_length = buffer_view.byteLength;

                indices.resize(accessor.count);
                switch(accessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                        const u8* start = (const u8*)(&buffer.data.at(0) + byte_offset);
                        std::vector<u8> tmp(start, start + byte_length/sizeof(u8));
                        for (u32 i = 0; i < indices.size(); i++) {
                            indices[i] = tmp[i];
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                        const u16* start = (const u16*)(&buffer.data.at(0) + byte_offset);
                        std::vector<u16> tmp(start, start + byte_length/sizeof(u16));
                        for (u32 i = 0; i < indices.size(); i++) {
                            indices[i] = tmp[i];
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                        const u32* start = (const u32*)(&buffer.data.at(0) + byte_offset);
                        indices = std::vector<u32>(start, start + byte_length/sizeof(u32));
                        break;
                    }
                    default: {
                        assert(false);
                    }
                }                 

                TriangleMesh helios_mesh;
                helios_mesh.Create(std::move(vertices), std::move(indices), 
                                   std::move(normals), std::move(uvs),
                                   std::move(tangents));

                helios_scene.AddEntity(helios_mesh, primitive.material);
            }
        }
    }

    static void LoadNode(const fs::path& path_to_scene, const tinygltf::Node& node,
                         const dmat4& node_transform_ws, const tinygltf::Model& model, 
                         Helios::Scene& helios_scene) {
        LoadCamera(node, node_transform_ws, model, helios_scene);
        LoadLight(node, node_transform_ws, model, helios_scene);
        LoadMesh(path_to_scene, node, node_transform_ws, model, helios_scene);
        printf("Info: Node %s loaded\n", node.name.c_str());
    }

    static void LoadScene(const fs::path& path_to_scene, const tinygltf::Scene& scene, const tinygltf::Model& model, Helios::Scene& helios_scene) {
        // Start with root nodes
        printf("Info: Loading scene...\n");
        for (u32 i = 0; i < scene.nodes.size(); i++) {
            dmat4 node_transform_ws = dmat4(1.0);
            const auto& node = model.nodes[scene.nodes[i]];

            if (node.scale.size() == 3) {
                node_transform_ws = scale(dmat4(1.0), make_vec3(node.scale.data()));
            }
            if (node.rotation.size() == 4) {
                node_transform_ws = toMat4(make_quat(node.rotation.data()))*node_transform_ws;
            }
            if (node.translation.size() == 3) {
                node_transform_ws = translate(dmat4(1.0f), make_vec3(node.translation.data()))*node_transform_ws;
            }

            LoadNode(path_to_scene, node, node_transform_ws, model, helios_scene);

            for (u32 j = 0; j < node.children.size(); j++) {
                const auto& child = model.nodes[node.children[j]];

                dmat4 child_transform_ws = dmat4(1.0);
                if (child.scale.size() == 3) {
                    child_transform_ws = scale(dmat4(1.0), make_vec3(child.scale.data()));
                }
                if (child.rotation.size() == 4) {
                    child_transform_ws = toMat4(make_quat(child.rotation.data()))*child_transform_ws;
                }
                if (child.translation.size() == 3) {
                    child_transform_ws = translate(dmat4(1.0f), make_vec3(child.translation.data()))*child_transform_ws;
                }
                child_transform_ws = node_transform_ws*child_transform_ws;

                LoadNode(path_to_scene, child, child_transform_ws, model, helios_scene);
            }
        }
        printf("Info: Scene Loaded\n");
    }

    Scene* Scene::LoadFromFile(const char* path_to_file, int width, int height) {
        tinygltf::TinyGLTF importer;
        tinygltf::Model model;

        std::string err;
        std::string warn;

        bool success = importer.LoadASCIIFromFile(&model, &err, &warn, path_to_file);
        if (!warn.empty()) {
            printf("%s\n", warn.c_str());
        }
        if (!err.empty()) {
            printf("%s\n", err.c_str());
        }
        if (!success) {
            return nullptr;
        }

        // Store path to scene file
        fs::path path_to_scene = std::filesystem::path(path_to_file).parent_path();

        // Check if we have a valid scene
        if (model.defaultScene == -1) {
            return nullptr;
        }

        const auto& scene = model.scenes[model.defaultScene];
        Scene* helios_scene = new Scene();
        helios_scene->Create();
        LoadScene(path_to_scene, scene, model, *helios_scene);

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