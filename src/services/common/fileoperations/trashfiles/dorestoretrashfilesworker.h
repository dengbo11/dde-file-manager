/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DORESTORETRASHFILESWORKER_H
#define DORESTORETRASHFILESWORKER_H

#include "dfm_common_service_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "fileoperations/fileoperationutils/abstractworker.h"

#include <QObject>

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DoRestoreTrashFilesWorker : public AbstractWorker
{
    friend class RestoreTrashFiles;
    Q_OBJECT
    explicit DoRestoreTrashFilesWorker(QObject *parent = nullptr);

public:
    virtual ~DoRestoreTrashFilesWorker() override;

protected:
    bool doWork() override;
};

DSC_END_NAMESPACE

#endif   // DOCUTFILESWORKER_H
