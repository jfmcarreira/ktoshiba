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

void BSMWidget::init()
{
    High = i18n("High");
    Low = i18n("Low");
    Never = i18n("Never");
    min30 = i18n("30 min");
    min20 = i18n("20 min");
    min15 = i18n("15 min");
    min10 = i18n("10 min");
    min05 = i18n("5 min");
    min03 = i18n("3 min");
    min01 = i18n("1 min");
}


void BSMWidget::proc100Slot( int speed )
{
    if (speed == 1)
        proc100tl->setText(Low);
    else
        proc100tl->setText(High);
}


void BSMWidget::proc75Slot( int speed )
{
    if (speed == 1)
        proc75tl->setText(Low);
    else
        proc75tl->setText(High);
}


void BSMWidget::proc50Slot( int speed )
{
    if (speed == 1)
        proc50tl->setText(Low);
    else
        proc50tl->setText(High);
}


void BSMWidget::proc25Slot( int speed )
{
    if (speed == 1)
        proc25tl->setText(Low);
    else
        proc25tl->setText(High);
}


void BSMWidget::bright100Slot( int bright )
{
    bright100tl->setNum(bright100s->maxValue() - bright);
}


void BSMWidget::bright75Slot( int bright )
{
    bright75tl->setNum(bright75s->maxValue() - bright);
}


void BSMWidget::bright50Slot( int bright )
{
    bright50tl->setNum(bright50s->maxValue() - bright);
}


void BSMWidget::bright25Slot( int bright )
{
    bright25tl->setNum(bright25s->maxValue() - bright);
}


void BSMWidget::okSlot()
{
    emit okPressed();
}


void BSMWidget::display100Slot( int time )
{
    switch (time) {
        case 0:
            display100tl->setText(Never);
            break;
        case 1:
            display100tl->setText(min30);
            break;
        case 2:
            display100tl->setText(min20);
            break;
        case 3:
            display100tl->setText(min15);
            break;
        case 4:
            display100tl->setText(min10);
            break;
        case 5:
            display100tl->setText(min05);
            break;
        case 6:
            display100tl->setText(min03);
            break;
        case 7:
            display100tl->setText(min01);
            break;
    }
}


void BSMWidget::display75Slot( int time )
{
    switch (time) {
        case 0:
            display75tl->setText(Never);
            break;
        case 1:
            display75tl->setText(min30);
            break;
        case 2:
            display75tl->setText(min20);
            break;
        case 3:
            display75tl->setText(min15);
            break;
        case 4:
            display75tl->setText(min10);
            break;
        case 5:
            display75tl->setText(min05);
            break;
        case 6:
            display75tl->setText(min03);
            break;
        case 7:
            display75tl->setText(min01);
            break;
    }
}


void BSMWidget::display50Slot( int time )
{
    switch (time) {
        case 0:
            display50tl->setText(Never);
            break;
        case 1:
            display50tl->setText(min30);
            break;
        case 2:
            display50tl->setText(min20);
            break;
        case 3:
            display50tl->setText(min15);
            break;
        case 4:
            display50tl->setText(min10);
            break;
        case 5:
            display50tl->setText(min05);
            break;
        case 6:
            display50tl->setText(min03);
            break;
        case 7:
            display50tl->setText(min01);
            break;
    }
}


void BSMWidget::display25Slot( int time )
{
    switch (time) {
        case 0:
            display25tl->setText(Never);
            break;
        case 1:
            display25tl->setText(min30);
            break;
        case 2:
            display25tl->setText(min20);
            break;
        case 3:
            display25tl->setText(min15);
            break;
        case 4:
            display25tl->setText(min10);
            break;
        case 5:
            display25tl->setText(min05);
            break;
        case 6:
            display25tl->setText(min03);
            break;
        case 7:
            display25tl->setText(min01);
            break;
    }
}


void BSMWidget::hdd100Slot( int time )
{
    switch (time) {
        case 0:
            hdd100tl->setText(Never);
            break;
        case 1:
            hdd100tl->setText(min30);
            break;
        case 2:
            hdd100tl->setText(min20);
            break;
        case 3:
            hdd100tl->setText(min15);
            break;
        case 4:
            hdd100tl->setText(min10);
            break;
        case 5:
            hdd100tl->setText(min05);
            break;
        case 6:
            hdd100tl->setText(min03);
            break;
        case 7:
            hdd100tl->setText(min01);
            break;
    }
}


void BSMWidget::hdd75Slot( int time )
{
    switch (time) {
        case 0:
            hdd75tl->setText(Never);
            break;
        case 1:
            hdd75tl->setText(min30);
            break;
        case 2:
            hdd75tl->setText(min20);
            break;
        case 3:
            hdd75tl->setText(min15);
            break;
        case 4:
            hdd75tl->setText(min10);
            break;
        case 5:
            hdd75tl->setText(min05);
            break;
        case 6:
            hdd75tl->setText(min03);
            break;
        case 7:
            hdd75tl->setText(min01);
            break;
    }
}


void BSMWidget::hdd50Slot( int time )
{
    switch (time) {
        case 0:
            hdd50tl->setText(Never);
            break;
        case 1:
            hdd50tl->setText(min30);
            break;
        case 2:
            hdd50tl->setText(min20);
            break;
        case 3:
            hdd50tl->setText(min15);
            break;
        case 4:
            hdd50tl->setText(min10);
            break;
        case 5:
            hdd50tl->setText(min05);
            break;
        case 6:
            hdd50tl->setText(min03);
            break;
        case 7:
            hdd50tl->setText(min01);
            break;
    }
}


void BSMWidget::hdd25Slot( int time )
{
    switch (time) {
        case 0:
            hdd25tl->setText(Never);
            break;
        case 1:
            hdd25tl->setText(min30);
            break;
        case 2:
            hdd25tl->setText(min20);
            break;
        case 3:
            hdd25tl->setText(min15);
            break;
        case 4:
            hdd25tl->setText(min10);
            break;
        case 5:
            hdd25tl->setText(min05);
            break;
        case 6:
            hdd25tl->setText(min03);
            break;
        case 7:
            hdd25tl->setText(min01);
            break;
    }
}
