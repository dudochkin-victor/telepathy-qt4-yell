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

#include "telepathy-initializer.h"

#include "examples/accounts-model/_gen/telepathy-initializer.moc.hpp"

#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingReady>

TelepathyInitializer::TelepathyInitializer(const Tp::AccountManagerPtr &am)
    : QObject(0),
      mAM(am),
      mAccountsModel(0),
      numConnections(0)
{
}

void TelepathyInitializer::run()
{
    Tp::registerTypes();
    connect(mAM->becomeReady(),
            SIGNAL(finished(Tp::PendingOperation *)),
            SLOT(onAMReady(Tp::PendingOperation *)));
}

Tp::AccountsModel *TelepathyInitializer::accountModel() const
{
    return mAccountsModel;
}

void TelepathyInitializer::onAMReady(Tp::PendingOperation *)
{
    Tp::Features features;
    features << Tp::Account::FeatureCore
             << Tp::Account::FeatureAvatar
             << Tp::Account::FeatureProtocolInfo
             << Tp::Account::FeatureCapabilities
             << Tp::Account::FeatureProfile;
    numConnections = mAM->allAccounts().size();
    foreach (Tp::AccountPtr account, mAM->allAccounts()) {
        connect(account->becomeReady(features),
                SIGNAL(finished(Tp::PendingOperation *)),
                SLOT(onAccountReady(Tp::PendingOperation *)));
    }
    checkFinished();
}

void TelepathyInitializer::onAccountReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
    Tp::AccountPtr account = Tp::AccountPtr::dynamicCast(pr->object());

    if (!account->connection().isNull()) {
        Tp::Features features;
        features << Tp::Connection::FeatureCore
                 << Tp::Connection::FeatureSelfContact
                 << Tp::Connection::FeatureSimplePresence
                 << Tp::Connection::FeatureRoster
                 << Tp::Connection::FeatureRosterGroups
                 << Tp::Connection::FeatureAccountBalance;
        connect(account->connection()->becomeReady(features),
                SIGNAL(finished(Tp::PendingOperation *)),
                SLOT(onConnectionReady(Tp::PendingOperation *)));
    }
    else {
        numConnections--;
        checkFinished();
    }
}

void TelepathyInitializer::onConnectionReady(Tp::PendingOperation *op)
{
    Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
    Tp::ConnectionPtr connection = Tp::ConnectionPtr::dynamicCast(pr->object());

    Tp::ContactManagerPtr manager = connection->contactManager();
    QList<Tp::ContactPtr> contacts = manager->allKnownContacts().toList();

    Tp::Features features;
    features << Tp::Contact::FeatureAlias
             << Tp::Contact::FeatureAvatarToken
             << Tp::Contact::FeatureSimplePresence
             << Tp::Contact::FeatureCapabilities
             << Tp::Contact::FeatureLocation
             << Tp::Contact::FeatureInfo
             << Tp::Contact::FeatureAvatarData;
    connect(manager->upgradeContacts(contacts, features),
            SIGNAL(finished(Tp::PendingOperation *)),
            SLOT(onContactsUpgraded(Tp::PendingOperation *)));
}

void TelepathyInitializer::onContactsUpgraded(Tp::PendingOperation *op)
{
    numConnections--;
    checkFinished();
}

void TelepathyInitializer::checkFinished()
{
    if (numConnections <= 0) {
        mAccountsModel = new Tp::AccountsModel(mAM);
        emit finished(this);
        deleteLater();
    }
}

