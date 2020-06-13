/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dtaskdialog.h"
#include "utils.h"
#include <dcircleprogress.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <ddialog.h>
#include <DApplicationHelper>

#include "dfmglobal.h"
#include "disomaster.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "fileoperations/filejob.h"
#include "dfmstandardpaths.h"

#include "xutil.h"
#include "app/define.h"
#include "shutil/fileutils.h"
#include "dialogs/dialogmanager.h"
#include "dfmtaskwidget.h"
#include "singleton.h"
#include "accessible/libframenamedefine.h"
#include "controllers/vaultcontroller.h"

ErrorHandle::~ErrorHandle()
{
}

DFileCopyMoveJob::Action ErrorHandle::handleError(DFileCopyMoveJob *job, DFileCopyMoveJob::Error error,
                                                   const DAbstractFileInfo *sourceInfo,
                                                   const DAbstractFileInfo *targetInfo)
{
    if (m_actionOfError != DFileCopyMoveJob::NoAction) {
        DFileCopyMoveJob::Action action = m_actionOfError;
        m_actionOfError = DFileCopyMoveJob::NoAction;

        return action;
    }

    switch (error) {
    case DFileCopyMoveJob::FileExistsError:
    case DFileCopyMoveJob::DirectoryExistsError:
    {
        if (sourceInfo->fileUrl() == targetInfo->fileUrl()) {
            return DFileCopyMoveJob::CoexistAction;
        }

        emit onConflict(sourceInfo->fileUrl(), targetInfo->fileUrl());
        job->togglePause();
    }
        break;
    case DFileCopyMoveJob::UnknowUrlError: {
        DDialog dialog("Error", QCoreApplication::translate("DTaskDialog", "This action is not supported"));
        dialog.setIcon(QIcon::fromTheme("dialog-error"), QSize(64, 64));
        dialog.exec();
    }
    // fall-through
    case DFileCopyMoveJob::UnknowError:
        return DFileCopyMoveJob::CancelAction;
    default:
        job->togglePause();
        emit onError(job->errorString());
        break;
    }

    return DFileCopyMoveJob::NoAction;
}

static QString formatTime(int second)
{
    quint8 s = second % 60;
    quint8 m = static_cast<quint8>(second / 60);
    quint8 h = m / 60;
    quint8 d = h / 24;

    m = m % 60;
    h = h % 24;

    QString time_string;

    if (d > 0) {
        time_string.append(QString::number(d)).append(" d");
    }

    if (h > 0) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(h)).append(" h");
    }

    if (m > 0) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(m)).append(" m");
    }

    if (s > 0 || time_string.isEmpty()) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(s)).append(" s");
    }

    return time_string;
}

int DTaskDialog::MaxHeight = 0;

DTaskDialog::DTaskDialog(QWidget *parent) :
    DAbstractDialog(parent),m_flag(false)
{
    initUI();
    initConnect();

    mapNotCompleteVaultTask.clear();
}

void DTaskDialog::initUI()
{
    QFont f = font();
    f.setPixelSize(14); // 固定字体大小不随系统字体大小改变。。label显示不全
    setFont(f);

    setWindowFlags((windowFlags() & ~ Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window | Qt::WindowMinMaxButtonsHint);
    setFixedWidth(m_defaultWidth);
    setObjectName(DIALOGS_TASK_DIALOG);

    m_titlebar = new DTitlebar(this);
    m_titlebar->setObjectName(DIALOGS_TASK_DIALOG_TITLE_BAR);
    m_titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    m_titlebar->setMenuVisible(false);
    m_titlebar->setIcon(QIcon::fromTheme("dde-file-manager"));
    m_titlebar->setStyleSheet("background-color:rgba(0, 0, 0, 0)");

    m_taskListWidget = new QListWidget;
    m_taskListWidget->setObjectName(DIALOGS_TASK_DIALOG_TASK_LIST_WIDGET);
    m_taskListWidget->setSelectionMode(QListWidget::NoSelection);
    m_taskListWidget->viewport()->setAutoFillBackground(false);
    m_taskListWidget->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titlebar);
    mainLayout->addWidget(m_taskListWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    //setFixedWidth(qMax(m_titlebar->width(), m_taskListWidget->width()));

    moveToCenter();
}

void DTaskDialog::initConnect()
{
}

QListWidget *DTaskDialog::getTaskListWidget()
{
    return m_taskListWidget;
}

void DTaskDialog::setTitle(QString title)
{
    m_titlebar->setTitle(title);
}

void DTaskDialog::setTitle(int taskCount)
{
    QString title;
    if (taskCount == 1) {
        title = QObject::tr("1 task in progress");
    } else {
        title = QObject::tr("%1 tasks in progress").arg(QString::number(taskCount));
    }
    setTitle(title);
}

void DTaskDialog::addTask(const QMap<QString, QString> &jobDetail)
{
    if (jobDetail.contains("jobId")) {
        QString strTaskId = jobDetail.value("jobId");
        if (m_jobIdItems.contains(strTaskId))
            return;
        DFMTaskWidget *wid = new DFMTaskWidget;
        wid->setTaskId(jobDetail.value("jobId"));
        connect(wid, &DFMTaskWidget::heightChanged, this, &DTaskDialog::adjustSize);
        connect(wid, &DFMTaskWidget::butonClicked, this, [this, wid, jobDetail](DFMTaskWidget::BUTTON bt) {
            int code = -1;
            if (bt == DFMTaskWidget::STOP) {
                this->close();
                handleTaskClose(jobDetail);
            } else if (bt == DFMTaskWidget::SKIP) {
                code = 2;
            } else if (bt == DFMTaskWidget::REPLACE) {
                code = 1;
            } else if (bt == DFMTaskWidget::COEXIST) {
                code = 0;
            }

            if (code!=-1) {
                QMap<QString, QVariant> response;
                response.insert("code", code);
                response.insert("applyToAll", wid->getButton(DFMTaskWidget::CHECKBOX_NOASK)->isChecked());
                emit conflictRepsonseConfirmed(jobDetail, response);
            }
        });

        // P.S. conflictHided and conflictShowed never used..we don't emit this signal
        addTaskWidget(wid);
    }
}

void DTaskDialog::blockShutdown()
{
    if (m_reply.value().isValid()) {
        return ;
    }

    QDBusInterface loginManager("org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            QDBusConnection::systemBus());

    QList<QVariant> arg;
    arg << QString("shutdown:sleep:")                            // what
        << qApp->applicationDisplayName()                        // who
        << QObject::tr("Files are being processed")              // why
        << QString("block");                                     // mode

    int fd = -1;
    m_reply = loginManager.callWithArgumentList(QDBus::Block, "Inhibit", arg);
    if (m_reply.isValid()) {
        fd = m_reply.value().fileDescriptor();
    }

    if (fd > 0){
        QObject::connect(this, &DTaskDialog::closed, this, [this](){
            QDBusReply<QDBusUnixFileDescriptor> tmp = m_reply; //::close(fd);
            m_reply = QDBusReply<QDBusUnixFileDescriptor>();
        });
    }
}

void DTaskDialog::addTaskWidget(DFMTaskWidget *wid)
{
    if (!wid) {
        return;
    }
    blockShutdown();
    QListWidgetItem *item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    m_taskListWidget->addItem(item);
    m_taskListWidget->setItemWidget(item, wid);
    m_jobIdItems.insert(wid->taskId(), item);

    wid->setObjectName(QString("%1_%2").arg(DIALOGS_TASK_DIALOG_TASK_LIST_ITEM).arg(m_taskListWidget->count()));

    setTitle(m_taskListWidget->count());
    adjustSize();
    setModal(false);
    show();
    QTimer::singleShot(100, this, &DTaskDialog::raise);
}

bool DTaskDialog::isHaveVaultTask(const DUrlList &sourceUrls, const DUrl &targetUrl)
{
    DUrlList::const_iterator itr = sourceUrls.begin();
    for(; itr != sourceUrls.end(); ++itr)
    {
        QString str = (*itr).toString() + targetUrl.toString();
        if(VaultController::isVaultFile(str)
                || str.contains("dfmvault://")){
            return true;
        }
    }
    return false;
}

void DTaskDialog::showVaultDeleteDialog(DFMTaskWidget *wid)
{
    if (!wid) {
        return;
    }
    blockShutdown();
    QListWidgetItem *item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    m_taskListWidget->addItem(item);
    m_taskListWidget->setItemWidget(item, wid);
    m_jobIdItems.insert(wid->taskId(), item);

    wid->setObjectName(QString("%1_%2").arg(DIALOGS_TASK_DIALOG_TASK_LIST_ITEM).arg(m_taskListWidget->count()));
    m_titlebar->setTitle(tr("The File Vault is progressing delete task, please do nothing!"));
    adjustSize();
    setModal(true);
    show();
}

DFileCopyMoveJob::Handle *DTaskDialog::addTaskJob(DFileCopyMoveJob *job)
{
    DFMTaskWidget *wid = new DFMTaskWidget;
    wid->setTaskId(QString::number(quintptr(job), 16));

    // 判断任务是否属于保险箱任务,如果是，记录到容器
    if(isHaveVaultTask(job->sourceUrlList(), job->targetUrl())){
        mapNotCompleteVaultTask.insert(job);

        // 结束当前保险箱未完成任务时，将保险箱任务记录移除
        connect(this, &DTaskDialog::sigStopJob, job, [job, this](){
            mapNotCompleteVaultTask.remove(job);
            emit job->stop();
        });
    }
    //! 保存任务widget与状态
    m_flagMap.insert(wid, false);

    ErrorHandle *handle = new ErrorHandle(wid);
    job->setErrorHandle(handle, thread());
    connect(handle, &ErrorHandle::onConflict, wid, &DFMTaskWidget::setConflictMsg);
    connect(handle, &ErrorHandle::onError, wid, &DFMTaskWidget::setErrorMsg);
    connect(wid, &DFMTaskWidget::heightChanged, this, &DTaskDialog::adjustSize);
    connect(this, &DTaskDialog::closed, job,&DFileCopyMoveJob::stop);
    connect(this, &DTaskDialog::closed,job,&DFileCopyMoveJob::taskDailogClose);

    connect(wid, &DFMTaskWidget::butonClicked, job, [job, wid, handle, this](DFMTaskWidget::BUTTON bt) {
        DFileCopyMoveJob::Action action = DFileCopyMoveJob::NoAction;
        QCloseEvent event;
        switch (bt) {
            case DFMTaskWidget::PAUSE:
            emit job->togglePause();
                break;
            case DFMTaskWidget::STOP:

//                this->closeEvent(&event);

            // 结束当前指定任务时，如果该任务属于保险箱未完成任务，则删除记录
            if(mapNotCompleteVaultTask.contains(job)){
                mapNotCompleteVaultTask.remove(job);
            }

            emit job->stop();

                break;
            case DFMTaskWidget::SKIP:
                action = DFileCopyMoveJob::SkipAction;
            break;
            case DFMTaskWidget::REPLACE:
                if (job->error() == DFileCopyMoveJob::DirectoryExistsError) {
                    action = DFileCopyMoveJob::MergeAction;
                } else if (job->error() == DFileCopyMoveJob::FileExistsError) {
                    action = DFileCopyMoveJob::ReplaceAction;
                } else {
                    action = DFileCopyMoveJob::RetryAction;
                }
            break;
            case DFMTaskWidget::COEXIST:
                action = DFileCopyMoveJob::CoexistAction;
            break;
        default:
            break;
        }

        if (action == DFileCopyMoveJob::NoAction) {
            return;
        }

        handle->setActionOfError(action);
        if (wid->getButton(DFMTaskWidget::CHECKBOX_NOASK)->isChecked()) {
            job->setActionOfErrorType(job->error(), action);
        }
        if (job->state() == DFileCopyMoveJob::PausedState) {
            job->togglePause();
        }
    });

    connect(job, &DFileCopyMoveJob::progressChanged, wid, &DFMTaskWidget::onProgressChanged);
    connect(job, &DFileCopyMoveJob::speedUpdated, wid, [wid](qint64 speed){
        //wid->onSpeedUpdated(speed);
        QString sp = FileUtils::formatSize(speed) + "/s";
        QString rmTime;
        qint64 totalsize = wid->property("totalDataSize").toLongLong();
        qreal progress = wid->property("progress").toReal();
        if (speed>0) {
            rmTime = formatTime(int(totalsize * (1 - progress) / speed));
        }
        wid->setSpeedText(sp, rmTime);
    });

    connect(job, &DFileCopyMoveJob::stateChanged, wid, &DFMTaskWidget::onStateChanged);
    connect(job, &DFileCopyMoveJob::fileStatisticsFinished, wid, [wid, job] {
        wid->setProperty("totalDataSize" ,job->totalDataSize());
    });
    wid->setProperty("totalDataSize" ,job->totalDataSize());

    connect(job, &DFileCopyMoveJob::currentJobChanged, this, [this, job, wid](const DUrl from, const DUrl to){
        //正在执行当前槽函数时，job线程一结束，判断job线程是否结束
        if(job->isFinished())
        {
            this->removeTaskJob(job);

            // 判断当前任务是否是保险箱任务，如果是，则消除记录
            if(mapNotCompleteVaultTask.contains(job)){
                mapNotCompleteVaultTask.remove(job);
            }
            //! 设置任务widget对应的状态
            m_flagMap[wid] = true;
            return;
        }
        QMap<QString, QString> data;
        bool isDelete = false; // 删除模式下不需要显示速度
        if (job->mode() == DFileCopyMoveJob::CopyMode) {
            data["type"] = "copy";
        } else if (job->mode() == DFileCopyMoveJob::CutMode) {
            data["type"] = "move";
        } else if (job->mode() == DFileCopyMoveJob::MoveMode) {
            if (job->targetUrl().isValid()) {
                data["type"] = "move";
            } else {
                isDelete = true;
                data["type"] = "delete";
            }
        }

        data["sourcePath"] = from.path();
        data["file"] = from.fileName();
        data["targetPath"] = to.path();
        data["destination"] = to.isValid() ? to.parentUrl().path() : QString();
        bool ok = false;
        qint64 speed =  wid->property("speed").toLongLong(&ok);
        if (ok && !isDelete) {
            data["speed"] = FileUtils::formatSize(speed) + "/s";
        }
        qint64 totalDataSize = wid->property("totalDataSize").toLongLong(&ok);
        bool okp = false;
        qreal progress = wid->property("progress").toReal(&okp);
        if (ok && okp && speed>0) {
            data["remainTime"] = formatTime(int(totalDataSize * (1 - progress) / speed));
        }

        if (job->state() != DFileCopyMoveJob::RunningState) {
            if (job->error() == DFileCopyMoveJob::FileExistsError
                    || job->error() == DFileCopyMoveJob::DirectoryExistsError) {
                data["status"] = "conflict";
            } else if (job->error() != DFileCopyMoveJob::NoError) {
                data["status"] = "error";
                bool supprotRetry = job->supportActions(job->error()).testFlag(DFileCopyMoveJob::RetryAction);
                data["supprotRetry"] = supprotRetry ? "true" : "false";
                data["errorMsg"] = job->errorString();
            }
        }

        this->updateData(wid, data);
        //正在执行当前槽函数时，job线程一结束，判断job线程是否结束
        if(job->isFinished())
        {
            this->removeTaskJob(job);

            // 判断当前任务是否是保险箱任务，如果是，则消除记录
            if(mapNotCompleteVaultTask.contains(job)){
                mapNotCompleteVaultTask.remove(job);
            }
        }

        //! 设置任务widget对应的状态
        m_flagMap[wid] = true;
        if(m_flag && getFlagMapValueIsTrue())
        {
            //! 有点击关闭任务窗口，进行窗口关闭
            this->close();
        }
    });
    connect(job, &DFileCopyMoveJob::errorChanged, wid, [wid](DFileCopyMoveJob::Error error) {
        wid->getButton(DFMTaskWidget::PAUSE)->setEnabled(error == DFileCopyMoveJob::NoError);
    }, Qt::QueuedConnection);


    /*之前的显示进度百分比的元件CircleProgressAnimatePad，
     * 当在读取大量文件状态时，元件外边会有个转圈的动画效果*/
    if (!job->fileStatisticsIsFinished()) {
        wid->setProgressValue(0); // start
    }

    wid->getButton(DFMTaskWidget::PAUSE)->setEnabled(job->error() == DFileCopyMoveJob::NoError);

    if (isHaveVaultTask(job->sourceUrlList(), job->targetUrl())
            && job->mode() != DFileCopyMoveJob::CopyMode){
        if (!job->targetUrl().isValid()) {
            showVaultDeleteDialog(wid);
        } else {
            addTaskWidget(wid);
        }
    }else {
        addTaskWidget(wid);
    }

    return handle;
}
void DTaskDialog::adjustSize()
{
    int listHeight = 2;
    for (int i = 0; i < m_taskListWidget->count(); i++) {
        QListWidgetItem *item = m_taskListWidget->item(i);
        int h = m_taskListWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    if (listHeight < qApp->desktop()->availableGeometry().height() - 60) {
        m_taskListWidget->setFixedHeight(listHeight);
        setFixedHeight(listHeight + 60);
        MaxHeight = height();
    } else {
        setFixedHeight(MaxHeight);
    }

    layout()->setSizeConstraint(QLayout::SetNoConstraint);
    moveYCenter();
}

void DTaskDialog::moveYCenter()
{
    QRect qr = frameGeometry();
    QPoint cp;
    if (parent()) {
        cp = static_cast<QWidget *>(parent())->geometry().center();
    } else {
        cp = qApp->desktop()->availableGeometry().center();
    }
    qr.moveCenter(cp);
    move(x(), qr.y());
}

void DTaskDialog::removeTaskByPath(QString jobId)
{
    if (m_jobIdItems.contains(jobId)) {
        QListWidgetItem *item = m_jobIdItems.value(jobId);
        if (item) {
            m_taskListWidget->removeItemWidget(item);
            m_taskListWidget->takeItem(m_taskListWidget->row(item));
            m_jobIdItems.remove(jobId);
        }

        setTitle(m_taskListWidget->count());
        if (m_taskListWidget->count() == 0) {
            close();
        }
    }
}

void DTaskDialog::handleTaskClose(const QMap<QString, QString> &jobDetail)
{
    qDebug() << jobDetail;
    removeTask(jobDetail, false);
    setTitle(m_taskListWidget->count());
    if (jobDetail.contains("type")) {
        emit abortTask(jobDetail);
    }
}

void DTaskDialog::removeTask(const QMap<QString, QString> &jobDetail, bool adjustSize)
{
    if (jobDetail.contains("jobId")) {
        removeTaskByPath(jobDetail.value("jobId"));

        if (adjustSize) {
            this->adjustSize();
        }
    }
}

void DTaskDialog::removeTaskJob(void *job)
{
    removeTaskByPath(QString::number(quintptr(job), 16));
    adjustSize();
}

void DTaskDialog::removeTaskImmediately(const QMap<QString, QString> &jobDetail)
{
    if (m_taskListWidget->count() > 1) {
        delayRemoveTask(jobDetail);
    } else {
        removeTask(jobDetail);
    }
}

void DTaskDialog::delayRemoveTask(const QMap<QString, QString> &jobDetail)
{
    QTimer::singleShot(2000, this, [ = ]() {
        removeTask(jobDetail);
    });
}

void DTaskDialog::updateData(DFMTaskWidget *wid, const QMap<QString, QString> &data)
{
    if (!wid) {
        return;
    }

    QString file, destination, speed, remainTime, progress, status, srcPath, targetPath;
    QString msg1, msg2;

    if (data.contains("optical_op_type")) {
        wid->getButton(DFMTaskWidget::PAUSE)->setEnabled(false);
        status = data["optical_op_status"];
        progress = data["optical_op_progress"];

        msg1 = (data["optical_op_type"] == QString::number(FileJob::JobType::OpticalBlank)
               ? tr("Erasing disc %1, please wait...")
               : tr("Burning disc %1, please wait...")).arg(data["optical_op_dest"]);
        msg2 = "";
        if (data["optical_op_type"] != QString::number(FileJob::JobType::OpticalBlank)) {
            const QHash<QString, QString> msg2map = {
                {"0", ""}, // unused right now
                {"1", tr("Writing data...")},
                {"2", tr("Verifying data...")}
            };
            msg2 = msg2map.value(data["optical_op_phase"], "");
        }
        wid->setMsgText(msg1, msg2);
        wid->setSpeedText(data["optical_op_speed"], "");

        if (status == QString::number(DISOMasterNS::DISOMaster::JobStatus::Stalled)) {
            wid->setProgressValue(-1);// stop
        }
        else if (status == QString::number(DISOMasterNS::DISOMaster::JobStatus::Running)) {
            //wid->onProgressChanged(progress.toInt(), 0);
            wid->onBurnProgressChanged(progress.toInt(), 0);
        }
        return;
    }

    if (data.contains("file")) {
        file = data.value("file");
    }
    if (data.contains("destination")) {
        destination = data.value("destination");
    }
    if (data.contains("speed")) {
        speed = data.value("speed");
    }
    if (data.contains("remainTime")) {
        remainTime = data.value("remainTime");
    }

    if (data.contains("progress")) {
        progress = data.value("progress");
    }

    if (data.contains("sourcePath")) {
        srcPath = data.value("sourcePath");
    }

    if (data.contains("targetPath")) {
        targetPath = data.value("targetPath");
    }

    if (data.contains("status")) {
        status = data.value("status");
    }

    QString speedStr = "%1";
    QString remainStr = "%1";
    if (data.contains("type")) { // 放到data中
        if (!file.isEmpty()) {
            if (data.value("type") == "copy") {
                msg1 = tr("Copying %1").arg(file);
                msg2 = tr("to %2").arg(destination);

            } else if (data.value("type") == "move") {
                msg1 = tr("Moving %1").arg(file);
                msg2 = tr("to %2").arg(destination);
            } else if (data.value("type") == "restore") {
                msg1 = tr("Restoring %1").arg(file);
                msg2 = tr("to %2").arg(destination);
            } else if (data.value("type") == "delete") {
                msg1 = tr("Deleting %1").arg(file);
                msg2 = tr("");
            } else if (data.value("type") == "trash") {
                msg1 = tr("Trashing %1").arg(file);
                msg2 = tr("");
            }
        }

        if (status == "restoring") {
            wid->setProgressValue(-1);// stop
        } else if (status == "calculating") {
            msg2 = tr("Calculating space, please wait");
            wid->setProgressValue(-1);// stop
        } else if (status == "conflict") {
            msg1 = QString(tr("%1 already exists in target folder")).arg(file);
            msg2 = QString(tr("Original path %1 Target path %2")).arg(QFileInfo(srcPath).absolutePath(), QFileInfo(targetPath).absolutePath());

            // fix 回收站还原文件时 若目标目录存在同名文件 需要展示 “合并” “替换” 等按钮
            // fix 如果不做回收站判断，同一目录下会发生闪烁
            QString trashPath(DFMStandardPaths::location(DFMStandardPaths::TrashPath));
            if (srcPath.startsWith(trashPath)) {
                wid->setConflictMsg(DUrl::fromLocalFile(srcPath), DUrl::fromLocalFile(targetPath));
            }

            if (QFileInfo(srcPath).isDir() &&
                    QFileInfo(targetPath).isDir()) {
                wid->setButtonText(DFMTaskWidget::REPLACE, tr("Merge"));

            } else {
                wid->setButtonText(DFMTaskWidget::REPLACE, tr("Replace"));
            }
        } else if (status == "error") {
             QString supprotRetry = data.value("supprotRetry"); // 这个需要新加字段
             wid->setErrorMsg(data.value("errorMsg"));// 这个需要新加字段
             wid->hideButton(DFMTaskWidget::REPLACE, supprotRetry.compare("true", Qt::CaseInsensitive)!=0);
        } else if (!status.isEmpty()) {
            wid->setProgressValue(0); // start
        } else {
            wid->setErrorMsg("");
        }

        speedStr = speedStr.arg(speed);
        remainStr = remainStr.arg(remainTime);
        wid->setMsgText(msg1, msg2);
        wid->setSpeedText(speedStr, remainStr);
    }

    if (!progress.isEmpty()) {
        wid->onProgressChanged(progress.toDouble(), 0);
    }
}

bool DTaskDialog::bHaveNotCompletedVaultTask()
{
    return (mapNotCompleteVaultTask.count() > 0) ? true : false;
}

void DTaskDialog::showDialogOnTop()
{
    this->activateWindow();
    dialogManager->taskDialog()->showNormal();
}

void DTaskDialog::stopVaultTask()
{
    emit sigStopJob();
}

bool DTaskDialog::getFlagMapValueIsTrue()
{
    bool flg = false;
    for(bool i : m_flagMap.values())
    {
        flg = i;
    }
    return flg;
}

void DTaskDialog::handleUpdateTaskWidget(const QMap<QString, QString> &jobDetail,
        const QMap<QString, QString> &data)
{
    if (jobDetail.contains("jobId")) {
        QString jobId = jobDetail.value("jobId");
        if (m_jobIdItems.contains(jobId)) {
            QListWidgetItem *item = m_jobIdItems.value(jobId);
            DFMTaskWidget *w = item ? static_cast<DFMTaskWidget *>(item->listWidget()->itemWidget(item)): nullptr;
            if (w) {
                updateData(w, data);
                // 预防界面不刷，pangu出现过界面不会刷新的问题
                w->repaint();
                qApp->processEvents();
            }
        }
    }
}

void DTaskDialog::closeEvent(QCloseEvent *event)
{
    //! 记录是否点击关闭按钮
    m_flag = true;
    if(getFlagMapValueIsTrue())
    {
        for (QListWidgetItem *item : m_jobIdItems.values()) {
            DFMTaskWidget *w = static_cast<DFMTaskWidget *>(m_taskListWidget->itemWidget(item));
            if (w) {
                w->getButton(DFMTaskWidget::STOP)->click();
                m_taskListWidget->removeItemWidget(item);
                m_taskListWidget->takeItem(m_taskListWidget->row(item));
                m_jobIdItems.remove(m_jobIdItems.key(item));
            }
        }
        //! 清空数据
        m_flag = false;
        m_flagMap.clear();

        // 任务对话框关闭时，清空未完成保险箱任务记录
        mapNotCompleteVaultTask.clear();

        QDialog::closeEvent(event);
    }

    emit closed();
}

void DTaskDialog::keyPressEvent(QKeyEvent *event)
{
    //handle escape key press event for emitting close event
    if (event->key() == Qt::Key_Escape) {
        emit close();
    }
    QDialog::keyPressEvent(event);
}

