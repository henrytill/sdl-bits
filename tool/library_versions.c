#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <SDL.h>
#include <SDL_ttf.h>

enum {
    UNHANDLED = SDL_LOG_CATEGORY_CUSTOM
};

static void log_freetype_version(void) {
    FT_Library library = NULL;
    FT_Int version_major;
    FT_Int version_minor;
    FT_Int version_patch;

    int error = FT_Init_FreeType(&library);
    if (error != 0) {
        SDL_LogError(UNHANDLED, "Failed to initialize FreeType");
        return;
    }
    FT_Library_Version(library, &version_major, &version_minor, &version_patch);
    SDL_LogInfo(UNHANDLED,
                "We are linking against FreeType version %u.%u.%u\n",
                version_major,
                version_minor,
                version_patch);
    FT_Done_FreeType(library);
}

static void log_sdl_version(void) {
    SDL_version version_compiled;
    SDL_version version_linked;

    SDL_VERSION(&version_compiled);
    SDL_GetVersion(&version_linked);
    SDL_LogInfo(UNHANDLED,
                "We compiled against SDL version %u.%u.%u ...\n",
                version_compiled.major,
                version_compiled.minor,
                version_compiled.patch);
    SDL_LogInfo(UNHANDLED,
                "... and we are linking against SDL version %u.%u.%u.\n",
                version_linked.major,
                version_linked.minor,
                version_linked.patch);
}

static void log_sdl_ttf_version(void) {
    SDL_version version_compiled;

    SDL_TTF_VERSION(&version_compiled);
    const SDL_version *version_linked = TTF_Linked_Version();
    SDL_LogInfo(UNHANDLED,
                "We compiled against SDL_ttf version %u.%u.%u ...\n",
                version_compiled.major,
                version_compiled.minor,
                version_compiled.patch);
    SDL_LogInfo(UNHANDLED,
                "... and we are linking against SDL_ttf version %u.%u.%u.\n",
                version_linked->major,
                version_linked->minor,
                version_linked->patch);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

    log_freetype_version();
    log_sdl_version();
    log_sdl_ttf_version();
    return EXIT_SUCCESS;
}
