# Versioning Guide

## Semantic Versioning

uvc2gl follows [Semantic Versioning 2.0.0](https://semver.org/):

```
MAJOR.MINOR.PATCH[-PRERELEASE]
```

- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)
- **PRERELEASE**: Optional suffix for unstable builds

## Version Locations

The version is defined in **two places** that must stay in sync:

1. **`src/core/Version.h`** - Source code constants (authoritative)
2. **`CMakeLists.txt`** - CMake project version (line 2)

## Branch Strategy

### main branch (stable releases)
```cpp
constexpr const char* VERSION = "1.0.0";
constexpr const char* VERSION_PRERELEASE = "";
```

### dev branch (development)
```cpp
constexpr const char* VERSION = "1.1.0-dev";
constexpr const char* VERSION_PRERELEASE = "dev";
```

### Release workflow
```bash
# On dev branch - working towards 1.1.0
VERSION = "1.1.0-dev"

# Ready for release - create release branch
git checkout -b release/1.1.0
# Update Version.h: VERSION = "1.1.0-rc1"

# After testing - merge to main
git checkout main
git merge release/1.1.0
# Update Version.h: VERSION = "1.1.0", PRERELEASE = ""
git tag v1.1.0

# Bump dev version
git checkout dev
# Update Version.h: VERSION = "1.2.0-dev"
```

## Release Checklist

1. Update `src/core/Version.h`:
   - Set `VERSION` to release version (e.g., "1.1.0")
   - Set `VERSION_PRERELEASE` to `""`
   - Update `VERSION_MAJOR/MINOR/PATCH`

2. Update `CMakeLists.txt`:
   - Update `project(uvc2gl VERSION x.y.z)`

3. Update `README.md` if needed

4. Commit: `git commit -m "chore: bump version to 1.1.0"`

5. Tag: `git tag -a v1.1.0 -m "Release v1.1.0"`

6. Push: `git push origin main --tags`

## Pre-release Suffixes

- `-dev`: Active development
- `-alpha`: Early testing (feature incomplete)
- `-beta`: Feature complete, testing needed
- `-rc1`: Release candidate (stable, final testing)

## Checking Version

```bash
# Command line
./uvc2gl --version
./uvc2gl -v

# In UI (right-click menu, bottom)
uvc2gl v1.0.0-dev
```

## Binary Releases

The `VERSION` file is **not** included in binary releases. The version is:
- Embedded in the compiled binary via `Version.h`
- Shown with `--version` flag
- Displayed in the UI

For GitHub releases, name the release: `v1.0.0` and the binary: `uvc2gl-1.0.0-linux-x86_64`
