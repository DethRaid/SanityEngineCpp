#pragma once

#include <chrono>
#include <queue>

#include "core/async/synchronized_resource.hpp"
#include "renderer/camera_matrix_buffer.hpp"
#include "renderer/handles.hpp"
#include "renderer/render_components.hpp"
#include "renderer/renderpasses/denoiser_pass.hpp"
#include "renderer/renderpasses/forward_pass.hpp"
#include "renderer/standard_material.hpp"
#include "rhi/bind_group.hpp"
#include "rhi/mesh_data_store.hpp"
#include "rhi/raytracing_structs.hpp"
#include "rhi/render_pipeline_state.hpp"
#include "rx/core/ptr.h"
#include "rx/core/vector.h"
#include "settings.hpp"

namespace renderer {
    class RenderCommandList;
    class RenderDevice;
} // namespace renderer

struct GLFWwindow;

namespace renderer {
    /*!
     * \brief All the information needed to decide whether or not to issue a drawcall for an object
     */
    struct VisibleObjectCullingInformation {
        /*!
         * \brief Min and max of this object's bounding box, along the x axis
         */
        Vec2f aabb_x_min_max{};

        /*!
         * \brief Min and max of this object's bounding box, along the y axis
         */
        Vec2f aabb_y_min_max{};

        /*!
         * \brief Min and max of this object's bounding box, along the z axis
         */
        Vec2f aabb_z_min_max{};

        Mat4x4f model_matrix{};

        Uint32 vertex_count{};

        Uint32 start_vertex_location{};
    };

    /*!
     * \brief Data that remains constant for the entire frame
     */
    struct PerFrameData {
        /*!
         * \brief Number of seconds since the program started
         */
        Float32 time_since_start{0};
    };

    /*!
     * \brief Renderer class that uses a clustered forward lighting algorithm
     *
     * It won't actually do that for a while, but having a strong name is very useful
     */
    class Renderer {
    public:
        explicit Renderer(GLFWwindow* window, const Settings& settings_in);

        void begin_frame(uint64_t frame_count);

        void render_all(SynchronizedResourceAccessor<entt::registry>& registry, const World& world);

        void end_frame() const;

        void add_raytracing_objects_to_scene(const Rx::Vector<RaytracingObject>& new_objects);

        TextureHandle create_image(const ImageCreateInfo& create_info);

        [[nodiscard]] TextureHandle create_image(const ImageCreateInfo& create_info,
                                                 const void* image_data,
                                                 const com_ptr<ID3D12GraphicsCommandList4>& commands);

        [[nodiscard]] Rx::Optional<TextureHandle> get_image_handle(const Rx::String& name);

        [[nodiscard]] Image& get_image(const Rx::String& image_name) const;

        [[nodiscard]] Image& get_image(TextureHandle handle) const;

        void schedule_texture_destruction(const TextureHandle& image_handle);

        [[nodiscard]] StandardMaterialHandle allocate_standard_material(const StandardMaterial& material);

        [[nodiscard]] Buffer& get_standard_material_buffer_for_frame(Uint32 frame_idx) const;

        void deallocate_standard_material(StandardMaterialHandle handle);

        [[nodiscard]] RenderDevice& get_render_device() const;

        [[nodiscard]] MeshDataStore& get_static_mesh_store() const;

        void begin_device_capture() const;

        void end_device_capture() const;

        [[nodiscard]] TextureHandle get_noise_texture() const;

        [[nodiscard]] TextureHandle get_pink_texture() const;

        [[nodiscard]] TextureHandle get_default_normal_roughness_texture() const;

        [[nodiscard]] TextureHandle get_default_specular_color_emission_texture() const;

        [[nodiscard]] RaytracableGeometryHandle create_raytracing_geometry(const Buffer& vertex_buffer,
                                                                           const Buffer& index_buffer,
                                                                           const Rx::Vector<Mesh>& meshes,
                                                                           ID3D12GraphicsCommandList4* commands);

        [[nodiscard]] Rx::Ptr<BindGroup> bind_global_resources_for_frame(Uint32 frame_idx);

        [[nodiscard]] Buffer& get_model_matrix_for_frame(Uint32 frame_idx);
        Uint32 add_model_matrix_to_frame(const TransformComponent& transform, const Uint32 frame_idx);

    private:
#pragma region Cube
        Rx::Vector<StandardVertex> cube_vertices = Rx::Array{
            // Front
            StandardVertex{.position = {-0.5f, 0.5f, 0.5f}, .normal = {0, 0, 1}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, -0.5f, 0.5f}, .normal = {0, 0, 1}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, -0.5f, 0.5f}, .normal = {0, 0, 1}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, 0.5f, 0.5f}, .normal = {0, 0, 1}, .color = 0xFFCDCDCD, .texcoord = {}},

            // Right
            StandardVertex{.position = {-0.5f, -0.5f, -0.5f}, .normal = {-1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, 0.5f, 0.5f}, .normal = {-1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, -0.5f, 0.5f}, .normal = {-1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, 0.5f, -0.5f}, .normal = {-1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},

            // Left
            StandardVertex{.position = {0.5f, 0.5f, 0.5f}, .normal = {1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, -0.5f, -0.5f}, .normal = {1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, -0.5f, 0.5f}, .normal = {1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, 0.5f, -0.5f}, .normal = {1, 0, 0}, .color = 0xFFCDCDCD, .texcoord = {}},

            // Back
            StandardVertex{.position = {0.5f, 0.5f, -0.5f}, .normal = {0, 0, -1}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, -0.5f, -0.5f}, .normal = {0, 0, -1}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, -0.5f, -0.5f}, .normal = {0, 0, -1}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, 0.5f, -0.5f}, .normal = {0, 0, -1}, .color = 0xFFCDCDCD, .texcoord = {}},

            // Top
            StandardVertex{.position = {-0.5f, 0.5f, -0.5f}, .normal = {0, 1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, 0.5f, 0.5f}, .normal = {0, 1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, 0.5f, -0.5f}, .normal = {0, 1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, 0.5f, 0.5f}, .normal = {0, 1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},

            // Bottom
            StandardVertex{.position = {0.5f, -0.5f, 0.5f}, .normal = {0, -1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, -0.5f, -0.5f}, .normal = {0, -1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {0.5f, -0.5f, -0.5f}, .normal = {0, -1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
            StandardVertex{.position = {-0.5f, -0.5f, 0.5f}, .normal = {0, -1, 0}, .color = 0xFFCDCDCD, .texcoord = {}},
        };

        Rx::Vector<Uint32> cube_indices = Rx::Array{
            // front face
            0,
            1,
            2, // first triangle
            0,
            3,
            1, // second triangle

            // left face
            4,
            5,
            6, // first triangle
            4,
            7,
            5, // second triangle

            // right face
            8,
            9,
            10, // first triangle
            8,
            11,
            9, // second triangle

            // back face
            12,
            13,
            14, // first triangle
            12,
            15,
            13, // second triangle

            // top face
            16,
            18,
            17, // first triangle
            16,
            17,
            19, // second triangle

            // bottom face
            20,
            21,
            22, // first triangle
            20,
            23,
            21, // second triangle
        };
#pragma endregion

        std::chrono::high_resolution_clock::time_point start_time;

        Settings settings;

        glm::uvec2 output_framebuffer_size;

        Rx::Ptr<RenderDevice> device;

        Rx::Ptr<MeshDataStore> static_mesh_storage;

        PerFrameData per_frame_data;
        Rx::Vector<Rx::Ptr<Buffer>> per_frame_data_buffers;

        Rx::Ptr<CameraMatrixBuffer> camera_matrix_buffers;

        Rx::Vector<StandardMaterial> standard_materials;
        Rx::Vector<StandardMaterialHandle> free_material_handles;
        Rx::Vector<Rx::Ptr<Buffer>> material_device_buffers;

        Rx::Map<Rx::String, Uint32> image_name_to_index;
        Rx::Vector<Rx::Ptr<Image>> all_images;

        std::array<Light, MAX_NUM_LIGHTS> lights;
        Rx::Vector<Rx::Ptr<Buffer>> light_device_buffers;

        std::queue<Mesh> pending_raytracing_upload_meshes;
        bool raytracing_scene_dirty{false};

        TextureHandle noise_texture_handle;
        TextureHandle pink_texture_handle;
        TextureHandle normal_roughness_texture_handle;
        TextureHandle specular_emission_texture_handle;

        Rx::Vector<Rx::Ptr<RenderPass>> render_passes;

#pragma region Initialization
        void create_static_mesh_storage();

        void create_per_frame_buffers();

        void create_material_data_buffers();

        void create_light_buffers();

        void create_builtin_images();

        void load_noise_texture(const Rx::String& filepath);

        void create_render_passes();
#pragma endregion

        [[nodiscard]] Rx::Vector<const Image*> get_texture_array() const;

        void update_cameras(entt::registry& registry, Uint32 frame_idx) const;

        void upload_material_data(Uint32 frame_idx);

#pragma region 3D Scene
        Rx::Vector<RaytracableGeometry> raytracing_geometries;

        Rx::Vector<RaytracingObject> raytracing_objects;

        Rx::Vector<Rx::Ptr<Buffer>> model_matrix_buffers;

        Rx::Vector<Rx::Ptr<Rx::Concurrency::Atomic<Uint32>>> next_unused_model_matrix_per_frame;

        RaytracingScene raytracing_scene;

        Rx::Ptr<Buffer> visible_objects_buffer;

        void rebuild_raytracing_scene(const com_ptr<ID3D12GraphicsCommandList4>& commands);

        void update_lights(entt::registry& registry, Uint32 frame_idx);

#pragma endregion
    };
} // namespace renderer
