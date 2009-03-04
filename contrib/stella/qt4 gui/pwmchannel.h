#ifndef PWMCHANNEL_STELLA_H
#define PWMCHANNEL_STELLA_H

#include <QtGui/QFrame>

namespace Ui {
    class pwmchannelui;
}

class pwmchannel : public QFrame {
    Q_OBJECT
    Q_DISABLE_COPY(pwmchannel)
public:
    explicit pwmchannel(unsigned char channel_no, const QString& stella_host, QWidget *parent = 0);
    virtual ~pwmchannel();
    unsigned char getValue();
    void setValue(unsigned char value, bool noPropagation = false);
    void update();
    unsigned char channel_no;

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::pwmchannelui *m_ui;
    QString settings_path;
    bool noPropagation;

private Q_SLOTS:
    void on_btnRandom_clicked();
    void slider_valueChanged(int value);
    void on_linename_textEdited(const QString& text);

Q_SIGNALS:
    void value_changed(unsigned char value, unsigned char channel);
};

#endif // PWMCHANNEL_H
