// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QGroupBox>
#include <QList>
#include <QTemporaryDir>

#include "net/Request.h"
#include "ui/themes/ThemeRepository.h"

class QLabel;
class QPushButton;
class QTableWidget;

// Folder in which ThemeManager looks for items of the given kind.
QDir themeRepoTargetDir(ThemeRepoKind kind);

// "Get more themes": lists the theme repository's items and installs one per Download click.
class ThemeDownloadWidget : public QGroupBox {
    Q_OBJECT

   public:
    explicit ThemeDownloadWidget(QWidget* parent = nullptr);

    // Re-checks which rows are already installed and updates their buttons.
    void refreshInstalledState();

   signals:
    void itemInstalled(ThemeRepoKind kind, const QString& id);

   private:
    void loadList();
    void showStatus(const QString& text, bool showRetry);
    void populate();
    void setRowButton(int row);
    void download(int row);

    QLabel* m_status;
    QPushButton* m_retry;
    QTableWidget* m_table;
    QList<ThemeRepoItem> m_items;
    QList<int> m_downloadingRows;
    Net::Request::Ptr m_listRequest;
    QList<Net::Request::Ptr> m_downloads;
    QTemporaryDir m_tempDir;
};
