#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QMenu>
#include <QAction>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scanner(nullptr), isScanning(false) {
    
    favManager = new FavoritesManager(this);
    
    setupUI();
    setupStyle();
    
    // Connect favorites changed signal if needed, though we update UI manually often
    connect(favManager, &FavoritesManager::favoritesChanged, this, &MainWindow::updateFavoritesUI);
    
    // Initial Load of Favorites into Table?
    // User Requirement: "On startup: Loads favorites automaticall, Displays them immediately without scanning"
    // However, we don't know the SIZE of favorites unless we scan them?
    // Or we just check if they exist.  The prompt implies they should be listed.
    // We will verify existence and show them.
    
    for (const QString &path : favManager->getFavorites()) {
        QFileInfo info(path);
        if (info.exists() && info.isDir()) {
             // We can do a quick size check or just show "Unknown" until scan?
             // Prompt says "Displays results...". I will do a quick calc on main thread or just 0 for now
             // to keep startup fast.
             addTableDataType(path, 0, true);
        }
    }
}

MainWindow::~MainWindow() {
    if (scanner) {
        scanner->stop();
        scanner->wait();
    }
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Top Bar
    QHBoxLayout *topLayout = new QHBoxLayout();
    pathInput = new QLineEdit(this);
    pathInput->setPlaceholderText("Select directory to scan...");
    browseBtn = new QPushButton("Browse...", this);
    scanBtn = new QPushButton("Scan", this);
    
    // Min size filter
    QLabel *minSizeLabel = new QLabel("Min Size (MB):", this);
    minSizeSpinBox = new QSpinBox(this);
    minSizeSpinBox->setRange(0, 10000);
    minSizeSpinBox->setValue(50);
    minSizeSpinBox->setSuffix(" MB");
    
    topLayout->addWidget(pathInput);
    topLayout->addWidget(browseBtn);
    topLayout->addWidget(minSizeLabel);
    topLayout->addWidget(minSizeSpinBox);
    topLayout->addWidget(scanBtn);
    
    // Center Table
    resultsTable = new QTableWidget(this);
    resultsTable->setColumnCount(3);
    resultsTable->setHorizontalHeaderLabels({"Folder Path", "Size", "Fav"});
    resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    resultsTable->setColumnWidth(1, 100);
    resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    resultsTable->setColumnWidth(2, 50);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Bottom Bar
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    deleteSelectedBtn = new QPushButton("Delete Selected", this);
    deleteAllBtn = new QPushButton("Delete All", this);
    bottomLayout->addStretch();
    bottomLayout->addWidget(deleteSelectedBtn);
    bottomLayout->addWidget(deleteAllBtn);
    
    // Status Bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0); // Indeterminate
    progressBar->setVisible(false);
    statusLabel = new QLabel("Ready", this);
    statusBar()->addPermanentWidget(progressBar);
    statusBar()->addWidget(statusLabel);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(resultsTable);
    mainLayout->addLayout(bottomLayout);

    // Connections
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::browseFolder);
    connect(scanBtn, &QPushButton::clicked, this, &MainWindow::startScan);
    connect(resultsTable, &QTableWidget::cellClicked, this, &MainWindow::toggleFavorite);
    connect(resultsTable, &QTableWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(deleteSelectedBtn, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    connect(deleteAllBtn, &QPushButton::clicked, this, &MainWindow::deleteAll);
}

void MainWindow::setupStyle() {
    // Dark Theme
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette p = qApp->palette();
    p.setColor(QPalette::Window, QColor(53, 53, 53));
    p.setColor(QPalette::WindowText, Qt::white);
    p.setColor(QPalette::Base, QColor(25, 25, 25));
    p.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    p.setColor(QPalette::ToolTipBase, Qt::white);
    p.setColor(QPalette::ToolTipText, Qt::white);
    p.setColor(QPalette::Text, Qt::white);
    p.setColor(QPalette::Button, QColor(53, 53, 53));
    p.setColor(QPalette::ButtonText, Qt::white);
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Link, QColor(42, 130, 218));
    p.setColor(QPalette::Highlight, QColor(42, 130, 218));
    p.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(p);

    // Additional Stylesheet
    QString style = R"(
        QMainWindow { background-color: #353535; }
        QPushButton {
            background-color: #454545;
            border: 1px solid #606060;
            border-radius: 4px;
            padding: 5px;
            color: white;
        }
        QPushButton:hover { background-color: #555555; }
        QPushButton:pressed { background-color: #252525; }
        QLineEdit {
            background-color: #252525;
            border: 1px solid #606060;
            border-radius: 4px;
            color: white;
            padding: 5px;
        }
        QTableWidget {
            background-color: #252525;
            gridline-color: #353535;
            color: white;
            border: none;
        }
        QHeaderView::section {
            background-color: #353535;
            color: white;
            padding: 4px;
            border: 1px solid #606060;
        }
    )";
    this->setStyleSheet(style);
}

void MainWindow::browseFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        pathInput->setText(dir);
    }
}

void MainWindow::startScan() {
    if (isScanning) {
        // Stop logic
        if (scanner) scanner->stop();
        scanBtn->setText("Scan");
        return;
    }

    QString path = pathInput->text();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please select a directory first.");
        return;
    }

    resultsTable->setRowCount(0);
    // Re-populate favorites
    for (const QString &fav : favManager->getFavorites()) {
        if (QFileInfo::exists(fav)) {
           // For simplicity in this logic, we clear and re-add favorites 
           // In a smarter app we might merge, but let's keep it simple.
           addTableDataType(fav, 0, true);
        }
    }

    scanBtn->setText("Stop");
    isScanning = true;
    progressBar->setVisible(true);
    statusLabel->setText("Scanning...");

    if (scanner) {
        scanner->deleteLater();
    }
    
    // Get min size in bytes from spinbox
    quint64 minSizeBytes = static_cast<quint64>(minSizeSpinBox->value()) * 1024ULL * 1024ULL;
    scanner = new CacheScanner(path, minSizeBytes, this);
    
    connect(scanner, &CacheScanner::progress, this, &MainWindow::onScanProgress);
    connect(scanner, &CacheScanner::cacheFound, this, &MainWindow::onCacheFound);
    connect(scanner, &CacheScanner::scanFinished, this, &MainWindow::onScanFinished);
    
    scanner->start();
}

void MainWindow::onScanProgress(const QString &path) {
    statusLabel->setText("Scanning: " + path);
}

void MainWindow::onCacheFound(CacheFolderInfo info) {
    // Check if already in table (e.g. from favorites) to update size/avoid dups?
    // For now, simpler: check for duplicates
    QList<QTableWidgetItem *> items = resultsTable->findItems(info.path, Qt::MatchExactly);
    if (!items.isEmpty()) {
        // Update size if it was a favorite added with 0 size
        int row = items.first()->row();
        resultsTable->item(row, 1)->setText(formatSize(info.sizeBytes));
        return; 
    }
    
    bool isFav = favManager->isFavorite(info.path);
    addTableDataType(info.path, info.sizeBytes, isFav);
}

void MainWindow::onScanFinished() {
    isScanning = false;
    scanBtn->setText("Scan");
    progressBar->setVisible(false);
    statusLabel->setText("Scan complete.");
}

void MainWindow::addTableDataType(const QString &path, quint64 size, bool isFav) {
    int row = resultsTable->rowCount();
    resultsTable->insertRow(row);
    
    QTableWidgetItem *pathItem = new QTableWidgetItem(path);
    pathItem->setFlags(pathItem->flags() ^ Qt::ItemIsEditable);
    resultsTable->setItem(row, 0, pathItem);
    
    QTableWidgetItem *sizeItem = new QTableWidgetItem(formatSize(size));
    sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);
    sizeItem->setData(Qt::UserRole, size); // Store raw bytes
    resultsTable->setItem(row, 1, sizeItem);
    
    QTableWidgetItem *favItem = new QTableWidgetItem(isFav ? "★" : "☆");
    favItem->setTextAlignment(Qt::AlignCenter);
    favItem->setFlags(favItem->flags() ^ Qt::ItemIsEditable);
    resultsTable->setItem(row, 2, favItem);
}

QString MainWindow::formatSize(quint64 sizeBytes) {
    if (sizeBytes < 1024) return QString::number(sizeBytes) + " B";
    if (sizeBytes < 1024 * 1024) return QString::number(sizeBytes / 1024.0, 'f', 2) + " KB";
    if (sizeBytes < 1024 * 1024 * 1024) return QString::number(sizeBytes / (1024.0 * 1024.0), 'f', 2) + " MB";
    return QString::number(sizeBytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

void MainWindow::toggleFavorite(int row, int col) {
    if (col != 2) return;
    
    QString path = resultsTable->item(row, 0)->text();
    bool currentFav = favManager->isFavorite(path);
    
    if (currentFav) {
        favManager->removeFavorite(path);
        resultsTable->item(row, 2)->setText("☆");
    } else {
        favManager->addFavorite(path);
        resultsTable->item(row, 2)->setText("★");
    }
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QTableWidgetItem *item = resultsTable->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *delAction = menu.addAction("Delete Folder");
    QAction *favAction = menu.addAction("Toggle Favorite");
    
    QAction *selected = menu.exec(resultsTable->viewport()->mapToGlobal(pos));
    
    int row = item->row();
    QString path = resultsTable->item(row, 0)->text();
    
    if (selected == delAction) {
        // Delete single
         if (QMessageBox::question(this, "Confirm", "Delete " + path + "?") == QMessageBox::Yes) {
             QDir dir(path);
             if (dir.removeRecursively()) {
                 resultsTable->removeRow(row);
             } else {
                 QMessageBox::critical(this, "Error", "Failed to delete folder.");
             }
         }
    } else if (selected == favAction) {
        toggleFavorite(row, 2);
    }
}

void MainWindow::deleteSelected() {
    QList<QTableWidgetItem *> selected = resultsTable->selectedItems();
    if (selected.isEmpty()) return;

    QSet<int> rows;
    for (auto *item : selected) rows.insert(item->row());
    
    if (QMessageBox::warning(this, "Confirm Deletion", 
                             QString("Are you sure you want to delete %1 folders?").arg(rows.size()), 
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // Sort rows descending to remove correctly
    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

    for (int row : sortedRows) {
        QString path = resultsTable->item(row, 0)->text();
        QDir dir(path);
        if (dir.exists()) {
            if (!dir.removeRecursively()) {
                statusLabel->setText("Failed to delete: " + path);
            } else {
                resultsTable->removeRow(row);
            }
        } else {
             resultsTable->removeRow(row);
        }
    }
    statusLabel->setText("Deletion complete.");
}

void MainWindow::deleteAll() {
    if (resultsTable->rowCount() == 0) return;
    
     if (QMessageBox::warning(this, "Confirm Deletion", 
                             "Are you sure you want to delete ALL found cache folders?", 
                             QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    int rows = resultsTable->rowCount();
    for (int i = rows - 1; i >= 0; i--) {
        QString path = resultsTable->item(i, 0)->text();
        QDir dir(path);
        if (dir.exists()) {
             if (dir.removeRecursively()) {
                 resultsTable->removeRow(i);
             }
        } else {
            resultsTable->removeRow(i);
        }
    }
    statusLabel->setText("All deleted.");
}

void MainWindow::updateFavoritesUI() {
    // Refresh indicators if needed, but we handle it locally mostly
}
