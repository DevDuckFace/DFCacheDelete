#include "CacheScanner.h"
#include <QDirIterator>
#include <QDebug>

CacheScanner::CacheScanner(const QString &rootPath, quint64 minSizeBytes, QObject *parent)
    : QThread(parent), m_rootPath(rootPath), m_minSizeBytes(minSizeBytes), m_stopRequested(false) {
}

void CacheScanner::stop() {
    m_stopRequested = true;
}

void CacheScanner::run() {
    m_stopRequested = false;
    QDir rootDir(m_rootPath);
    
    if (rootDir.exists()) {
        scanRecursive(rootDir);
    }
    
    emit scanFinished();
}

bool CacheScanner::isCacheFolder(const QString &folderName) {
    return folderName.toLower().contains("cache");
}

/*
 * Recursive scan logic.
 * We want to find folders that contain "cache" in their name.
 * If a folder IS a cache folder, add it and do NOT scan inside it (usually we delete the whole thing).
 * Or do we scan inside? The prompt says "Find ALL folders whose name CONTAINS...".
 * Usually if "AppData/Local/Temp/MyCache" is matches, we don't need to return "AppData/Local/Temp/MyCache/SubCache".
 * I will assume top-level match stops recursion for that branch.
 */
void CacheScanner::scanRecursive(const QDir &dir) {
    if (m_stopRequested) return;

    // Use QDirIterator for performance and explicit control
    QDirIterator it(dir.absolutePath(), QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDirIterator::NoIteratorFlags);
    
    while (it.hasNext()) {
        if (m_stopRequested) return;
        
        it.next();
        QFileInfo info = it.fileInfo();
        QString path = info.absoluteFilePath();
        QString folderName = info.fileName();

        emit progress(path);

        // Check if symbolic link - ignore
        if (info.isSymLink()) continue;

        if (isCacheFolder(folderName)) {
            // Found a cache folder
            // Calculate size
            quint64 size = calculateDirectorySize(QDir(path));
            
            // Only report if size >= minimum configured size
            if (size >= m_minSizeBytes) {
                emit cacheFound({path, size});
            }
            // Do not recurse into a cache folder we are going to delete/flag
        } else {
            // Recurse
            // Check permissions roughly by seeing if we can open it?
            // QDirIterator handles some of this, but if we can't read, usually we just skip.
            if (QDir(path).isReadable()) {
                scanRecursive(QDir(path));
            }
        }
    }
}

quint64 CacheScanner::calculateDirectorySize(const QDir &dir) {
    quint64 size = 0;
    QDirIterator it(dir.absolutePath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (m_stopRequested) return size;
        it.next();
        size += it.fileInfo().size();
    }
    return size;
}
