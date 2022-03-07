/*
 * Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILEOPERATERPROXY_P_H
#define FILEOPERATERPROXY_P_H

#include "fileoperaterproxy.h"

#include <QTimer>

DDP_CANVAS_BEGIN_NAMESPACE

class FileOperaterProxyPrivate : public QObject
{
    Q_OBJECT
public:

    enum CallBackFunc {
        kCallBackTouchFile,
        kCallBackTouchFolder,
        kCallBackCopyFiles,
        kCallBackCutFiles,
        kCallBackPasteFiles,
        kCallBackOpenFiles,
        kCallBackRenameFiles,
        kCallBackOpenFilesByApp,
        kCallBackMoveToTrash,
        kCallBackDeleteFiles
    };

    explicit FileOperaterProxyPrivate(FileOperaterProxy *q_ptr);

    void callBackTouchFile(const QUrl &target, const QVariantMap &customData);
    void callBackPasteFiles(const JobInfoPointer info);
    void callBackRenameFiles(const QList<QUrl> &targets);

    void delaySelectUrls(const QList<QUrl> &urls, int ms = 10);
    void doSelectUrls(const QList<QUrl> &urls);

public:
    FileOperaterProxy *const q;
    QSharedPointer<QTimer> selectTimer;
    DFMGLOBAL_NAMESPACE::OperaterCallback callBack;
};

DDP_CANVAS_END_NAMESPACE

Q_DECLARE_METATYPE(DDP_CANVAS_NAMESPACE::FileOperaterProxyPrivate::CallBackFunc)

#endif // FILEOPERATERPROXY_P_H
