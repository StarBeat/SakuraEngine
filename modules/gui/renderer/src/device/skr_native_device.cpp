#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGuiRenderer/device/skr_native_window.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"

namespace skr::gui
{
void SkrNativeDevice::init()
{
    // init render device
    _render_device = SkrNew<SkrRenderDevice>();
    _render_device->init();

    // init resource service
    _resource_device = SkrNew<SkrResourceDevice>();
    _resource_device->init();

    // get display metrics
    uint32_t       count;
    SMonitorHandle monitors[128];
    skr_get_all_monitors(&count, monitors);
    for (uint32_t i = 0; i < count; ++i)
    {
        SMonitorHandle monitor = monitors[i];
        int32_t        x, y, width, height;
        skr_monitor_get_extent(monitor, &width, &height);
        skr_monitor_get_position(monitor, &x, &y);

        auto out = _display_metrics.monitors.emplace_back();

        // name
        // device_id
        // native_size
        // max_resolution
        // display_area
        out.work_area = Recti::LTWH(x, y, width, height);
        // is_primary
        // features
    }

    // TODO. total data
    // _display_metrics.primary_display_area = _display_metrics.monitors[0].work_area;

    // init text device
    embedded_init_text_service(this);
}
void SkrNativeDevice::shutdown()
{
    // shutdown text device
    embedded_shutdown_text_service();

    // shutdown resource device
    _resource_device->shutdown();
    SkrDelete(_resource_device);

    // shutdown render device
    _render_device->shutdown();
    SkrDelete(_render_device);
}

// view
NotNull<IWindow*> SkrNativeDevice::create_window()
{
    auto view = SkrNew<SkrNativeWindow>(this);
    _all_windows.push_back(view);
    return make_not_null(view);
}
void SkrNativeDevice::destroy_window(NotNull<IWindow*> view)
{
    // erase it
    auto it = std::find(_all_windows.begin(), _all_windows.end(), view);
    if (it != _all_windows.end())
    {
        _all_windows.erase(it);
    }

    // delete
    SkrDelete(view.get());
}

void SkrNativeDevice::render_all_windows() SKR_NOEXCEPT
{
    // combine render graph
    for (auto window : _all_windows)
    {
        window->render_window()->render(window->native_layer(), window->absolute_size());
    }

    // commit render graph
    render_device()->render_graph()->compile();
    render_device()->render_graph()->execute();

    // present
    for (auto window : _all_windows)
    {
        window->render_window()->present();
    }
}

// display info
const DisplayMetrics& SkrNativeDevice::display_metrics() const
{
    return _display_metrics;
}

// resource management
NotNull<IUpdatableImage*> SkrNativeDevice::create_updatable_image(const UpdatableImageDesc& desc)
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return make_not_null<IUpdatableImage*>(nullptr);
}
void SkrNativeDevice::destroy_resource(NotNull<IResource*> resource)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

// canvas management
NotNull<ICanvas*> SkrNativeDevice::create_canvas()
{
    return embedded_create_canvas();
}
void SkrNativeDevice::destroy_canvas(NotNull<ICanvas*> canvas)
{
    embedded_destroy_canvas(canvas);
}

// text management
NotNull<IParagraph*> SkrNativeDevice::create_paragraph()
{
    return embedded_create_paragraph();
}
void SkrNativeDevice::destroy_paragraph(NotNull<IParagraph*> paragraph)
{
    embedded_destroy_paragraph(paragraph);
}

} // namespace skr::gui