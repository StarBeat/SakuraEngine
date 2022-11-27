#include "SkrGLTFTool/gltf_utils.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrGLTFTool/mesh_asset.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "cgpu/api.h"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

namespace skd
{
namespace asset
{
cgltf_data* ImportGLTFWithData(skr::string_view assetPath, skr::io::RAMService* ioService, struct skr_vfs_t* vfs) SKR_NOEXCEPT
{
    // prepare callback
    skr::task::event_t counter;
    struct CallbackData
    {
        skr_async_ram_destination_t destination;
        skr::task::event_t* pCounter;   
        skr::string u8Path;
    } callbackData;
    callbackData.pCounter = &counter;
    callbackData.u8Path = assetPath.data();
    // prepare io
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = callbackData.u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cgltf_options options = {};
        struct cgltf_data* gltf_data_ = nullptr;
        if (cbData->destination.bytes)
        {
            cgltf_result result = cgltf_parse(&options, cbData->destination.bytes, cbData->destination.size, &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                result = cgltf_load_buffers(&options, gltf_data_, cbData->u8Path.c_str());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
            }
        }
        sakura_free(cbData->destination.bytes);
        cbData->destination.bytes = (uint8_t*)gltf_data_;
        cbData->destination.size = MAGIC_SIZE_GLTF_PARSE_READY;
        cbData->pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    skr_async_request_t ioRequest = {};
    ioService->request(vfs, &ramIO, &ioRequest, &callbackData.destination);
    counter.wait(false);
    // parse
    if (callbackData.destination.size == MAGIC_SIZE_GLTF_PARSE_READY)
    {
        cgltf_data* gltf_data = (cgltf_data*)callbackData.destination.bytes;
        return gltf_data;
    }
    return nullptr;
}

void GetGLTFNodeTransform(const cgltf_node* node, skr_float3_t& translation, skr_float3_t& scale, skr_float4_t& rotation)
{
    if (node->has_translation)
        translation = { node->translation[0], node->translation[1], node->translation[2] };
    if (node->has_scale)
        scale = { node->scale[0], node->scale[1], node->scale[2] };
    if (node->has_rotation)
        rotation = { node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3] };
}

skr::span<const uint8_t> GetGLTFPrimitiveIndicesView(const cgltf_primitive* primitve, uint32_t& index_stride)
{
    index_stride = (uint32_t)primitve->indices->stride;

    const auto buffer_view = primitve->indices->buffer_view;
    const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
    const auto view_data = buffer_data + buffer_view->offset;
    const auto indices_count = primitve->indices->count;
    return skr::span<const uint8_t>(view_data + primitve->indices->offset, index_stride * indices_count);
}

skr::span<const uint8_t> GetGLTFPrimitiveAttributeView(const cgltf_primitive* primitve, cgltf_attribute_type type, uint32_t& stride)
{
    for (uint32_t i = 0; i < primitve->attributes_count; i++)
    {
        const auto& attribute = primitve->attributes[i];
        if (attribute.type == type)
        {
            stride = (uint32_t)attribute.data->stride;
            
            const auto buffer_view = attribute.data->buffer_view;
            const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
            const auto view_data = buffer_data + buffer_view->offset;
            return skr::span<const uint8_t>(view_data + attribute.data->offset, attribute.data->stride * attribute.data->count);
        }
    }
    return {};
}

skr::span<const uint8_t> GetGLTFPrimitiveAttributeView(const cgltf_primitive* primitve, const char* semantics, uint32_t& stride)
{
    for (uint32_t type = 0; type < cgltf_attribute_type_custom + 1; type++)
    {
        const auto refStr = kGLTFAttributeTypeLUT[type];
        eastl::string_view semantics_sv = semantics;
        if (semantics_sv.starts_with(refStr))
        {
            return GetGLTFPrimitiveAttributeView(primitve, (cgltf_attribute_type)type, stride);
        }
    }
    return {};
}

void EmplaceGLTFPrimitiveIndexBuffer(const cgltf_primitive* primitve, eastl::vector<uint8_t>& buffer, skr_index_buffer_entry_t& index_buffer)
{
    uint32_t index_stride = 0;
    const auto ib_view = GetGLTFPrimitiveIndicesView(primitve, index_stride);

    index_buffer.buffer_index = 0;
    index_buffer.first_index = 0; //TODO: ?
    index_buffer.index_offset = (uint32_t)buffer.size();
    index_buffer.index_count = (uint32_t)ib_view.size() / index_stride;
    index_buffer.stride = index_stride;
    buffer.insert(buffer.end(), ib_view.data(), ib_view.data() + ib_view.size());
}

void EmplaceGLTFPrimitiveVertexBufferAttribute(const cgltf_primitive* primitve, cgltf_attribute_type type, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv)
{
    skr::span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    vertex_attribtue_slice = GetGLTFPrimitiveAttributeView(primitve, type, attribute_stride);

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = (uint32_t)buffer.size();
    buffer.insert(buffer.end(), vertex_attribtue_slice.data(), vertex_attribtue_slice.data() + vertex_attribtue_slice.size());
}

void EmplaceGLTFPrimitiveVertexBufferAttribute(const cgltf_primitive* primitve, const char* semantics, eastl::vector<uint8_t>& buffer, skr_vertex_buffer_entry_t& out_vbv)
{
    skr::span<const uint8_t> vertex_attribtue_slice = {};
    uint32_t attribute_stride = 0;
    vertex_attribtue_slice = GetGLTFPrimitiveAttributeView(primitve, semantics, attribute_stride);

    out_vbv.buffer_index = 0;
    out_vbv.stride = attribute_stride;
    out_vbv.offset = (uint32_t)buffer.size();
    buffer.insert(buffer.end(), vertex_attribtue_slice.data(), vertex_attribtue_slice.data() + vertex_attribtue_slice.size());
}

void EmplaceAllGLTFMeshIndices(const cgltf_mesh* mesh, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    out_primitives.resize(mesh->primitives_count);
    // record all indices
    for (uint32_t j = 0; j < mesh->primitives_count; j++)
    {
        const auto gltf_prim = mesh->primitives + j;
        EmplaceGLTFPrimitiveIndexBuffer(gltf_prim, buffer, out_primitives[j].index_buffer);
    }
}

void EmplaceAllGLTFMeshVertices(const cgltf_mesh* mesh, const CGPUVertexLayout* layout, eastl::vector<uint8_t>& buffer, eastl::vector<skr_mesh_primitive_t>& out_primitives)
{
    if (layout != nullptr)
    {
    const auto& shuffle_layout = *layout;
    for (uint32_t k = 0; k < shuffle_layout.attribute_count; k++)
    {
        // geometry cache friendly layout
        // | prim0-pos | prim1-pos | prim0-tangent | prim1-tangent | ...
        for (uint32_t j = 0; j < mesh->primitives_count; j++)
        {
            auto& prim = out_primitives[j];
            prim.vertex_buffers.resize(shuffle_layout.attribute_count);

            const auto gltf_prim = mesh->primitives + j;
            const auto& shuffle_attrib = shuffle_layout.attributes[k];
            EmplaceGLTFPrimitiveVertexBufferAttribute(gltf_prim, shuffle_attrib.semantic_name, buffer, prim.vertex_buffers[k]);
        }
    }
    }
    else 
    {
        SKR_UNREACHABLE_CODE();
    }
}

void CookGLTFStaticMeshData(const cgltf_data* gltf_data, SMeshCookConfig* cfg, skr_mesh_resource_t& out_resource, eastl::vector<eastl::vector<uint8_t>>& out_bins)
{
    eastl::vector<uint8_t> shuffled_buffer = {};
    
    skr_guid_t shuffle_layout_id = cfg->vertexType;
    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.isZero()) 
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }

    out_resource.name = gltf_data->meshes[0].name;
    if (out_resource.name.empty()) out_resource.name = "gltfMesh";
    // record primitvies
    for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
    {
        const auto node_ = gltf_data->nodes + i;
        auto& mesh_section = out_resource.sections.emplace_back();
        mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
        GetGLTFNodeTransform(node_, mesh_section.translation, mesh_section.scale, mesh_section.rotation);
        if (node_->mesh != nullptr)
        {
            eastl::vector<skr_mesh_primitive_t> new_primitives;
            // record all indices
            EmplaceAllGLTFMeshIndices(node_->mesh, shuffled_buffer, new_primitives);
            EmplaceAllGLTFMeshVertices(node_->mesh, shuffle_layout_name ? &shuffle_layout : nullptr, shuffled_buffer, new_primitives);
            for (uint32_t j = 0; j < node_->mesh->primitives_count; j++)
            {
                auto& prim = new_primitives[j];
                prim.vertex_layout_id = shuffle_layout_id;
                prim.material_inst = make_zeroed<skr_guid_t>(); // TODO: Material assignment
                mesh_section.primive_indices.emplace_back(out_resource.primitives.size() + j);
            }
            out_resource.primitives.reserve(out_resource.primitives.size() + new_primitives.size());
            out_resource.primitives.insert(out_resource.primitives.end(), new_primitives.begin(), new_primitives.end());
        }
    }
    {
        // record buffer bins
        auto& output_bin = out_resource.bins.emplace_back();
        output_bin.index = 0;
        output_bin.byte_length = shuffled_buffer.size();
        output_bin.used_with_index = true;
        output_bin.used_with_vertex = true;
    }

    // output one buffer contains vertices & indices
    out_bins.emplace_back(shuffled_buffer);
}

} // namespace asset
} // namespace skd