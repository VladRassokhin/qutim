/****************************************************************************
 *  modulemanager.h
 *
 *  Copyright (c) 2009 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*****************************************************************************/

#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include "libqutim_global.h"
#include "plugin.h"
#include <QStringList>
#include <QMultiMap>
#include "event.h"

namespace qutim_sdk_0_3
{
	class ModuleManagerPrivate;
	typedef QList<ExtensionInfo> ExtensionList;

	class LIBQUTIM_EXPORT ModuleManager : public QObject
	{
		Q_OBJECT
	protected:
		// Constructor
		ModuleManager(QObject *parent = 0);
		// Destructor
		virtual ~ModuleManager();

		// Fields
		QMultiMap<Plugin *, ExtensionInfo> getExtensions(const QMetaObject *service_meta) const;
		QMultiMap<Plugin *, ExtensionInfo> getExtensions(const char *interface_id) const;

		// Methods
		void loadPlugins(const QStringList &additional_paths = QStringList());
		QObject *initExtension(const QMetaObject *service_meta);

		// Virtual Methods
		virtual QList<ExtensionInfo> coreExtensions() const = 0;
		virtual void initExtensions();

		// Inline Methods
		template<typename T>
		inline QMultiMap<Plugin *, ExtensionInfo> getExtensions()
		{
			return getExtensions(&T::staticMetaObject);
		}

		template<typename T>
		inline T *initExtension()
		{
			return static_cast<T *>(initExtension(&T::staticMetaObject));
		}
	private:
		// Friend functions
		friend bool isCoreInited();
		friend GeneratorList moduleGenerators(const QMetaObject *);
		friend GeneratorList moduleGenerators(const char *);
		friend ProtocolMap allProtocols();

		// Static Fields
		static ModuleManager *self;
		ModuleManagerPrivate *p;
	};

//	LIBQUTIM_EXPORT void registerModule(const char *name, const char *description, const char *face, const QMetaObject *meta, int min = 0, int max = -1);
//	inline void registerModule(const char *name, const char *description, const char *face, int min = 0, int max = -1)
//	{ registerModule(name, description, face, NULL, min, max); }
//	inline void registerModule(const char *name, const char *description, const QMetaObject *meta, int min = 0, int max = -1)
//	{ registerModule(name, description, NULL, meta, min, max); }
//
//	template<typename T, int Min, int Max>
//	class ModuleHelper
//	{
//	public:
//		inline ModuleHelper(const char *name, const char *description)
//		{
//			registerModule(name, description, qobject_interface_iid<T *>(), meta_helper<T>(reinterpret_cast<T *>(0)), Min, Max);
//		}
//	private:
//		template <typename F>
//		inline const QMetaObject *meta_helper(const QObject *obj)
//		{ return &F::staticMetaObject; }
//		template <typename F>
//		inline const QMetaObject *meta_helper(const void *obj)
//		{ return NULL; }
//	};
//
//	template <typename T>
//	class SingleModuleHelper : public ModuleHelper<T, 1, 1>
//	{
//	public:
//		inline SingleModuleHelper(const char *name, const char *description) : ModuleHelper<T, 1, 1>(name, description) {}
//	};
//
//	template <typename T>
//	class MultiModuleHelper : public ModuleHelper<T, 0, -1>
//	{
//	public:
//		inline MultiModuleHelper(const char *name, const char *description) : ModuleHelper<T, 0, -1>(name, description) {}
//	};
}

#endif // MODULEMANAGER_H
