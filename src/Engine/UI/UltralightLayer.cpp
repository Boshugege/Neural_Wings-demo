#include "UltralightLayer.h"

#include <AppCore/Platform.h>
#include <Ultralight/platform/Config.h>

using ultralight::BitmapSurface;
using ultralight::KeyEvent;
using ultralight::MouseEvent;
using ultralight::ScrollEvent;

UltralightLayer::UltralightLayer()
    : m_texture{}, m_textureReady(false), m_visible(false) {}

UltralightLayer::~UltralightLayer()
{
    Shutdown();
}

bool UltralightLayer::Initialize(uint32_t width, uint32_t height, const std::string &basePath)
{
    m_basePath = basePath;

    auto &platform = ultralight::Platform::instance();
    ultralight::Config config;
    config.resource_path_prefix = "resources/";
    platform.set_config(config);

    platform.set_file_system(ultralight::GetPlatformFileSystem(ultralight::String(basePath.c_str())));
    platform.set_font_loader(ultralight::GetPlatformFontLoader());
    platform.set_logger(ultralight::GetDefaultLogger("ultralight.log"));

    m_renderer = ultralight::Renderer::Create();
    if (!m_renderer)
        return false;

    ultralight::ViewConfig viewConfig;
    viewConfig.is_accelerated = false;
    viewConfig.is_transparent = true;
    viewConfig.initial_focus = true;

    m_view = m_renderer->CreateView(width, height, viewConfig, nullptr);
    if (!m_view)
        return false;

    return true;
}

void UltralightLayer::Shutdown()
{
    if (m_textureReady)
    {
        UnloadTexture(m_texture);
        m_textureReady = false;
    }
    m_view = nullptr;
    m_renderer = nullptr;
}

void UltralightLayer::SetVisible(bool visible)
{
    m_visible = visible;
    if (m_view)
    {
        if (m_visible)
        {
            m_view->Focus();
        }
        else
        {
            m_view->Unfocus();
        }
    }
}

bool UltralightLayer::IsVisible() const
{
    return m_visible;
}

void UltralightLayer::LoadRoute(const std::string &route)
{
    if (!m_view)
        return;

    const std::string url = BuildIndexUrl(route);
    m_view->LoadURL(ultralight::String(url.c_str()));
    m_view->Focus();
}

void UltralightLayer::Resize(uint32_t width, uint32_t height)
{
    if (!m_view)
        return;

    if (m_view->width() == width && m_view->height() == height)
        return;

    m_view->Resize(width, height);
    m_view->set_needs_paint(true);
}

void UltralightLayer::Update()
{
    if (!m_renderer || !m_view)
        return;

    if (m_visible)
    {
        m_renderer->Update();
        m_renderer->RefreshDisplay(0);
        m_renderer->Render();
        UpdateTextureFromView();
    }
}

void UltralightLayer::HandleInput()
{
    if (!m_view || !m_visible)
        return;

    // ── Mouse events ──────────────────────────────────────────────
    Vector2 mouse = GetMousePosition();
    const bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    if (mousePressed)
    {
        m_view->Focus();
    }

    MouseEvent moveEvent;
    moveEvent.type = MouseEvent::kType_MouseMoved;
    moveEvent.x = static_cast<int>(mouse.x);
    moveEvent.y = static_cast<int>(mouse.y);
    moveEvent.button = MouseEvent::kButton_None;
    m_view->FireMouseEvent(moveEvent);

    if (mousePressed)
    {
        MouseEvent downEvent;
        downEvent.type = MouseEvent::kType_MouseDown;
        downEvent.x = static_cast<int>(mouse.x);
        downEvent.y = static_cast<int>(mouse.y);
        downEvent.button = MouseEvent::kButton_Left;
        m_view->FireMouseEvent(downEvent);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        MouseEvent upEvent;
        upEvent.type = MouseEvent::kType_MouseUp;
        upEvent.x = static_cast<int>(mouse.x);
        upEvent.y = static_cast<int>(mouse.y);
        upEvent.button = MouseEvent::kButton_Left;
        m_view->FireMouseEvent(upEvent);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f)
    {
        ScrollEvent scrollEvent;
        scrollEvent.type = ScrollEvent::kType_ScrollByPixel;
        scrollEvent.delta_x = 0;
        scrollEvent.delta_y = static_cast<int>(-wheel * 120.0f);
        m_view->FireScrollEvent(scrollEvent);
    }

    // ── Keyboard events ───────────────────────────────────────────

    // Build current modifier mask
    unsigned modifiers = 0;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        modifiers |= ultralight::KeyEvent::kMod_ShiftKey;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        modifiers |= ultralight::KeyEvent::kMod_CtrlKey;
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
        modifiers |= ultralight::KeyEvent::kMod_AltKey;

    // 1. Character input  (typed printable characters, with auto-repeat)
    int charCode;
    while ((charCode = GetCharPressed()) != 0)
    {
        ultralight::KeyEvent evt;
        evt.type = ultralight::KeyEvent::kType_Char;
        evt.modifiers = modifiers;

        // Convert Unicode code-point to a short UTF-8 string
        char utf8[5] = {};
        if (charCode < 0x80)
        {
            utf8[0] = static_cast<char>(charCode);
        }
        else if (charCode < 0x800)
        {
            utf8[0] = static_cast<char>(0xC0 | (charCode >> 6));
            utf8[1] = static_cast<char>(0x80 | (charCode & 0x3F));
        }
        else if (charCode < 0x10000)
        {
            utf8[0] = static_cast<char>(0xE0 | (charCode >> 12));
            utf8[1] = static_cast<char>(0x80 | ((charCode >> 6) & 0x3F));
            utf8[2] = static_cast<char>(0x80 | (charCode & 0x3F));
        }
        else
        {
            utf8[0] = static_cast<char>(0xF0 | (charCode >> 18));
            utf8[1] = static_cast<char>(0x80 | ((charCode >> 12) & 0x3F));
            utf8[2] = static_cast<char>(0x80 | ((charCode >> 6) & 0x3F));
            utf8[3] = static_cast<char>(0x80 | (charCode & 0x3F));
        }

        evt.text = ultralight::String(utf8);
        evt.unmodified_text = evt.text;
        m_view->FireKeyEvent(evt);
    }

    // 2. Special (non-printable) keys  –  mapped to Windows virtual-key codes
    struct KeyMapping
    {
        int raylibKey;
        int vk;
    };
    static const KeyMapping specialKeys[] = {
        {KEY_BACKSPACE, 0x08}, // VK_BACK
        {KEY_TAB, 0x09},       // VK_TAB
        {KEY_ENTER, 0x0D},     // VK_RETURN
        {KEY_DELETE, 0x2E},    // VK_DELETE
        {KEY_LEFT, 0x25},      // VK_LEFT
        {KEY_UP, 0x26},        // VK_UP
        {KEY_RIGHT, 0x27},     // VK_RIGHT
        {KEY_DOWN, 0x28},      // VK_DOWN
        {KEY_HOME, 0x24},      // VK_HOME
        {KEY_END, 0x23},       // VK_END
        {KEY_ESCAPE, 0x1B},    // VK_ESCAPE
    };

    for (const auto &k : specialKeys)
    {
        if (IsKeyPressed(k.raylibKey))
        {
            ultralight::KeyEvent evt;
            evt.type = ultralight::KeyEvent::kType_RawKeyDown;
            evt.virtual_key_code = k.vk;
            evt.native_key_code = k.vk;
            evt.modifiers = modifiers;
            m_view->FireKeyEvent(evt);
        }
        if (IsKeyReleased(k.raylibKey))
        {
            ultralight::KeyEvent evt;
            evt.type = ultralight::KeyEvent::kType_KeyUp;
            evt.virtual_key_code = k.vk;
            evt.native_key_code = k.vk;
            evt.modifiers = modifiers;
            m_view->FireKeyEvent(evt);
        }
    }

    // 3. Ctrl+letter shortcuts  (Select-All, Copy, Paste, Cut, Undo, Redo)
    if (modifiers & ultralight::KeyEvent::kMod_CtrlKey)
    {
        static const KeyMapping ctrlKeys[] = {
            {KEY_A, 'A'},
            {KEY_C, 'C'},
            {KEY_V, 'V'},
            {KEY_X, 'X'},
            {KEY_Z, 'Z'},
            {KEY_Y, 'Y'},
        };
        for (const auto &k : ctrlKeys)
        {
            if (IsKeyPressed(k.raylibKey))
            {
                ultralight::KeyEvent down;
                down.type = ultralight::KeyEvent::kType_RawKeyDown;
                down.virtual_key_code = k.vk;
                down.native_key_code = k.vk;
                down.modifiers = modifiers;
                m_view->FireKeyEvent(down);
            }
            if (IsKeyReleased(k.raylibKey))
            {
                ultralight::KeyEvent up;
                up.type = ultralight::KeyEvent::kType_KeyUp;
                up.virtual_key_code = k.vk;
                up.native_key_code = k.vk;
                up.modifiers = modifiers;
                m_view->FireKeyEvent(up);
            }
        }
    }
}

void UltralightLayer::Draw()
{
    if (!m_visible || !m_textureReady)
        return;

    DrawTexture(m_texture, 0, 0, WHITE);
}

void UltralightLayer::EnsureTexture(uint32_t width, uint32_t height)
{
    if (m_textureReady && m_texture.width == static_cast<int>(width) &&
        m_texture.height == static_cast<int>(height))
        return;

    if (m_textureReady)
    {
        UnloadTexture(m_texture);
        m_textureReady = false;
    }

    m_rgbaBuffer.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

    Image image = {};
    image.data = m_rgbaBuffer.data();
    image.width = static_cast<int>(width);
    image.height = static_cast<int>(height);
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    m_texture = LoadTextureFromImage(image);
    m_textureReady = true;
}

void UltralightLayer::UpdateTextureFromView()
{
    if (!m_view)
        return;

    ultralight::Surface *surface = m_view->surface();
    if (!surface)
        return;

    if (surface->dirty_bounds().IsEmpty())
        return;

    auto *bitmapSurface = static_cast<BitmapSurface *>(surface);
    ultralight::RefPtr<ultralight::Bitmap> bitmap = bitmapSurface->bitmap();
    if (!bitmap || bitmap->IsEmpty())
        return;

    const uint32_t width = bitmap->width();
    const uint32_t height = bitmap->height();
    const uint32_t rowBytes = bitmap->row_bytes();

    EnsureTexture(width, height);

    const unsigned char *src = static_cast<const unsigned char *>(bitmap->LockPixels());
    unsigned char *dst = m_rgbaBuffer.data();

    for (uint32_t y = 0; y < height; ++y)
    {
        const unsigned char *row = src + (static_cast<size_t>(y) * rowBytes);
        for (uint32_t x = 0; x < width; ++x)
        {
            const size_t srcIndex = static_cast<size_t>(x) * 4;
            const size_t dstIndex = (static_cast<size_t>(y) * width + x) * 4;
            dst[dstIndex + 0] = row[srcIndex + 2];
            dst[dstIndex + 1] = row[srcIndex + 1];
            dst[dstIndex + 2] = row[srcIndex + 0];
            dst[dstIndex + 3] = row[srcIndex + 3];
        }
    }

    bitmap->UnlockPixels();
    surface->ClearDirtyBounds();

    UpdateTexture(m_texture, m_rgbaBuffer.data());
}

std::string UltralightLayer::BuildIndexUrl(const std::string &route) const
{
    std::string filePath = m_basePath;
    if (!filePath.empty() && (filePath.back() != '\\' && filePath.back() != '/'))
    {
        filePath += "/";
    }
    filePath += "ui/dist/index.html";
    for (char &ch : filePath)
    {
        if (ch == '\\')
        {
            ch = '/';
        }
    }

#if defined(_WIN32)
    std::string url = "file:///" + filePath;
#else
    std::string url = "file://" + filePath;
#endif

    if (!route.empty())
    {
        if (route[0] == '#')
            url += route;
        else
            url += "#/" + route;
    }

    return url;
}

std::string UltralightLayer::GetCurrentRoute() const
{
    if (!m_view)
        return "";

    // Evaluate JavaScript to get the current route
    // window.location.hash gives us the current hash
    ultralight::String result = m_view->EvaluateScript("window.location.hash");

    // ultralight::String should have utf8() method
    return result.utf8().data();
}

std::string UltralightLayer::GetAppState(const std::string &key) const
{
    if (!m_view)
        return "";

    // Evaluate JavaScript to get a value from window.vueAppState
    std::string script = "JSON.stringify(window.vueAppState." + key + ")";
    ultralight::String result = m_view->EvaluateScript(script.c_str());
    std::string resultStr = result.utf8().data();

    // Remove surrounding quotes if it's a string value
    if (resultStr.length() >= 2 && resultStr.front() == '"' && resultStr.back() == '"')
    {
        resultStr = resultStr.substr(1, resultStr.length() - 2);
    }

    return resultStr;
}

void UltralightLayer::ExecuteScript(const std::string &script)
{
    if (!m_view)
        return;

    m_view->EvaluateScript(script.c_str());
}
