// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "FusionTheme.h"

// MonoLauncher's built-in theme: warm dark browns with a banana accent.
// The palette lives here; the stylesheet is :/themes/mono/mono.qss.
class MonoTheme : public FusionTheme {
   public:
    ~MonoTheme() override = default;

    QString id() override;
    QString name() override;
    QString tooltip() override;
    bool hasStyleSheet() override;
    QString appStyleSheet() override;
    QPalette colorScheme() override;
    double fadeAmount() override;
    QColor fadeColor() override;
};
