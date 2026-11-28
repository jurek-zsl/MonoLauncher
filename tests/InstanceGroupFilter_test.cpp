#include <QTest>

#include <QStandardItemModel>

#include "ui/instanceview/InstanceProxyModel.h"
#include "ui/instanceview/InstanceView.h"

class InstanceGroupFilterTest : public QObject {
    Q_OBJECT

    static QStandardItemModel* makeModel(QObject* parent)
    {
        auto* model = new QStandardItemModel(parent);
        const QList<std::pair<QString, QString>> rows{ { "Jungle Survival", "Modded" },
                                                       { "Tech Factory", "Modded" },
                                                       { "Vanilla 1.21", "Vanilla" },
                                                       { "Loose", "" } };
        for (const auto& [name, group] : rows) {
            auto* item = new QStandardItem(name);
            item->setData(group, InstanceViewRoles::GroupRole);
            model->appendRow(item);
        }
        return model;
    }

   private slots:
    void test_noFilterShowsAll()
    {
        InstanceProxyModel proxy;
        proxy.setSourceModel(makeModel(&proxy));
        QCOMPARE(proxy.rowCount(), 4);
        QVERIFY(!proxy.hasGroupFilter());
    }

    void test_filterByGroup()
    {
        InstanceProxyModel proxy;
        proxy.setSourceModel(makeModel(&proxy));
        proxy.setGroupFilter("Modded");
        QVERIFY(proxy.hasGroupFilter());
        QCOMPARE(proxy.groupFilter(), QString("Modded"));
        QCOMPARE(proxy.rowCount(), 2);
        proxy.setGroupFilter("Vanilla");
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.index(0, 0).data().toString(), QString("Vanilla 1.21"));
    }

    void test_filterUngrouped()
    {
        InstanceProxyModel proxy;
        proxy.setSourceModel(makeModel(&proxy));
        proxy.setGroupFilter("");
        QCOMPARE(proxy.rowCount(), 1);
        QCOMPARE(proxy.index(0, 0).data().toString(), QString("Loose"));
    }

    void test_clearFilter()
    {
        InstanceProxyModel proxy;
        proxy.setSourceModel(makeModel(&proxy));
        proxy.setGroupFilter("Modded");
        proxy.clearGroupFilter();
        QVERIFY(!proxy.hasGroupFilter());
        QCOMPARE(proxy.rowCount(), 4);
    }
};

QTEST_GUILESS_MAIN(InstanceGroupFilterTest)

#include "InstanceGroupFilter_test.moc"
