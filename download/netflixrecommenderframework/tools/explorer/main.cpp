/**
 * Copyright (C) 2006-2007 Benjamin C. Meyer (ben at meyerhome dot net)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "database.h"
#include "movie.h"
#include "user.h"

#include "ui_mainwindow.h"

#include <QtGui/QtGui>

class UserModel : public QAbstractTableModel
{

    Q_OBJECT

public:
    UserModel(DataBase *db, QObject * parent = 0) : QAbstractTableModel(parent), user(db)
    {}
    int rowCount(const QModelIndex &parent) const
    {
        return (parent.row() > 0) ? 0 : user.votes();
    }
    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        if (section == 0) return "Movie";
        if (section == 1) return "Score";
        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole: {
                switch (index.column()) {
                case 0: return user.movie(index.row());
                case 1: return user.score(index.row());
                }
                return index.row();
                break;
            }
        case Qt::TextAlignmentRole:
            return Qt::AlignRight;
        }
        return QVariant();
    }
    void setId(int id)
    {
        user.setId(id);
        reset();
    }

private:
    User user;
};


class DBModel : public QAbstractTableModel
{

    Q_OBJECT

public:
    DBModel(QObject * parent = 0) : QAbstractTableModel(parent)
    {
        db = new DataBase();
        db->load();
    }
    ~DBModel()
    {
        delete db;
    }

    int rowCount(const QModelIndex &parent) const
    {
        return (parent.row() > 0) ? 0 : db->totalVotes();
    }
    int columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return 3;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
            return QAbstractTableModel::headerData(section, orientation, role);
        }
        if (section == 0) return "Movie";
        if (section == 1) return "User";
        if (section == 2) return "Score";
        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole: {
                Movie m = Movie::getMovie(db, index.row());
                int v = index.row() - m.dataBaseOffset();
                switch (index.column()) {
                case 0: return m.id();
                case 1: return m.user(v);
                case 2: return m.score(v);
                }
                return index.row();
                break;
            }
        case Qt::TextAlignmentRole:
            return Qt::AlignRight;
        }
        return QVariant();
    }

    DataBase *db;
};

class Window : public QWidget
{

    Q_OBJECT

public:
    Window(QWidget *parent = 0) : QWidget(parent)
    {
        ui.setupUi(this);

        // Qt versions less then 4.2.2 will not handle the default 30px size very well
        // sense that makes a view longer (height wise) then int.
        ui.votes->verticalHeader()->setDefaultSectionSize(20);
        ui.user->verticalHeader()->setDefaultSectionSize(20);
        ui.votes->setAlternatingRowColors(true);
        ui.votes->setModel(&db);
        QHeaderView *header = ui.votes->horizontalHeader();
        QFontMetrics fm(ui.votes->horizontalHeader()->font());
        int movieSize = qMax(header->sectionSizeHint(0), qMax(fm.width("17770"), fm.width("Movie")));
        int scoreSize = qMax(header->sectionSizeHint(2), fm.width("Score"));
        header->resizeSection(0, movieSize);
        header->resizeSection(2, scoreSize);
        ui.user->horizontalHeader()->resizeSection(0, movieSize);
        ui.user->horizontalHeader()->resizeSection(1, scoreSize);

        userModel = new UserModel(db.db);
        ui.user->setModel(userModel);

        ui.movieId->setRange(1, db.db->totalMovies());
        ui.userId->setRange(1, db.db->totalVotes());
        connect(ui.movieId, SIGNAL(valueChanged(int)), this, SLOT(movieIdChanged(int)));
        connect(ui.userId, SIGNAL(valueChanged(int)), this, SLOT(userIdChanged(int)));
        connect(ui.votes, SIGNAL(pressed(const QModelIndex &)), this, SLOT(changeUser(const QModelIndex &)));
        connect(ui.user, SIGNAL(pressed(const QModelIndex &)), this, SLOT(changeMovie(const QModelIndex &)));
    }
    ~Window()
    {
        delete userModel;
    }

private slots:
    void movieIdChanged(int newId)
    {
        Movie movie(db.db, newId);
        int row = movie.dataBaseOffset();
        QModelIndex idx = db.index(row, 0);
        ui.votes->scrollTo(idx, QAbstractItemView::PositionAtTop);
        ui.votes->setCurrentIndex(idx);
    }
    void userIdChanged(int newId)
    {
        userModel->setId(newId);
    }
    void changeMovie(const QModelIndex &index)
    {
        ui.movieId->setValue(index.sibling(index.row(), 0).data().toInt());
        //movieIdChanged(index.sibling(0, 0).data().toInt());
    }
    void changeUser(const QModelIndex &index)
    {
        ui.userId->setValue(index.sibling(index.row(), 1).data().toInt());
        //userIdChanged(index.sibling(1, 0).data().toInt());
    }

private:
    Ui::Netflix ui;
    DBModel db;
    UserModel *userModel;
};

int main(int argc, char **argv)
{
    QApplication application( argc, argv );

    Window window;
    window.show();

    return application.exec();
}

#include "main.moc"
