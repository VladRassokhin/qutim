/****************************************************************************
 *  networkproxy.cpp
 *
 *  Copyright (c) 2010 by Prokhin Alexey <alexey.prokhin@yandex.ru>
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

#include "networkproxy.h"
#include "objectgenerator.h"
#include "dataforms.h"
#include "account.h"
#include <QHash>
#include <QIcon>

namespace qutim_sdk_0_3
{

class NetworkProxyManagerPrivate
{
public:
	Protocol *protocol;
};

static QHash<Protocol*, NetworkProxyManager*> managers;
static void ensureManagers()
{
	static bool isInited = false;
	if(!isInited && ObjectGenerator::isInited()) {
		foreach(const ObjectGenerator *gen, ObjectGenerator::module<NetworkProxyManager>()) {
			NetworkProxyManager *manager = gen->generate<NetworkProxyManager>();
			managers.insert(manager->protocol(), manager);
		}
		isInited = true;
	}
}

static QHash<QString, NetworkProxyInfo*> proxies;
static void ensureInfos()
{
	static bool isInited = false;
	if(!isInited && ObjectGenerator::isInited()) {
		ensureManagers();
		proxies.insert("socks5", Socks5ProxyInfo::instance());
		proxies.insert("http", HttpProxyInfo::instance());
		foreach (NetworkProxyManager *manager, managers) {
			foreach (NetworkProxyInfo *info, manager->proxies()) {
				// ensure that we don't have another info with the same name
				Q_ASSERT(!proxies.contains(info->name()) || proxies.value(info->name()) == info);
				proxies.insert(info->name(), info);
			}
		}
		isInited = true;
	}
}

static DataItem getSettingsImpl(const Config &cfg)
{
	DataItem item;
	item.addSubitem(StringDataItem("host", QT_TRANSLATE_NOOP("Proxy", "Host"), cfg.value("host", QString())));
	item.addSubitem(IntDataItem("port", QT_TRANSLATE_NOOP("Proxy", "Port"), cfg.value("port", 0), 0, 65535));
	item.addSubitem(StringDataItem("user", QT_TRANSLATE_NOOP("Proxy", "User name"), cfg.value("user", QString())));
	item.addSubitem(StringDataItem("password", QT_TRANSLATE_NOOP("Proxy", "Password"), cfg.value("password", QString(), Config::Crypted), 0, true));
	return item;
}

static void saveSettingsImpl(const DataItem &settings, Config &cfg, const QString &name)
{
	cfg.setValue("type", name);
	cfg.setValue("host", settings.subitem("host").data());
	cfg.setValue("port", settings.subitem("port").data());
	cfg.setValue("user", settings.subitem("user").data());
	cfg.setValue("password", settings.subitem("password").data(), Config::Crypted);
}

NetworkProxyInfo::NetworkProxyInfo()
{
}

NetworkProxyInfo::~NetworkProxyInfo()
{
}

QIcon NetworkProxyInfo::icon()
{
	return QIcon();
}

NetworkProxyInfo *NetworkProxyInfo::proxy(const QString &name)
{
	ensureInfos();
	// FIXME: the method could return already destroyed NetworkProxyInfo.
	return proxies.value(name);
}

QList<NetworkProxyInfo*> NetworkProxyInfo::allProxies()
{
	ensureInfos();
	return proxies.values();
}

void NetworkProxyInfo::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

NetworkProxyManager::NetworkProxyManager(Protocol *protocol) :
	p(new NetworkProxyManagerPrivate)
{
	p->protocol = protocol;
}

NetworkProxyManager::~NetworkProxyManager()
{
}

Protocol *NetworkProxyManager::protocol()
{
	return p->protocol;
}

NetworkProxyManager *NetworkProxyManager::get(Protocol *protocol)
{
	ensureManagers();
	return managers.value(protocol);
}

QList<NetworkProxyManager*> NetworkProxyManager::allManagers()
{
	ensureManagers();
	return managers.values();
}

static DataItem getCurrentSettingsImpl(const Config &cfg)
{
	QString type = cfg.value("type", QString());
	NetworkProxyInfo *proxy = NetworkProxyInfo::proxy(type);
	DataItem item = proxy ? proxy->settings(cfg) : DataItem();
	if (item.subitem("type").isNull())
		item << DataItem("type", QT_TR_NOOP("Type"), type);
	return item;
}

DataItem NetworkProxyManager::settings()
{
	Config cfg = Config().group("proxy");
	if (cfg.value("disabled", true))
		return DataItem();
	return getCurrentSettingsImpl(cfg);
}

DataItem NetworkProxyManager::settings(Account *account)
{
	Config cfg = account->config("proxy");
	if (cfg.value("useGlobalProxy", true))
		cfg = Config().group("proxy");
	if (cfg.value("disabled", false))
		return DataItem();
	return getCurrentSettingsImpl(cfg);
}

QNetworkProxy NetworkProxyManager::toNetworkProxy(const DataItem &settings)
{
	QNetworkProxy proxy(QNetworkProxy::NoProxy);
	if (settings.isNull() || !settings.hasSubitems())
		return proxy;
	QNetworkProxy::ProxyType type;
	QString typeStr = settings.subitem("type").data().toString();
	if (typeStr == "socks5")
		type = QNetworkProxy::Socks5Proxy;
	else if (typeStr == "http")
		type = QNetworkProxy::HttpProxy;
	else
		return proxy;
	proxy.setHostName(settings.subitem("host").data().toString());
	proxy.setPort(settings.subitem("port").data(1));
	proxy.setType(type);
	QString user = settings.subitem("user").data().toString();
	if (!user.isEmpty()) {
		proxy.setUser(user);
		proxy.setPassword(settings.subitem("password").data().toString());
	}
	return proxy;
}

void NetworkProxyManager::virtual_hook(int id, void *data)
{
	Q_UNUSED(id);
	Q_UNUSED(data);
}

QString Socks5ProxyInfo::name()
{
	return "socks5";
}

LocalizedString Socks5ProxyInfo::description()
{
	return QT_TRANSLATE_NOOP("Proxy", "Socks 5");
}

DataItem Socks5ProxyInfo::settings(const Config &config)
{
	return getSettingsImpl(config);
}

void Socks5ProxyInfo::saveSettings(Config config, const DataItem &settings)
{
	saveSettingsImpl(settings, config, "socks5");
}

Socks5ProxyInfo *Socks5ProxyInfo::instance()
{
	static Socks5ProxyInfo info;
	return &info;
}

Socks5ProxyInfo::Socks5ProxyInfo()
{
}

QString HttpProxyInfo::name()
{
	return "http";
}

LocalizedString HttpProxyInfo::description()
{
	return QT_TRANSLATE_NOOP("Proxy", "HTTP");
}


DataItem HttpProxyInfo::settings(const Config &config)
{
	return getSettingsImpl(config);
}

void HttpProxyInfo::saveSettings(Config config, const DataItem &settings)
{
	saveSettingsImpl(settings, config, "http");
}

HttpProxyInfo *HttpProxyInfo::instance()
{
	static HttpProxyInfo info;
	return &info;
}

HttpProxyInfo::HttpProxyInfo()
{
}

} // namespace qutim_sdk_0_3
