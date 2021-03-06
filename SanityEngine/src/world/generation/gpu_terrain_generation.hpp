#pragma once

#include <d3d12.h>
#include <winrt/base.h>

#include "core/types.hpp"

struct TerrainData;

namespace renderer {
    class RenderDevice;
    class Renderer;
} // namespace renderer

using winrt::com_ptr;

namespace terraingen {
    /*!
     * \brief Creates the PSOs that will be used for terrain generation
     */
    void initialize(renderer::RenderDevice& device);

    /*!
     * \brief Places oceans in the provided heightmap
     *
     * \param commands The command list to record ocean-placing commands in to
     * \param renderer The renderer which will render this terrain
     * \param sea_level Height of the sea, on average
     * \param data The TerrainData that will hold the ocean depth texture
     */
    void place_oceans(const com_ptr<ID3D12GraphicsCommandList4>& commands,
                      renderer::Renderer& renderer,
                      Uint32 sea_level,
                      TerrainData& data);

    /*!
     * \brief Does some useful version of flood-filling water
     *
     * \param commands The command list to perform the water filling on
     * \param renderer The renderer that holds the terrain images
     * \param data The terrain data to compute water flow on
     */
    void compute_water_flow(const com_ptr<ID3D12GraphicsCommandList4>& commands, renderer::Renderer& renderer, TerrainData& data);
} // namespace terraingen
