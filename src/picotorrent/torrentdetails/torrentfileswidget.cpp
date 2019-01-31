#include "torrentfileswidget.hpp"

#include <QTreeView>
#include <QVBoxLayout>

#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../filestorageitemdelegate.hpp"
#include "../filestorageitemmodel.hpp"
#include "../torrenthandle.hpp"
#include "../torrentstatus.hpp"
#include "../translator.hpp"

class MinimumTreeView : public QTreeView
{
public:
    QSize sizeHint() const override
    {
        return QSize(-1, 120);
    }
};

namespace lt = libtorrent;
using pt::TorrentFilesWidget;

TorrentFilesWidget::TorrentFilesWidget()
{
    m_filesModel = new FileStorageItemModel();
    m_filesView  = new MinimumTreeView();

    m_filesView->setItemDelegate(new FileStorageItemDelegate());
    m_filesView->setModel(m_filesModel);
    m_filesView->setColumnWidth(FileStorageItemModel::Columns::Name, 200);
    m_filesView->setColumnWidth(FileStorageItemModel::Columns::Size, 60);
    m_filesView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_filesView->setSelectionMode(QTreeView::ExtendedSelection);
    m_filesView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    auto layout = new QVBoxLayout();
    layout->addWidget(m_filesView);
    layout->setContentsMargins(2, 2, 2, 2);

    this->clear();
    this->setLayout(layout);
}

void TorrentFilesWidget::clear()
{
    m_filesModel->clearTree();
}

void TorrentFilesWidget::refresh(QList<pt::TorrentHandle*> const& torrents)
{
    if (torrents.count() != 1)
    {
        return;
    }

    TorrentHandle* torrent = torrents.at(0);
    TorrentStatus  status  = torrent->status();
    auto ti                = status.torrentFile.lock();

    if (!ti)
    {
        return;
    }

    std::vector<int64_t> progress;
    torrent->fileProgress(progress, lt::torrent_handle::piece_granularity);

    // Only update progress if we have the same selection as previous
    if (m_currentSelection == torrent->infoHash())
    {
        m_filesModel->setProgress(progress);
    }
    else
    {
        m_filesModel->rebuildTree(ti);
        m_filesModel->setPriorities(torrent->getFilePriorities());
        m_filesModel->setProgress(progress);

        if (ti->num_files() > 1)
        {
            auto root = m_filesView->rootIndex();

            for (int i = 0; i < m_filesModel->rowCount(root); i++)
            {
                auto idx = m_filesModel->index(i, 0, root);
                m_filesView->expand(idx);
            }

            m_filesView->setRootIsDecorated(true);
        }
        else
        {
            // If the file name is the same as torrent name, this is a single file
            // torrent which downloads directly to the save path. In that case, hide
            // the root decorator.
            if (ti->files().file_path(lt::file_index_t{0}) == ti->name())
            {
                m_filesView->setRootIsDecorated(false);
            }
            else
            {
                m_filesView->expandAll();
                m_filesView->setRootIsDecorated(true);
            }
        }

        m_currentSelection = torrent->infoHash();
    }
}
