// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "search_impl.h"
#include <QSet>
#include <QMap>

template<class T>
class PrefixSearch : public SearchImpl<T>
{
public:
    /** ***********************************************************************/
    PrefixSearch(){
    }



    /** ***********************************************************************/
    PrefixSearch(const PrefixSearch &rhs){
        _invertedIndex = rhs._invertedIndex;
    }



    /** ***********************************************************************/
    virtual ~PrefixSearch(){
    }



    /** ***********************************************************************/
    void add(const T &t, const QStringList &aliases) override {
        for (const QString & str : aliases) {
            // Build an inverted index
            QStringList words = str.split(SEPARATOR_REGEX, QString::SkipEmptyParts);
            for (QString &w : words){
                w.toLower();
                _invertedIndex[w].insert(t);
            }
        }
    }



    /** ***********************************************************************/
    void remove(const T &t) override {
        // Remove all mapping from the inverted index which map on t. If the mapped
        // set is emtpy remove the entire mapping.
        typename InvertedIndex::iterator it = _invertedIndex.begin();
        while (it != _invertedIndex.end()) {
            it->remove(t);
            if (it->empty())
                it =_invertedIndex.erase(it);
            else ++it;
        }
    }



    /** ***********************************************************************/
    void reset() override {
        _invertedIndex.clear();
    }



    /** ***********************************************************************/
    QList<T> search(const QString &req) const override {
        QSet<T>* resSet = nullptr; // Constraint (1): resSet == nullptr
        QStringList words = req.split(SEPARATOR_REGEX, QString::SkipEmptyParts);
        if (words.empty()) return QList<T>(); // Constraint (2): words is not empty
        for (QString &w : words) {
            typename InvertedIndex::const_iterator lb;
            QSet<T> tmpSet;
            lb = _invertedIndex.lowerBound(w);
            while (lb != _invertedIndex.cend() && lb.key().startsWith(w))
                tmpSet.unite(lb++.value());
            if (resSet == nullptr)		// (1)&&(2)  |-  Constraint resSet != nullptr (3)
                resSet = new QSet<T>(tmpSet);
            else
                resSet->intersect(tmpSet);
        }
        // Safe to not check resSet != nullptr since (3) holds
        QList<T> res = resSet->toList();
        delete resSet;
        return res;
    }

protected:
    typedef QMap<QString, QSet<T>> InvertedIndex;
    InvertedIndex _invertedIndex;
};
























//template<typename T>
//uint qHash(const std::shared_ptr<T> &p)
//{
//    return qHash(p.get());
//}

//    class CaseInsensitiveCompare;
//    class CaseInsensitiveComparePrefix;

//template<class C>
//struct PrefixSearch<C>::CaseInsensitiveCompare
//{
//    inline bool operator()( Posting const &lhs, Posting const &rhs ) const {
//		return (*this)(lhs.first, rhs.first);
//	}

//    inline bool operator()( QString const &lhs, Posting const &rhs ) const {
//		return (*this)(lhs, rhs.first);
//	}

//    inline bool operator()( Posting const &lhs, QString const &rhs ) const {
//		return (*this)(lhs.first, rhs);
//	}

//	inline bool operator()( QString const &lhs, QString const &rhs ) const {
//		return lhs.compare(rhs, Qt::CaseInsensitive)<0;
//	}
//};

///** ***************************************************************************/
//template<class C>
//struct PrefixSearch<C>::CaseInsensitiveComparePrefix
//{
//	inline bool operator()( Posting const &pre, Posting const &rhs ) const {
//		return (*this)(pre.first, rhs.first);
//	}

//	inline bool operator()( QString const &pre, Posting const &rhs ) const {
//		return (*this)(pre, rhs.first);
//	}

//	inline bool operator()( Posting const &pre, QString const &rhs ) const {
//		return (*this)(pre.first, rhs);
//	}

//	inline bool operator()( QString const& pre, QString const& rhs ) const	{
//		QString::const_iterator a,b;
//		a = pre.cbegin();
//		b = rhs.cbegin();
//		QChar ca,cb;
//		while (a != pre.cend() && b != rhs.cend()){
//			ca = a++->toLower();
//			cb = b++->toLower();
//			if (ca < cb)
//				return true;
//			if (ca > cb)
//				return false;
//		}
//		return false;
//	}
//};
