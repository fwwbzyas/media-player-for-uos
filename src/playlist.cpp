#include "playlist.h"
#include "ui_playlist.h"

#include	<QDebug>
#include <QListWidgetItem>
#include <QAction>
#include <QMenu>
#include <QFileInfo>
#include <QSignalBlocker>
#include <QRandomGenerator>
#include	<QTimer>
#include	<QFile>


PlayList::PlayList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlayList)
{
    ui->setupUi(this);

    auto *removeAction = new QAction(tr("从列表移除"), this);
    removeAction->setShortcut(QKeySequence(Qt::Key_Delete));
    removeAction->setShortcutContext(Qt::WidgetShortcut);
    ui->listWidget->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, [this] { remove(); });
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QWidget::customContextMenuRequested, this,
            [this, removeAction](const QPoint &position) {
        auto *item = ui->listWidget->itemAt(position);
        if (!item)
            return;
        ui->listWidget->setCurrentItem(item);
        QMenu menu(this);
        menu.addAction(removeAction);
        menu.exec(ui->listWidget->viewport()->mapToGlobal(position));
    });
    auto *missingFileTimer = new QTimer(this);
    missingFileTimer->setInterval(2000);
    connect(missingFileTimer, &QTimer::timeout, this, &PlayList::removeMissingFiles);
    missingFileTimer->start();

    connect(ui->listWidget, &QListWidget::currentRowChanged, [&](int row) {
        Q_UNUSED(row);
        emit selectedChanged(current());
    });
}

PlayList::~PlayList()
{
    delete ui;
}

void PlayList::next() {
    if (ui->listWidget->count() == 0)
        return;

    int _row = ui->listWidget->currentRow();

    if (!loop) {
        if (shuffle)
            _row = QRandomGenerator::system()->generate() % ui->listWidget->count();
        else if (_row == -1 ||_row == ui->listWidget->count() - 1)
            _row = 0;
        else
            _row++;
    }

    ui->listWidget->setCurrentRow(_row);
}

void PlayList::prev() {
    int _row = ui->listWidget->currentRow();
    if (_row == -1 || ui->listWidget->count() == 0)
        return;
    if (_row == 0)
        _row = ui->listWidget->count() - 1;
    else
        _row--;
    ui->listWidget->setCurrentRow(_row);
}

QUrl PlayList::current() {
    if (ui->listWidget->currentRow() == -1)
        return QUrl();
    return ui->listWidget->item(ui->listWidget->currentRow())->data(Qt::UserRole).toUrl();
}

void PlayList::add(QUrl url) {
    if (url.isEmpty() || !url.isValid() ||
        (url.isLocalFile() && !QFileInfo::exists(url.toLocalFile())))
        return;


    QListWidgetItem *item = new QListWidgetItem();
    item->setText(url.fileName());
    item->setData(Qt::UserRole, url.toString());

    if (shuffle) {
        ui->listWidget->insertItem(ui->listWidget->currentRow() + 1, item);
    } else {
        ui->listWidget->addItem(item);
    }
}

int PlayList::count() const {
    return ui->listWidget->count();
}

QUrl PlayList::remove() {
    const int row = ui->listWidget->currentRow();
    if (row < 0)
        return {};
    const QUrl removed = current();
    {
        const QSignalBlocker blocker(ui->listWidget);
        delete ui->listWidget->takeItem(row);
        ui->listWidget->setCurrentRow(-1);
    }
    emit selectedChanged(QUrl());
    return removed;
}

void PlayList::removeMissingFiles() {
    const QUrl before = current();
    {
        const QSignalBlocker blocker(ui->listWidget);
        bool removedCurrent = false;
        for (int row = count() - 1; row >= 0; --row) {
            const QUrl url = ui->listWidget->item(row)->data(Qt::UserRole).toUrl();
            if (url.isLocalFile() && !QFileInfo::exists(url.toLocalFile())) {
                removedCurrent |= row == ui->listWidget->currentRow();
                delete ui->listWidget->takeItem(row);
            }
        }
        if (removedCurrent)
            ui->listWidget->setCurrentRow(-1);
    }
    if (before != current())
        emit selectedChanged(current());
}
