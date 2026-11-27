// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QDir>
#include <QList>
#include <QString>
#include <QUrl>

#include <optional>

#include "Result.h"

// Items published as release assets by a theme repository such as PrismLauncher/Themes.
enum class ThemeRepoKind { Theme, Icons, CatPack };

struct ThemeRepoItem {
    QString displayName;
    QString folderName;
    ThemeRepoKind kind;
    QUrl downloadUrl;
};

// Maps an asset name like "Nord-theme.zip" to an item, or nullopt for unrelated assets.
std::optional<ThemeRepoItem> parseThemeRepoAsset(const QString& assetName, const QUrl& downloadUrl);

// Parses a GitHub "release" API response into items, sorted by kind, then by name.
Result<QList<ThemeRepoItem>> parseThemeRepoRelease(const QByteArray& json);

QString themeRepoKindLabel(ThemeRepoKind kind);

bool isThemeRepoItemInstalled(const ThemeRepoItem& item, const QDir& targetDir);
