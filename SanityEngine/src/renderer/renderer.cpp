#include "renderer.hpp"

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include "../core/components.hpp"
#include "../core/constants.hpp"
#include "../rhi/render_device.hpp"
#include "camera_matrix_buffer.hpp"
#include "components.hpp"

namespace renderer {
    std::vector<uint8_t> load_shader(const std::string& shader_filename) {
        auto* shader_file = fopen(shader_filename.c_str(), "rb");
        if(shader_file == nullptr) {
            spdlog::error("Could not open shader file '{}'", shader_filename);
            return {};
        }

        fseek(shader_file, 0, SEEK_END);
        const auto file_size = ftell(shader_file);

        auto shader = std::vector<uint8_t>(file_size);

        rewind(shader_file);

        fread(shader.data(), sizeof(uint8_t), file_size, shader_file);
        fclose(shader_file);

        return shader;
    }

    Renderer::Renderer(GLFWwindow* window, uint32_t num_frames)
        : render_device{make_render_device(rhi::RenderBackend::D3D12, window, num_frames)} {
        make_static_mesh_storage();

        create_debug_pipeline();
    }

    void Renderer::render_scene(entt::registry& registry) {
        render_device->begin_frame();

        const auto frame_idx = render_device->get_cur_backbuffer_idx();

        auto command_list = render_device->create_render_command_list();
        command_list->set_debug_name("Main Render Command List");

        update_cameras(registry, *command_list, frame_idx);

        render_3d_scene(registry, *command_list, frame_idx);

        render_device->submit_command_list(std::move(command_list));

        render_device->end_frame();
    }

    StaticMeshRenderable Renderer::create_static_mesh(const std::vector<BveVertex>& vertices, const std::vector<uint32_t>& indices) const {
        const auto mesh_start_idx = static_mesh_storage->add_mesh(vertices, indices);

        StaticMeshRenderable renderable{};
        renderable.first_index = mesh_start_idx;
        renderable.num_indices = indices.size();

        return renderable;
    }

    void Renderer::make_static_mesh_storage() {
        rhi::BufferCreateInfo vertex_create_info{};
        vertex_create_info.name = "Static Mesh Vertex Buffer";
        vertex_create_info.size = STATIC_MESH_VERTEX_BUFFER_SIZE;
        vertex_create_info.usage = rhi::BufferUsage::VertexBuffer;

        auto vertex_buffer = render_device->create_buffer(vertex_create_info);

        rhi::BufferCreateInfo index_buffer_create_info{};
        index_buffer_create_info.name = "Static Mesh Index Buffer";
        index_buffer_create_info.size = STATIC_MESH_INDEX_BUFFER_SIZE;
        index_buffer_create_info.usage = rhi::BufferUsage::IndexBuffer;

        auto index_buffer = render_device->create_buffer(index_buffer_create_info);

        static_mesh_storage = std::make_unique<rhi::MeshDataStore>(*render_device, std::move(vertex_buffer), std::move(index_buffer));
    }

    void Renderer::create_debug_pipeline() {
        rhi::RenderPipelineStateCreateInfo debug_pipeline_create_info{};
        debug_pipeline_create_info.vertex_shader = load_shader("data/shaders/debug.vertex.dxil");
        debug_pipeline_create_info.pixel_shader = load_shader("data/shaders/debug.pixel.dxil");

        debug_pipeline_create_info.rasterizer_state.cull_mode = rhi::CullMode::None;

        debug_pipeline_create_info.depth_stencil_state.enable_depth_test = false;
        debug_pipeline_create_info.depth_stencil_state.enable_depth_write = false;

        debug_pipeline = render_device->create_render_pipeline_state(debug_pipeline_create_info);

        spdlog::info("Created debug pipeline");
    }

    void Renderer::update_cameras(const entt::registry& registry, rhi::RenderCommandList& commands, const uint32_t frame_idx) {
        auto camera_view = registry.view<TransformComponent, CameraComponent>();
        for(const auto entity : camera_view) {
            const auto& transform = registry.get<TransformComponent>(entity);
            const auto& camera = registry.get<CameraComponent>(entity);

            CameraMatrices matrices;
            matrices.calculate_view_matrix(transform);
            matrices.calculate_projection_matrix(camera);

            camera_matrix_buffers->set_camera_matrices(camera.idx, matrices);
        }
    }

    void Renderer::render_3d_scene(entt::registry& registry, rhi::RenderCommandList& command_list, const uint32_t frame_idx) const {
        const auto framebuffer = render_device->get_backbuffer_framebuffer();

        command_list.set_framebuffer(*framebuffer);

        command_list.set_pipeline_state(*debug_pipeline);

        command_list.bind_mesh_data(*static_mesh_storage);

        auto static_mesh_view = registry.view<StaticMeshRenderable>();

        for(const auto entity : static_mesh_view) {
            const auto mesh_renderable = registry.get<StaticMeshRenderable>(entity);

            command_list.draw(mesh_renderable.num_indices, mesh_renderable.first_index);
        }
    }
} // namespace renderer
