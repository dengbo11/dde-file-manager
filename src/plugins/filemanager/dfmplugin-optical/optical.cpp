/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#include "optical.h"
#include "utils/opticalhelper.h"
#include "utils/opticalfileshelper.h"
#include "mastered/masteredmediafileinfo.h"
#include "mastered/masteredmediafilewatcher.h"
#include "mastered/masteredmediadiriterator.h"
#include "views/opticalmediawidget.h"
#include "menus/opticalmenuscene.h"

#include "services/common/menu/menuservice.h"
#include "services/filemanager/workspace/workspace_defines.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"

DPOPTICAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE

void Optical::initialize()
{
    UrlRoute::regScheme(Global::kBurn, "/", OpticalHelper::icon(), true);
    InfoFactory::regClass<MasteredMediaFileInfo>(Global::kBurn);
    WatcherFactory::regClass<MasteredMediaFileWatcher>(Global::kBurn);
    DirIteratorFactory::regClass<MasteredMediaDirIterator>(Global::kBurn);

    connect(OpticalHelper::winServIns(), &WindowsService::windowCreated, this,
            [this]() {
                addOpticalCrumbToTitleBar();
            },
            Qt::DirectConnection);

    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted,
            this, &Optical::onDeviceUnmounted, Qt::DirectConnection);
    connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved,
            this, &Optical::onDeviceUnmounted, Qt::DirectConnection);
    // for blank disc
    connect(DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged, this,
            [this](const QString &id, const QString &property, const QVariant &val) {
                if (id.contains(QRegularExpression("/sr[0-9]*$"))
                    && property == GlobalServerDefines::DeviceProperty::kOptical && !val.toBool()) {
                    onDeviceUnmounted(id);
                }
            },
            Qt::DirectConnection);

    MenuService::service()->registerScene(OpticalMenuSceneCreator::name(), new OpticalMenuSceneCreator);
}

bool Optical::start()
{
    addFileOperations();
    addCustomTopWidget();
    addDelegateSettings();

    return true;
}

dpf::Plugin::ShutdownFlag Optical::stop()
{
    return kSync;
}

void Optical::addOpticalCrumbToTitleBar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        TitleBar::CustomCrumbInfo info;
        info.scheme = Global::kBurn;
        info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == Global::kBurn; };
        info.seperateCb = [](const QUrl &url) -> QList<TitleBar::CrumbData> {
            QList<TitleBar::CrumbData> ret;
            QUrl curUrl(url);
            while (true) {
                auto fileInfo = InfoFactory::create<AbstractFileInfo>(curUrl);
                if (!fileInfo)
                    break;
                QString &&displayText = fileInfo->fileDisplayName();
                ret.push_front(TitleBar::CrumbData(curUrl, displayText));
                if (fileInfo->parentUrl() == QUrl::fromLocalFile(QDir::homePath())) {
                    ret.front().iconName = "media-optical-symbolic";
                    break;
                }
                curUrl = fileInfo->parentUrl();
            }
            return ret;
        };
        OpticalHelper::titleServIns()->addCustomCrumbar(info);
    });
}

void Optical::addFileOperations()
{
    OpticalHelper::workspaceServIns()->addScheme(Global::kBurn);
    OpticalHelper::workspaceServIns()->setWorkspaceMenuScene(Global::kBurn, OpticalMenuSceneCreator::name());

    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &OpticalFilesHelper::openFilesHandle;
    fileOpeationsHandle->copy = [](const quint64 windowId,
                                   const QList<QUrl> sources,
                                   const QUrl target,
                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags) -> JobHandlePointer {
        Q_UNUSED(windowId)
        Q_UNUSED(flags)
        OpticalFilesHelper::pasteFilesHandle(sources, target);
        return {};
    };
    fileOpeationsHandle->cut = [](const quint64 windowId,
                                  const QList<QUrl> sources,
                                  const QUrl target,
                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags) -> JobHandlePointer {
        Q_UNUSED(windowId)
        Q_UNUSED(flags)
        OpticalFilesHelper::pasteFilesHandle(sources, target, false);
        return {};
    };
    fileOpeationsHandle->writeUrlsToClipboard = &OpticalFilesHelper::writeUrlToClipboardHandle;
    fileOpeationsHandle->openInTerminal = &OpticalFilesHelper::openInTerminalHandle;
    fileOpeationsHandle->deletes = &OpticalFilesHelper::deleteFilesHandle;
    fileOpeationsHandle->moveToTash = &OpticalFilesHelper::deleteFilesHandle;
    fileOpeationsHandle->linkFile = &OpticalFilesHelper::linkFileHandle;
    OpticalHelper::fileOperationsServIns()->registerOperations(Global::kBurn, fileOpeationsHandle);
}

void Optical::addCustomTopWidget()
{
    Workspace::CustomTopWidgetInfo info;
    info.scheme = Global::kBurn;
    info.keepShow = false;
    info.createTopWidgetCb = []() {
        return new OpticalMediaWidget;
    };
    info.showTopWidgetCb = [](QWidget *w, const QUrl &url) {
        bool ret { true };
        OpticalMediaWidget *mediaWidget = qobject_cast<OpticalMediaWidget *>(w);
        if (mediaWidget)
            ret = mediaWidget->updateDiscInfo(url);

        return ret;
    };

    OpticalHelper::workspaceServIns()->addCustomTopWidget(info);
}

void Optical::addDelegateSettings()
{
    OpticalHelper::dlgateServIns()->registerTransparentHandle(Global::kBurn, [](const QUrl &url) -> bool {
        return !OpticalHelper::burnIsOnDisc(url);
    });
    OpticalHelper::dlgateServIns()->registerUrlTransform(Global::kBurn, &OpticalHelper::tansToLocalFile);
}

void Optical::onDeviceUnmounted(const QString &id)
{
    if (id.contains(QRegularExpression("sr[0-9]*$"))) {
        const QString &&volTag { id.mid(id.lastIndexOf("/") + 1) };
        QUrl url { QString("burn:///dev/%1/disc_files/").arg(volTag) };
        dpfSlotChannel->push("dfmplugin_workspace", "slot_CloseTab", url);
    }
}
