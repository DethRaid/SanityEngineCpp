#pragma once

#include <cstdint>

#include "lights.hpp"
#include "mesh.hpp"

namespace renderer {
    struct StaticMeshRenderableComponent {
        Mesh mesh;
        rhi::RaytracingMesh rt_mesh;
    };

    struct CameraComponent {
        uint32_t idx;

        double fov{90};
        double aspect_ratio{9.0f / 16.0f};
        double near_clip_plane{0.01};
    };

    struct LightComponent {
        LightHandle handle;

        Light light;
    };
} // namespace renderer
