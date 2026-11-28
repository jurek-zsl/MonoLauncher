// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QWidget>

#include <optional>

class QAbstractItemModel;
class QListWidget;

// Left sidebar of the main window: "All instances" plus one entry per group, with instance counts.
class LibrarySidebar : public QWidget {
    Q_OBJECT

   public:
    explicit LibrarySidebar(QAbstractItemModel* instances, QWidget* parent = nullptr);

    // Rebuilds the list from the instance model, keeping the current selection when possible.
    void refresh();

   signals:
    // nullopt = all instances; "" = ungrouped instances; otherwise a group name.
    void groupSelected(std::optional<QString> group);

   private:
    void onCurrentRowChanged(int row);

    QAbstractItemModel* m_instances;
    QListWidget* m_list;
    std::optional<QString> m_selected;
    bool m_refreshing = false;
};
