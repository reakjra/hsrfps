#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

#define SUBKEY      L"Software\\Cognosphere\\Star Rail"
#define VALUE_NAME  L"GraphicsSettings_Model_h2986158309"
#define FPS_TAG     "\"FPS\":"
#define TARGET_FPS  "120"

/* the game stores this as utf-8 json with one trailing nul byte, and sizeof keeps it */
static const BYTE default_model[] =
    "{\"FPS\":120,\"EnableVSync\":true,\"RenderScale\":1.0,"
    "\"ResolutionQuality\":3,\"ShadowQuality\":3,\"LightQuality\":3,"
    "\"CharacterQuality\":3,\"EnvDetailQuality\":3,\"ReflectionQuality\":3,"
    "\"SFXQuality\":3,\"BloomQuality\":3,\"AAMode\":1,"
    "\"EnableMetalFXSU\":false,\"EnableHalfResTransparent\":false,"
    "\"EnableSelfShadow\":1,\"DlssQuality\":0,\"ParticleTrailSmoothness\":3}";

static void fail(const WCHAR *message)
{
    MessageBoxW(NULL, message, L"hsrfps", MB_OK | MB_ICONERROR);
}

static const BYTE *find_fps_digits(const BYTE *blob, DWORD len)
{
    const DWORD tag_len = sizeof(FPS_TAG) - 1;

    for (DWORD i = 0; i + tag_len <= len; i++)
        if (memcmp(blob + i, FPS_TAG, tag_len) == 0)
            return blob + i + tag_len;

    return NULL;
}

static BYTE *with_target_fps(const BYTE *blob, DWORD len, DWORD *out_len)
{
    const BYTE *digits = find_fps_digits(blob, len);
    if (!digits)
        return NULL;

    const BYTE *rest = digits;
    while (rest < blob + len && *rest >= '0' && *rest <= '9')
        rest++;
    if (rest == digits)
        return NULL;

    const DWORD head = (DWORD)(digits - blob);
    const DWORD tail = len - (DWORD)(rest - blob);
    const DWORD fps_len = sizeof(TARGET_FPS) - 1;

    BYTE *out = HeapAlloc(GetProcessHeap(), 0, head + fps_len + tail);
    if (!out)
        return NULL;

    CopyMemory(out, blob, head);
    CopyMemory(out + head, TARGET_FPS, fps_len);
    CopyMemory(out + head + fps_len, rest, tail);
    *out_len = head + fps_len + tail;
    return out;
}

static BYTE *read_model(HKEY key, DWORD *len)
{
    DWORD type = 0, size = 0;

    if (RegQueryValueExW(key, VALUE_NAME, NULL, &type, NULL, &size) != ERROR_SUCCESS)
        return NULL;
    if (type != REG_BINARY || size == 0)
        return NULL;

    BYTE *blob = HeapAlloc(GetProcessHeap(), 0, size);
    if (!blob)
        return NULL;

    if (RegQueryValueExW(key, VALUE_NAME, NULL, &type, blob, &size) != ERROR_SUCCESS) {
        HeapFree(GetProcessHeap(), 0, blob);
        return NULL;
    }

    *len = size;
    return blob;
}

static BOOL apply_target_fps(HKEY key)
{
    DWORD current_len = 0;
    BYTE *current = read_model(key, &current_len);

    DWORD patched_len = 0;
    BYTE *patched = current
        ? with_target_fps(current, current_len, &patched_len)
        : with_target_fps(default_model, sizeof(default_model), &patched_len);

    if (current)
        HeapFree(GetProcessHeap(), 0, current);

    if (!patched) {
        fail(L"Star Rail's graphics settings are not in the expected format.");
        return FALSE;
    }

    const LSTATUS status = RegSetValueExW(key, VALUE_NAME, 0, REG_BINARY, patched, patched_len);
    HeapFree(GetProcessHeap(), 0, patched);

    if (status != ERROR_SUCCESS) {
        fail(L"Could not write Star Rail's graphics settings.");
        return FALSE;
    }

    return TRUE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command_line, int show)
{
    (void)instance;
    (void)previous;
    (void)command_line;
    (void)show;

    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, SUBKEY, 0, NULL, 0,
                        KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &key, NULL) != ERROR_SUCCESS) {
        fail(L"Could not open Star Rail's registry key.");
        return 1;
    }

    const BOOL ok = apply_target_fps(key);
    RegCloseKey(key);
    return ok ? 0 : 1;
}
