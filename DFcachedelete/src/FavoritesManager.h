#ifndef FAVORITESMANAGER_H
#define FAVORITESMANAGER_H

#include <QString>
#include <QSet>
#include <QObject>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QMutex>

class FavoritesManager : public QObject {
    Q_OBJECT
public:
    explicit FavoritesManager(QObject *parent = nullptr);
    ~FavoritesManager();

    void addFavorite(const QString &path);
    void removeFavorite(const QString &path);
    bool isFavorite(const QString &path) const;
    QSet<QString> getFavorites() const;

    void save();
    void load();

signals:
    void favoritesChanged();

private:
    QSet<QString> m_favorites;
    QString m_configPath;
    mutable QMutex m_mutex;

    void ensureConfigDirExists();
};

#endif // FAVORITESMANAGER_H
