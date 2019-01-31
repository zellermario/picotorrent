#include "torrenthandle.hpp"

#include <libtorrent/announce_entry.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "session.hpp"
#include "torrentstatus.hpp"

namespace lt = libtorrent;
using pt::TorrentHandle;

TorrentHandle::TorrentHandle(pt::Session* session, lt::torrent_handle const& th)
    : m_session(session)
{
    m_th = std::make_unique<lt::torrent_handle>(th);
    m_ts = std::make_unique<lt::torrent_status>(th.status());
}

TorrentHandle::~TorrentHandle()
{
}

void TorrentHandle::fileProgress(std::vector<std::int64_t>& progress, int flags) const
{
    m_th->file_progress(progress, flags);
}

void TorrentHandle::forceReannounce()
{
    m_th->force_reannounce();
}

void TorrentHandle::forceRecheck()
{
    /*th.force_recheck();

    lt::torrent_status ts = th.status();

    bool paused = ((ts.flags & lt::torrent_flags::paused)
        && !(ts.flags & lt::torrent_flags::auto_managed));

    if (paused)
    {
        th.resume();
        m_state->pauseAfterChecking.insert(hash);
    }*/

    m_th->force_recheck();
}

std::vector<lt::download_priority_t> TorrentHandle::getFilePriorities() const
{
    return m_th->get_file_priorities();
}

void TorrentHandle::getPeerInfo(std::vector<lt::peer_info>& peers) const
{
    m_th->get_peer_info(peers);
}

lt::sha1_hash TorrentHandle::infoHash()
{
    return m_th->info_hash();
}

void TorrentHandle::moveStorage(QString const& newPath)
{
    m_th->move_storage(newPath.toStdString());
}

void TorrentHandle::pause()
{
    m_th->unset_flags(lt::torrent_flags::auto_managed);
    m_th->pause(lt::torrent_handle::graceful_pause);
}

void TorrentHandle::queueUp()
{
    m_th->queue_position_up();
}

void TorrentHandle::queueDown()
{
    m_th->queue_position_down();
}

void TorrentHandle::queueTop()
{
    m_th->queue_position_top();
}

void TorrentHandle::queueBottom()
{
    m_th->queue_position_bottom();
}

void TorrentHandle::remove()
{
    m_session->removeTorrent(this);
}

void TorrentHandle::removeFiles()
{
    m_session->removeTorrent(this, lt::session_handle::delete_files);
}

void TorrentHandle::resume()
{
    m_th->set_flags(lt::torrent_flags::auto_managed);
    m_th->clear_error();
    m_th->resume();
}

void TorrentHandle::resumeForce()
{
    m_th->unset_flags(lt::torrent_flags::auto_managed);
    m_th->clear_error();
    m_th->resume();
}

pt::TorrentStatus TorrentHandle::status()
{
    std::stringstream hash;
    hash << m_ts->info_hash;

    auto eta = std::chrono::seconds(0);

    if ((m_ts->total_wanted - m_ts->total_wanted_done > 0) && m_ts->download_payload_rate > 0)
    {
        eta = std::chrono::seconds((m_ts->total_wanted - m_ts->total_wanted_done) / m_ts->download_payload_rate);
    }

    float ratio = 0;

    if (m_ts->all_time_download > 0)
    {
        ratio = static_cast<float>(m_ts->all_time_upload) / static_cast<float>(m_ts->all_time_download);
    }

    TorrentStatus ts;
    ts.addedOn             = QDateTime::fromSecsSinceEpoch(m_ts->added_time);
    ts.availability        = m_ts->distributed_copies;
    ts.completedOn         = m_ts->completed_time > 0 ? QDateTime::fromSecsSinceEpoch(m_ts->completed_time) : QDateTime();
    ts.downloadPayloadRate = m_ts->download_payload_rate;
    ts.eta                 = eta;
    ts.infoHash            = QString::fromStdString(hash.str());
    ts.name                = QString::fromStdString(m_ts->name);
    ts.paused              = (m_th->flags() & lt::torrent_flags::paused) == lt::torrent_flags::paused;
    ts.peersCurrent        = m_ts->num_peers - m_ts->num_seeds;
    ts.peersTotal          = m_ts->list_peers - m_ts->list_seeds;
    ts.pieces              = m_ts->pieces;
    ts.progress            = m_ts->progress;
    ts.queuePosition       = static_cast<int>(m_ts->queue_position);
    ts.ratio               = ratio;
    ts.savePath            = QString::fromStdString(m_ts->save_path);
    ts.seedsCurrent        = m_ts->num_seeds;
    ts.seedsTotal          = m_ts->list_seeds;
    ts.torrentFile         = m_ts->torrent_file;
    ts.totalWanted         = m_ts->total_wanted;
    ts.uploadPayloadRate   = m_ts->upload_payload_rate;
    return ts;
}

std::vector<lt::announce_entry> TorrentHandle::trackers() const
{
    return m_th->trackers();
}

void TorrentHandle::updateStatus(lt::torrent_status const& ts)
{
    m_ts = std::make_unique<lt::torrent_status>(ts);
}

lt::torrent_handle& TorrentHandle::wrappedHandle()
{
    return *m_th.get();
}
