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

#include <kstandarddirs.h>
#include <kprocess.h>
#include <kmessagebox.h>

void KCMKToshibaGeneral::changedSlot()
{
    emit(changed());
}


void KCMKToshibaGeneral::setupHelperSlot()
{
    QString helper = KStandardDirs::findExe("ktosh_helper");
    if (helper.isEmpty())
        helper = KStandardDirs::findExe("klaptop_acpi_helper");
    if (helper.isEmpty()) {
        KMessageBox::sorry(0, i18n("The helper cannot be found.\n"
			   "Please make sure that it is installed corectly"),
			   i18n("KToshiba"));
        return;
    }

    QString kdesu = KStandardDirs::findExe("kdesu");
    if (!kdesu.isEmpty()) {
        int rc = KMessageBox::warningContinueCancel(0,
		i18n("You will need to supply a root password "
		     "to allow the privileges of ktosh_helper to change."),
		i18n("KToshiba"), KStdGuiItem::cont(), "");
        if (rc == KMessageBox::Continue) {
            KProcess kproc;
            kproc << kdesu;
            kproc << "-u";
            kproc << "root";
            kproc << "chown root "+helper+"; chmod +s "+helper;
            kproc.start(KProcess::Block);
        }
    } else {
        KMessageBox::sorry(0, i18n("The helper cannot be enabled because kdesu cannot be found.\n"
			   "Please make sure that it is installed correctly."),
			   i18n("KToshiba"));
    }
}
