/********************************************************************************
** Form generated from reading ui file 'stellaserver.ui'
**
** Created: Thu Mar 12 23:14:31 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_STELLASERVER_H
#define UI_STELLASERVER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StellaServer
{
public:
    QAction *actionBroadcastChanges;
    QAction *actionFade_Test_1;
    QAction *actionFade_Test_2;
    QAction *actionSet_speed_rate;
    QAction *actionFade_Test_3;
    QAction *actionFade_Test_4;
    QAction *actionChannelCount;
    QAction *actionRefetch;
    QAction *actionResend;
    QWidget *page;
    QGridLayout *gridLayout;
    QSpacerItem *verticalSpacer_5;
    QSpacerItem *horizontalSpacer_4;
    QLabel *lblConnect;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *btnAbortConnection;
    QSpacerItem *verticalSpacer_6;
    QWidget *pwmchannels;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QToolButton *btnLoadEEPROM;
    QToolButton *btnSaveEEPROM;
    QFrame *frame_4;
    QHBoxLayout *horizontalLayout_5;
    QToolButton *btnTestcase;
    QSpacerItem *horizontalSpacer;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *btnAbout;
    QToolButton *btnClose;
    QSpacerItem *verticalSpacer;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *predefinedlist;
    QPushButton *btnRemovePredefined;
    QPushButton *btnSavePredefined;
    QWidget *page_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_3;
    QSpinBox *spinBeat;
    QSpacerItem *horizontalSpacer_2;
    QFrame *frame_7;
    QHBoxLayout *horizontalLayout_9;
    QToolButton *btnAbout_2;
    QToolButton *btnClose_2;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QStackedWidget *StellaServer)
    {
        if (StellaServer->objectName().isEmpty())
            StellaServer->setObjectName(QString::fromUtf8("StellaServer"));
        StellaServer->resize(585, 316);
        actionBroadcastChanges = new QAction(StellaServer);
        actionBroadcastChanges->setObjectName(QString::fromUtf8("actionBroadcastChanges"));
        actionBroadcastChanges->setCheckable(true);
        actionFade_Test_1 = new QAction(StellaServer);
        actionFade_Test_1->setObjectName(QString::fromUtf8("actionFade_Test_1"));
        actionFade_Test_2 = new QAction(StellaServer);
        actionFade_Test_2->setObjectName(QString::fromUtf8("actionFade_Test_2"));
        actionSet_speed_rate = new QAction(StellaServer);
        actionSet_speed_rate->setObjectName(QString::fromUtf8("actionSet_speed_rate"));
        actionFade_Test_3 = new QAction(StellaServer);
        actionFade_Test_3->setObjectName(QString::fromUtf8("actionFade_Test_3"));
        actionFade_Test_4 = new QAction(StellaServer);
        actionFade_Test_4->setObjectName(QString::fromUtf8("actionFade_Test_4"));
        actionChannelCount = new QAction(StellaServer);
        actionChannelCount->setObjectName(QString::fromUtf8("actionChannelCount"));
        actionRefetch = new QAction(StellaServer);
        actionRefetch->setObjectName(QString::fromUtf8("actionRefetch"));
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/arrow-left.png")), QIcon::Normal, QIcon::Off);
        actionRefetch->setIcon(icon);
        actionResend = new QAction(StellaServer);
        actionResend->setObjectName(QString::fromUtf8("actionResend"));
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/icons/arrow-right.png")), QIcon::Normal, QIcon::Off);
        actionResend->setIcon(icon1);
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        gridLayout = new QGridLayout(page);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalSpacer_5 = new QSpacerItem(20, 121, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_5, 0, 1, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(204, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 1, 0, 1, 1);

        lblConnect = new QLabel(page);
        lblConnect->setObjectName(QString::fromUtf8("lblConnect"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lblConnect->sizePolicy().hasHeightForWidth());
        lblConnect->setSizePolicy(sizePolicy);
        lblConnect->setText(QString::fromUtf8("TEXT"));
        lblConnect->setAlignment(Qt::AlignCenter);
        lblConnect->setWordWrap(true);

        gridLayout->addWidget(lblConnect, 1, 1, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(204, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_5, 1, 2, 1, 1);

        btnAbortConnection = new QPushButton(page);
        btnAbortConnection->setObjectName(QString::fromUtf8("btnAbortConnection"));

        gridLayout->addWidget(btnAbortConnection, 2, 1, 1, 1);

        verticalSpacer_6 = new QSpacerItem(20, 121, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_6, 3, 1, 1, 1);

        StellaServer->addWidget(page);
        pwmchannels = new QWidget();
        pwmchannels->setObjectName(QString::fromUtf8("pwmchannels"));
        verticalLayout = new QVBoxLayout(pwmchannels);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        frame_3 = new QFrame(pwmchannels);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(frame_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        btnLoadEEPROM = new QToolButton(frame_3);
        btnLoadEEPROM->setObjectName(QString::fromUtf8("btnLoadEEPROM"));
        QIcon icon2;
        icon2.addPixmap(QPixmap(QString::fromUtf8(":/icons/document-open.png")), QIcon::Normal, QIcon::Off);
        btnLoadEEPROM->setIcon(icon2);
        btnLoadEEPROM->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(btnLoadEEPROM);

        btnSaveEEPROM = new QToolButton(frame_3);
        btnSaveEEPROM->setObjectName(QString::fromUtf8("btnSaveEEPROM"));
        QIcon icon3;
        icon3.addPixmap(QPixmap(QString::fromUtf8(":/icons/document-save.png")), QIcon::Normal, QIcon::Off);
        btnSaveEEPROM->setIcon(icon3);
        btnSaveEEPROM->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_3->addWidget(btnSaveEEPROM);


        horizontalLayout_4->addWidget(frame_3);

        frame_4 = new QFrame(pwmchannels);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        horizontalLayout_5 = new QHBoxLayout(frame_4);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        btnTestcase = new QToolButton(frame_4);
        btnTestcase->setObjectName(QString::fromUtf8("btnTestcase"));
        QIcon icon4;
        icon4.addPixmap(QPixmap(QString::fromUtf8(":/icons/flag.png")), QIcon::Normal, QIcon::Off);
        btnTestcase->setIcon(icon4);
        btnTestcase->setPopupMode(QToolButton::InstantPopup);
        btnTestcase->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        horizontalLayout_5->addWidget(btnTestcase);


        horizontalLayout_4->addWidget(frame_4);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        frame_2 = new QFrame(pwmchannels);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        btnAbout = new QToolButton(frame_2);
        btnAbout->setObjectName(QString::fromUtf8("btnAbout"));
        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/icons/help-hint.png")), QIcon::Normal, QIcon::Off);
        btnAbout->setIcon(icon5);

        horizontalLayout_2->addWidget(btnAbout);

        btnClose = new QToolButton(frame_2);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));
        QIcon icon6;
        icon6.addPixmap(QPixmap(QString::fromUtf8(":/icons/dialog-close.png")), QIcon::Normal, QIcon::Off);
        btnClose->setIcon(icon6);

        horizontalLayout_2->addWidget(btnClose);


        horizontalLayout_4->addWidget(frame_2);


        verticalLayout->addLayout(horizontalLayout_4);

        verticalSpacer = new QSpacerItem(20, 395, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        frame = new QFrame(pwmchannels);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        predefinedlist = new QComboBox(frame);
        predefinedlist->setObjectName(QString::fromUtf8("predefinedlist"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(predefinedlist->sizePolicy().hasHeightForWidth());
        predefinedlist->setSizePolicy(sizePolicy1);
        predefinedlist->setEditable(true);

        horizontalLayout->addWidget(predefinedlist);

        btnRemovePredefined = new QPushButton(frame);
        btnRemovePredefined->setObjectName(QString::fromUtf8("btnRemovePredefined"));
        QIcon icon7;
        icon7.addPixmap(QPixmap(QString::fromUtf8(":/icons/edit-delete.png")), QIcon::Normal, QIcon::Off);
        btnRemovePredefined->setIcon(icon7);

        horizontalLayout->addWidget(btnRemovePredefined);

        btnSavePredefined = new QPushButton(frame);
        btnSavePredefined->setObjectName(QString::fromUtf8("btnSavePredefined"));
        btnSavePredefined->setIcon(icon3);

        horizontalLayout->addWidget(btnSavePredefined);


        verticalLayout->addWidget(frame);

        StellaServer->addWidget(pwmchannels);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        verticalLayout_2 = new QVBoxLayout(page_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_3 = new QLabel(page_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_6->addWidget(label_3);

        spinBeat = new QSpinBox(page_2);
        spinBeat->setObjectName(QString::fromUtf8("spinBeat"));
        spinBeat->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        spinBeat->setMinimum(1);
        spinBeat->setMaximum(200);

        horizontalLayout_6->addWidget(spinBeat);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        frame_7 = new QFrame(page_2);
        frame_7->setObjectName(QString::fromUtf8("frame_7"));
        frame_7->setFrameShape(QFrame::StyledPanel);
        frame_7->setFrameShadow(QFrame::Raised);
        horizontalLayout_9 = new QHBoxLayout(frame_7);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        btnAbout_2 = new QToolButton(frame_7);
        btnAbout_2->setObjectName(QString::fromUtf8("btnAbout_2"));
        btnAbout_2->setIcon(icon5);

        horizontalLayout_9->addWidget(btnAbout_2);

        btnClose_2 = new QToolButton(frame_7);
        btnClose_2->setObjectName(QString::fromUtf8("btnClose_2"));
        btnClose_2->setIcon(icon6);

        horizontalLayout_9->addWidget(btnClose_2);


        horizontalLayout_6->addWidget(frame_7);


        verticalLayout_2->addLayout(horizontalLayout_6);

        verticalSpacer_2 = new QSpacerItem(20, 260, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        StellaServer->addWidget(page_2);
#ifndef QT_NO_SHORTCUT
        label->setBuddy(predefinedlist);
#endif // QT_NO_SHORTCUT

        retranslateUi(StellaServer);

        StellaServer->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(StellaServer);
    } // setupUi

    void retranslateUi(QStackedWidget *StellaServer)
    {
        StellaServer->setWindowTitle(QApplication::translate("StellaServer", "StackedWidget", 0, QApplication::UnicodeUTF8));
        actionBroadcastChanges->setText(QApplication::translate("StellaServer", "Stella Broadcast Changes", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionBroadcastChanges->setToolTip(QApplication::translate("StellaServer", "Broadcast changes 1s after the last change", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionFade_Test_1->setText(QApplication::translate("StellaServer", "Fade Test 1 (Up)", 0, QApplication::UnicodeUTF8));
        actionFade_Test_2->setText(QApplication::translate("StellaServer", "Fade Test 2 (Down)", 0, QApplication::UnicodeUTF8));
        actionSet_speed_rate->setText(QApplication::translate("StellaServer", "Set speed rate", 0, QApplication::UnicodeUTF8));
        actionFade_Test_3->setText(QApplication::translate("StellaServer", "Fade test 3 (Flashy)", 0, QApplication::UnicodeUTF8));
        actionFade_Test_4->setText(QApplication::translate("StellaServer", "Regression Test 4 (1-0 Flicker)", 0, QApplication::UnicodeUTF8));
        actionChannelCount->setText(QApplication::translate("StellaServer", "Visible Channels", 0, QApplication::UnicodeUTF8));
        actionRefetch->setText(QApplication::translate("StellaServer", "Refetch", 0, QApplication::UnicodeUTF8));
        actionResend->setText(QApplication::translate("StellaServer", "Resend", 0, QApplication::UnicodeUTF8));
        btnAbortConnection->setText(QApplication::translate("StellaServer", "Abort", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("StellaServer", "EEProm:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnLoadEEPROM->setToolTip(QApplication::translate("StellaServer", "Load values from stella eeprom", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnLoadEEPROM->setText(QApplication::translate("StellaServer", "Load", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnSaveEEPROM->setToolTip(QApplication::translate("StellaServer", "Save current values to eeprom", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnSaveEEPROM->setText(QApplication::translate("StellaServer", "Save", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnTestcase->setToolTip(QApplication::translate("StellaServer", "Testcases like Fading up/down", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnTestcase->setText(QApplication::translate("StellaServer", "Testcases", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnAbout->setToolTip(QApplication::translate("StellaServer", "About stella server", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnAbout->setText(QApplication::translate("StellaServer", "About stella server", 0, QApplication::UnicodeUTF8));
        btnClose->setText(QApplication::translate("StellaServer", "Close stella server connection", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("StellaServer", "Predefined sets:", 0, QApplication::UnicodeUTF8));
        btnRemovePredefined->setText(QApplication::translate("StellaServer", "Remove", 0, QApplication::UnicodeUTF8));
        btnSavePredefined->setText(QApplication::translate("StellaServer", "Save", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_3->setToolTip(QApplication::translate("StellaServer", "Beats per minute", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("StellaServer", "BPM:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnAbout_2->setToolTip(QApplication::translate("StellaServer", "About stella server", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnAbout_2->setText(QApplication::translate("StellaServer", "About stella server", 0, QApplication::UnicodeUTF8));
        btnClose_2->setText(QApplication::translate("StellaServer", "Close stella server connection", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(StellaServer);
    } // retranslateUi

};

namespace Ui {
    class StellaServer: public Ui_StellaServer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STELLASERVER_H
