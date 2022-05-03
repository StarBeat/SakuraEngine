#include "json/reader.h"
#include "EASTL/string.h"

namespace skr::json
{
// TODO: error handling
template <>
void Read(simdjson::ondemand::value&& json, bool& b) { b = json.get_bool(); }
template <>
void Read(simdjson::ondemand::value&& json, int32_t& b) { b = json.get_int64(); }
template <>
void Read(simdjson::ondemand::value&& json, uint32_t& b) { b = json.get_uint64(); }
template <>
void Read(simdjson::ondemand::value&& json, int64_t& b) { b = json.get_int64(); }
template <>
void Read(simdjson::ondemand::value&& json, uint64_t& b) { b = json.get_uint64(); }
template <>
void Read(simdjson::ondemand::value&& json, double& b) { b = json.get_double(); }
template <>
void Read(simdjson::ondemand::value&& json, eastl::string& str)
{
    std::string_view view = json.get_string();
    str = eastl::string(view.data(), view.length());
}
} // namespace skr::json