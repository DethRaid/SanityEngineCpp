#pragma once

#include <d3d12.h>
#include <winrt/base.h>

#include "rx/core/vector.h"

using winrt::com_ptr;

namespace renderer {
    class DescriptorAllocator {
    public:
        DescriptorAllocator(com_ptr<ID3D12DescriptorHeap> heap_in, UINT descriptor_size_in);

        DescriptorAllocator(const DescriptorAllocator& other) = delete;
        DescriptorAllocator& operator=(const DescriptorAllocator& other) = delete;

        DescriptorAllocator(DescriptorAllocator&& old) noexcept;
        DescriptorAllocator& operator=(DescriptorAllocator&& old) noexcept;

        ~DescriptorAllocator() = default;

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE get_next_free_descriptor();

        void return_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle);

    private:
        com_ptr<ID3D12DescriptorHeap> heap;

        UINT descriptor_size;

        INT next_free_descriptor{0};

        Rx::Vector<D3D12_CPU_DESCRIPTOR_HANDLE> available_handles;
    };
} // namespace renderer
