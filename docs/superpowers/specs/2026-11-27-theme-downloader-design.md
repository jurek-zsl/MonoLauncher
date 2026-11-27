# Theme downloader in Appearance settings — design

Status: approved 2026-11-27

## Intent

Let MonoLauncher users install community themes, icon packs and cat packs from
the [PrismLauncher/Themes](https://github.com/PrismLauncher/Themes) repository
without leaving the launcher. It has to feel built into the existing
customization settings and stay very simple: names and a Download button.

## Goals

- A "Get more themes" box in Settings → Appearance, below the existing
  Theme / Icons / Cat pack dropdowns.
- Each row: display name, type label (Theme / Icons / Cat pack), and a
  Download button. Already-installed items show a disabled "Installed" button.
- Download installs the item, refreshes the dropdowns, and selects the new
  item in its dropdown so it applies immediately.
- A failed list fetch or download shows a short message with Retry.

## Non-goals

Previews, search/filter, updating installed items, uninstalling.

## Anti-goals

- Must not block the UI while fetching or downloading.
- Must not write outside the launcher's `themes/`, `iconthemes/`, `catpacks/` folders.

## Constraints and facts

- The Themes repo publishes every item as a separate asset on its latest
  GitHub Release. Asset names end in `-theme.zip`, `-icons.zip` or
  `-catpack.zip` (e.g. `Nord-theme.zip`, `Fluent-Dark-icons.zip`,
  `Ore.UI.-.Dark.Amethyst-theme.zip` — GitHub turns spaces into dots).
- Zip layouts: `themes/<Name>/…`, `icons/<Name>/…`, `cats/<Name>/…`.
  These map to the launcher's `themes/`, `iconthemes/`, `catpacks/` folders
  (`ThemeManager::getApplicationThemesFolder()`, `getIconThemesFolder()`,
  `getCatPacksFolder()`).
- The launcher already has `MMCZip::extractDir(file, subdir, dir)` and the
  `NetJob` / `Net::Download` network stack.
- The unauthenticated GitHub API allows 60 requests/hour; one request per list load is fine.

## Approaches

1. **Inline box in the Appearance page, list from the latest release's assets (chosen).**
   One API call, one zip per item, matches "built into the existing settings".
2. Separate pop-up dialog: rejected, the user wants it inside the existing settings.
3. Read the repo's file tree and fetch files individually: rejected, many more
   API calls and the files would have to be reassembled.

## Decided

- The list source is `releases/latest` of a configurable repo: new CMake cache
  option `Launcher_THEMES_REPO` (default `PrismLauncher/Themes`) exposed via BuildConfig.
- Display name = asset name minus the suffix, with `.` and `-` shown as spaces.
- "Installed" = a folder with that name exists in the target directory (the
  name is taken from the zip's top-level folder after install; before install,
  matched from the asset-derived name).
- After install: `ThemeManager::refresh()`, repopulate the dropdowns, select the new item.

## Left to the builder

Exact widget layout, where the list-parsing code lives, and the in-row error
presentation, as long as they match the sketch above.
