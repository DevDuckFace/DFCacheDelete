#include "FavoritesManager.h"
#include <QDebug>

FavoritesManager::FavoritesManager(QObject *parent) : QObject(parent) {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // AppDataLocation usually ends in Organization/AppName or similar. 
    // We want specifically %APPDATA%/CacheCleaner if we follow the spec strictly, 
    // but QStandardPaths::AppDataLocation is the standard Qt way. 
    // On Windows dealing with "CacheCleaner" directly:
    // QStandardPaths::AppDataLocation on Windows often maps to C:/Users/<User>/AppData/Roaming/<AppName> 
    // if organization name is not set, or <Org>/<AppName>.
    // We will set OrganizationName in main.cpp.
    
    m_configPath = appDataLocation + "/favorites.json";
    load();
}

FavoritesManager::~FavoritesManager() {
    save();
}

void FavoritesManager::addFavorite(const QString &path) {
    QMutexLocker locker(&m_mutex);
    QString cleanPath = QDir::toNativeSeparators(path);
    if (!m_favorites.contains(cleanPath)) {
        m_favorites.insert(cleanPath);
        locker.unlock();
        save();
        emit favoritesChanged();
    }
}

void FavoritesManager::removeFavorite(const QString &path) {
    QMutexLocker locker(&m_mutex);
    QString cleanPath = QDir::toNativeSeparators(path);
    if (m_favorites.contains(cleanPath)) {
        m_favorites.remove(cleanPath);
        locker.unlock();
        save();
        emit favoritesChanged();
    }
}

bool FavoritesManager::isFavorite(const QString &path) const {
    QMutexLocker locker(&m_mutex);
    return m_favorites.contains(QDir::toNativeSeparators(path));
}

QSet<QString> FavoritesManager::getFavorites() const {
    QMutexLocker locker(&m_mutex);
    return m_favorites;
}

void FavoritesManager::ensureConfigDirExists() {
    QFileInfo info(m_configPath);
    QDir dir = info.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void FavoritesManager::save() {
    ensureConfigDirExists();
    QMutexLocker locker(&m_mutex);

    QJsonArray array;
    for (const auto &fav : m_favorites) {
        array.append(fav);
    }

    QJsonObject root;
    root["favorites"] = array;

    QJsonDocument doc(root);
    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void FavoritesManager::load() {
    QMutexLocker locker(&m_mutex);
    m_favorites.clear();

    QFile file(m_configPath);
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject root = doc.object();
            if (root.contains("favorites") && root["favorites"].isArray()) {
                QJsonArray array = root["favorites"].toArray();
                for (const auto &val : array) {
                    m_favorites.insert(val.toString());
                }
            }
        }
        file.close();
    }
}
