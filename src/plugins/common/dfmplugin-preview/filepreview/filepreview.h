/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef FILEPREVIEW_H
#define FILEPREVIEW_H

#include "dfmplugin_filepreview_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_filepreview {
class FilePreview : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "filepreview.json")

    DPF_EVENT_NAMESPACE(DPFILEPREVIEW_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_PreviewDialog_Show)

public:
    virtual void initialize() override;
    virtual bool start() override;
};
}   // namespace dfmplugin_filepreview
#endif   // FILEPREVIEW_H
