/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <klocale.h>

void KCMKToshibaGeneral::changedSlot()
{
    emit(changed());
}

// I don't like this approach, but anyway... I'll change it if I find another way of doing it...
void KCMKToshibaGeneral::cmdFnEscSlot( int slot )
{
    if (slot == 17) {
        FnEscle->show();
        FnEscle->setFocus();
    } else
        FnEscle->hide();
}


void KCMKToshibaGeneral::cmdFnF1Slot( int slot )
{
    if (slot == 17) {
        FnF1le->show();
        FnF1le->setFocus();
    } else
        FnF1le->hide();
}


void KCMKToshibaGeneral::cmdFnF2Slot( int slot )
{
    if (slot == 17) {
        FnF2le->show();
        FnF2le->setFocus();
    } else
        FnF2le->hide();
}


void KCMKToshibaGeneral::cmdFnF3Slot( int slot )
{
    if (slot == 17) {
        FnF3le->show();
        FnF3le->setFocus();
    } else
        FnF3le->hide();
}


void KCMKToshibaGeneral::cmdFnF4Slot( int slot )
{
    if (slot == 17) {
        FnF4le->show();
        FnF4le->setFocus();
    } else
        FnF4le->hide();
}


void KCMKToshibaGeneral::cmdFnF5Slot( int slot )
{
    if (slot == 17) {
        FnF5le->show();
        FnF5le->setFocus();
    } else
        FnF5le->hide();
}


void KCMKToshibaGeneral::cmdFnF6Slot( int slot )
{
    if (slot == 17) {
        FnF6le->show();
        FnF6le->setFocus();
    } else
        FnF6le->hide();
}


void KCMKToshibaGeneral::cmdFnF7Slot( int slot )
{
    if (slot == 17) {
        FnF7le->show();
        FnF7le->setFocus();
    } else
        FnF7le->hide();
}


void KCMKToshibaGeneral::cmdFnF8Slot( int slot )
{
    if (slot == 17) {
        FnF8le->show();
        FnF8le->setFocus();
    } else
        FnF8le->hide();
}


void KCMKToshibaGeneral::cmdFnF9Slot( int slot )
{
    if (slot == 17) {
        FnF9le->show();
        FnF9le->setFocus();
    } else
        FnF9le->hide();
}


void KCMKToshibaGeneral::brightSliderSlot( int bright )
{
    if (!bright)
        hpbright100tl->setNum(7);
}


void KCMKToshibaGeneral::proc100SliderSlot( int speed )
{
    if (speed == 1)
        hpproc100tl->setText(i18n("Low"));
    else
        hpproc100tl->setText(i18n("High"));
}


void KCMKToshibaGeneral::proc75SliderSlot( int speed )
{
    if (speed == 1)
        hpproc75tl->setText(i18n("Low"));
    else
        hpproc75tl->setText(i18n("High"));
}


void KCMKToshibaGeneral::proc50SliderSlot( int speed )
{
    if (speed == 1)
        hpproc50tl->setText(i18n("Low"));
    else
        hpproc50tl->setText(i18n("High"));
}


void KCMKToshibaGeneral::proc25SliderSlot( int speed )
{
    if (speed == 1)
        hpproc25tl->setText(i18n("Low"));
    else
        hpproc25tl->setText(i18n("High"));
}
