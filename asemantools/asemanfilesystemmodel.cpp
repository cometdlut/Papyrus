#include "asemanfilesystemmodel.h"

#include <QFileSystemWatcher>
#include <QDir>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDateTime>
#include <QFileInfo>
#include <QTimerEvent>
#include <QTimer>
#include <QDebug>


class AsemanFileSystemModelPrivate
{
public:
    bool showDirs;
    bool showDotAndDotDot;
    bool showDirsFirst;
    bool showFiles;
    bool showHidden;
    QStringList nameFilters;
    QString folder;
    int sortField;

    QList<QFileInfo> list;
    QMimeDatabase mdb;

    QFileSystemWatcher *watcher;
    QTimer *refresh_timer;
};

class SortUnitType
{
public:
    SortUnitType(): num(0){}
    QChar ch;
    quint64 num;
};

QList<SortUnitType> analize_file_name(const QString &fileName)
{
    QList<SortUnitType> res;
    for(int i=0; i<fileName.length(); i++)
    {
        const QChar &ch = fileName[i];
        if(ch.isNumber())
        {
            int num = QString(ch).toInt();
            if(res.isEmpty() || !res.last().ch.isNull() )
                res << SortUnitType();

            SortUnitType & resUnit = res[res.length()-1];
            resUnit.num = resUnit.num*10 + num;
        }
        else
        {
            SortUnitType unit;
            unit.ch = ch;
            res << unit;
        }
    }

    return res;
}

AsemanFileSystemModelPrivate *fileListSort_private_data = 0;
bool fileListSort(const QFileInfo &f1, const QFileInfo &f2)
{
    if(fileListSort_private_data->showDirsFirst)
    {
        if(f1.isDir() && !f2.isDir())
            return true;
        if(!f1.isDir() && f2.isDir())
            return false;
    }

    const QString & s1 = f1.fileName();
    const QString & s2 = f2.fileName();

    const QList<SortUnitType> &ul1 = analize_file_name(s1);
    const QList<SortUnitType> &ul2 = analize_file_name(s2);

    for(int i=0; i<ul1.length() && i<ul2.length(); i++)
    {
        const SortUnitType &u1 = ul1.at(i);
        const SortUnitType &u2 = ul2.at(i);

        if(u1.ch.isNull() && !u2.ch.isNull())
            return true;
        if(!u1.ch.isNull() && u2.ch.isNull())
            return false;
        if(!u1.ch.isNull() && !u2.ch.isNull())
        {
            if(u1.ch < u2.ch)
                return true;
            if(u1.ch > u2.ch)
                return false;
        }
        if(u1.ch.isNull() && u2.ch.isNull())
        {
            if(u1.num < u2.num)
                return true;
            if(u1.num > u2.num)
                return false;
        }
    }

    return ul1.length() < ul2.length();
}



AsemanFileSystemModel::AsemanFileSystemModel(QObject *parent) :
    QAbstractListModel(parent)
{
    p = new AsemanFileSystemModelPrivate;
    p->showDirs = true;
    p->showDotAndDotDot = false;
    p->showDirsFirst = true;
    p->showFiles = true;
    p->showHidden = false;
    p->sortField = AsemanFileSystemModel::Size;
    p->refresh_timer = 0;

    p->watcher = new QFileSystemWatcher(this);

    p->refresh_timer = new QTimer(this);
    p->refresh_timer->setInterval(10);

    connect(p->watcher, SIGNAL(directoryChanged(QString)), SLOT(refresh()));
    connect(p->watcher, SIGNAL(fileChanged(QString))     , SLOT(refresh()));

    connect(p->refresh_timer, SIGNAL(timeout()), SLOT(reinit_buffer()));
}

void AsemanFileSystemModel::setShowDirs(bool stt)
{
    if(p->showDirs == stt)
        return;

    p->showDirs = stt;
    emit showDirsChanged();

    refresh();
}

bool AsemanFileSystemModel::showDirs() const
{
    return p->showDirs;
}

void AsemanFileSystemModel::setShowDotAndDotDot(bool stt)
{
    if(p->showDotAndDotDot == stt)
        return;

    p->showDotAndDotDot = stt;
    emit showDotAndDotDotChanged();

    refresh();
}

bool AsemanFileSystemModel::showDotAndDotDot() const
{
    return p->showDotAndDotDot;
}

void AsemanFileSystemModel::setShowDirsFirst(bool stt)
{
    if(p->showDirsFirst == stt)
        return;

    p->showDirsFirst = stt;
    emit showDirsFirstChanged();

    refresh();
}

bool AsemanFileSystemModel::showDirsFirst() const
{
    return p->showDirsFirst;
}

void AsemanFileSystemModel::setShowFiles(bool stt)
{
    if(p->showFiles == stt)
        return;

    p->showFiles = stt;
    emit showFilesChanged();

    refresh();
}

bool AsemanFileSystemModel::showFiles() const
{
    return p->showFiles;
}

void AsemanFileSystemModel::setShowHidden(bool stt)
{
    if(p->showHidden == stt)
        return;

    p->showHidden = stt;
    emit showHiddenChanged();

    refresh();
}

bool AsemanFileSystemModel::showHidden() const
{
    return p->showHidden;
}

void AsemanFileSystemModel::setNameFilters(const QStringList &list)
{
    if(p->nameFilters == list)
        return;

    p->nameFilters = list;
    emit nameFiltersChanged();

    refresh();
}

QStringList AsemanFileSystemModel::nameFilters() const
{
    return p->nameFilters;
}

void AsemanFileSystemModel::setFolder(const QString &url)
{
    if(p->folder == url)
        return;

    if(!p->folder.isEmpty())
        p->watcher->removePath(p->folder);

    p->folder = url;
    if(!p->folder.isEmpty())
        p->watcher->addPath(p->folder);

    emit folderChanged();

    refresh();
}

QString AsemanFileSystemModel::folder() const
{
    return p->folder;
}

void AsemanFileSystemModel::setSortField(int field)
{
    if(p->sortField == field)
        return;

    p->sortField = field;
    emit sortFieldChanged();

    refresh();
}

int AsemanFileSystemModel::sortField() const
{
    return p->sortField;
}

QString AsemanFileSystemModel::parentFolder() const
{
    return QFileInfo(p->folder).dir().absolutePath();
}

const QFileInfo &AsemanFileSystemModel::id(const QModelIndex &index) const
{
    return p->list.at( index.row() );
}

int AsemanFileSystemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return count();
}

QVariant AsemanFileSystemModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    const QFileInfo &info = id(index);
    switch(role)
    {
    case FilePath:
        result = info.filePath();
        break;

    case FileName:
        result = info.fileName();
        break;

    case FileMime:
        result = p->mdb.mimeTypeForFile(info.filePath()).name();
        break;

    case FileSize:
        result = info.size();
        break;

    case FileSuffix:
        result = info.suffix();
        break;

    case FileBaseName:
        result = info.completeBaseName();
        break;

    case FileIsDir:
        result = info.isDir();
        break;

    case FileModifiedDate:
        result = info.lastModified();
        break;

    case FileCreatedDate:
        result = info.created();
        break;
    }

    return result;
}

QHash<qint32, QByteArray> AsemanFileSystemModel::roleNames() const
{
    static QHash<qint32, QByteArray> *res = 0;
    if( res )
        return *res;

    res = new QHash<qint32, QByteArray>();
    res->insert( FilePath, "filePath");
    res->insert( FileName, "fileName");
    res->insert( FileMime, "fileMime");
    res->insert( FileSize, "fileSize");
    res->insert( FileSuffix, "fileSuffix");
    res->insert( FileBaseName, "fileBaseName");
    res->insert( FileIsDir, "fileIsDir");
    res->insert( FileModifiedDate, "fileModifiedDate");
    res->insert( FileCreatedDate, "fileCreatedDate");

    return *res;
}

int AsemanFileSystemModel::count() const
{
    return p->list.count();
}

void AsemanFileSystemModel::refresh()
{
    p->refresh_timer->stop();
    p->refresh_timer->start();
}

QVariant AsemanFileSystemModel::get(int idx, const QString &roleName)
{
    qint32 role = roleNames().key(roleName.toUtf8());
    return data(index(idx), role);
}

void AsemanFileSystemModel::reinit_buffer()
{
    p->refresh_timer->stop();

    int filter = 0;
    if(p->showDirs)
        filter = filter | QDir::Dirs;
    if(!p->showDotAndDotDot)
        filter = filter | QDir::NoDotAndDotDot;
    if(p->showFiles)
        filter = filter | QDir::Files;
    if(p->showHidden)
        filter = filter | QDir::Hidden;

    QStringList list;
    if(filter && !p->folder.isEmpty())
        list = QDir(p->folder).entryList(static_cast<QDir::Filter>(filter));

    QList<QFileInfo> res;
    foreach(const QString &fileName, list)
        res << QFileInfo(p->folder + "/" + fileName);

    if(!p->nameFilters.isEmpty())
        for(int i=0; i<res.count(); i++)
        {
            const QFileInfo &inf = res.at(i);

            QStringList suffixes;
            if(!inf.suffix().isEmpty())
                suffixes << inf.suffix();
            else
                suffixes = p->mdb.mimeTypeForFile(inf.filePath()).suffixes();

            bool founded = inf.isDir();
            foreach(const QString &sfx, suffixes)
                if(p->nameFilters.contains("*."+sfx, Qt::CaseInsensitive))
                {
                    founded = true;
                    break;
                }

            if(!founded)
            {
                res.removeAt(i);
                i--;
            }
        }

    fileListSort_private_data = p;
    qStableSort(res.begin(), res.end(), fileListSort);

    changed(res);
}

void AsemanFileSystemModel::changed(const QList<QFileInfo> &list)
{
    bool count_changed = (list.count()==p->list.count());

    for( int i=0 ; i<p->list.count() ; i++ )
    {
        const QFileInfo &file = p->list.at(i);
        if( list.contains(file) )
            continue;

        beginRemoveRows(QModelIndex(), i, i);
        p->list.removeAt(i);
        i--;
        endRemoveRows();
    }

    QList<QFileInfo> temp_list = list;
    for( int i=0 ; i<temp_list.count() ; i++ )
    {
        const QFileInfo &file = temp_list.at(i);
        if( p->list.contains(file) )
            continue;

        temp_list.removeAt(i);
        i--;
    }
    while( p->list != temp_list )
        for( int i=0 ; i<p->list.count() ; i++ )
        {
            const QFileInfo &file = p->list.at(i);
            int nw = temp_list.indexOf(file);
            if( i == nw )
                continue;

            beginMoveRows( QModelIndex(), i, i, QModelIndex(), nw>i?nw+1:nw );
            p->list.move( i, nw );
            endMoveRows();
        }

    for( int i=0 ; i<list.count() ; i++ )
    {
        const QFileInfo &file = list.at(i);
        if( p->list.contains(file) )
            continue;

        beginInsertRows(QModelIndex(), i, i );
        p->list.insert( i, file );
        endInsertRows();
    }

    if(count_changed)
        emit countChanged();
}

AsemanFileSystemModel::~AsemanFileSystemModel()
{
    delete p;
}
