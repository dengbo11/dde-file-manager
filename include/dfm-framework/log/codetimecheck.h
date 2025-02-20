// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODETIMECHECK_H
#define CODETIMECHECK_H

#include <dfm-framework/dfm_framework_global.h>

#ifndef DPF_NO_CHECK_TIME   //make use

#    include <QMessageLogContext>

// QMessageLogContext类型的上下文构造
#    define CodeCheckLogContext                           \
        {                                                 \
            __FILE__, __LINE__, __FUNCTION__, "TimeCheck" \
        }
// 检查点Begin的宏定义，可在任意执行代码块中使用
#    define dpfCheckTimeBegin() dpf::CodeCheckTime::begin(CodeCheckLogContext)
// 检查点End的宏定义，可在任意执行代码块中使用
#    define dpfCheckTimeEnd() dpf::CodeCheckTime::end(CodeCheckLogContext)
#else   // define DPF_NO_CHECK_TIME
// 检查点Begin的宏定义，可在任意执行代码块中使用
#    define dpfCheckTimeBegin()
// 检查点End的宏定义，可在任意执行代码块中使用
#    define dpfCheckTimeEnd()
#endif   // DPF_NO_CHECK_TIME

DPF_BEGIN_NAMESPACE

class CodeCheckTime final
{
public:
    explicit CodeCheckTime() = delete;
    static void setLogCacheDayCount(uint dayCount);
    static uint logCacheDayCount();
    static void begin(const QMessageLogContext &context);
    static void end(const QMessageLogContext &context);
};

DPF_END_NAMESPACE

#endif   // CODETIMECHECK_H
