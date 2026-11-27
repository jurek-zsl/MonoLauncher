# Theme Downloader Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A "Get more themes" box in Settings → Appearance that lists items from the PrismLauncher/Themes latest release and installs and selects one with a single Download click.

**Architecture:** Pure logic (release JSON → item list, zip → installed folder) lives in `launcher/ui/themes/ThemeRepository.{h,cpp}` inside `Launcher_logic` and is unit-tested. A small `ThemeDownloadWidget` does the network calls and renders rows. `AppearanceWidget` embeds it (full settings page only), and on install refreshes `ThemeManager` and selects the new item in the right dropdown.

**Tech Stack:** C++20, Qt 6 Widgets, the launcher's `Net::Request` / `Net::ApiRequest`, `MMCZip::extractDir`, Qt Test via `tests/CMakeLists.txt`.

**Spec:** `docs/superpowers/specs/2026-11-27-theme-downloader-design.md`

## Global Constraints

- The list source is `https://api.github.com/repos/<Launcher_THEMES_REPO>/releases/latest`; `Launcher_THEMES_REPO` is a CMake cache STRING defaulting to `PrismLauncher/Themes`, exposed as `BuildConfig.THEMES_REPO`.
- Asset suffix → kind → zip prefix → target folder:
  `-theme.zip` → Theme → `themes/` → `ThemeManager::getApplicationThemesFolder()`;
  `-icons.zip` → Icons → `icons/` → `getIconThemesFolder()`;
  `-catpack.zip` → CatPack → `cats/` → `getCatPacksFolder()`.
- Theme, icon theme and cat pack IDs are their folder name (`CustomTheme` uses `dir().dirName()`, `IconTheme(dir.dirName(), …)`, `JsonCatPack(manifestInfo.dir().dirName())`).
- UI copy: group title "Get more themes"; type labels "Theme", "Icons", "Cat pack"; buttons "Download", "Downloading…", "Installed", "Retry".
- No previews, search, update or uninstall. Never block the UI thread on network I/O.
- Build: `JAVA_HOME=$(/usr/libexec/java_home -v 17) cmake --build build --config Debug`; tests: `ctest --test-dir build -C Debug -R ThemeRepository --output-on-failure`.

## Review Focus

- Offline or GitHub API rate limit (HTTP 403): status line shows an error and a Retry button; nothing crashes. (Task 3, manual check)
- Double-clicking Download or starting a second download: the row's button is disabled while its download runs. (Task 3)
- Zip without the expected `themes/` / `icons/` / `cats/` top folder: install fails with a message and writes nothing. (Task 2 test)
- Asset name that doesn't match the real folder name (dots vs spaces): selection uses the folder actually extracted, not the guessed one. (Task 2 test)
- Leaving the settings page during a download: callbacks are bound to the widget, so a destroyed widget gets no callback. (Task 3)

---

### Task 1: Parse the release into items

**Files:**
- Create: `launcher/ui/themes/ThemeRepository.h`, `launcher/ui/themes/ThemeRepository.cpp`
- Modify: `launcher/CMakeLists.txt` (add both files next to the other `ui/themes/` sources in the logic sources list)
- Modify: `CMakeLists.txt` (add `Launcher_THEMES_REPO` cache option near `Launcher_BUG_TRACKER_URL`), `buildconfig/BuildConfig.h` (`QString THEMES_REPO;`), `buildconfig/BuildConfig.cpp.in` (`THEMES_REPO = "@Launcher_THEMES_REPO@";`)
- Create: `tests/ThemeRepository_test.cpp`, `tests/testdata/ThemeRepository/release.json`
- Modify: `tests/CMakeLists.txt` (append `ThemeRepository_test.cpp` to `TEST_SOURCES`)

**Interfaces:**
- Produces:
  ```cpp
  enum class ThemeRepoKind { Theme, Icons, CatPack };
  struct ThemeRepoItem { QString displayName; QString folderName; ThemeRepoKind kind; QUrl downloadUrl; };
  std::optional<ThemeRepoItem> parseThemeRepoAsset(const QString& assetName, const QUrl& downloadUrl);
  Result<QList<ThemeRepoItem>> parseThemeRepoRelease(const QByteArray& json);
  QString themeRepoKindLabel(ThemeRepoKind kind);          // "Theme" / "Icons" / "Cat pack" (tr())
  bool isThemeRepoItemInstalled(const ThemeRepoItem& item, const QDir& targetDir);
  ```
  (`Result` is the launcher's existing `std::expected`-style type used by `MMCZip`.)

- [ ] **Step 1: Write the fixture and failing tests**

`release.json`: `{"tag_name":"x","assets":[ … ]}` with assets (each `{"name":…,"browser_download_url":"https://example.invalid/<name>"}`): `Nord-theme.zip`, `Fluent-Dark-icons.zip`, `Teawie-Prison-catpack.zip`, `Ore.UI.-.Dark.Amethyst-theme.zip`, `Amoled-theme.zip`, `README.md`.

Tests in `ThemeRepositoryTest`:
- `test_parseAsset`: `Nord-theme.zip` → kind Theme, folder `"Nord"`, display `"Nord"`; `Fluent-Dark-icons.zip` → Icons, folder `"Fluent-Dark"`, display `"Fluent Dark"`; `Teawie-Prison-catpack.zip` → CatPack, folder `"Teawie-Prison"`; `Ore.UI.-.Dark.Amethyst-theme.zip` → folder `"Ore UI - Dark Amethyst"`, display `"Ore UI Dark Amethyst"`; `README.md` → `std::nullopt`.
- `test_parseRelease`: 5 items; order is all Themes, then Icons, then CatPacks, names case-insensitively sorted within a kind: `Amoled`, `Nord`, `Ore UI Dark Amethyst`, `Fluent Dark`, `Teawie Prison`; `downloadUrl` of the first is `https://example.invalid/Amoled-theme.zip`.
- `test_parseReleaseInvalid`: `parseThemeRepoRelease("not json")` and `parseThemeRepoRelease("{}")` both return an error.
- `test_installed`: in a `QTemporaryDir`, `isThemeRepoItemInstalled` is false, then true after `mkdir("Nord")`.

- [ ] **Step 2: Run tests, expect build failure** (`ThemeRepository.h` missing).

- [ ] **Step 3: Implement the functions and the CMake/BuildConfig option.**
Rules: folderName = asset name minus suffix with `.` → space; displayName = folderName with `-` → space, then `QString::simplified()`. `parseThemeRepoRelease` errors when the JSON is invalid or has no `assets` array; skips assets `parseThemeRepoAsset` rejects.

- [ ] **Step 4: Build and run** `ctest --test-dir build -C Debug -R ThemeRepository --output-on-failure` → PASS.

- [ ] **Step 5: Commit** `feat(themes): parse theme repository releases`.

### Task 2: Install a downloaded zip

**Files:**
- Modify: `launcher/ui/themes/ThemeRepository.{h,cpp}`
- Create: `tests/testdata/ThemeRepository/Nord-theme.zip`, `bad-layout.zip`, `Dotted-theme.zip`, `escape-theme.zip`
- Modify: `tests/ThemeRepository_test.cpp`

**Interfaces:**
- Consumes: `ThemeRepoKind` (Task 1).
- Produces: `Result<QString> installThemeRepoZip(const QString& zipPath, ThemeRepoKind kind, const QDir& targetDir);` returns the installed folder name (the first path component of the extracted files, relative to `targetDir`).

- [ ] **Step 1: Create fixtures** (from a scratch dir, with Python's `zipfile`):
  `Nord-theme.zip` with `themes/Nord/theme.json` = `{"name":"Nord","colors":{}}`;
  `bad-layout.zip` with `Nord/theme.json` (no `themes/` prefix);
  `Dotted-theme.zip` with `themes/Real Name/theme.json`;
  `escape-theme.zip` with `themes/../../evil.txt`.

- [ ] **Step 2: Write failing tests**
- `test_install`: installing `Nord-theme.zip` as Theme into a temp dir returns `"Nord"` and `<tmp>/Nord/theme.json` exists.
- `test_installActualFolderName`: `Dotted-theme.zip` returns `"Real Name"`.
- `test_installBadLayout`: `bad-layout.zip` returns an error and the temp dir stays empty.
- `test_installEscape`: `escape-theme.zip` returns an error, and no `evil.txt` exists in the temp dir or its parent.

- [ ] **Step 3: Run, expect failure** (`installThemeRepoZip` undefined).

- [ ] **Step 4: Implement** with `MMCZip::extractDir(zipPath, prefix, targetDir.absolutePath())`; prefix per Global Constraints. An empty extracted list → error `"Archive has no %1 folder"` (prefix). Relies on `extractSubDir`'s existing outside-target check and cleanup.

- [ ] **Step 5: Run** the ctest command → PASS.

- [ ] **Step 6: Commit** `feat(themes): install theme repository zips`.

### Task 3: ThemeDownloadWidget

**Files:**
- Create: `launcher/ui/widgets/ThemeDownloadWidget.h`, `launcher/ui/widgets/ThemeDownloadWidget.cpp`
- Modify: `launcher/CMakeLists.txt` (add to the UI sources next to `AppearanceWidget`)

**Interfaces:**
- Consumes: everything from Tasks 1–2; `APPLICATION->network()`, `APPLICATION->themeManager()`, `Net::ApiRequest::makeByteArray`, `Net::Request::makeFile`.
- Produces:
  ```cpp
  class ThemeDownloadWidget : public QGroupBox {
      Q_OBJECT
  public:
      explicit ThemeDownloadWidget(QWidget* parent = nullptr);
      void refreshInstalledState();
  signals:
      void itemInstalled(ThemeRepoKind kind, const QString& id);
  };
  QDir themeRepoTargetDir(ThemeRepoKind kind);   // maps kind → ThemeManager folder
  ```

- [ ] **Step 1: Implement the widget.**
Layout: group box titled "Get more themes"; a status row (`QLabel` + "Retry" `QPushButton`, both hidden when the list is shown); a `QTableWidget` with 3 columns (name, type label, button), no header, no editing, no grid, rows sorted as returned. On construction, fetch the list with `Net::ApiRequest::makeByteArray(QUrl("https://api.github.com/repos/" + BuildConfig.THEMES_REPO + "/releases/latest"))`, using `this` as the context of every `connect`. Status while loading: "Loading…". On failure: "Couldn't load the theme list: <error>" + Retry. Button per row: "Installed" (disabled) when `isThemeRepoItemInstalled(item, themeRepoTargetDir(kind))`, else "Download". On click: disable, show "Downloading…", download with `Net::Request::makeFile` into a member `QTemporaryDir`, then `installThemeRepoZip`; success → emit `itemInstalled(kind, folder)` and call `refreshInstalledState()`; failure → `QMessageBox::warning` with the error and re-enable "Download".

- [ ] **Step 2: Build** the `monolauncher` target → compiles with no new warnings.

- [ ] **Step 3: Commit** `feat(themes): add theme download widget`.

### Task 4: Embed in Appearance settings and select on install

**Files:**
- Modify: `launcher/ui/widgets/AppearanceWidget.{h,cpp}`

**Interfaces:**
- Consumes: `ThemeDownloadWidget`, `itemInstalled(ThemeRepoKind, QString)`.

- [ ] **Step 1: Implement.**
In the constructor, only when `themesOnly` is false (the full settings page, not the setup wizard), create a `ThemeDownloadWidget` and insert it into the widget's top-level layout directly after `themingBox`. On `itemInstalled(kind, id)`: `APPLICATION->themeManager()->refresh()`, `loadThemeSettings()`, then in the combo for that kind (`widgetStyleComboBox` / `iconsComboBox` / `catPackComboBox`) `setCurrentIndex(findData(id))` when found, so the existing `currentIndexChanged` handlers apply it. Also call `refreshInstalledState()` from the existing reload button handler.

- [ ] **Step 2: Build, install and run** (`cmake --install build --config Debug`, `open install/MonoLauncher.app`). Settings → Appearance shows "Get more themes" with rows; download "Nord" → the Theme dropdown switches to Nord and the window recolors; the row shows "Installed". Download "Fluent-Dark" icons → icons change. With Wi-Fi off, reopening settings shows the error and Retry.

- [ ] **Step 3: Run the full test suite** `ctest --test-dir build -C Debug --output-on-failure` → all pass.

- [ ] **Step 4: Commit** `feat(themes): theme downloader in appearance settings`.
