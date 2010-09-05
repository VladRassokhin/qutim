/*
    Floaties

    Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#include "floaties.h"
#include <qutim/icon.h>
#include <qutim/debug.h>
#include <qutim/mimeobjectdata.h>
#include <QDesktopWidget>
#include <QApplication>
#include <QTime>
#include <qutim/event.h>

void FloatiesPlugin::init()
{
	addAuthor(QT_TRANSLATE_NOOP("Author", "Ruslan Nigmatullin"),
		QT_TRANSLATE_NOOP("Task", "Author"),
		QLatin1String("euroelessar@gmail.com"));
	setInfo(QT_TRANSLATE_NOOP("Plugin", "Floaties"),
			QT_TRANSLATE_NOOP("Plugin", "Implementation of floaty contacts"),
			PLUGIN_VERSION(0, 2, 0, 0),
			ExtensionIcon("bookmark-new"));
	m_eventId = Event::registerType("contact-list-drop");
	m_model = 0;
//	m_action = new ActionGenerator(QIcon(), QT_TRANSLATE_NOOP("Floaties", "Remove from Desktop"),
//								   this, SLOT(onRemoveContact(QObject*)));
}

bool FloatiesPlugin::load()
{
	if (!m_view) {
		if (QObject *contactList = getService("ContactList")) {
			QWidget *widget = contactList->property("widget").value<QWidget*>();
			m_view = widget->findChild<QAbstractItemView*>();
		}
	}
	if (!m_view)
		return false;
	m_model = new FloatiesItemModel(this);
	
	Config cfg;
	cfg.beginGroup("floaties");
	int size = cfg.beginArray("entities");
	for (int i = 0; i < size; i++) {
		cfg.setArrayIndex(i);
		Protocol *proto = allProtocols().value(cfg.value("protocol", QString()));
		if (!proto)
			continue;
		Account *account = proto->account(cfg.value("account", QString()));
		if (!account)
			continue;
		ChatUnit *unit = account->unit(cfg.value("id", QString()), true);
		Contact *contact = qobject_cast<Contact*>(unit);
		if (!contact || m_contacts.contains(contact))
			continue;
		ContactWidget *widget = createWidget(contact);
		widget->restoreGeometry(cfg.value("geometry", QByteArray()));
		widget->show();
	}
	
	qApp->installEventFilter(this);
	return true;
}

bool FloatiesPlugin::unload()
{
	qApp->removeEventFilter(this);
	Config cfg;
	cfg.beginGroup("floaties");
	cfg.remove("entities");
	cfg.beginArray("entities");
	QMap<Contact*, ContactWidget*>::iterator it = m_contacts.begin();
	for (int i = 0; it != m_contacts.end(); it++, i++) {
		cfg.setArrayIndex(i);
		Contact *contact = it.key();
		cfg.setValue("protocol", contact->account()->protocol()->id());
		cfg.setValue("account", contact->account()->id());
		cfg.setValue("id", contact->id());
		cfg.setValue("geometry", it.value()->saveGeometry());
	}
	qDeleteAll(m_contacts);
	m_contacts.clear();
	delete m_model;
	m_model = 0;
	return false;
}

bool FloatiesPlugin::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == qApp && event->type() == Event::eventType()) {
		Event *ev = static_cast<Event*>(event);
		if (ev->id == m_eventId) {
			QPoint pos = ev->at<QPoint>(0);
			Contact *contact = ev->at<Contact*>(1);
			if (contact && !m_contacts.contains(contact)) {
				ContactWidget *widget = createWidget(contact);
				widget->move(pos);
				widget->show();
			}
		}
	}
	return Plugin::eventFilter(obj, event);
}

void FloatiesPlugin::onRemoveContact(QObject *obj)
{
	bool deleted = !qobject_cast<Contact*>(obj);
	Contact *contact = static_cast<Contact*>(obj);
	ContactWidget *widget = m_contacts.take(contact);
	m_model->removeContact(contact);
	if (!deleted && widget) {
		widget->deleteLater();
//		contact->removeAction(m_action);
	}
}

ContactWidget *FloatiesPlugin::createWidget(qutim_sdk_0_3::Contact *contact)
{
	QPersistentModelIndex index = m_model->addContact(contact);
	ContactWidget *widget = new ContactWidget(index, m_view, contact);
	connect(widget, SIGNAL(wantDie(QObject*)), this, SLOT(onRemoveContact(QObject*)));
	m_contacts.insert(contact, widget);
//	contact->addAction(m_action);
	return widget;
}

QUTIM_EXPORT_PLUGIN(FloatiesPlugin)
