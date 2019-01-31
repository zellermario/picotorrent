#pragma once

#include <cstdint>
#include <memory>

#include <QDateTime>
#include <QString>

#include <libtorrent/bitfield.hpp>
#include <libtorrent/torrent_info.hpp>

namespace pt
{
    struct TorrentStatus
    {
        QDateTime                                     addedOn;
        float                                         availability;
        QDateTime                                     completedOn;
        int                                           downloadPayloadRate;
        std::chrono::seconds                          eta;
        QString                                       infoHash;
        QString                                       name;
        bool                                          paused;
        int                                           peersCurrent;
        int                                           peersTotal;
        libtorrent::bitfield                          pieces;
        float                                         progress;
        int                                           queuePosition;
        float                                         ratio;
        QString                                       savePath;
        int                                           seedsCurrent;
        int                                           seedsTotal;
        std::weak_ptr<const libtorrent::torrent_info> torrentFile;
        std::int64_t                                  totalWanted;
        int                                           uploadPayloadRate;
    };
}
