#include "simplecontactlist.h"
#include "simplecontactlistview.h"
#include "simplecontactlistmodel.h"
#include "simplecontactlistitem.h"
#include "src/modulemanagerimpl.h"
#include "libqutim/protocol.h"
#include "libqutim/account.h"
#include "libqutim/icon.h"
#include "libqutim/settingslayer.h"
#include <QTreeView>
#include <QDebug>
#include <QStringBuilder>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>

namespace Core
{
	namespace SimpleContactList
	{
		static CoreSingleModuleHelper<Module> contact_list_static(
				QT_TRANSLATE_NOOP("Plugin", "Simple ContactList"),
				QT_TRANSLATE_NOOP("Plugin", "Default qutIM contact list realization. Just simple")
				);

		struct ModulePrivate
		{
			QWidget *widget;
			TreeView *view;
			Model *model;
			ActionToolBar *main_toolbar;
			ActionToolBar *bottom_toolbar;
		};

		Module::Module() : p(new ModulePrivate)
		{
			p->widget = new QWidget;
			QVBoxLayout *layout = new QVBoxLayout(p->widget);
			layout->setMargin(0);

			p->main_toolbar = new ActionToolBar(p->widget);
			layout->addWidget(p->main_toolbar);

			QMenu *menu = new QMenu(tr("Main menu"), p->main_toolbar);
			menu->addAction(Icon("configure"), tr("&Settings..."), this, SLOT(onConfigureClicked()));
			menu->addAction(Icon("application-exit"), tr("&Quit"), qApp, SLOT(quit()));

			QAction *menuAction = new QAction(Icon("show-menu"), tr("Main menu"), menu);
			menuAction->setMenu(menu);
			p->main_toolbar->addAction(menuAction);

			p->view = new TreeView(p->widget);
			layout->addWidget(p->view);

			p->widget->setLayout(layout);
			p->model = new Model(p->view);
			p->view->setModel(p->model);
			p->widget->show();

			p->bottom_toolbar = new ActionToolBar(p->widget);
			layout->addWidget(p->bottom_toolbar);
			foreach(Protocol *proto, allProtocols()) {
				connect(proto, SIGNAL(accountCreated(qutim_sdk_0_3::Account*)), this, SLOT(onAccountCreated(qutim_sdk_0_3::Account*)));
				foreach(Account *account, proto->accounts()) {
					onAccountCreated(account);
				}
			}
			//set widget size (test) TODO
			QRect geom = Config("other/screenGeometry").value<QRect>("contactList",QRect());
			if (geom.isEmpty()) {
				int width = 150; //TODO
				geom = QApplication::desktop()->availableGeometry(QCursor::pos());
				geom.setX(geom.width() - width);
				geom.setWidth(width);
			}
			p->widget->setGeometry(geom);
		}

		Module::~Module()
		{
			Config geometry = Config("other/screenGeometry");
			geometry.setValue("contactList",p->widget->geometry());
			geometry.sync();
		}

		void Module::addContact(ChatUnit *unit)
		{
			if(Contact *contact = qobject_cast<Contact *>(unit))
				p->model->addContact(contact);
		}

		void Module::removeContact(ChatUnit *unit)
		{
			if(Contact *contact = qobject_cast<Contact *>(unit))
				p->model->removeContact(contact);
		}

		void Module::removeAccount(Account *account)
		{
			foreach(Contact *contact, account->findChildren<Contact *>())
				removeContact(contact);
		}

		void Module::addButton(ActionGenerator *generator)
		{
			p->main_toolbar->addAction(generator);
		}

		void Module::onConfigureClicked()
		{
			Settings::showWidget();
		}

		void Module::onAccountCreated(Account *account)
		{
			//TODO add account icon
			QAction *action = p->bottom_toolbar->addAction(Icon("user-online"),
														   account->name(),
														   this, SLOT(onAccountButtonClicked()));
			action->setData(QVariant::fromValue<MenuController *>(account));
			foreach (Contact *contact, account->findChildren<Contact *>()) {
				//FIXME
				addContact(contact);
			}
		}

		void Module::onAccountButtonClicked()
		{
			QAction *action = qobject_cast<QAction *>(sender());
			Q_ASSERT(action);
			if(Account *account = qobject_cast<Account *>(action->data().value<MenuController *>()))
				account->showMenu(QCursor::pos());
		}
	}
}
