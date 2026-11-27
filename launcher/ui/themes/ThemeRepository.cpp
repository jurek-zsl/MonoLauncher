// SPDX-License-Identifier: GPL-3.0-only
#include "ThemeRepository.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <array>
#include <utility>

namespace {
constexpr std::array<std::pair<const char*, ThemeRepoKind>, 3> s_suffixes{ {
    { "-theme.zip", ThemeRepoKind::Theme },
    { "-icons.zip", ThemeRepoKind::Icons },
    { "-catpack.zip", ThemeRepoKind::CatPack },
} };
}  // namespace

std::optional<ThemeRepoItem> parseThemeRepoAsset(const QString& assetName, const QUrl& downloadUrl)
{
    for (const auto& [suffix, kind] : s_suffixes) {
        const QString suffixStr = QString::fromLatin1(suffix);
        if (!assetName.endsWith(suffixStr) || assetName.size() == suffixStr.size())
            continue;
        // GitHub replaces spaces in asset names with dots.
        QString folderName = assetName.chopped(suffixStr.size()).replace('.', ' ');
        QString displayName = QString(folderName).replace('-', ' ').simplified();
        return ThemeRepoItem{ displayName, folderName, kind, downloadUrl };
    }
    return std::nullopt;
}

Result<QList<ThemeRepoItem>> parseThemeRepoRelease(const QByteArray& json)
{
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return std::unexpected{ QCoreApplication::translate("ThemeRepository", "Invalid response from the theme repository.") };
    const auto assets = doc.object().value("assets");
    if (!assets.isArray())
        return std::unexpected{ QCoreApplication::translate("ThemeRepository", "The theme repository release has no files.") };

    QList<ThemeRepoItem> items;
    for (const auto& value : assets.toArray()) {
        const auto asset = value.toObject();
        if (auto item = parseThemeRepoAsset(asset.value("name").toString(), QUrl(asset.value("browser_download_url").toString())))
            items.append(*item);
    }
    std::sort(items.begin(), items.end(), [](const ThemeRepoItem& a, const ThemeRepoItem& b) {
        if (a.kind != b.kind)
            return a.kind < b.kind;
        return a.displayName.compare(b.displayName, Qt::CaseInsensitive) < 0;
    });
    return items;
}

QString themeRepoKindLabel(ThemeRepoKind kind)
{
    switch (kind) {
        case ThemeRepoKind::Theme:
            return QCoreApplication::translate("ThemeRepository", "Theme");
        case ThemeRepoKind::Icons:
            return QCoreApplication::translate("ThemeRepository", "Icons");
        case ThemeRepoKind::CatPack:
            return QCoreApplication::translate("ThemeRepository", "Cat pack");
    }
    return {};
}

bool isThemeRepoItemInstalled(const ThemeRepoItem& item, const QDir& targetDir)
{
    return targetDir.exists(item.folderName);
}
