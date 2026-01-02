#ifndef CACHESCANNER_H
#define CACHESCANNER_H

#include <QThread>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <atomic>

struct CacheFolderInfo {
    QString path;
    quint64 sizeBytes;
};

class CacheScanner : public QThread {
    Q_OBJECT
public:
    explicit CacheScanner(const QString &rootPath, quint64 minSizeBytes = 0, QObject *parent = nullptr);
    void stop();

signals:
    void progress(QString currentPath);
    void cacheFound(CacheFolderInfo info);
    void scanFinished();

protected:
    void run() override;

private:
    QString m_rootPath;
    quint64 m_minSizeBytes;
    std::atomic<bool> m_stopRequested;
    
    void scanRecursive(const QDir &dir);
    quint64 calculateDirectorySize(const QDir &dir);
    bool isCacheFolder(const QString &folderName);
};

#endif // CACHESCANNER_H
