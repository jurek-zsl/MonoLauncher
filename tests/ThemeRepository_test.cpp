#include <QTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "ui/themes/ThemeRepository.h"

class ThemeRepositoryTest : public QObject {
    Q_OBJECT

    static QByteArray readTestData(const QString& name)
    {
        QFile f(QFINDTESTDATA("testdata/ThemeRepository/" + name));
        if (!f.open(QIODevice::ReadOnly))
            return {};
        return f.readAll();
    }

   private slots:
    void test_parseAsset()
    {
        const QUrl url("https://example.invalid/x.zip");

        auto nord = parseThemeRepoAsset("Nord-theme.zip", url);
        QVERIFY(nord.has_value());
        QCOMPARE(nord->kind, ThemeRepoKind::Theme);
        QCOMPARE(nord->folderName, QString("Nord"));
        QCOMPARE(nord->displayName, QString("Nord"));
        QCOMPARE(nord->downloadUrl, url);

        auto fluent = parseThemeRepoAsset("Fluent-Dark-icons.zip", url);
        QVERIFY(fluent.has_value());
        QCOMPARE(fluent->kind, ThemeRepoKind::Icons);
        QCOMPARE(fluent->folderName, QString("Fluent-Dark"));
        QCOMPARE(fluent->displayName, QString("Fluent Dark"));

        auto teawie = parseThemeRepoAsset("Teawie-Prison-catpack.zip", url);
        QVERIFY(teawie.has_value());
        QCOMPARE(teawie->kind, ThemeRepoKind::CatPack);
        QCOMPARE(teawie->folderName, QString("Teawie-Prison"));

        auto ore = parseThemeRepoAsset("Ore.UI.-.Dark.Amethyst-theme.zip", url);
        QVERIFY(ore.has_value());
        QCOMPARE(ore->folderName, QString("Ore UI - Dark Amethyst"));
        QCOMPARE(ore->displayName, QString("Ore UI Dark Amethyst"));

        QVERIFY(!parseThemeRepoAsset("README.md", url).has_value());
    }

    void test_parseRelease()
    {
        auto items = parseThemeRepoRelease(readTestData("release.json"));
        if (!items)
            QFAIL(items.error().toLatin1());
        QStringList names;
        for (const auto& item : *items)
            names << item.displayName;
        QCOMPARE(names, (QStringList{ "Amoled", "Nord", "Ore UI Dark Amethyst", "Fluent Dark", "Teawie Prison" }));
        QCOMPARE(items->first().downloadUrl, QUrl("https://example.invalid/Amoled-theme.zip"));
    }

    void test_parseReleaseInvalid()
    {
        QVERIFY(!parseThemeRepoRelease("not json").has_value());
        QVERIFY(!parseThemeRepoRelease("{}").has_value());
    }

    void test_installed()
    {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        QDir dir(tmp.path());
        auto nord = parseThemeRepoAsset("Nord-theme.zip", QUrl("https://example.invalid/Nord-theme.zip"));
        QVERIFY(nord.has_value());
        QVERIFY(!isThemeRepoItemInstalled(*nord, dir));
        QVERIFY(dir.mkdir("Nord"));
        QVERIFY(isThemeRepoItemInstalled(*nord, dir));
    }
    void test_install()
    {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        auto result = installThemeRepoZip(QFINDTESTDATA("testdata/ThemeRepository/Nord-theme.zip"), ThemeRepoKind::Theme, QDir(tmp.path()));
        if (!result)
            QFAIL(result.error().toLatin1());
        QCOMPARE(*result, QString("Nord"));
        QVERIFY(QFile::exists(QDir(tmp.path()).filePath("Nord/theme.json")));
    }

    void test_installActualFolderName()
    {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        auto result = installThemeRepoZip(QFINDTESTDATA("testdata/ThemeRepository/Dotted-theme.zip"), ThemeRepoKind::Theme, QDir(tmp.path()));
        if (!result)
            QFAIL(result.error().toLatin1());
        QCOMPARE(*result, QString("Real Name"));
    }

    void test_installBadLayout()
    {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        auto result = installThemeRepoZip(QFINDTESTDATA("testdata/ThemeRepository/bad-layout.zip"), ThemeRepoKind::Theme, QDir(tmp.path()));
        QVERIFY(!result.has_value());
        QVERIFY(QDir(tmp.path()).isEmpty());
    }

    void test_installEscape()
    {
        QTemporaryDir outer;
        QVERIFY(outer.isValid());
        QDir outerDir(outer.path());
        QVERIFY(outerDir.mkpath("a/themes"));
        QDir target(outerDir.filePath("a/themes"));
        auto result = installThemeRepoZip(QFINDTESTDATA("testdata/ThemeRepository/escape-theme.zip"), ThemeRepoKind::Theme, target);
        QVERIFY(!result.has_value());
        QVERIFY(!QFile::exists(target.filePath("evil.txt")));
        QVERIFY(!QFile::exists(outerDir.filePath("a/evil.txt")));
        QVERIFY(!QFile::exists(outerDir.filePath("evil.txt")));
    }
};

QTEST_GUILESS_MAIN(ThemeRepositoryTest)

#include "ThemeRepository_test.moc"
