
#include "realimagetreeitem.h"

// Constructor, which takes its parent, an Image and a name
RealImageTreeItem::RealImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name)
  :BaseClass(parent, itkImage), name(name){
    imageKeeper = getVTKConnector();
}

//clones an existing TreeItem
TreeItem *RealImageTreeItem::clone(TreeItem *clonesParent) const {
	RealImageTreeItem *c = new RealImageTreeItem(clonesParent, peekITKImage(), name );
	cloneChildren(c);
	return c;
}

//returns the number of columns (always 1, cause there is only the name of the Item)
int RealImageTreeItem::columnCount() const {
	return 1;
}

//returns the name of the TreeItem
QVariant RealImageTreeItem::do_getData_DisplayRole(int c) const {
	if (c==0) 
		return name;
	else 
		return QVariant::Invalid;
}

//returns the color of the TreeItem
QVariant RealImageTreeItem::do_getData_BackgroundRole(int column) const {
	return QBrush( Qt::GlobalColor::red );
}


//returns the properties of a TreeItem
Qt::ItemFlags RealImageTreeItem::flags(int column) const {
    if (column != 0) 
		return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;  
}

//sets the name of the TreeItem for column 0 and return true, if succeed
bool RealImageTreeItem::setData(int c, const QVariant &value) {
	if (c==0 && static_cast<QMetaType::Type>(value.type()) == QMetaType::QString) {
		name = value.toString();
		return true;
	} 
	return false;
}

template <typename T>
inline T clip(T min, T val, T max) {
	return std::max<T>( std::min<T>( val, max), min );
}

