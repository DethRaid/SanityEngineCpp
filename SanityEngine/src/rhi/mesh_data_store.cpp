#include "mesh_data_store.hpp"

#include "Tracy.hpp"
#include "TracyD3D12.hpp"
#include "rhi/helpers.hpp"
#include "rhi/render_device.hpp"
#include "rx/core/log.h"

namespace renderer {
    RX_LOG("MeshDataStore", logger);

    MeshDataStore::MeshDataStore(RenderDevice& device_in, Rx::Ptr<Buffer> vertex_buffer_in, Rx::Ptr<Buffer> index_buffer_in)
        : device{&device_in}, vertex_buffer{Rx::Utility::move(vertex_buffer_in)}, index_buffer{Rx::Utility::move(index_buffer_in)} {

        vertex_bindings.reserve(4);
        vertex_bindings.push_back(VertexBufferBinding{vertex_buffer.get(), offsetof(StandardVertex, position), sizeof(StandardVertex)});
        vertex_bindings.push_back(VertexBufferBinding{vertex_buffer.get(), offsetof(StandardVertex, normal), sizeof(StandardVertex)});
        vertex_bindings.push_back(VertexBufferBinding{vertex_buffer.get(), offsetof(StandardVertex, color), sizeof(StandardVertex)});
        vertex_bindings.push_back(VertexBufferBinding{vertex_buffer.get(), offsetof(StandardVertex, texcoord), sizeof(StandardVertex)});
    }

    MeshDataStore::~MeshDataStore() {
        device->schedule_buffer_destruction(Rx::Utility::move(vertex_buffer));
        device->schedule_buffer_destruction(Rx::Utility::move(index_buffer));
    }

    const Rx::Vector<VertexBufferBinding>& MeshDataStore::get_vertex_bindings() const { return vertex_bindings; }

    const Buffer& MeshDataStore::get_index_buffer() const { return *index_buffer; }

    void MeshDataStore::begin_adding_meshes(ID3D12GraphicsCommandList4* commands) const {
        auto* vertex_resource = vertex_buffer->resource.get();
        auto* index_resource = index_buffer->resource.get();

        Rx::Vector<D3D12_RESOURCE_BARRIER> barriers{2};
        barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(vertex_resource,
                                                           D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                                           D3D12_RESOURCE_STATE_COPY_DEST);
        barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(index_resource,
                                                           D3D12_RESOURCE_STATE_INDEX_BUFFER,
                                                           D3D12_RESOURCE_STATE_COPY_DEST);

        commands->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
    }

    Mesh MeshDataStore::add_mesh(const Rx::Vector<StandardVertex>& vertices,
                                 const Rx::Vector<Uint32>& indices,
                                 ID3D12GraphicsCommandList4* commands) {
        ZoneScoped;

        TracyD3D12Zone(RenderDevice::tracy_context, commands, "MeshDataStore::add_mesh");
        PIXScopedEvent(commands, PIX_COLOR_DEFAULT, "MeshDataStore::add_mesh");

        logger->verbose("Adding mesh with %u vertices and %u indices", vertices.size(), indices.size());

        const auto vertex_data_size = static_cast<Uint32>(vertices.size() * sizeof(StandardVertex));
        const auto index_data_size = static_cast<Uint32>(indices.size() * sizeof(Uint32));

        // Offset the indices so they'll refer to the right vertex
        Rx::Vector<Uint32> offset_indices;
        offset_indices.reserve(indices.size());

        indices.each_fwd([&](const Uint32 idx) { offset_indices.push_back(idx + next_vertex_offset); });

        auto* vertex_resource = vertex_buffer->resource.get();
        auto* index_resource = index_buffer->resource.get();

        const auto index_buffer_byte_offset = static_cast<Uint32>(next_index_offset * sizeof(Uint32));

        upload_data_with_staging_buffer(commands, *device, vertex_resource, vertices.data(), vertex_data_size, next_free_vertex_byte);

        upload_data_with_staging_buffer(commands,
                                        *device,
                                        index_resource,
                                        offset_indices.data(),
                                        index_data_size,
                                        index_buffer_byte_offset);

        const auto vertex_offset = static_cast<Uint32>(next_free_vertex_byte / sizeof(StandardVertex));

        next_free_vertex_byte += vertex_data_size;

        const auto index_offset = next_index_offset;

        next_vertex_offset += static_cast<Uint32>(vertices.size());
        next_index_offset += static_cast<Uint32>(indices.size());

        return {.first_vertex = vertex_offset,
                .num_vertices = static_cast<Uint32>(vertices.size()),
                .first_index = index_offset,
                .num_indices = static_cast<Uint32>(indices.size())};
    }

    void MeshDataStore::end_adding_meshes(ID3D12GraphicsCommandList4* commands) const {
        auto* vertex_resource = vertex_buffer->resource.get();
        auto* index_resource = index_buffer->resource.get();

        Rx::Vector<D3D12_RESOURCE_BARRIER> barriers{2};
        barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(vertex_resource,
                                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                                           D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(index_resource,
                                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                                           D3D12_RESOURCE_STATE_INDEX_BUFFER);

        commands->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
    }

    void MeshDataStore::bind_to_command_list(ID3D12GraphicsCommandList4* commands) const {
        const auto& vertex_bindings = get_vertex_bindings();

        // If we have more than 16 vertex attributes, we probably have bigger problems
        Rx::Array<D3D12_VERTEX_BUFFER_VIEW[16]> vertex_buffer_views{};
        for(Uint32 i = 0; i < vertex_bindings.size(); i++) {
            const auto& binding = vertex_bindings[i];
            const auto* buffer = static_cast<const Buffer*>(binding.buffer);

            D3D12_VERTEX_BUFFER_VIEW view{};
            view.BufferLocation = buffer->resource->GetGPUVirtualAddress() + binding.offset;
            view.SizeInBytes = buffer->size - binding.offset;
            view.StrideInBytes = binding.vertex_size;

            vertex_buffer_views[i] = view;
        }

        commands->IASetVertexBuffers(0, static_cast<UINT>(vertex_bindings.size()), vertex_buffer_views.data());

        const auto& index_buffer = get_index_buffer();

        D3D12_INDEX_BUFFER_VIEW index_view{};
        index_view.BufferLocation = index_buffer.resource->GetGPUVirtualAddress();
        index_view.SizeInBytes = index_buffer.size;
        index_view.Format = DXGI_FORMAT_R32_UINT;

        commands->IASetIndexBuffer(&index_view);

        commands->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
} // namespace renderer
