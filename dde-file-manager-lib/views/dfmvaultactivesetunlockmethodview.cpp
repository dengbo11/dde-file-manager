#include "dfmvaultactivesetunlockmethodview.h"
#include "operatorcenter.h"

#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QToolTip>
#include <QRegExp>
#include <QRegExpValidator>
#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <DPasswordEdit>
#include <DLabel>

DFMVaultActiveSetUnlockMethodView::DFMVaultActiveSetUnlockMethodView(QWidget *parent)
    : QWidget(parent)
    , m_pTypeCombo(nullptr)
    , m_pPassword(nullptr)
    , m_pRepeatPassword(nullptr)
    , m_pPasswordHintLabel(nullptr)
    , m_pTips(nullptr)
//    , m_pOtherMethod(nullptr)
    , m_pNext(nullptr)
{
    // 设置开锁方式标签
    QLabel *pLabel = new QLabel(tr("Set Vault Password"), this);
    QFont font = pLabel->font();
    font.setBold(true);
    font.setPixelSize(18);
    pLabel->setFont(font);
    pLabel->setAlignment(Qt::AlignHCenter);

    // 类型
    DLabel *pTypeLabel = new DLabel(tr("Method"), this);
    m_pTypeCombo = new QComboBox(this);
    QStringList lstItems;
    lstItems << tr("Manual")/* << tr("Random")*/;
    m_pTypeCombo->addItems(lstItems);
    connect(m_pTypeCombo, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(slotTypeChanged(int)));

    // 设置密码
    m_pPasswordLabel = new DLabel(tr("Password"),this);
    m_pPassword = new DPasswordEdit(this);
    m_pPassword->lineEdit()->setPlaceholderText(tr("At least 8 characters, and contain A-Z, a-z, 0-9, and symbols"));
    connect(m_pPassword, &DPasswordEdit::textEdited,
            this, &DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(m_pPassword, &DPasswordEdit::editingFinished,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditFinished);
    connect(m_pPassword, &DPasswordEdit::textChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditing);
    connect(m_pPassword, &DPasswordEdit::focusChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged);


    // 重复密码
    m_pRepeatPasswordLabel = new DLabel(tr("Repeat password"), this);
    m_pRepeatPassword = new DPasswordEdit(this);
    m_pRepeatPassword->lineEdit()->setPlaceholderText(tr("Input the password again"));
    connect(m_pRepeatPassword, &DPasswordEdit::textEdited,
            this, &DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(m_pRepeatPassword, &DPasswordEdit::editingFinished,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished);
    connect(m_pRepeatPassword, &DPasswordEdit::textChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditing);
    connect(m_pRepeatPassword, &DPasswordEdit::focusChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged);

    // 提示信息
    m_pPasswordHintLabel = new DLabel(tr("Hint"), this);
    m_pTips = new QLineEdit(this);
    m_pTips->setMaxLength(14);
    m_pTips->setPlaceholderText(tr("Password Hint"));



    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    m_pNext->setFixedSize(452, 30);
    m_pNext->setEnabled(false);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSetUnlockMethodView::slotNextBtnClicked);

    // 其他解锁方式
//    m_pOtherMethod = new QCheckBox(tr("Fingerprint"), this);
//    m_pOtherMethod->setVisible(false);
    // 长度
//    m_pLengthLabel = new QLabel(tr("Length"), this);
//    m_pLengthSlider = new QSlider(this);
//    m_pLengthSlider->setOrientation(Qt::Horizontal);
//    m_pLengthSlider->setRange(8, 24);
//    connect(m_pLengthSlider, &QSlider::valueChanged,
//            this, &DFMVaultActiveSetUnlockMethodView::slotLengthChanged);
//    m_pLengthEdit = new QLineEdit(this);
//    m_pLengthEdit->setMaximumWidth(60);
//    m_pLengthEdit->setReadOnly(true);
//    m_pLengthEdit->setText(tr("8Bit"));
//    m_pStrengthLabel = new QLabel(tr("Week"), this);
//    m_pStrengthLabel->setStyleSheet("color: #FF4500");
//    m_pStrengthLabel->setMaximumWidth(50);
//    m_pLengthLabel->setVisible(false);
//    m_pLengthSlider->setVisible(false);
//    m_pLengthEdit->setVisible(false);
//    m_pStrengthLabel->setVisible(false);

    // 结果
//    m_pResultLabel = new QLabel(tr("Result"), this);
//    m_pResultEdit = new QLineEdit(tr(""), this);
//    connect(m_pResultEdit, &QLineEdit::textChanged,
//            this, &DFMVaultActiveSetUnlockMethodView::slotGenerateEditChanged);
//    m_pResultLabel->setVisible(false);
//    m_pResultEdit->setVisible(false);

    // 生成随即密码按钮
//    m_pGenerateBtn = new QPushButton(tr("Generate"), this);
//    connect(m_pGenerateBtn, &QPushButton::clicked,
//            this, &DFMVaultActiveSetUnlockMethodView::slotGeneratePasswordBtnClicked);
//    m_pGenerateBtn->setVisible(false);

    // 布局
    play1 = new QGridLayout();
    play1->setMargin(0);

    play1->addWidget(pTypeLabel, 0, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pTypeCombo, 0, 1, 1, 5);

    play1->addWidget(m_pPasswordLabel, 1, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pPassword, 1, 1, 1,5);

    play1->addWidget(m_pRepeatPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pRepeatPassword, 2, 1, 1, 5);

    play1->addWidget(m_pPasswordHintLabel, 3, 0, 1, 1, Qt::AlignLeft);
    play1->addWidget(m_pTips, 3, 1, 1, 5);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel);
    play->addSpacing(15);
    play->addLayout(play1);
    play->addStretch();
    play->addWidget(m_pNext, 0, Qt::AlignCenter);

    // 创建文件夹与目录
    if(!OperatorCenter::getInstance().createDirAndFile()) return;
}

void DFMVaultActiveSetUnlockMethodView::clearText()
{
    m_pPassword->clear();
    m_pPassword->hideAlertMessage();
    m_pPassword->lineEdit()->setStyleSheet("");
    m_pRepeatPassword->clear();
    m_pRepeatPassword->hideAlertMessage();
    m_pRepeatPassword->lineEdit()->setStyleSheet("");
    m_pTips->clear();
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditing()
{
    bool ok = checkPassword(m_pPassword->text());
    if (ok) {
        if(checkInputInfo()){
            m_pNext->setEnabled(true);
        }
    }
    m_pNext->setEnabled(false);
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditFinished()
{
    bool ok = checkPassword(m_pPassword->text());
    if(!ok){
        m_pNext->setEnabled(false);
        m_pPassword->lineEdit()->setStyleSheet("background-color:rgba(241, 57, 50, 0.15)");
        m_pPassword->showAlertMessage(tr("At least 8 characters, and contain A-Z, a-z, 0-9, and symbols"),TIPS_TIME);
    } else {
        if(checkInputInfo()){
            m_pNext->setEnabled(true);
        }
    }
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditFocusChanged(bool bFocus)
{
    if(bFocus){
        m_pPassword->lineEdit()->setStyleSheet("");
        m_pPassword->hideAlertMessage();
    }
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished()
{
    bool ok = checkRepeatPassword();
    if(!ok){
        m_pRepeatPassword->lineEdit()->setStyleSheet("background-color:rgba(241, 57, 50, 0.15)");
        m_pRepeatPassword->showAlertMessage(tr("Passwords do not match"), TIPS_TIME);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditing()
{
    const QString &strRepeatPassword = m_pRepeatPassword->text();
    const QString &strPassword = m_pPassword->text();

    bool bSizeMatch = strRepeatPassword.size() == strPassword.size();
    if (bSizeMatch) {
        if (checkPassword(m_pPassword->text())) {
            if (checkRepeatPassword()) {
                m_pNext->setEnabled(true);
                return;
            } else {
                 m_pRepeatPassword->showAlertMessage(tr("Passwords do not match"));
            }
        }
    }
    m_pNext->setEnabled(false);
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFocusChanged(bool bFocus)
{
    if(bFocus){
        m_pRepeatPassword->lineEdit()->setStyleSheet("");
        m_pRepeatPassword->hideAlertMessage();
    }
}

void DFMVaultActiveSetUnlockMethodView::slotGenerateEditChanged(const QString &str)
{
    if(!checkPassword(str)){
        m_pNext->setEnabled(false);
    }else{
        m_pNext->setEnabled(true);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotNextBtnClicked()
{
    QString strPassword = m_pPassword->text();
    QString strPasswordHint = m_pTips->text();
    if(OperatorCenter::getInstance().saveSaltAndClipher(strPassword, strPasswordHint)
            && OperatorCenter::getInstance().createKey(strPassword, USER_KEY_LENGTH))
        emit sigAccepted();
}

void DFMVaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    if(index){  // 随机

        play1->removeWidget(m_pPasswordLabel);
        play1->removeWidget(m_pPassword);
        play1->removeWidget(m_pRepeatPasswordLabel);
        play1->removeWidget(m_pRepeatPassword);
//        play1->removeWidget(m_pOtherMethod);

        m_pPasswordLabel->setVisible(false);
        m_pPassword->setVisible(false);
        m_pRepeatPasswordLabel->setVisible(false);
        m_pRepeatPassword->setVisible(false);
//        m_pOtherMethod->setVisible(false);

//        play1->addWidget(m_pLengthLabel, 2, 0, 1, 1, Qt::AlignLeft);
//        play1->addWidget(m_pLengthSlider, 2, 1, 1, 3);
//        play1->addWidget(m_pLengthEdit, 2, 4, 1, 1);
//        play1->addWidget(m_pStrengthLabel, 2, 5, 1, 1);
//        play1->addWidget(m_pResultLabel, 3, 0, 1, 1, Qt::AlignLeft);
//        play1->addWidget(m_pResultEdit, 3, 1, 1, 5);
//        play1->addWidget(m_pGenerateBtn, 5, 2, 1, 2, Qt::AlignHCenter);

//        m_pLengthLabel->setVisible(true);
//        m_pLengthSlider->setVisible(true);
//        m_pLengthEdit->setVisible(true);
//        m_pStrengthLabel->setVisible(true);
//        m_pResultLabel->setVisible(true);
//        m_pResultEdit->setVisible(true);
//        m_pGenerateBtn->setVisible(true);

        // 检测密码的正确性
//        slotGenerateEditChanged(m_pResultEdit->text());

    }else{  // 手动

        play1->addWidget(m_pPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
        play1->addWidget(m_pPassword, 2, 1, 1, 5);
        play1->addWidget(m_pRepeatPasswordLabel, 3, 0, 1, 1, Qt::AlignLeft);
        play1->addWidget(m_pRepeatPassword, 3, 1, 1, 5);
//        play1->addWidget(m_pOtherMethod, 5, 2, 1, 2, Qt::AlignHCenter);

        m_pPasswordLabel->setVisible(true);
        m_pPassword->setVisible(true);
        m_pRepeatPasswordLabel->setVisible(true);
        m_pRepeatPassword->setVisible(true);
//        m_pOtherMethod->setVisible(true);

//        play1->removeWidget(m_pLengthLabel);
//        play1->removeWidget(m_pLengthSlider);
//        play1->removeWidget(m_pLengthEdit);
//        play1->removeWidget(m_pStrengthLabel);
//        play1->removeWidget(m_pResultLabel);
//        play1->removeWidget(m_pResultEdit);
//        play1->removeWidget(m_pGenerateBtn);

//        m_pLengthLabel->setVisible(false);
//        m_pLengthSlider->setVisible(false);
//        m_pLengthEdit->setVisible(false);
//        m_pStrengthLabel->setVisible(false);
//        m_pResultLabel->setVisible(false);
//        m_pResultEdit->setVisible(false);
//        m_pGenerateBtn->setVisible(false);

        // 检测密码正确性
        slotRepeatPasswordEditFinished();
    }
}

//void DFMVaultActiveSetUnlockMethodView::slotLengthChanged(int length)
//{
//    m_pLengthEdit->setText(QString("%1%2").arg(length).arg(tr("Bit")));
//    if(length > 7 && length < 12){
//        m_pStrengthLabel->setText(tr("Week"));
//        m_pStrengthLabel->setStyleSheet("color: #FF4500");
//    }else if(length >= 12 && length < 19){
//        m_pStrengthLabel->setText(tr("Medium"));
//        m_pStrengthLabel->setStyleSheet("color: #EEC900");
//    }else if(length >= 19 && length < 25){
//        m_pStrengthLabel->setText(tr("Strong"));
//        m_pStrengthLabel->setStyleSheet("color: #32CD32");
//    }
//}

void DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength(const QString &password)
{
    DPasswordEdit* pPasswordEdit = qobject_cast<DPasswordEdit*>(sender());
    if(password.length() > PASSWORD_LENGHT_MAX){
        pPasswordEdit->setText(password.mid(0, PASSWORD_LENGHT_MAX));
    }
}

//void DFMVaultActiveSetUnlockMethodView::slotGeneratePasswordBtnClicked()
//{
//    QString strPassword = OperatorCenter::getInstance().autoGeneratePassword(m_pLengthSlider->value());
//    m_pResultEdit->setText(strPassword);
//}

bool DFMVaultActiveSetUnlockMethodView::checkPassword(const QString &password)
{
    QString strPassword = password;

//    QRegExp rx("^(?![^a-zA-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegExp rx("^(?![^a-z]+$)(?![^A-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegExpValidator v(rx);
    int pos = 0;
    QValidator::State res;
    res = v.validate(strPassword, pos);
    if(QValidator::Acceptable != res){
        return false;

    } else {
        return true;
    }
}

bool DFMVaultActiveSetUnlockMethodView::checkRepeatPassword()
{
    const QString &strRepeatPassword = m_pRepeatPassword->text();
    const QString &strPassword = m_pPassword->text();
    if(strRepeatPassword != strPassword)
    {
        return false;
    } else {
        return true;
    }
}

bool DFMVaultActiveSetUnlockMethodView::checkInputInfo()
{
    if(checkPassword(m_pPassword->text()) && checkRepeatPassword()){
        return true;
    } else {
        return false;
    }
}
