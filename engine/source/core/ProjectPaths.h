#pragma once

#include <filesystem>
#include <system_error>

namespace eng::ProjectPaths
{
    inline std::filesystem::path ResolveResource(const std::filesystem::path& requested)
    {
        std::error_code error;
        if (requested.empty() || requested.is_absolute() ||
            std::filesystem::exists(requested, error))
        {
            return requested;
        }

        std::filesystem::path projectRelative;
        bool atStart = true;
        for (const auto& part : requested)
        {
            if (atStart && (part == "." || part == ".."))
            {
                continue;
            }
            atStart = false;
            projectRelative /= part;
        }

        std::filesystem::path directory = std::filesystem::current_path(error);
        for (int depth = 0; !error && depth < 8; ++depth)
        {
            const std::filesystem::path candidate = directory / projectRelative;
            if (std::filesystem::exists(candidate, error))
            {
                return candidate.lexically_normal();
            }

            const std::filesystem::path parent = directory.parent_path();
            if (parent == directory)
            {
                break;
            }
            directory = parent;
        }

        return requested;
    }
}
