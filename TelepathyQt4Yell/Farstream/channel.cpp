/*
 * This file is part of TelepathyQt4Yell
 *
 * Copyright © 2009-2011 Collabora Ltd. <http://www.collabora.co.uk/>
 * Copyright © 2009 Nokia Corporation
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

#include <TelepathyQt4Yell/Farstream/Channel>

#include "TelepathyQt4Yell/Farstream/_gen/channel.moc.hpp"

#include <TelepathyQt4/Channel>
#include <TelepathyQt4/Connection>

#include <TelepathyQt4Yell/CallChannel>
#include <TelepathyQt4Yell/Farstream/global.h>

#include <telepathy-glib/channel.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/dbus.h>

#include <gst/gst.h>

namespace Tpy
{

struct TELEPATHY_QT4_YELL_FS_NO_EXPORT PendingTfChannel::Private
{
    Private(const CallChannelPtr &channel)
        : mChannel(channel),
          mTfChannel(0)
    {
    }

    static void onTfChannelNewFinish(GObject *sourceObject, GAsyncResult *res, gpointer userData);

    CallChannelPtr mChannel;
    TfChannel *mTfChannel;
};

PendingTfChannel::PendingTfChannel(const FarstreamChannelFactoryPtr &fcf,
        const CallChannelPtr &channel)
    : Tp::PendingOperation(fcf),
      mPriv(new PendingTfChannel::Private(channel))
{
    if (!mPriv->mChannel->handlerStreamingRequired()) {
        setFinishedWithError(TP_QT4_ERROR_NOT_AVAILABLE,
                QLatin1String("Handler streaming not required"));
        return;
    }

    TpDBusDaemon *dbus = tp_dbus_daemon_dup(0);
    if (!dbus) {
        setFinishedWithError(TP_QT4_ERROR_NOT_AVAILABLE,
                QLatin1String("Unable to connect to D-Bus"));
        return;
    }

    Tp::ConnectionPtr connection = mPriv->mChannel->connection();
    if (connection.isNull()) {
        setFinishedWithError(TP_QT4_ERROR_NOT_AVAILABLE,
                QLatin1String("Connection not available"));
        g_object_unref(dbus);
        return;
    }

    TpConnection *gconnection = tp_connection_new(dbus,
            connection->busName().toAscii(),
            connection->objectPath().toAscii(),
            0);
    g_object_unref(dbus);
    dbus = 0;
    if (!gconnection) {
        setFinishedWithError(TP_QT4_ERROR_NOT_AVAILABLE,
                QLatin1String("Unable to construct TpConnection"));
        return;
    }

    TpChannel *gchannel = tp_channel_new(gconnection,
            channel->objectPath().toAscii(),
            TP_QT4_YELL_IFACE_CHANNEL_TYPE_CALL.latin1(),
            (TpHandleType) channel->targetHandleType(),
            channel->targetHandle(),
            0);
    g_object_unref(gconnection);
    gconnection = 0;
    if (!gchannel) {
        setFinishedWithError(TP_QT4_ERROR_NOT_AVAILABLE,
                QLatin1String("Unable to construct TpChannel"));
        return;
    }

    tf_channel_new_async(gchannel, PendingTfChannel::Private::onTfChannelNewFinish, this);
    g_object_unref(gchannel);
}

PendingTfChannel::~PendingTfChannel()
{
    delete mPriv;
}

void PendingTfChannel::Private::onTfChannelNewFinish(GObject *sourceObject,
        GAsyncResult *res, gpointer userData)
{
    qDebug() << "PendingTfChannel::Private::onTfChannelNewFinish: ";
    qDebug() << "    sourceObject=" << sourceObject;
    qDebug() << "    result=" << res;

    PendingTfChannel *self = reinterpret_cast<PendingTfChannel *>(userData);

    GError *error = NULL;
    TfChannel *ret = TF_CHANNEL(g_async_initable_new_finish(
                G_ASYNC_INITABLE(sourceObject), res, &error));
    if (error) {
        qDebug() << "PendingTfChannel::Private::onTfChannelNewFinish: error " << error->message;
        g_clear_error(&error);
        self->setFinishedWithError(TP_QT4_ERROR_NOT_AVAILABLE, QLatin1String(error->message));
        return;
    }

    self->mPriv->mTfChannel = ret;
    self->setFinished();
}

TfChannel *PendingTfChannel::tfChannel() const
{
    return mPriv->mTfChannel;
}

FarstreamChannelFactoryPtr FarstreamChannelFactory::create()
{
    return FarstreamChannelFactoryPtr(new FarstreamChannelFactory());
}

FarstreamChannelFactory::FarstreamChannelFactory()
    : mPriv(0)
{
}

FarstreamChannelFactory::~FarstreamChannelFactory()
{
}

PendingTfChannel *FarstreamChannelFactory::createTfChannel(const CallChannelPtr &channel)
{
    PendingTfChannel *ptf = new PendingTfChannel(FarstreamChannelFactoryPtr(this), channel);
    return ptf;
}

} // Tpy
