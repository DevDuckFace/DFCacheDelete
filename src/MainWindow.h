#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QSpinBox>

#include "CacheScanner.h"
#include "FavoritesManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void browseFolder();
    void startScan();
    void onScanProgress(const QString &path);
    void onCacheFound(CacheFolderInfo info);
    void onScanFinished();
    
    void deleteSelected();
    void deleteAll();
    void toggleFavorite(int row, int col);
    void showContextMenu(const QPoint &pos);
    
    void updateFavoritesUI();

private:
    void setupUI();
    void setupStyle();
    void addTableDataType(const QString &path, quint64 size, bool isFav);
    QString formatSize(quint64 sizeBytes);

    // UI Elements
    QLineEdit *pathInput;
    QPushButton *browseBtn;
    QPushButton *scanBtn;
    QTableWidget *resultsTable;
    QPushButton *deleteSelectedBtn;
    QPushButton *deleteAllBtn;
    QSpinBox *minSizeSpinBox;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    // Core
    CacheScanner *scanner;
    FavoritesManager *favManager;
    bool isScanning;
    
    // Icons (cached textual or standard)
    // We will use unicode stars for simplicity if no icons resource
};

#endif // MAINWINDOW_H
