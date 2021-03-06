/*
    Copyright (C) 2014 Aseman
    http://aseman.co

    Papyrus is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Papyrus is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PAPYRUSMACROS_H
#define PAPYRUSMACROS_H

#include <QtGlobal>
#include <QDir>
#include <QThread>

#include "asemantools/asemanapplication.h"

#define DATA_DB_CONNECTION "data_sqlite"

#define PAPYRUS_VERSION "1.0.0"

#define DEMO_PAPERS_LIMIT 20

#define LANG_ON_STARTUP

#define HOME_PATH   AsemanApplication::homePath()
#define BACKUP_PATH AsemanApplication::backupsPath()
#define TEMP_PATH   AsemanApplication::tempPath()
#define CAMERA_PATH AsemanApplication::cameraPath()
#define LOG_PATH    AsemanApplication::logPath()
#define CONFIG_PATH AsemanApplication::confsPath()

#ifdef SUPER_DEBUG
#define BEGIN_FNC_DEBUG \
    qDebug() << QThread::currentThread() << "Begin " << __LINE__ << __PRETTY_FUNCTION__ ;
#define END_FNC_DEBUG \
    qDebug() << QThread::currentThread() << "End " << __LINE__ << __PRETTY_FUNCTION__ ;
#else
#define BEGIN_FNC_DEBUG
#define END_FNC_DEBUG
#endif

#define DROPBOX_APP_KEY    "js992omoncolisd"
#define DROPBOX_APP_SECRET "csojevx79okw1dh"

#endif // PAPYRUSMACROS_H
