/***************************************************************************
 *   Copyright (C) 2009 by david   *
 *   david.graeff@web.de           *
 *                                                                         *
 *   StellaControl is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   StellaControl is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with StellaControl.  If not, see <http://www.gnu.org/licenses/> *
 ***************************************************************************/

#include "pwmchannel.h"
#include "ui_pwmchannel.h"
#include <cstdlib> //rand
#include <QSettings>

pwmchannel::pwmchannel(unsigned char channel_no, const QString& stella_host, QWidget *parent) :
    QFrame(parent),
    channel_no(channel_no),
    m_ui(new Ui::pwmchannelui)
{
    QSettings settings;
    noPropagation = false;
    m_ui->setupUi(this);
    settings_path = QLatin1String("hosts/") + stella_host +
                    QLatin1String("/channel") + QString::number(channel_no) + QLatin1String("/");
    connect(m_ui->btnRandom, SIGNAL(clicked()), SLOT(on_btnRandom_clicked()));
    connect(m_ui->slider, SIGNAL(valueChanged(int)), SLOT(slider_valueChanged(int)));
    QString default_name = tr("Channel") + QString::number(channel_no);
    m_ui->linename->setText(settings.value(settings_path+QLatin1String("name"),default_name).toString());
}

pwmchannel::~pwmchannel()
{
    delete m_ui;
}

void pwmchannel::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void pwmchannel::update() {
    if (m_ui->btnFadeDown->isChecked()) {
        m_ui->slider->setValue(m_ui->slider->value()-1);
        if (m_ui->slider->value()==m_ui->slider->minimum()) m_ui->btnNoFade->setChecked(true);
    } else if (m_ui->btnFadeUp->isChecked()) {
        m_ui->slider->setValue(m_ui->slider->value()+1);
        if (m_ui->slider->value()==m_ui->slider->maximum()) m_ui->btnNoFade->setChecked(true);
    }
}

unsigned char pwmchannel::getValue() {
    return m_ui->slider->value();
}

void pwmchannel::setValue(unsigned char value, bool noPropagation) {
    this->noPropagation = noPropagation;
    m_ui->slider->setValue((int)value);
    this->noPropagation = false;
}

void pwmchannel::on_btnRandom_clicked() {
    m_ui->slider->setValue(rand() % 256);
}

void pwmchannel::slider_valueChanged(int value) {
    if (this->noPropagation) return;
    emit value_changed((unsigned char)value, channel_no);
}

void pwmchannel::on_linename_textEdited(const QString& text)
{
    QSettings().setValue(settings_path+QLatin1String("name"), text);
}
