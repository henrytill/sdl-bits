#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL.h>
#include <SDL_ttf.h>

static void log_freetype_version(void) {
    FT_Library library = NULL;
    FT_Int version_major;
    FT_Int version_minor;
    FT_Int version_patch;
    int error;

    error = FT_Init_FreeType(&library);
    if (error != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize FreeType");
        return;
    }
    FT_Library_Version(library, &version_major, &version_minor, &version_patch);
    SDL_LogInfo(SDL_LOG_CATEGORY_TEST,
                "We are linking against FreeType version %u.%u.%u\n",
                version_major,
                version_minor,
                version_patch);
    FT_Done_FreeType(library);
}

static void log_sdl_version(void) {
    SDL_version sdl_version_compiled;
    SDL_version sdl_version_linked;

    SDL_VERSION(&sdl_version_compiled);
    SDL_GetVersion(&sdl_version_linked);
    SDL_LogInfo(SDL_LOG_CATEGORY_TEST,
                "We compiled against SDL version %u.%u.%u ...\n",
                sdl_version_compiled.major,
                sdl_version_compiled.minor,
                sdl_version_compiled.patch);
    SDL_LogInfo(SDL_LOG_CATEGORY_TEST,
                "... and we are linking against SDL version %u.%u.%u.\n",
                sdl_version_linked.major,
                sdl_version_linked.minor,
                sdl_version_linked.patch);
}

static void log_sdl_ttf_version(void) {
    SDL_version sdl_ttf_version_compiled;

    SDL_TTF_VERSION(&sdl_ttf_version_compiled);
    const SDL_version *sdl_ttf_version_linked = TTF_Linked_Version();
    SDL_LogInfo(SDL_LOG_CATEGORY_TEST,
                "We compiled against SDL_ttf version %u.%u.%u ...\n",
                sdl_ttf_version_compiled.major,
                sdl_ttf_version_compiled.minor,
                sdl_ttf_version_compiled.patch);
    SDL_LogInfo(SDL_LOG_CATEGORY_TEST,
                "... and we are linking against SDL_ttf version %u.%u.%u.\n",
                sdl_ttf_version_linked->major,
                sdl_ttf_version_linked->minor,
                sdl_ttf_version_linked->patch);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    log_freetype_version();
    log_sdl_version();
    log_sdl_ttf_version();

    return EXIT_SUCCESS;
}
