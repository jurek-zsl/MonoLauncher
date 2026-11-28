#include <QTest>

#include "ui/themes/MonoTheme.h"

class MonoThemeTest : public QObject {
    Q_OBJECT
   private slots:
    void test_identity()
    {
        MonoTheme theme;
        QCOMPARE(theme.id(), QString("mono"));
        QCOMPARE(theme.name(), QString("Mono Dark"));
        QCOMPARE(theme.qtTheme(), QString("Fusion"));
        QVERIFY(theme.hasStyleSheet());
    }

    void test_palette()
    {
        MonoTheme theme;
        const QPalette p = theme.colorScheme();
        QCOMPARE(p.color(QPalette::Active, QPalette::Window), QColor("#17130f"));
        QCOMPARE(p.color(QPalette::Active, QPalette::Base), QColor("#17130f"));
        QCOMPARE(p.color(QPalette::Active, QPalette::Button), QColor("#1e1915"));
        QCOMPARE(p.color(QPalette::Active, QPalette::Text), QColor("#ede6de"));
        QCOMPARE(p.color(QPalette::Active, QPalette::Highlight), QColor("#3d311f"));
        QCOMPARE(p.color(QPalette::Active, QPalette::HighlightedText), QColor("#ede6de"));
        QCOMPARE(p.color(QPalette::Active, QPalette::Link), QColor("#e8b64a"));
    }
};

QTEST_GUILESS_MAIN(MonoThemeTest)

#include "MonoTheme_test.moc"
