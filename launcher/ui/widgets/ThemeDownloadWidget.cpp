// SPDX-License-Identifier: GPL-3.0-only
#include "ThemeDownloadWidget.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "Application.h"
#include "BuildConfig.h"
#include "net/ApiRequest.h"
#include "ui/themes/ThemeManager.h"

QDir themeRepoTargetDir(ThemeRepoKind kind)
{
    auto* themes = APPLICATION->themeManager();
    switch (kind) {
        case ThemeRepoKind::Theme:
            return themes->getApplicationThemesFolder();
        case ThemeRepoKind::Icons:
            return themes->getIconThemesFolder();
        case ThemeRepoKind::CatPack:
            return themes->getCatPacksFolder();
    }
    return {};
}

ThemeDownloadWidget::ThemeDownloadWidget(QWidget* parent) : QGroupBox(tr("Get more themes"), parent)
{
    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_retry = new QPushButton(tr("Retry"), this);
    connect(m_retry, &QPushButton::clicked, this, &ThemeDownloadWidget::loadList);

    auto* statusRow = new QHBoxLayout;
    statusRow->addWidget(m_status, 1);
    statusRow->addWidget(m_retry);

    m_table = new QTableWidget(0, 3, this);
    m_table->horizontalHeader()->hide();
    m_table->verticalHeader()->hide();
    m_table->setShowGrid(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->setMinimumHeight(240);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(statusRow);
    layout->addWidget(m_table);

    loadList();
}

void ThemeDownloadWidget::showStatus(const QString& text, bool showRetry)
{
    m_status->setText(text);
    m_status->setVisible(!text.isEmpty());
    m_retry->setVisible(showRetry);
}

void ThemeDownloadWidget::loadList()
{
    showStatus(tr("Loading…"), false);
    m_table->setVisible(false);

    const QUrl url(QString("https://api.github.com/repos/%1/releases/latest").arg(BuildConfig.THEMES_REPO));
    auto [request, response] = Net::ApiRequest::makeByteArray(url);
    m_listRequest = request;
    auto* req = request.get();
    connect(req, &Task::finished, this, [this, req, response] {
        if (req != m_listRequest.get())
            return;  // superseded by a retry
        const bool ok = req->wasSuccessful();
        const QString failReason = req->failReason();
        const QByteArray body = *response;
        m_listRequest.reset();  // response is owned by the request
        if (!ok) {
            showStatus(tr("Couldn't load the theme list: %1").arg(failReason), true);
            return;
        }
        auto items = parseThemeRepoRelease(body);
        if (!items) {
            showStatus(tr("Couldn't load the theme list: %1").arg(items.error()), true);
            return;
        }
        m_items = *items;
        populate();
    });
    request->setNetwork(APPLICATION->network());
    request->start();
}

void ThemeDownloadWidget::populate()
{
    m_downloadingRows.clear();
    m_table->setRowCount(m_items.size());
    for (int row = 0; row < m_items.size(); ++row) {
        const auto& item = m_items[row];
        m_table->setItem(row, 0, new QTableWidgetItem(item.displayName));
        m_table->setItem(row, 1, new QTableWidgetItem(themeRepoKindLabel(item.kind)));
        setRowButton(row);
    }
    showStatus(m_items.isEmpty() ? tr("No themes available.") : QString(), false);
    m_table->setVisible(!m_items.isEmpty());
}

void ThemeDownloadWidget::setRowButton(int row)
{
    const auto& item = m_items[row];
    auto* button = new QPushButton(m_table);
    if (m_downloadingRows.contains(row)) {
        button->setText(tr("Downloading…"));
        button->setEnabled(false);
    } else if (isThemeRepoItemInstalled(item, themeRepoTargetDir(item.kind))) {
        button->setText(tr("Installed"));
        button->setEnabled(false);
    } else {
        button->setText(tr("Download"));
        connect(button, &QPushButton::clicked, this, [this, row] { download(row); });
    }
    m_table->setCellWidget(row, 2, button);
}

void ThemeDownloadWidget::refreshInstalledState()
{
    for (int row = 0; row < m_items.size(); ++row)
        setRowButton(row);
}

void ThemeDownloadWidget::download(int row)
{
    if (row < 0 || row >= m_items.size() || m_downloadingRows.contains(row) || !m_tempDir.isValid())
        return;
    const ThemeRepoItem item = m_items[row];
    m_downloadingRows.append(row);
    setRowButton(row);

    const QString zipPath = m_tempDir.filePath(QString("%1-%2.zip").arg(static_cast<int>(item.kind)).arg(row));
    auto request = Net::Request::makeFile(item.downloadUrl, zipPath);
    m_downloads.append(request);
    auto* req = request.get();
    connect(req, &Task::finished, this, [this, req, row, item, zipPath] {
        m_downloadingRows.removeAll(row);
        Result<QString> installed = req->wasSuccessful() ? installThemeRepoZip(zipPath, item.kind, themeRepoTargetDir(item.kind))
                                                         : Result<QString>(std::unexpected{ req->failReason() });
        m_downloads.removeIf([req](const Net::Request::Ptr& p) { return p.get() == req; });
        QFile::remove(zipPath);
        setRowButton(row);
        if (!installed) {
            QMessageBox::warning(this, tr("Download failed"),
                                 tr("Couldn't install %1: %2").arg(item.displayName, installed.error()));
            return;
        }
        emit itemInstalled(item.kind, *installed);
        refreshInstalledState();
    });
    request->setNetwork(APPLICATION->network());
    request->start();
}
