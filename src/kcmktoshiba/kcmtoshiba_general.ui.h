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


void KCMKToshibaGeneral::hpSlot()
{
    emit hpSignal( 0 );
}


void KCMKToshibaGeneral::dvdSlot()
{
    emit dvdSignal( 1 );
}


void KCMKToshibaGeneral::presentationSlot()
{
    emit presSignal( 2 );
}


void KCMKToshibaGeneral::hwChangedSlot( int bg )
{
    bg = 0;
    emit hwChanged();
}
