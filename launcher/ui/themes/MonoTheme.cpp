// SPDX-License-Identifier: GPL-3.0-only
#include "MonoTheme.h"

#include <QFile>
#include <QObject>

QString MonoTheme::id()
{
    return "mono";
}

QString MonoTheme::name()
{
    return QObject::tr("Mono Dark");
}

QString MonoTheme::tooltip()
{
    return QObject::tr("MonoLauncher's default theme");
}

QPalette MonoTheme::colorScheme()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor("#17130f"));
    p.setColor(QPalette::WindowText, QColor("#ede6de"));
    p.setColor(QPalette::Base, QColor("#17130f"));
    p.setColor(QPalette::AlternateBase, QColor("#1a1612"));
    p.setColor(QPalette::ToolTipBase, QColor("#26201b"));
    p.setColor(QPalette::ToolTipText, QColor("#ede6de"));
    p.setColor(QPalette::PlaceholderText, QColor("#8a7d70"));
    p.setColor(QPalette::Text, QColor("#ede6de"));
    p.setColor(QPalette::Button, QColor("#1e1915"));
    p.setColor(QPalette::ButtonText, QColor("#ede6de"));
    p.setColor(QPalette::BrightText, QColor("#e0786a"));
    p.setColor(QPalette::Light, QColor("#332a23"));
    p.setColor(QPalette::Midlight, QColor("#2a231d"));
    p.setColor(QPalette::Mid, QColor("#26201b"));
    p.setColor(QPalette::Dark, QColor("#120f0c"));
    p.setColor(QPalette::Shadow, QColor("#0a0806"));
    p.setColor(QPalette::Link, QColor("#e8b64a"));
    p.setColor(QPalette::LinkVisited, QColor("#d9a43a"));
    p.setColor(QPalette::Highlight, QColor("#3d311f"));
    p.setColor(QPalette::HighlightedText, QColor("#ede6de"));
    p.setColor(QPalette::Accent, QColor("#e8b64a"));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor("#6f6358"));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#6f6358"));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#6f6358"));
    return fadeInactive(p, fadeAmount(), fadeColor());
}

double MonoTheme::fadeAmount()
{
    return 0.5;
}

QColor MonoTheme::fadeColor()
{
    return QColor("#17130f");
}

bool MonoTheme::hasStyleSheet()
{
    return true;
}

QString MonoTheme::appStyleSheet()
{
    QFile file(":/themes/mono/mono.qss");
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return QString::fromUtf8(file.readAll());
}
