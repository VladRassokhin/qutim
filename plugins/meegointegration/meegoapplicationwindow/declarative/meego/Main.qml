/****************************************************************************
**
** qutIM instant messenger
**
** Copyright (C) 2011 Ruslan Nigmatullin <euroelessar@ya.ru>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

import QtQuick 1.0
import com.nokia.meego 1.0
import org.qutim 0.3

PageStackWindow {
	id: root
	ServiceManager {
		id: serviceManager
	}
	property variant contactList:  serviceManager.contactList
	property variant chat:  serviceManager.chatLayer
	property variant settings:  serviceManager.settingsLayer
	Connections {
		target: root.chat
		onShown: {
			if (root.chat.activeSession)
				tabGroup.currentTab = chatTab
		}
	}
	PasswordDialog{
		id:passwordDialog

	}

	SettingsDialog{
		id:settingsDialog

	}
	AuthDialog {
		id:authDialog
	}
	JoinGroupChatDialog
	{
		id:joinGroupChatDialog
	}

	AboutDialog {
		id:aboutDialog
	}
	AddContactDialog
	{
		id:addContactDialog
	}

	initialPage: Page {



		AnimatedTabGroup {
			id: tabGroup
			anchors.fill: parent

			currentTab: contactListTab
			
			ContactList {
				id: contactListTab
				contactList: root.contactList
				chat: root.chat

			}

			ChatChannelList {
				id: channelListTab
				chat: root.chat
			}
			Chat {
				id: chatTab
				chat: root.chat
			}
			Page {
				id: conferenceUsersTab
				Column {
					spacing: 10

					Text {
						text: "TODO: Implement user list"
					}
				}
			}
			SettingsPage {
				id: settingsTab
				model: settings.model
			}
		}
		tools: ToolBarLayout {
			ButtonRow {
				style: TabButtonStyle { }
				TabIcon {
					platformIconId: "toolbar-contact"
					tab: contactListTab
				}
				TabIcon {
					platformIconId: "toolbar-list"
					tab: channelListTab
				}
				TabIcon {
					platformIconId: "toolbar-new-chat"
					tab: chatTab
					enabled: chat.activeSession !== null
				}
				TabIcon {
					platformIconId: "toolbar-new-chat"
					tab: conferenceUsersTab
					enabled: chat.activeSession !== null && chat.activeSession.unit.conference
				}
				TabIcon {
					platformIconId: "toolbar-settings"
					tab: settingsTab
				}
			}

			ToolIcon {
				property variant menu: tabGroup.currentTab.menu
				//				visible: menu !== undefined
				//				Menu {
				//					id: menu
				//					visualParent: root.pageStack
				//				}
				platformIconId: "toolbar-view-menu"
				onClicked:mainMenu.open()
//				onClicked: (menu.status == DialogStatus.Closed)
//					   ? menu.open()
//					   : menu.close()
			}

		}
		Menu {
		    id: mainMenu

		    content: MenuLayout {
			MenuItem {
				text: "Show/hide offline contacts"
			    onClicked: contactListTab.showOffline=!contactListTab.showOffline;
			}
			MenuItem {
				text: "Join group chat"
			    onClicked: joinGroupChatDialog.open();
			}
			MenuItem {
				text: "About qutIM"
			    onClicked: aboutDialog.open();
			}
			MenuItem {
				text: "Add contact"
			    onClicked: addContactDialog.open();
			}
			MenuItem {
				text: "Settings"
			    onClicked: settingsDialog.open();
			}
		    }
		}
	}
}