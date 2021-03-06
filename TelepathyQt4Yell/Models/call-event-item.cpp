/*
 * This file is part of TelepathyQt4Yell Models
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

#include <TelepathyQt4Yell/Models/CallEventItem>

#include "TelepathyQt4Yell/Models/_gen/call-event-item.moc.hpp"

#include <TelepathyQt/Contact>

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT CallEventItem::Private
{
    Private(QTime duration, const Tp::ContactPtr &endActor,
            Tpy::CallStateChangeReason endReason, const QString &detailedEndReason)
        : mDuration(duration),
          mEndActor(endActor),
          mEndReason(endReason),
          mDetailedEndReason(detailedEndReason)
    {
    }

    QTime mDuration;
    Tp::ContactPtr mEndActor;
    Tpy::CallStateChangeReason mEndReason;
    QString mDetailedEndReason;
};

CallEventItem::CallEventItem(const Tp::ContactPtr &sender,
    const Tp::ContactPtr &receiver, const QDateTime &time,
    const QTime &duration, Tp::ContactPtr &endActor,
    Tpy::CallStateChangeReason &endReason, const QString &detailedEndReason,
    QObject *parent)
    : EventItem(sender, receiver, time, parent),
      mPriv(new Private(duration, endActor, endReason, detailedEndReason))
{
}

CallEventItem::~CallEventItem()
{
    delete mPriv;
}

QTime CallEventItem::duration() const
{
    return mPriv->mDuration;
}

Tp::ContactPtr CallEventItem::endActor() const
{
    return mPriv->mEndActor;
}

Tpy::CallStateChangeReason CallEventItem::endReason() const
{
    return mPriv->mEndReason;
}

QString CallEventItem::detailedEndReason() const
{
    return mPriv->mDetailedEndReason;
}

bool CallEventItem::missedCall() const
{
    return (endActor().isNull() &&
            endReason() == Tpy::CallStateChangeReasonNoAnswer &&
            duration().elapsed() <= 0);
}

bool CallEventItem::rejectedCall() const
{
    return (!endActor().isNull() &&
            endReason() == Tpy::CallStateChangeReasonUserRequested &&
            duration().elapsed() <= 0 &&
            detailedEndReason() == TP_QT_ERROR_REJECTED);
}

}
