// SPDX-License-Identifier: GPL-3.0-only
#include "ThemeRepository.h"

#include "MMCZip.h"
#include "archive/ArchiveReader.h"

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

static QString zipPrefix(ThemeRepoKind kind)
{
    switch (kind) {
        case ThemeRepoKind::Theme:
            return "themes/";
        case ThemeRepoKind::Icons:
            return "icons/";
        case ThemeRepoKind::CatPack:
            return "cats/";
    }
    return {};
}

// Finds the single item folder under prefix ("themes/Nord/…" -> "Nord").
static Result<QString> findItemFolder(const QString& zipPath, const QString& prefix)
{
    MMCZip::ArchiveReader reader(zipPath);
    TRY(reader.collectFiles(true))

    QString folder;
    for (const QString& path : reader.getFiles()) {
        if (!path.startsWith(prefix))
            continue;
        const QString rest = path.mid(prefix.size());
        const QString first = rest.section('/', 0, 0);
        if (!rest.contains('/') || first.isEmpty() || first == "." || first == "..")
            return std::unexpected{ QCoreApplication::translate("ThemeRepository", "Unexpected file in archive: %1").arg(path) };
        if (!folder.isEmpty() && folder != first)
            return std::unexpected{ QCoreApplication::translate("ThemeRepository", "Archive contains more than one item.") };
        folder = first;
    }
    if (folder.isEmpty())
        return std::unexpected{ QCoreApplication::translate("ThemeRepository", "Archive has no %1 folder.").arg(prefix) };
    return folder;
}

Result<QString> installThemeRepoZip(const QString& zipPath, ThemeRepoKind kind, const QDir& targetDir)
{
    const QString prefix = zipPrefix(kind);
    TRY_INTO(const QString folder, findItemFolder(zipPath, prefix))

    // The archive writer refuses to extract through symlinks, so resolve them in the target path first.
    const QString canonical = targetDir.canonicalPath();
    const QDir target(canonical.isEmpty() ? targetDir.absolutePath() : canonical);
    if (target.exists(folder))
        return std::unexpected{ QCoreApplication::translate("ThemeRepository", "%1 is already installed.").arg(folder) };

    TRY(MMCZip::extractDir(zipPath, prefix, target.absolutePath()))
    return folder;
}
