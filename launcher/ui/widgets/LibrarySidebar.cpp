// SPDX-License-Identifier: GPL-3.0-only
#include "LibrarySidebar.h"

#include <QAbstractItemModel>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include "ui/instanceview/InstanceView.h"

namespace {
enum Roles { GroupRole = Qt::UserRole + 1, IsAllRole, CountRole };

// Draws the entry normally, then its instance count right-aligned in a muted color.
class CountDelegate : public QStyledItemDelegate {
   public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        const QString count = QString::number(index.data(CountRole).toInt());
        const int countWidth = opt.fontMetrics.horizontalAdvance(count) + 12;
        opt.rect.setRight(opt.rect.right() - countWidth);
        // background across the full row, text in the shortened rect
        QStyleOptionViewItem bg(option);
        initStyleOption(&bg, index);
        bg.text.clear();
        bg.widget->style()->drawControl(QStyle::CE_ItemViewItem, &bg, painter, bg.widget);
        opt.backgroundBrush = Qt::NoBrush;
        opt.state &= ~QStyle::State_Selected;
        opt.state &= ~QStyle::State_MouseOver;
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        painter->save();
        QColor muted = option.palette.color(QPalette::Text);
        muted.setAlphaF(0.55);
        painter->setPen(muted);
        painter->drawText(option.rect.adjusted(0, 0, -10, 0), Qt::AlignRight | Qt::AlignVCenter, count);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 28));
        return size;
    }
};
}  // namespace

LibrarySidebar::LibrarySidebar(QAbstractItemModel* instances, QWidget* parent) : QWidget(parent), m_instances(instances)
{
    setObjectName("librarySidebar");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(200);

    auto* heading = new QLabel(tr("Groups"), this);
    heading->setObjectName("librarySidebarHeading");

    m_list = new QListWidget(this);
    m_list->setObjectName("librarySidebarList");
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setItemDelegate(new CountDelegate(m_list));
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setAttribute(Qt::WA_MacShowFocusRect, false);
    connect(m_list, &QListWidget::currentRowChanged, this, &LibrarySidebar::onCurrentRowChanged);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 12, 8, 8);
    layout->setSpacing(4);
    layout->addWidget(heading);
    layout->addWidget(m_list, 1);

    connect(m_instances, &QAbstractItemModel::rowsInserted, this, &LibrarySidebar::refresh);
    connect(m_instances, &QAbstractItemModel::rowsRemoved, this, &LibrarySidebar::refresh);
    connect(m_instances, &QAbstractItemModel::modelReset, this, &LibrarySidebar::refresh);
    connect(m_instances, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex&, const QModelIndex&, const QList<int>& roles) {
                if (roles.isEmpty() || roles.contains(InstanceViewRoles::GroupRole))
                    refresh();
            });

    refresh();
}

void LibrarySidebar::refresh()
{
    QMap<QString, int> counts;  // sorted by group name
    int ungrouped = 0;
    const int total = m_instances->rowCount();
    for (int row = 0; row < total; ++row) {
        const QString group = m_instances->index(row, 0).data(InstanceViewRoles::GroupRole).toString();
        if (group.isEmpty())
            ++ungrouped;
        else
            ++counts[group];
    }

    m_refreshing = true;
    m_list->clear();
    auto addEntry = [this](const QString& text, int count, std::optional<QString> group) {
        auto* item = new QListWidgetItem(text, m_list);
        item->setData(IsAllRole, !group.has_value());
        item->setData(GroupRole, group.value_or(QString()));
        item->setData(CountRole, count);
        return item;
    };
    QListWidgetItem* current = addEntry(tr("All instances"), total, std::nullopt);
    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        auto* item = addEntry(it.key(), it.value(), it.key());
        if (m_selected && *m_selected == it.key())
            current = item;
    }
    if (ungrouped > 0 && !counts.isEmpty()) {
        auto* item = addEntry(tr("Ungrouped"), ungrouped, QString());
        if (m_selected && m_selected->isEmpty())
            current = item;
    }
    m_list->setCurrentItem(current);
    m_refreshing = false;

    // The selected group may have disappeared; fall back to all instances.
    const std::optional<QString> now =
        current->data(IsAllRole).toBool() ? std::nullopt : std::optional<QString>(current->data(GroupRole).toString());
    if (now != m_selected) {
        m_selected = now;
        emit groupSelected(m_selected);
    }
}

void LibrarySidebar::onCurrentRowChanged(int row)
{
    if (m_refreshing || row < 0)
        return;
    auto* item = m_list->item(row);
    m_selected = item->data(IsAllRole).toBool() ? std::nullopt : std::optional<QString>(item->data(GroupRole).toString());
    emit groupSelected(m_selected);
}
