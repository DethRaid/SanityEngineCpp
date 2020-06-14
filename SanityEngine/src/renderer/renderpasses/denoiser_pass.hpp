#pragma once

#include <glm/fwd.hpp>
#include <rx/core/ptr.h>

#include "renderer/handles.hpp"
#include "renderer/renderpass.hpp"
#include "rhi/framebuffer.hpp"
#include "rhi/render_pipeline_state.hpp"

namespace renderer {
    class ForwardPass;
    class Renderer;

    class DenoiserPass final : public virtual RenderPass {
    public:
        /*!
         * \brief Constructs a new denoiser pass to denoise some stuff
         *
         * \param renderer_in The renderer which will be executing this pass
         * \param render_resolution The resolution to render at. May or may not equal the final resolution
         * \param forward_pass the pass which this denoise pass will denoise the output of
         */
        explicit DenoiserPass(Renderer& renderer_in, const glm::uvec2& render_resolution, const ForwardPass& forward_pass);

        void execute(ID3D12GraphicsCommandList4* commands, entt::registry& registry, uint32_t frame_idx) override;

        [[nodiscard]] TextureHandle get_output_image() const;

    private:
        Renderer* renderer;

        Rx::Ptr<rhi::RenderPipelineState> accumulation_pipeline;

        /*!
         * \brief Handle to the texture that holds the accumulated scene
         */
        TextureHandle accumulation_target_handle;

        /*!
         * \brief Handle to the texture that holds the final denoised image
         */
        TextureHandle denoised_color_target_handle;

        /*!
         * \brief Framebuffer for the final denoised image
         */
        Rx::Ptr<rhi::Framebuffer> denoised_framebuffer;

        Rx::Ptr<rhi::Buffer> denoiser_material_buffer;

        void create_images_and_framebuffer(const glm::uvec2& render_resolution);

        void create_material(const ForwardPass& forward_pass);
    };
} // namespace renderer
