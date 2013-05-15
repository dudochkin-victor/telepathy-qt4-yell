/*
 * This file is part of TelepathyQt4
 *
 * Copyright (C) 2010 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QApplication>

#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/AccountFactory>
#include <TelepathyQt4/ConnectionFactory>

#include "chatwindow.h"

using namespace Tp;

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    Tp::registerTypes();

    AccountFactoryPtr  accountFactory = AccountFactory::create(QDBusConnection::sessionBus(),
                                                               Features() << Account::FeatureCore);

    ConnectionFactoryPtr  connectionFactory = ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                               Features() <<  Connection::FeatureSelfContact
                                                               << Connection::FeatureCore);

    ClientRegistrarPtr registrar = ClientRegistrar::create(accountFactory, connectionFactory);
    ChatWindow* mainWindow = new ChatWindow();

    AbstractClientPtr handler(mainWindow);
    registrar->registerClient(handler, QString::fromLatin1("ExampleChatHandler"));

    return app.exec();
    delete mainWindow;
}
