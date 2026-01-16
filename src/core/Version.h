#pragma once

namespace uvc2gl {

// Semantic Versioning: MAJOR.MINOR.PATCH[-PRERELEASE]
// Update these for each release:
// - main branch: stable versions (1.0.0, 1.1.0, 2.0.0)
// - dev branch: pre-release versions (1.1.0-dev, 2.0.0-alpha)

constexpr const char* VERSION = "1.0.0-dev";
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;
constexpr const char* VERSION_PRERELEASE = "dev";  // Empty string "" for stable releases

} // namespace uvc2gl
