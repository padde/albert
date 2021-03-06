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

#include "proposallist.h"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDebug>

/******************************************************************************/
/************************  B E G I N   P R I V A T E  *************************/
/******************************************************************************/
/****************************************************************************///
class ProposalList::ItemDelegate final : public QStyledItemDelegate
{
public:
    Qt::KeyboardModifiers mods;
    bool showInfo;
    bool showAction;

	void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override
	{
		QStyleOptionViewItemV4 option = options;
		initStyleOption(&option, index);

		//	QStyledItemDelegate::paint(painter, option, index);
		painter->save();
		QStyle *style = option.widget->style();
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
//		QRect contentsRect = style->subElementRect(QStyle::SE_ItemViewItemText,
//												   &option,
//												   option.widget);

		/* Draw icon */
//		QRect iconRect(contentsRect.topLeft(), option.decorationSize);
//		iconRect.translate( (a-option.decorationSize.width())/2, (a-option.decorationSize.height())/2);
		QRect iconRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget);
        painter->drawPixmap(iconRect, index.data(Qt::DecorationRole + mods).value<QIcon>().pixmap(option.decorationSize));

		/* Drawing text differs dependent on the mode and selection */
        if (showInfo)
		{
			/*
			 * fm(x) := fontmetrics of x
			 * DR := DisplayRole
			 * UR := UserRole
			 *  +---------------------+----------------------------------------+
			 *  |                     |                                        |
			 *  |   +-------------+   |                                        |
			 *  |   |             |   |                                        |
			 *  |   |             |   |a*fm(DR)/(fm(DR)+fm(UR))    DisplayRole |
			 * a|   |     icon    |   |                                        |
			 *  |   |             |   |                                        |
			 *  |   |             |   +----------------------------------------+
			 *  |   |             |   |                                        |
			 *  |   +-------------+   |a*fm(UR)/(fm(DR)+fm(UR))     UserRole+x |
			 *  |                     |                                        |
			 * +---------------------------------------------------------------+
			 */

			QRect DisplayRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
			DisplayRect.adjust(3,0,0,-5);  // Empirical
			QFont font = option.font;
			painter->setFont(font);
			QString text = QFontMetrics(font).elidedText(
                        index.data(Qt::DisplayRole + mods).toString(),
						option.textElideMode,
						DisplayRect.width());
			painter->drawText(DisplayRect, Qt::AlignTop|Qt::AlignLeft, text);
			font.setPixelSize(12);
			painter->setFont(font);
			text = QFontMetrics(font).elidedText(
                        index.data(
                            ((option.state & QStyle::State_Selected) && showAction)
                            ? Qt::UserRole+1 + mods : Qt::ToolTipRole + mods)
                        .toString(),
						option.textElideMode,
						DisplayRect.width());
			painter->drawText(DisplayRect, Qt::AlignBottom|Qt::AlignLeft, text);
		}
        else
        {
            QRect DisplayRect = option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
            QString text = QFontMetrics(option.font).elidedText(
                        index.data(Qt::DisplayRole + mods).toString(),
                        option.textElideMode,
                        DisplayRect.width());
            painter->drawText(DisplayRect, Qt::AlignVCenter|Qt::AlignLeft, text);
        }
		painter->restore();
	}
};


/******************************************************************************/
/**************************  E N D   P R I V A T E  ***************************/
/******************************************************************************/



/** ***************************************************************************/
ProposalList::ProposalList(QWidget *parent) : QListView(parent) {
    QSettings s;
    _itemDelegate = new ProposalList::ItemDelegate;
    _itemDelegate->mods = Qt::NoModifier;
    _itemDelegate->showInfo = s.value(CFG_SHOW_INFO, CFG_SHOW_INFO_DEF).toBool();
    _itemDelegate->showAction = s.value(CFG_SHOW_ACTION, CFG_SHOW_ACTION_DEF).toBool();
    _maxItems  = s.value(CFG_MAX_PROPOSALS, CFG_MAX_PROPOSALS_DEF).toUInt();

    setItemDelegate(_itemDelegate);
    setUniformItemSizes(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}



/** ***************************************************************************/
ProposalList::~ProposalList() {
    QSettings s;
    s.setValue(CFG_SHOW_INFO, _itemDelegate->showInfo);
    s.setValue(CFG_SHOW_ACTION, _itemDelegate->showAction);
    s.setValue(CFG_MAX_PROPOSALS, _maxItems);
}



/** ***************************************************************************/
bool ProposalList::eventFilter(QObject*, QEvent *event)
{
    if (model() == nullptr)
        return false;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // Display different subtexts according to the KeyboardModifiers
        if ( (key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt)){
            _itemDelegate->mods = keyEvent->modifiers();
            update(currentIndex());
            return true;
        }

        // Navigation
        if (key == Qt::Key_Up || key == Qt::Key_Down
            || key == Qt::Key_PageDown || key == Qt::Key_PageUp) {

            /* I this is the first item pass key up through for the
             * command history */
            if (key == Qt::Key_Up && (!currentIndex().isValid() || currentIndex().row()==0))
                return false;

            ProposalList::keyPressEvent(keyEvent);
            return true;
        }

        // Selection
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            if (!currentIndex().isValid()){
                if (model()->rowCount() > 0)
                    setCurrentIndex(model()->index(0,0));
                else // TODO: Not so easy anymore with  informational results
                    return true;
            }
            model()->data(currentIndex(), Qt::UserRole + keyEvent->modifiers());
            window()->hide();
//			// Do not accept since the inpuline needs
//			// to store the request in history
            return false;
        }

//        // Show actions
//        if (key == Qt::Key_Tab) {
//            if (currentIndex().isValid())
//                (isExpanded(currentIndex())) ? collapse(currentIndex()) : expand(currentIndex());
//            return true;
//        }
    }

    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        // Display different subtexts according to the KeyboardModifiers
        if ( (key == Qt::Key_Control || key == Qt::Key_Meta || key == Qt::Key_Alt)){
            _itemDelegate->mods = keyEvent->modifiers();
            update(currentIndex());
            return true;
        }
    }
    return false;
}



/** ***************************************************************************/
void ProposalList::resizeEvent(QResizeEvent *e)
{
    qDebug() << e->oldSize() << e->size() << this->size();
}



/** ***************************************************************************/
QSize ProposalList::sizeHint() const
{
    if (model() == nullptr) return QSize();
    uint curr = model()->rowCount();
    int nToShow = _maxItems<curr?_maxItems:curr;
	return QSize(width(), nToShow*sizeHintForRow(0));
}



/** ***************************************************************************/
void ProposalList::reset()
{
    if (model() == nullptr) return;

    // Reset the views state
    QListView::reset();

    // Make the size of this widget be adjusted (size hint changed)
    updateGeometry();

    // Show if not empty and make first item current
    if ( model()->rowCount() > 0 ){
        setCurrentIndex(model()->index(0,0));
        show();
    }
    else
        hide();
}



/** ***************************************************************************/
void ProposalList::setShowInfo(bool b) {
    _itemDelegate->showInfo=b;
}



/** ***************************************************************************/
void ProposalList::setShowAction(bool b) {
    _itemDelegate->showAction=b;
}



/** ***************************************************************************/
void ProposalList::setMaxItems(uint maxItems) {
    _maxItems = maxItems;
}



/** ***************************************************************************/
bool ProposalList::showInfo() const {
    return _itemDelegate->showInfo;
}



/** ***************************************************************************/
bool ProposalList::showAction() const {
    return _itemDelegate->showAction;
}



/** ***************************************************************************/
bool ProposalList::maxItems() const {
    return _maxItems;
}
