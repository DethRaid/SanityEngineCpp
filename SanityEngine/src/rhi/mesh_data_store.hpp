#pragma once

#include <memory>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/logger.h>

#include "resources.hpp"

struct StandardVertex {
    glm::vec3 position;
    glm::vec3 normal;
    uint32_t color{0xFFFFFFFF};
    uint32_t material_idx{0};
    glm::vec2 texcoord;
};

namespace rhi {
    class ResourceCommandList;
    class RenderDevice;

    struct Mesh {
        uint32_t first_vertex{0};
        uint32_t num_vertices{0};

        uint32_t first_index{0};
        uint32_t num_indices{0};
    };

    /*!
     * \brief Binding for a vertex buffer
     */
    struct VertexBufferBinding {
        /*!
         * \brief The buffer to bind
         */
        Buffer* buffer;

        /*!
         * \brief Offset in bytes where the relevant data starts
         */
        uint32_t offset;

        /*!
         * \brief Size of a vertex, in bytes
         */
        uint32_t vertex_size;
    };

    class MeshDataStore {
    public:
        MeshDataStore(RenderDevice& device_in, std::unique_ptr<Buffer> vertex_buffer_in, std::unique_ptr<Buffer> index_buffer_in);

        MeshDataStore(const MeshDataStore& other) = delete;
        MeshDataStore& operator=(const MeshDataStore& other) = delete;

        MeshDataStore(MeshDataStore&& old) noexcept = default;
        MeshDataStore& operator=(MeshDataStore&& old) noexcept = delete;

        ~MeshDataStore();

        [[nodiscard]] const std::vector<VertexBufferBinding>& get_vertex_bindings() const;

        [[nodiscard]] const Buffer& get_index_buffer() const;

        /*!
         * \brief Prepares the vertex and index buffers to receive new mesh data
         */
        void begin_adding_meshes(const ComPtr<ID3D12GraphicsCommandList4>& commands) const;

        /*!
         * \brief Adds new mesh data to the vertex and index buffers. Must be called after `begin_mesh_data_upload` and before
         * `end_mesh_data_upload`
         */
        [[nodiscard]] Mesh add_mesh(const std::vector<StandardVertex>& vertices,
                                    const std::vector<uint32_t>& indices,
                                    const ComPtr<ID3D12GraphicsCommandList4>& commands);

        /*!
         * Prepares the vertex and index buffers to be rendered with
         */
        void end_adding_meshes(const ComPtr<ID3D12GraphicsCommandList4>& commands) const;

        void bind_to_command_list(const ComPtr<ID3D12GraphicsCommandList4>& commands) const;

    private:
        std::shared_ptr<spdlog::logger> logger;

        RenderDevice* device;

        std::unique_ptr<Buffer> vertex_buffer;

        std::unique_ptr<Buffer> index_buffer;

        std::vector<VertexBufferBinding> vertex_bindings{};

        /*!
         * \brief Index of the byte in the vertex buffer where the next mesh can be uploaded to
         *
         * I'll eventually need a way to unload meshes, but that's more complicated
         */
        uint32_t next_free_vertex_byte{0};

        /*!
         * \brief The offset in the vertex buffer, in vertices, where the next mesh's vertex data should start
         */
        uint32_t next_vertex_offset{0};

        /*!
         * \brief The offset in the index buffer where the next mesh's indices should start
         */
        uint32_t next_index_offset{0};
    };
} // namespace rhi
